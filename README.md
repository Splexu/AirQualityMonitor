AirQualityMonitor
=================

Opis projektu
-------------
Aplikacja monitorująca jakość powietrza, w C++ z użyciem frameworku Qt 6.8.3.

Wymagania
---------
- Qt 6.8.3 (MinGW 64-bit)
- CMake 3.16 lub nowszy
- Kompilator MinGW (np. dostarczony z Qt)

Instalacja i budowanie
----------------------
1. Zainstaluj Qt 6.8.3 z MinGW (np. w C:\Qt\6.8.3\mingw_64).
2. Sklonuj repozytorium:
   git clone https://github.com/Splexu/AirQualityMonitor.git
3. Przejdź do folderu projektu:
   cd AirQualityMonitor
4. Utwórz folder build:
   mkdir build
   cd build
5. Skonfiguruj CMake:
   cmake -G "MinGW Makefiles" -DQt6_DIR=C:/Qt/6.8.3/mingw_64/lib/cmake/Qt6 ..
6. Zbuduj projekt:
   cmake --build .
7. Skopiuj biblioteki Qt:
   C:\Qt\6.8.3\mingw_64\bin\windeployqt.exe AirQualityMonitor.exe

Uruchomienie
------------
- Główna aplikacja: AirQualityMonitor.exe
- Testy jednostkowe: AirQualityMonitorTests.exe

Pliki
-----
- main.cpp: Punkt wejścia aplikacji
- MainWindow.cpp/h: Logika głównego okna
- AirQualityManager.cpp/h: Zarządzanie danymi z API
- tests.cpp: Testy jednostkowe
- AirQualityMonitor.rar: Archiwum z dodatkowymi danymi

Autor
-----
Splexu (Michał)
