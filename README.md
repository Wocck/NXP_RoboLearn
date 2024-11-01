# Instrukcja instalacji środowiska

## Opis

W tym przewodniku skonfigurujemy środowisko programistyczne Zephyr RTOS na systemie Windows, korzystając z **Visual Studio Code** (VS Code) jako edytora kodu. Visual Studio Code jest popularnym, darmowym edytorem kodu, który wspiera różne rozszerzenia i funkcje przydatne przy programowaniu z użyciem Zephyr RTOS. 

### Pobierz Visual Studio Code

Visual Studio Code można pobrać i zainstalować z [oficjalnej strony VS Code](https://code.visualstudio.com/).

---

## Wybierz i Zaktualizuj System Operacyjny

1. Otwórz **Start** > **Ustawienia** > **Aktualizacja i zabezpieczenia** > **Windows Update**.
2. Wybierz **Sprawdź aktualizacje** i zainstaluj wszystkie dostępne aktualizacje.

## Instalacja Wymaganych Narzędzi

Następnie zainstalujesz wymagane narzędzia za pomocą menedżera pakietów Chocolatey.

### Wymagane wersje narzędzi:

| Narzędzie                          | Minimalna wersja |
|------------------------------------|------------------|
| [CMake](https://cmake.org/)        | 3.20.5          |
| [Python](https://www.python.org/)  | 3.11            |
|[J-Link Software and Documentation Pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) | Wymagana dla J-Link |



### Instalacja narzędzi na Windows
Poniższa instalacja jest zgodna z instrukcją na oficjalnej stronie [Zephyr RTOS](https://docs.zephyrproject.org/latest/develop/getting_started/index.html). Używa się w niej oprogramowania *Chocolatey* dla ułatwienia instalacji. Jednak nie jest to konieczne, użytkownik może zainstalować poniższe zależności i programy z oficjalnych stron dystrybucyjnych.

1. **Zainstaluj Chocolatey**.
   - Otwórz terminal **Windows PowerShell** jako Administrator.
   - Wprowadź poniższą komendę:
     ```bat
     Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
     ```
   - Sprawdź poprawność instalacji uruchamiając komendę ```choco -v``` w nowym terminalu **cmd.exe**.


2. **Wyłącz globalne potwierdzanie w Chocolatey aby uniknąć ciągłego potwierdzania**:
   ```bat
   choco feature enable -n allowGlobalConfirmation
   ```

3. **Używając choco zainstaluj wymagane zależności**
    ```bat
    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
    choco install ninja gperf python311 git dtc-msys2 wget 7zip
    ```
    **Uwaga** Po instalacji upenij się że w zmiennej środowiskowej *Path* ustawiona jest poprawna wersja Pythona (3.11). Domyślnie Python instalowany jest pod ścieżką:
    ```C:\Users\TwojaNazwaUżytkownika\AppData\Local\Programs\Python\PythonXX```. Aby zapewnić poprawne działanie do zmiennej środowiskowej *Path* powinniśmy dodać:
    ```bat
    C:\Users\TwojaNazwaUżytkownika\AppData\Local\Programs\Python\Python311\Scripts\
    C:\Users\TwojaNazwaUżytkownika\AppData\Local\Programs\Python\Python311\
    ```  
    Po tych operacjach gdy wpiszemy w **cmd.exe** ```python --version``` powinniśmy otrzymać:
    
    ```Python 3.11.*```

    Następnie zainstaluj pakiet **pyocd** do flash-owania płytki: ```pip install pyocd```

4. **Zamknij i otwórz ponownie terminal *cmd.exe* jako zwykły użytkownik**
    - Przejdź to stworzonego folderu na projekt (zobacz instrukcje na gałęzi *main*) i stwórz i aktywuj wirtówalne środowisko
    ```bat
    python -m venv .venv
    .venv\Scripts\activate
    ```
    - Zainstaluj **West** i pobierz kod źródłowy **Zephyr** 
    ```bat
    pip install west
    west init zephyrproject
    cd zephyrproject
    west update
    west zephyr-export
    ```
    - Zainstaluj wymagane zależności Pythona
    ```bat
    pip install -r zephyrproject\zephyr\scripts\requirements.txt
    ```

5. **Zainstaluj Zephyr Software Development Kit (SKD)**
    ```bat
    cd zephyrproject\zephyr
    west sdk install
    ```

6. **Sprawdź poprawność instalacji**
    1. Podłącz płytkę do komputera przy użyciu portu **Debug USB**
    2. W Visual Studio Code otwórz folder z projektem (*NXP_RoboLearn*)
    3. Wybierz *File -> Open Workspace from File* i wybierz plik `zephyr-workspace.code-workspace`
    4. Aby uruchomić zdefiniowane taski do kompilacji i flash-owania płytki, użyj skrótu klawiszowego `Ctrl+Shift+P` i wpisz `Tasks: Run Task` i wybierz odpowiednią opcję.
    5. Najpierw zbuduj projekt przy użyciu **west build** a następnie wgraj go na płytkę przy użyciu **west flash**.
    6. Na płytce powinna zacząć migać zielona dioda LED

### Możliwe przyczyny błędów podczas próby kompilacji

 - Brak zainstalowanego Pythona w wersji 3.11: sprawdź czy w terminalu PowerShell wpisując `python --version` otrzymujesz `Python 3.11.*`
 - Brak zainstalowanego **pyodc** w Python311 do flash-owania płytki: zainstaluj go przy użyciu `pip install pyocd` w terminalu
 - Zła struktura folderów - aby zdefiniowane taski działały poprawnie potrzebna jest następująca struktura projektu:
    ```
    C:\Users\username
    ├── zephyr-sdk-0.17.0
    ├── NXP_Proj
        ├── .venv
        ├── zephyrproject
        │   ├── zephyr
        │   └── ...
        ├── NXP_RoboLearn
        │   └── ...
    ```
    Istotne jest aby folder nadrzędny dla projektu miał nazwę `NXP_Proj` oraz aby folder z SDK miał nazwę `zephyr-sdk-<version>`. Należy sprawdzić czy w pliku `zephyr-workspace.code-workspace` ścieżki do folderów są poprawne.
 - W przypadku zmiany nazwy folderów należy wejść do folderu `zephyrproject` i w wirtualnym środowisku pythona (.venv) wpisać `west update` oraz `west zephyr-export` aby zaktualizować ścieżki do folderów.
 - Upewnij się że masz zainstalowane wymagane rozszerzenia w VS Code (patrz `extensions` w pliku `zephyr-workspace.code-workspace`)

### Ćwiczenia

#### Wgraj i uruchom program na płytce:

- Skonfiguruj środowisko i wgraj kod na płytkę.
- Dioda LED powinna migać co sekundę, co potwierdza działanie pętli głównej.

#### Monitorowanie portu szeregowego (Serial Monitor):

Aby zobaczyć komunikaty wypisywane na konsolę przez `std::cout` i `printf`, otwórz **Serial Monitor** w Visual Studio Code:

1. W dolnym pasku wybierz ikonę **Serial Monitor**.
2. Wybierz odpowiedni **port szeregowy** (np. COM3, COM4 itp.), do którego podłączona jest płytka.
3. Ustaw **baud rate** na 115200.
4. Po nawiązaniu połączenia powinieneś zobaczyć wyjście z programu.

#### Debugowanie zmiennych czasu wykonania:

1. Otwórz debuger (**Run and Debug (ctrl+shift+D**) i umieść punkty przerwania w funkcjach `measure_time_cout` oraz `measure_time_printf`.
2. Za każdym razem, gdy kod zatrzyma się na tych funkcjach, sprawdź wartości zmiennych:
   - `cout_duration` – czas wykonania operacji `std::cout`.
   - `printf_duration` – czas wykonania operacji `printf`.

#### Analiza wyników:

- Porównaj czasy wykonania `cout_duration` i `printf_duration`. Zwróć uwagę, czy jedna z tych funkcji zajmuje znacznie więcej czasu.
- Na podstawie uzyskanych wyników przeanalizuj, jakie różnice występują między `std::cout` a `printf` w kontekście ich wydajności na systemach wbudowanych.