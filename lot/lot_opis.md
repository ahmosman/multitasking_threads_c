# Koncepcja Rozwiązania Problemu Synchronizacji Samolotów na lotnisku

## Opis Problemu
Na lotnisku lądują i startują samoloty. Aby to zrobić, potrzebują wyłącznego dostępu do pasa startowego. Lotnisko ma ograniczoną pojemność i może pomieścić ustaloną liczbę N samolotów. Jeśli liczba samolotów na lotnisku jest mniejsza niż K (K < N), to priorytet w dostępie do pasa mają samoloty lądujące.

## Cel Zadania
Celem zadania jest zaimplementowanie programu, który synchronizuje samoloty w dostępie do pasa startowego oraz monitoruje ilość samolotów na lotnisku.

## Koncepcja Rozwiązania

### Zasoby
1. Pasa startowego - synchronizowany za pomocą mutexa.
2. Miejsca na lotnisku - synchronizowane za pomocą mutexa.
3. Warunków do sygnalizowania dostępności pasa startowego oraz dostępności miejsca na lotnisku.

### Struktury Danych
1. `airport_num`: Liczba samolotów aktualnie na lotnisku.
2. `takeoff_waiting_num`: Liczba samolotów oczekujących na start.
3. `landing_waiting_num`: Liczba samolotów oczekujących na lądowanie.
4. Stałe `K` i `N`: Ograniczenia związane z liczbą samolotów i miejsc na lotnisku.

### Wątki
1. **Wątek Lądowania:**
   - Samoloty oczekujące na lądowanie inkrementują licznik oczekujących na lądowanie.
   - Jeśli lotnisko jest pełny lub więcej niż jeden samolot czeka na lądowanie, samolot czeka na sygnał o dostępie do miejsca na lotnisku.
   - Po uzyskaniu dostępu, samolot ląduje, inkrementuje liczbę samolotów na lotnisku i sygnalizuje dostęp do pasa startowego.

2. **Wątek Startu:**
   - Samoloty oczekujące na start inkrementują licznik oczekujących na start.
   - Samolot oczekuje na dostęp do pasa startowego.
   - Po uzyskaniu dostępu, samolot startuje, dekrementuje liczbę samolotów na lotnisku i liczbę oczekujących na start.
   - Jeśli ilość samolotów na lotnisku jest mniejsza niż N, sygnalizuje dostęp do miejsca na lotnisku.

## Implementacja
Powyższa koncepcja została zaimplementowana w języku C przy użyciu wątków i mechanizmów synchronizacji, takich jak mutexy i warunki.
