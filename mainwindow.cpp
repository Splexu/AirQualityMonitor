#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <cmath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Wpisz nazwę miejscowości...");

    addressLineEdit = new QLineEdit(this);
    addressLineEdit->setPlaceholderText("Wpisz adres (np. Polanka 3, Poznań)...");

    radiusLineEdit = new QLineEdit(this);
    radiusLineEdit->setPlaceholderText("Wpisz promień w km...");

    findStationsButton = new QPushButton("Znajdź stacje w promieniu", this);
    saveDataButton = new QPushButton("Zapisz dane do JSON", this);

    stationListWidget = new QListWidget(this);
    sensorListWidget = new QListWidget(this);
    measurementListWidget = new QListWidget(this);
    analysisTextEdit = new QTextEdit(this);
    analysisTextEdit->setReadOnly(true);
    analysisTextEdit->setFixedHeight(100);
    analysisTextEdit->setObjectName("analysisTextEdit"); 

    periodComboBox = new QComboBox(this);
    periodComboBox->addItem("Ostatni dzień", 1);
    periodComboBox->addItem("Ostatni tydzień", 7);
    periodComboBox->addItem("Ostatni miesiąc", 30);

    series = new QLineSeries(this);
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Wykres pomiarów");

    QDateTimeAxis *axisX = new QDateTimeAxis(this);
    axisX->setTickCount(10);
    axisX->setFormat("yyyy-MM-dd HH:mm");
    axisX->setTitleText("Czas");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis(this);
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText("Wartość");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);

    aqManager = new AirQualityManager(this);

    auto *layout = new QVBoxLayout(central);
    layout->addWidget(searchLineEdit);

    auto *radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(addressLineEdit);
    radiusLayout->addWidget(radiusLineEdit);
    radiusLayout->addWidget(findStationsButton);
    radiusLayout->addWidget(saveDataButton);
    layout->addLayout(radiusLayout);

    layout->addWidget(stationListWidget);
    layout->addWidget(sensorListWidget);
    layout->addWidget(measurementListWidget);
    layout->addWidget(periodComboBox);
    layout->addWidget(chartView);
    layout->addWidget(analysisTextEdit);

    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(stationListWidget, &QListWidget::itemClicked, this, &MainWindow::onStationClicked);
    connect(sensorListWidget, &QListWidget::itemClicked, this, &MainWindow::onSensorClicked);
    connect(findStationsButton, &QPushButton::clicked, this, &MainWindow::onFindStationsInRadiusClicked);
    connect(saveDataButton, &QPushButton::clicked, this, &MainWindow::onSaveDataClicked);
    connect(aqManager, &AirQualityManager::stationsFetched, this, &MainWindow::onStationsFetched);
    connect(aqManager, &AirQualityManager::sensorsFetched, this, &MainWindow::onSensorsFetched);
    connect(aqManager, &AirQualityManager::measurementsFetched, this, &MainWindow::onMeasurementsFetched);
    connect(aqManager, &AirQualityManager::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(periodComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onPeriodChanged);

    aqManager->fetchStations();
}

void MainWindow::onStationsFetched(const QList<Station> &stationsList) {
    stations = stationsList;
    onSearchTextChanged(searchLineEdit->text());
    stationListWidget->clear();
    for (const auto &s : stations) {
        stationListWidget->addItem(s.name);
    }
    saveStationsToJson(stations);
}

void MainWindow::onStationClicked(QListWidgetItem *item) {
    int index = stationListWidget->row(item);
    if (index >= 0 && index < stations.size()) {
        aqManager->fetchSensors(stations[index].id);
    }
}

void MainWindow::onSensorsFetched(const QList<Sensor> &sensorList) {
    sensors = sensorList;
    sensorListWidget->clear();
    measurementListWidget->clear();
    analysisTextEdit->clear();
    series->clear();
    for (const auto &s : sensors) {
        sensorListWidget->addItem(s.paramName);
    }
    saveSensorsToJson(sensors);
}

void MainWindow::onSensorClicked(QListWidgetItem *item) {
    int index = sensorListWidget->row(item);
    if (index >= 0 && index < sensors.size()) {
        aqManager->fetchSensorData(sensors[index].id);
    }
}

void MainWindow::onMeasurementsFetched(const QList<Measurement> &measurementsList) {
    measurements = measurementsList;
    measurementListWidget->clear();
    analysisTextEdit->clear();
    series->clear();

    if (measurements.isEmpty()) {
        measurementListWidget->addItem("Brak danych pomiarowych dla tego czujnika.");
    } else {
        for (const auto &m : measurements) {
            QString valueStr = (m.value >= 0) ? QString::number(m.value) : "brak";
            measurementListWidget->addItem(
                QString("%1: %2 (%3)").arg(m.paramName).arg(valueStr).arg(m.dateTime.toString())
                );
        }
        updateChart(measurements);
        analyzeMeasurements(measurements);
    }
    saveMeasurementsToJson(measurements);
}

void MainWindow::onSearchTextChanged(const QString &text) {
    stationListWidget->clear();
    filteredStations.clear();

    for (const auto &station : stations) {
        if (station.name.contains(text, Qt::CaseInsensitive)) {
            stationListWidget->addItem(station.name);
            filteredStations.append(station);
        }
    }
}

void MainWindow::onFindStationsInRadiusClicked() {
    QString address = addressLineEdit->text().trimmed();
    QString radiusText = radiusLineEdit->text().trimmed();

    if (address.isEmpty() || radiusText.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Proszę podać adres i promień.");
        return;
    }

    bool ok;
    double radius = radiusText.toDouble(&ok);
    if (!ok || radius <= 0) {
        QMessageBox::warning(this, "Błąd", "Promień musi być liczbą dodatnią.");
        return;
    }

    double targetLat = 0.0, targetLon = 0.0;
    if (address.contains("Polanka 3, Poznań", Qt::CaseInsensitive)) {
        targetLat = 52.4034;
        targetLon = 16.9500;
    } else {
        QMessageBox::warning(this, "Błąd", "Nieznany adres. Proszę podać np. 'Polanka 3, Poznań'.");
        return;
    }

    stationListWidget->clear();
    filteredStations.clear();

    for (const auto &station : stations) {
        double distance = calculateDistance(targetLat, targetLon, station.latitude, station.longitude);
        if (distance <= radius) {
            stationListWidget->addItem(station.name);
            filteredStations.append(station);
        }
    }

    if (filteredStations.isEmpty()) {
        stationListWidget->addItem("Brak stacji w zadanym promieniu.");
    }
}

void MainWindow::onSaveDataClicked() {
    saveStationsToJson(stations);
    saveSensorsToJson(sensors);
    saveMeasurementsToJson(measurements);
    QMessageBox::information(this, "Sukces", "Dane zostały zapisane do plików JSON.");
}

void MainWindow::onErrorOccurred(const QString &error) {
    QMessageBox::warning(this, "Błąd", error);

    QFile stationsFile("stations.json");
    QFile sensorsFile("sensors.json");
    QFile measurementsFile("measurements.json");

    if (stationsFile.exists() || sensorsFile.exists() || measurementsFile.exists()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Brak połączenia",
            "Nie udało się pobrać danych z sieci. Czy chcesz wczytać dane historyczne?",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            QList<Station> historicalStations = loadStationsFromJson();
            if (!historicalStations.isEmpty()) {
                stations = historicalStations;
                stationListWidget->clear();
                for (const auto &s : stations) {
                    stationListWidget->addItem(s.name);
                }
            }

            QList<Sensor> historicalSensors = loadSensorsFromJson();
            if (!historicalSensors.isEmpty()) {
                sensors = historicalSensors;
                sensorListWidget->clear();
                for (const auto &s : sensors) {
                    sensorListWidget->addItem(s.paramName);
                }
            }

            QList<Measurement> historicalMeasurements = loadMeasurementsFromJson();
            if (!historicalMeasurements.isEmpty()) {
                measurements = historicalMeasurements;
                measurementListWidget->clear();
                analysisTextEdit->clear();
                series->clear();
                for (const auto &m : measurements) {
                    QString valueStr = (m.value >= 0) ? QString::number(m.value) : "brak";
                    measurementListWidget->addItem(
                        QString("%1: %2 (%3)").arg(m.paramName).arg(valueStr).arg(m.dateTime.toString())
                        );
                }
                updateChart(measurements);
                analyzeMeasurements(measurements);
            }

            if (historicalStations.isEmpty() && historicalSensors.isEmpty() && historicalMeasurements.isEmpty()) {
                QMessageBox::information(this, "Informacja", "Brak dostępnych danych historycznych.");
            } else {
                QMessageBox::information(this, "Sukces", "Wczytano dostępne dane historyczne.");
            }
        }
    } else {
        QMessageBox::information(this, "Informacja", "Brak danych historycznych do wczytania.");
    }
}

void MainWindow::onPeriodChanged(const QString &period) {
    if (!measurements.isEmpty()) {
        updateChart(measurements);
    }
}

double MainWindow::calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double PI = 3.141592653589;
    const double R = 6371.0;

    lat1 = lat1 * PI / 180.0;
    lon1 = lon1 * PI / 180.0;
    lat2 = lat2 * PI / 180.0;
    lon2 = lon2 * PI / 180.0;

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
               cos(lat1) * cos(lat2) * sin(dLon / 2.0) * sin(dLon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    double distance = R * c;

    return distance;
}

void MainWindow::analyzeMeasurements(const QList<Measurement> &measurements) {
    if (measurements.isEmpty()) {
        analysisTextEdit->setText("Brak danych do analizy.");
        return;
    }

    double minValue = measurements.first().value;
    double maxValue = measurements.first().value;
    QDateTime minDateTime = measurements.first().dateTime;
    QDateTime maxDateTime = measurements.first().dateTime;
    double sum = 0.0;
    int validCount = 0;

    for (const auto &m : measurements) {
        if (m.value >= 0) {
            if (m.value < minValue) {
                minValue = m.value;
                minDateTime = m.dateTime;
            }
            if (m.value > maxValue) {
                maxValue = m.value;
                maxDateTime = m.dateTime;
            }
            sum += m.value;
            validCount++;
        }
    }

    double average = (validCount > 0) ? sum / validCount : 0.0;

    double trend = 0.0;
    if (validCount > 1) {
        double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumXX = 0.0;
        int i = 0;
        for (const auto &m : measurements) {
            if (m.value >= 0) {
                double x = i;
                double y = m.value;
                sumX += x;
                sumY += y;
                sumXY += x * y;
                sumXX += x * x;
                i++;
            }
        }
        double n = validCount;
        trend = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
    }

    QString analysisText = "Analiza danych:\n";
    analysisText += QString("Najmniejsza wartość: %1 (%2)\n").arg(minValue).arg(minDateTime.toString());
    analysisText += QString("Największa wartość: %1 (%2)\n").arg(maxValue).arg(maxDateTime.toString());
    analysisText += QString("Średnia wartość: %1\n").arg(average, 0, 'f', 2);
    analysisText += "Trend: ";
    if (trend > 0) {
        analysisText += "Rosnący";
    } else if (trend < 0) {
        analysisText += "Malejący";
    } else {
        analysisText += "Stabilny";
    }

    analysisTextEdit->setText(analysisText);
}

void MainWindow::saveStationsToJson(const QList<Station> &stations) {
    QJsonArray stationsArray;
    for (const auto &s : stations) {
        QJsonObject stationObj;
        stationObj["id"] = s.id;
        stationObj["name"] = s.name;
        stationObj["latitude"] = s.latitude;
        stationObj["longitude"] = s.longitude;
        stationsArray.append(stationObj);
    }

    QJsonDocument doc(stationsArray);
    QFile file("stations.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        QMessageBox::warning(this, "Błąd", "Nie udało się zapisać stacji do pliku JSON.");
    }
}

void MainWindow::saveSensorsToJson(const QList<Sensor> &sensors) {
    QJsonArray sensorsArray;
    for (const auto &s : sensors) {
        QJsonObject sensorObj;
        sensorObj["id"] = s.id;
        sensorObj["paramName"] = s.paramName;
        sensorsArray.append(sensorObj);
    }

    QJsonDocument doc(sensorsArray);
    QFile file("sensors.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        QMessageBox::warning(this, "Błąd", "Nie udało się zapisać sensorów do pliku JSON.");
    }
}

void MainWindow::saveMeasurementsToJson(const QList<Measurement> &measurements) {
    QJsonArray measurementsArray;
    for (const auto &m : measurements) {
        QJsonObject measurementObj;
        measurementObj["paramName"] = m.paramName;
        measurementObj["value"] = m.value;
        measurementObj["dateTime"] = m.dateTime.toString(Qt::ISODate);
        measurementsArray.append(measurementObj);
    }

    QJsonDocument doc(measurementsArray);
    QFile file("measurements.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        QMessageBox::warning(this, "Błąd", "Nie udało się zapisać pomiarów do pliku JSON.");
    }
}

QList<Station> MainWindow::loadStationsFromJson() {
    QList<Station> stations;
    QFile file("stations.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray arr = doc.array();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            Station s;
            s.id = obj["id"].toInt();
            s.name = obj["name"].toString();
            s.latitude = obj["latitude"].toDouble();
            s.longitude = obj["longitude"].toDouble();
            stations.append(s);
        }
        file.close();
    }
    return stations;
}

QList<Sensor> MainWindow::loadSensorsFromJson() {
    QList<Sensor> sensors;
    QFile file("sensors.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray arr = doc.array();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            Sensor s;
            s.id = obj["id"].toInt();
            s.paramName = obj["paramName"].toString();
            sensors.append(s);
        }
        file.close();
    }
    return sensors;
}

QList<Measurement> MainWindow::loadMeasurementsFromJson() {
    QList<Measurement> measurements;
    QFile file("measurements.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray arr = doc.array();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            Measurement m;
            m.paramName = obj["paramName"].toString();
            m.value = obj["value"].toDouble();
            m.dateTime = QDateTime::fromString(obj["dateTime"].toString(), Qt::ISODate);
            measurements.append(m);
        }
        file.close();
    }
    return measurements;
}

void MainWindow::updateChart(const QList<Measurement> &measurements) {
    series->clear();

    int days = periodComboBox->currentData().toInt();
    QDateTime now = QDateTime::currentDateTime();
    QDateTime cutoff = now.addDays(-days);

    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    for (const auto &m : measurements) {
        if (m.value >= 0 && m.dateTime >= cutoff) {
            series->append(m.dateTime.toMSecsSinceEpoch(), m.value);
            minValue = qMin(minValue, m.value);
            maxValue = qMax(maxValue, m.value);
        }
    }

    if (series->count() > 0) {
        QChart *chart = chartView->chart();
        QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axisX());
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axisY());

        axisX->setRange(cutoff, now);
        axisY->setRange(minValue * 0.9, maxValue * 1.1);
        chart->setTitle("Wykres pomiarów: " + measurements.first().paramName);
    } else {
        chartView->chart()->setTitle("Brak danych dla wybranego okresu");
    }
}
