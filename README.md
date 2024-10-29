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
| [Kompilator Devicetree](https://www.devicetree.org/) | 1.4.6 |

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
    - Uruchom ponownie terminal **cmd.exe**
    - Przejdź do ```cd zephyrproject\zephyr```
    - Zbuduj przykładowy projekt 
    ```bat
    west build -p always -b mimxrt1064_evk samples\basic\blinky
    ```
    - Następnie wgraj projekt na płytkę ```west flash```