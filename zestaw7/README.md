## Zadania - Zestaw 7
IPC - pamieć wspólna, semafory

Przydatne funkcje:

System V:

<sys/shm.h> <sys/ipc.h> - shmget, shmclt, shmat, shmdt

POSIX:

<sys/mman.h> - shm_open, shm_close, shm_unlink, mmap, munmap

### Zadanie 1 (50%)

Wykorzystując semafory i pamięć wspólną z IPC Systemu V należy zaimplementować program rozwiązujący poniższy problem synchronizacyjny.

Przy nabrzeżu stoi statek o pojemności N. Statek jest połączony z lądem mostkiem o pojemności K: (K < N). Na statek próbują dostać się pasażerowie, z tym, że na statek nie może ich wejść więcej niż N, a wchodząc na statek na mostku nie może być ich równocześnie więcej niż K. Statek okresowo wypływa w rejs. W momencie odpływania, kapitan statku musi dopilnować aby, na mostku nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować, by liczba pasażerów na statku nie przekroczyła N.

(10%) Pasażerowie posiadają bilety na I i II klasę.  Ci z pierwszą klasą mają pierwszeństwo wejścia na mostek i zajęcia lepszych miejsc na statku (dla uproszczenia liczba miejsc na statku nie ma podziału na klasy).

Pasażer i Kapitan to osobne programy, wykorzystujące pamięć wspólną, zsynchronizowane za pomocą semaforów binarnych i zliczających. Pasażerowie są tworzeni przez jeden proces macierzysty (funkcją fork). Identyfikatorem pracownika jest jego PID. Bufor statku i pomostu znajduje się w pamięci wspólnej. Odpowiednie zasoby do synchronizacji i komunikacji powinny być tworzone przez proces Kapitana. Proces ten jest również odpowiedzialny za usunięcie tych zasobów z systemu (przed zakończeniem pracy) - należy obsłużyć SIGINT.

Program Kapitana przyjmuje jako argument N (maksymalna ładowność statku) oraz K (maksymalna pojemność mostka)  i wypisuje cyklicznie na ekranie komunikaty o następujących zdarzeniach:

    oczekiwanie na pasażerów,
    ładowanie pasażerów na statek,
    rejs,
    rozładowanie pasażerów.

Program Pasażera przyjmuje jako argument rodzaj biletu (I albo II klasa) i wypisuje cyklicznie na ekranie komunikaty o następujących zdarzeniach:

    czekanie na wejście na mostek,
    wejście na mostek,
    wejście z mostku na pokład,
    rejs,
    opuszczenie pokładu.

Każdy komunikat Kapitana lub Pasażera powinien zawierać znacznik czasowy z dokładnością do mikrosekund (można tu wykorzystać funkcję clock_gettime z flagą CLOCK_MONOTONIC). Każdy komunikat Pasażera powinien ponadto zawierać informacje o swoim identyfikatorze -PID. Każde zdarzenie związane z ładowaniem na mostek/pokład i opuszczanie go powinno dodatkowo poinformować o liczbie wolnych/zajętych miejsc oraz jednostek.

Należy tak rozwiązać problem, aby nie dopuścić do sytuacji, jak na przykład:

    Pasażer II klasy wejdzie na mostek/ pokład przed oczekującym pasażerem I klasy,
    podczas odpływania statku na mostku będzie znajdował się jakiś Pasażer,
    Pasażerowie nie wejdą na pokład zgodnie z kolejnością wejścia na mostek z wyjątkiem przepuszczenia tych z pierwszej klasy,
    Kapitan wpuści pasażera na pełny statek albo mostek,
    Pasażer wejdzie na pokład bez wcześniejszego wejścia na mostek.
    opuszczenie statku przez Pasażera przed zakończeniem rejsu.

### Zadanie 2 (50%)

Wykorzystując semafory i pamięć wspólną z IPC-POSIX należy zaimplementować program rozwiązujący poniższy problem synchronizacyjny.

Przy taśmie transportowej pracują pracownicy, którzy mogą wrzucać na taśmę ładunki o masach odpowiednio od 1 do N jednostek (N jest wartością całkowitą). Na końcu taśmy stoi ciężarówka o ładowności X jednostek, którą należy zawsze załadować do pełna. Wszyscy pracownicy starają się układać paczki na taśmie najszybciej jak to możliwe. Taśma może przetransportować w danej chwili maksymalnie K sztuk paczek. Jednocześnie jednak taśma ma ograniczony udźwig: maksymalnie M jednostek masy, tak, że niedopuszczalne jest położenie np. samych najcięższych paczek (N*K>M). Po zapełnieniu ciężarówki na jej miejsce pojawia się natychmiast nowa o takich samych parametrach. Paczki „zjeżdżające” z taśmy muszą od razu trafić na samochód dokładnie w takiej kolejności, w jakiej zostały położone na taśmie.

Zakładamy, że pracownicy i ciężarówki to osobne procesy loader i trucker. Pracownicy są tworzeni przez jeden proces macierzysty (funkcją fork). Identyfikatorem pracownika jest jego PID. Taśma transportowa o ładowności K sztuk i M jednostek jest umieszczona w pamięci wspólnej. Pamięć wspólną i semafory tworzy i usuwa program ciężarówki trucker. Należy obsłużyć SIGINT, aby pousuwać pamięć wspólną i utworzone semafory. W przypadku uruchomienia programu loader przed uruchomieniem truckera, powinien zostać wypisany odpowiedni komunikat (obsłużony błąd spowodowany brakiem dostępu do nieutworzonej pamięci), W przypadku, gdy trucker kończy pracę, powinien zablokować semafor taśmy transportowej dla pracowników, załadować to, co pozostało na taśmie, poinformować pracowników, aby ze swojej strony pozamykali mechanizmy synchronizacyjne i pousuwać je.

Program ciężarówki przyjmuje jako argument X (ładowność ciężarówki) i wypisuje cyklicznie na ekranie komunikaty o następujących zdarzeniach:

    podjechanie pustej ciężarówki,
    czekanie na załadowanie paczki,
    ładowanie paczki do ciężarówki - identyfikator pracownika, czas, liczba jednostek - stan ciężarówki - ilość zajętych i wolnych miejsc,
    brak miejsca - odjazd i wyładowanie pełnej ciężarówki.

Program pracownika przyjmuje jako argument wartość od 1 do N (liczba jednostek paczki) i wypisuje cyklicznie na ekranie komunikaty o następujących zdarzeniach:

    Załadowanie paczki o N jednostkach z podaniem identyfikatora pracownika i czasu załadowania.
    Czekanie na zwolnienie taśmy.

Każdy komunikat ciężarówki lub pracownika powinien zawierać znacznik czasowy z dokładnością do mikrosekund. Każdy komunikat pracownika powinien ponadto zawierać informacje o swoim identyfikatorze -PID. Każde zdarzenie związane z ładowaniem na taśmę i zwalnianie jej powinno dodatkowo poinformować o liczbie wolnych/zajętych miejsc oraz jednostek.

Do implementacji programów należy wykorzystać semafory zliczające oraz pamięć współdzieloną P i V operujące na semaforach wielowartościowych (atomowe zmniejszanie i zwiększanie semafora o dowolną wartość) oraz pamięć wspólną.

Synchronizacja procesów musi wykluczać zakleszczenia i gwarantować sekwencję zdarzeń zgodną ze schematem działania taśmy. Niedopuszczalne jest na przykład:

    załadowanie paczki, kiedy przekroczona została maksymalna liczba jednostek albo maksymalna liczba paczek,
    pobranie paczki z taśmy, gdy osiągnięta została maksymalna ładowność ciężarówki,
    pobranie paczki w innej kolejności niż tej, w której zostały położone na taśmie.

### Zadanie 3 (25%) (w zastępstwie za 1 albo 2 zadanie)

Zrealizuj ten sam problem synchronizacyjny z zadania 1 albo 2, wykorzystując drugi standard: IPC - Posix albo IPC - system V.