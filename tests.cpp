#include <QtTest/QtTest>
#include "MainWindow.h"

/**
 * @class TestAirQualityMonitor
 * @brief Klasa testowa dla projektu AirQualityMonitor.
 */
class TestAirQualityMonitor : public QObject {
    Q_OBJECT

private slots:
    /**
     * @brief Testuje metodę calculateDistance w klasie MainWindow.
     */
    void testCalculateDistance() {
        MainWindow window;
        // Test dla odległości między punktami: (0, 0) i (1, 1) w stopniach
        // Oczekiwana odległość: ok. 157 km (przybliżenie dla małych kątów na Ziemi)
        double distance = window.calculateDistance(0.0, 0.0, 1.0, 1.0);
        QVERIFY2(qAbs(distance - 157.0) < 1.0, "Obliczona odległość jest niepoprawna");
    }

    /**
     * @brief Testuje metodę analyzeMeasurements w klasie MainWindow.
     */
    void testAnalyzeMeasurements() {
        MainWindow window;
        QList<Measurement> measurements;

        // Dodaj dane testowe
        Measurement m1;
        m1.paramName = "PM10";
        m1.value = 25.0;
        m1.dateTime = QDateTime::fromString("2025-04-10T12:00:00", Qt::ISODate);
        measurements.append(m1);

        Measurement m2;
        m2.paramName = "PM10";
        m2.value = 30.0;
        m2.dateTime = QDateTime::fromString("2025-04-10T13:00:00", Qt::ISODate);
        measurements.append(m2);

        window.analyzeMeasurements(measurements);

        QString analysisText = window.findChild<QTextEdit*>("analysisTextEdit")->toPlainText();
        QVERIFY(analysisText.contains("Najmniejsza wartość: 25"));
        QVERIFY(analysisText.contains("Największa wartość: 30"));
        QVERIFY(analysisText.contains("Średnia wartość: 27.5"));
        QVERIFY(analysisText.contains("Trend: Rosnący"));
    }
};

QTEST_MAIN(TestAirQualityMonitor)
#include "tests.moc" // Wymagane dla automatycznego generowania kodu moc
