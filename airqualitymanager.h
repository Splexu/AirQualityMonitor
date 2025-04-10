#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

/**
 * @struct Station
 * @brief Struktura reprezentująca stację pomiarową.
 */
struct Station {
    int id;            ///< Identyfikator stacji.
    QString name;      ///< Nazwa stacji.
    double latitude;   ///< Szerokość geograficzna.
    double longitude;  ///< Długość geograficzna.
};

/**
 * @struct Measurement
 * @brief Struktura reprezentująca pojedynczy pomiar.
 */
struct Measurement {
    QString paramName; ///< Nazwa parametru (np. PM10).
    double value;      ///< Wartość pomiaru.
    QDateTime dateTime;///< Data i czas pomiaru.
};

/**
 * @struct Sensor
 * @brief Struktura reprezentująca sensor w stacji.
 */
struct Sensor {
    int id;            ///< Identyfikator sensora.
    QString paramName; ///< Nazwa parametru mierzonego przez sensor.
};

/**
 * @class AirQualityManager
 * @brief Klasa zarządzająca pobieraniem danych z API GIOŚ.
 *
 * Odpowiada za komunikację z API, pobieranie danych o stacjach,
 * sensorach i pomiarach oraz obsługę błędów sieciowych.
 */
class AirQualityManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy AirQualityManager.
     * @param parent Wskaźnik na nadrzędny obiekt (domyślnie nullptr).
     */
    explicit AirQualityManager(QObject *parent = nullptr);

    /// @brief Pobiera listę wszystkich stacji z API.
    void fetchStations();

    /// @brief Pobiera pomiary dla danej stacji.
    void fetchMeasurements(int stationId);

    /// @brief Pobiera listę sensorów dla danej stacji.
    void fetchSensors(int stationId);

    /// @brief Pobiera dane pomiarowe dla danego sensora.
    void fetchSensorData(int sensorId);

signals:
    /// @brief Sygnał emitowany po pobraniu listy stacji.
    void stationsFetched(QList<Station>);

    /// @brief Sygnał emitowany po pobraniu listy sensorów.
    void sensorsFetched(QList<Sensor>);

    /// @brief Sygnał emitowany po pobraniu danych pomiarowych.
    void measurementsFetched(QList<Measurement>);

    /// @brief Sygnał emitowany w przypadku błędu sieciowego.
    void errorOccurred(const QString &error);

private slots:
    /// @brief Obsługuje odpowiedź API dla listy stacji.
    void onStationsReply(QNetworkReply *reply);

    /// @brief Obsługuje odpowiedź API dla pomiarów stacji.
    void onMeasurementsReply(QNetworkReply *reply);

    /// @brief Obsługuje odpowiedź API dla listy sensorów.
    void onSensorsReply(QNetworkReply *reply);

    /// @brief Obsługuje odpowiedź API dla danych sensora.
    void onSensorDataReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager; ///< Obiekt zarządzający żądaniami sieciowymi.
};
