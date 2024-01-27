# Koncepcja Rozwiązania Problemu Czytelników i Pisarzy

## Opis Problemu

1. Ustalona liczba procesów - N.
2. Procesy działają naprzemiennie w dwóch fazach: fazie relaksu i fazie korzystania z czytelni.
3. W fazie relaksu proces może zmienić swoją rolę: z pisarza na czytelnika lub z czytelnika na pisarza.
4. Przechodząc do fazy korzystania z czytelni, proces musi uzyskać dostęp w odpowiednim trybie.
5. Pisarz umieszcza dzieło w kolejce komunikatów, gdzie pozostaje do momentu, aż odczytają je wszyscy czytelnicy, którzy byli w roli czytelnika w momencie wydania dzieła.
6. Pojemność kolejki komunikatów (półki z książkami) jest ograniczona do wartości K.

## Struktura Rozwiązania

### Pamięć Współdzielona

- Segment pamięci współdzielonej o identyfikatorze 1234 zawierający 4 wartości int (mem_buf).

### Kolejki Komunikatów

- Kolejka books_msgid dla przesyłania informacji o książkach.
- Kolejka events_msgid dla wydarzeń związanych z czytelnikami.

### Semafor

- Tablica semaforów (semid) do synchronizacji dostępu do współdzielonych zasobów.

## Fazy Relaksu i Korzystania z Czytelni

- Każdy proces działa w nieskończonej pętli, naprzemiennie przechodząc między fazą relaksu a fazą korzystania z czytelni.
- Procesy losowo zmieniają rolę w fazie relaksu i oczekują przez losowy czas.
- W fazie korzystania z czytelni procesy uzyskują dostęp we właściwym trybie i podejmują odpowiednie działania.

## Ograniczenia

- Kolejka komunikatów books_msgid reprezentuje półkę z książkami, a jej pojemność jest ograniczona do wartości K.
- Proces pisarza umieszcza dzieło w kolejce, aż zostanie odczytane przez wszystkie procesy, które były w roli czytelnika w momencie wydania dzieła.

## Synchronizacja i Kontrola Dostępu

- Semafor semid jest używany do kontrolowania dostępu do współdzielonych zasobów.

## Dodatkowe Elementy

- Funkcje pomocnicze: performSrand, sleepRandomSeconds, getRandomRole.
- Pytania dotyczące szczegółów implementacji i komendy monitorowania/usuwania zasobów systemowych.