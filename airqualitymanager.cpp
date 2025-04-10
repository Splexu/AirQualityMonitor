#include "AirQualityManager.h"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

AirQualityManager::AirQualityManager(QObject *parent) : QObject(parent) {
    networkManager = new QNetworkAccessManager(this);
}

void AirQualityManager::fetchStations() {
    QUrl url("https://api.gios.gov.pl/pjp-api/rest/station/findAll");
    QNetworkRequest request(url);
    auto *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() { onStationsReply(reply); });
}

void AirQualityManager::fetchMeasurements(int stationId) {
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/data/getData/%1").arg(stationId));
    QNetworkRequest request(url);
    auto *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() { onMeasurementsReply(reply); });
}

void AirQualityManager::onStationsReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Błąd pobierania stacji: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QList<Station> stations;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Błąd parsowania danych stacji: nieprawidłowy format JSON");
        reply->deleteLater();
        return;
    }

    QJsonArray arr = doc.array();
    for (const auto &v : arr) {
        QJsonObject obj = v.toObject();
        Station s;
        s.id = obj["id"].toInt();
        s.name = obj["stationName"].toString();
        s.latitude = obj["gegrLat"].toString().toDouble();
        s.longitude = obj["gegrLon"].toString().toDouble();
        stations.append(s);
    }
    emit stationsFetched(stations);
    reply->deleteLater();
}

void AirQualityManager::onMeasurementsReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Błąd pobierania pomiarów: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QList<Measurement> measurements;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Błąd parsowania danych pomiarów: nieprawidłowy format JSON");
        reply->deleteLater();
        return;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray values = rootObj["values"].toArray();
    QString paramName = rootObj["key"].toString();

    for (const auto &v : values) {
        QJsonObject obj = v.toObject();
        if (obj["value"].isNull())
            continue;

        Measurement m;
        m.paramName = paramName;
        m.value = obj["value"].toDouble();
        m.dateTime = QDateTime::fromString(obj["date"].toString(), Qt::ISODate);
        measurements.append(m);
    }

    emit measurementsFetched(measurements);
    reply->deleteLater();
}

void AirQualityManager::fetchSensors(int stationId) {
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/station/sensors/%1").arg(stationId));
    QNetworkRequest request(url);
    auto *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() { onSensorsReply(reply); });
}

void AirQualityManager::onSensorsReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Błąd pobierania sensorów: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QList<Sensor> sensors;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Błąd parsowania danych sensorów: nieprawidłowy format JSON");
        reply->deleteLater();
        return;
    }

    QJsonArray arr = doc.array();
    for (const auto &v : arr) {
        QJsonObject obj = v.toObject();
        Sensor s;
        s.id = obj["id"].toInt();
        s.paramName = obj["param"].toObject()["paramName"].toString();
        sensors.append(s);
    }
    emit sensorsFetched(sensors);
    reply->deleteLater();
}

void AirQualityManager::fetchSensorData(int sensorId) {
    QUrl url(QString("https://api.gios.gov.pl/pjp-api/rest/data/getData/%1").arg(sensorId));
    QNetworkRequest request(url);
    auto *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() { onSensorDataReply(reply); });
}

void AirQualityManager::onSensorDataReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Błąd pobierania danych czujnika: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QList<Measurement> measurements;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Błąd parsowania danych czujnika: nieprawidłowy format JSON");
        reply->deleteLater();
        return;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray values = rootObj["values"].toArray();
    QString paramName = rootObj["key"].toString();

    for (const auto &v : values) {
        QJsonObject obj = v.toObject();
        if (obj["value"].isNull())
            continue;

        Measurement m;
        m.paramName = paramName;
        m.value = obj["value"].toDouble();
        m.dateTime = QDateTime::fromString(obj["date"].toString(), Qt::ISODate);
        measurements.append(m);
    }

    emit measurementsFetched(measurements);
    reply->deleteLater();
}
