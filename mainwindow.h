#pragma once
#include <QMap>
#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include "AirQualityManager.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QComboBox>

/**
 * @class MainWindow
 * @brief Główne okno aplikacji monitorującej jakość powietrza.
 *
 * Klasa ta zarządza interfejsem użytkownika, wyświetla dane pomiarowe,
 * wykresy i analizy, oraz obsługuje interakcje z użytkownikiem.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
     */
    MainWindow(QWidget *parent = nullptr);

    // Udostępnij metodę publicznie dla testów
    /**
     * @brief Oblicza odległość między dwoma punktami na Ziemi w kilometrach.
     * @param lat1 Szerokość geograficzna pierwszego punktu.
     * @param lon1 Długość geograficzna pierwszego punktu.
     * @param lat2 Szerokość geograficzna drugiego punktu.
     * @param lon2 Długość geograficzna drugiego punktu.
     * @return Odległość w kilometrach.
     */
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);

    /**
     * @brief Analizuje dane pomiarowe i wyświetla wyniki w sekcji analizy.
     * @param measurements Lista pomiarów do analizy.
     */
    void analyzeMeasurements(const QList<Measurement> &measurements);

private slots:
    /// @brief Slot wywoływany po pobraniu listy stacji.
    void onStationsFetched(const QList<Station> &stations);

    /// @brief Slot wywoływany po kliknięciu na stację w liście.
    void onStationClicked(QListWidgetItem *item);

    /// @brief Slot wywoływany po pobraniu listy sensorów.
    void onSensorsFetched(const QList<Sensor> &sensors);

    /// @brief Slot wywoływany po kliknięciu na sensor w liście.
    void onSensorClicked(QListWidgetItem *item);

    /// @brief Slot wywoływany po pobraniu danych pomiarowych.
    void onMeasurementsFetched(const QList<Measurement> &measurements);

    /// @brief Slot wywoływany przy zmianie tekstu w polu wyszukiwania.
    void onSearchTextChanged(const QString &text);

    /// @brief Slot wywoływany po kliknięciu przycisku wyszukiwania stacji w promieniu.
    void onFindStationsInRadiusClicked();

    /// @brief Slot wywoływany po kliknięciu przycisku zapisu danych do JSON.
    void onSaveDataClicked();

    /// @brief Slot wywoływany w przypadku błędu sieciowego.
    void onErrorOccurred(const QString &error);

    /// @brief Slot wywoływany przy zmianie wybranego okresu danych na wykresie.
    void onPeriodChanged(const QString &period);

private:
    /// @brief Zapisuje listę stacji do pliku JSON.
    void saveStationsToJson(const QList<Station> &stations);

    /// @brief Zapisuje listę sensorów do pliku JSON.
    void saveSensorsToJson(const QList<Sensor> &sensors);

    /// @brief Zapisuje listę pomiarów do pliku JSON.
    void saveMeasurementsToJson(const QList<Measurement> &measurements);

    /// @brief Wczytuje listę stacji z pliku JSON.
    QList<Station> loadStationsFromJson();

    /// @brief Wczytuje listę sensorów z pliku JSON.
    QList<Sensor> loadSensorsFromJson();

    /// @brief Wczytuje listę pomiarów z pliku JSON.
    QList<Measurement> loadMeasurementsFromJson();

    /// @brief Aktualizuje wykres na podstawie danych pomiarowych i wybranego okresu.
    void updateChart(const QList<Measurement> &measurements);

    QLineEdit *searchLineEdit;         ///< Pole wyszukiwania miejscowości.
    QLineEdit *addressLineEdit;        ///< Pole wprowadzania adresu.
    QLineEdit *radiusLineEdit;         ///< Pole wprowadzania promienia.
    QPushButton *findStationsButton;   ///< Przycisk wyszukiwania stacji w promieniu.
    QPushButton *saveDataButton;       ///< Przycisk zapisu danych do JSON.
    QListWidget *stationListWidget;    ///< Lista stacji.
    QListWidget *sensorListWidget;     ///< Lista sensorów.
    QListWidget *measurementListWidget;///< Lista pomiarów.
    QTextEdit *analysisTextEdit;       ///< Sekcja analizy danych.
    QComboBox *periodComboBox;         ///< Wybór okresu danych na wykresie.
    QChartView *chartView;             ///< Widok wykresu.
    QLineSeries *series;               ///< Seria danych na wykresie.
    AirQualityManager *aqManager;      ///< Obiekt zarządzający danymi z API.

    QList<Station> stations;           ///< Lista wszystkich stacji.
    QList<Station> filteredStations;   ///< Lista filtrowanych stacji.
    QList<Sensor> sensors;             ///< Lista sensorów dla wybranej stacji.
    QList<Measurement> measurements;   ///< Lista pomiarów dla wybranego sensora.
};
