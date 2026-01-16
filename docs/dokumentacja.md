# Dokumentacja Projektu: N-Body Simulation 2D

## 1. Opis projektu

Projekt **"nbody-cpp"** to zaawansowany symulator fizyczny układu wielu ciał (N-body) w przestrzeni dwuwymiarowej. Aplikacja została napisana w nowoczesnym języku C++ (standard C++20/23) z wykorzystaniem biblioteki **Raylib** do wizualizacji w czasie rzeczywistym.

Głównym problemem rozwiązywanym w projekcie jest symulacja ewolucji grawitacyjnej układu składającego się z dużej liczby cząstek (gwiazd, planet, asteroid). W klasycznym podejściu bezpośrednim (Direct Method), obliczenie sił działających między każdą parą ciał wiąże się ze złożonością obliczeniową . Przy dużej liczbie obiektów (rzędu - ) podejście to staje się nieefektywne.

Celem projektu było zaimplementowanie i porównanie wydajności algorytmu **Barnes-Hut**, który redukuje złożoność do poprzez aproksymację oddziaływań odległych grup ciał. Projekt pełni funkcję zarówno narzędzia badawczego (analiza wydajności, wizualizacja struktur galaktycznych), jak i edukacyjnego (demonstracja praw Keplera i dynamiki nieliniowej).

## 2. Funkcjonalności

Projekt oferuje szeroki wachlarz funkcjonalności podzielonych na logikę symulacji oraz interfejs użytkownika:

### 2.1 Modulrany Silnik Symulacji

#### Wielowątkowa obsługa algorytmów:

- **Brute Force (O(n^2)):** Dokładne obliczanie siły dla każdej pary ciał (referencja dla testów dokładności).
- **Barnes-Hut (O(n log n)):** Implementacja oparta na drzewie czwórkowym (QuadTree).
- **Barnes-Hut z sortowaniem Morton (O(n log n)):** Eksperymentalna wersja optymalizująca lokalność pamięci podręcznej (cache locality).

#### Integratory numeryczne:

- **Euler:** Prosty integrator, ale nie zapewnia symplektyczności.
- **Semi-symplectic Euler:** Zapewnia zachowanie energii (symplektyczność) przy niskim koszcie obliczeniowym.
- **Verlet integration:** Alternatywny integrator o wyższej stabilności.

#### Parametry fizyczne:

- Konfigurowalna stała grawitacji .
- Parametr **softening** zapobiegający osobliwościom przy bliskich spotkaniach ciał.
- Dynamiczny krok czasowy.

### 2.2 Wizualizacja i Interakcja

#### **Renderowanie:**

Wykorzystanie instancjonowania (instancing) do renderowania dziesiątek tysięcy cząstek przy wysokiej liczbie klatek na sekundę (FPS).

#### **Interaktywna kamera:**

Możliwość przybliżania (zoom), przesuwania (pan) oraz śledzenia środka masy układu.

#### **Kreacja Ciał ("Slingshot"):**

Intuicyjny mechanizm dodawania nowych ciał poprzez przeciągnięcie myszką – wektor prędkości zależy od długości i kierunku pociągnięcia.

#### **Scenariusze:** Wbudowane generatory układów początkowych:

TODO

## 3. Przebieg realizacji

Projekt został zrealizowany w podejściu modułowym, co ułatwia testowanie i dalszy rozwój.

### 3.1 Struktura Projektu (Opis plików)

Kod źródłowy został podzielony na następujące katalogi logiczne:

- **`base/`**: Fundamenty systemu.
- TODO

- **`math/`**: Biblioteka matematyczna.
- TODO

- **`sim/`**: Serce symulacji.
- TODO

- **`gfx/`**: Warstwa prezentacji.
- TODO

- **`apps/nbody/`**: Główna pętla aplikacji i punkt wejścia (`main.cpp`).

### 3.2 Opis Algorytmu Barnes-Hut

Algorytm Barnesa-Huta jest kluczowym elementem projektu. Pozwala on na efektywne symulowanie oddziaływań grawitacyjnych poprzez grupowanie ciał znajdujących się blisko siebie i traktowanie ich jako jednego "super-ciała" (monopole approximation) z punktu widzenia ciał odległych.

#### A. Budowa Drzewa Czwórkowego (QuadTree)

W każdym kroku symulacji budowane jest nowe drzewo czwórkowe.

1. **Korzeń:** Reprezentuje całą dostępną przestrzeń symulacji.
2. **Podział:** Jeśli w danym obszarze znajduje się więcej niż jedno ciało, jest on dzielony na cztery równe kwadraty (ćwiartki: NW, NE, SW, SE).
3. **Liście:** Proces podziału kończy się, gdy węzeł zawiera 0 lub 1 ciało.
4. **Dane węzłów:** Każdy węzeł wewnętrzny przechowuje:

- – sumaryczną masę wszystkich ciał w jego poddrzewie.
- – współrzędne środka masy (Center of Mass), obliczane wzorem:

#### B. Kryterium Akceptacji (MAC - Multipole Acceptance Criterion)

Podczas obliczania siły dla konkretnego ciała , algorytm przemierza drzewo od korzenia. Decyzja o tym, czy zejść głębiej do dzieci węzła, czy użyć aproksymacji, zależy od parametru (theta) oraz stosunku rozmiaru węzła do odległości od ciała .

Warunek aproksymacji:

Jeśli warunek jest spełniony, węzeł traktowany jest jako punkt materialny o masie w pozycji . Typowa wartość to .

#### C. Pseudokod

**1. Struktura Węzła:**

```cpp
struct Node {
    Vec2 center_of_mass;
    float total_mass;
    float size;        // Szerokość obszaru
    Node* children[4]; // Wskaźniki na ćwiartki
    Body* body;        // Jeśli liść, wskaźnik na ciało
    bool is_leaf;
};

```

**2. Wstawianie ciała do drzewa (Insert):**

```text
FUNCTION Insert(node, body):
    IF node.is_leaf AND node.body is NULL:
        node.body = body
        RETURN

    IF node.is_leaf AND node.body is NOT NULL:
        // Węzeł był liściem z ciałem, musimy go podzielić
        oldBody = node.body
        node.body = NULL
        node.is_leaf = FALSE
        Subdivide(node) // Tworzy 4 dzieci
        Insert(node, oldBody) // Wstawiamy stare ciało do odpowiedniego dziecka
        Insert(node, body)    // Wstawiamy nowe ciało
        RETURN

    // Węzeł jest wewnętrzny (gałąź)
    UpdateCenterOfMass(node, body)
    quadrant = GetQuadrant(node, body.position)
    Insert(node.children[quadrant], body)
END FUNCTION

```

**3. Obliczanie siły (Force Calculation):**

```text
FUNCTION ComputeForce(body, node, theta):
    d = Distance(body.pos, node.center_of_mass)

    // Unikamy liczenia siły ciała na samo siebie
    IF d == 0: RETURN

    // Sprawdzenie kryterium MAC
    IF node.is_leaf OR (node.size / d < theta):
        // Aproksymacja: traktujemy węzeł jako jedno ciało
        dir = Normalize(node.center_of_mass - body.pos)
        // Wzór z 'softening' dla stabilności numerycznej
        force_magnitude = (G * body.mass * node.total_mass) / (d*d + epsilon*epsilon)
        body.force += dir * force_magnitude
    ELSE:
        // Musimy zejść głębiej
        FOR each child in node.children:
            IF child is not empty:
                ComputeForce(body, child, theta)
END FUNCTION

```

### 3.3 Biblioteki zewnętrzne

1. **Raylib:** Biblioteka graficzna o niskim narzucie, idealna do prototypowania. Wykorzystana do:

- Tworzenia okna i kontekstu OpenGL.
- Obsługi wejścia (klawiatura, mysz).
- Rysowania prymitywów (koła, linie) w trybie _immediate_ oraz _batched_.

2. **Raygui:** Lekka biblioteka GUI typu "immediate mode" działająca na Raylib. Służy do rysowania paneli sterowania (suwaki do grawitacji, przyciski resetu).

## 4. Instrukcja użytkownika

### 4.1 Wymagania systemowe i kompilacja

Do zbudowania projektu wymagany jest kompilator wspierający C++23 (np. GCC 11+, Clang 13+, MSVC 2022) oraz system budowania CMake.

### 4.2 Kroki instalacji (Linux/macOS):\*\*

TODO

### 4.3 Obsługa programu

Po uruchomieniu aplikacji użytkownik zobaczy główne okno symulacji.

**[Miejsce na zrzut ekranu: Główny interfejs aplikacji z widoczną symulacją i panelem bocznym]**

**Sterowanie:**

- **Mysz (LPM):** Przeciągnij na pustym obszarze, aby dodać nowe ciało ("proca").
- **Mysz (Scroll):** Przybliżanie i oddalanie widoku (Zoom).
- **PPM (Przytrzymaj):** Przesuwanie kamery.
- **Spacja:** Pauza / Wznowienie symulacji.
- **R:** Reset symulacji do stanu początkowego.
- **F:** Włącz/wyłącz śledzenie najcięższego obiektu.
- **H:** Ukryj/pokaż panel sterowania (HUD).

**Panel Boczny (GUI):**
Umożliwia zmianę parametrów w locie:

1. **Simulation Speed:** Mnożnik kroku czasowego.
2. **Solver Type:** Wybór między _Direct_ a _Barnes-Hut_.
3. **Theta:** Suwak precyzji algorytmu BH (im mniejsza wartość, tym większa dokładność, ale wolniejsze działanie).
4. **Presets:** Rozwijana lista gotowych scenariuszy (np. "Galaxy Collision").
5. **Debug Draw:** Checkboxy włączające rysowanie siatki QuadTree.

**[Miejsce na zrzut ekranu: Wizualizacja drzewa QuadTree nałożona na ciała]**

## 5. Podsumowanie i wnioski

Projekt zakończył się sukcesem, realizując wszystkie założone cele funkcjonalne.

### 5.1 Osiągnięcia:\*\*

- Zaimplementowano w pełni funkcjonalny symulator N-body w języku C++.
- Algorytm Barnes-Hut poprawnie zredukował czas obliczeń, umożliwiając płynną symulację (60 FPS) dla układów rzędu 20 000 ciał na przeciętnej klasy laptopie, podczas gdy algorytm bezpośredni tracił płynność przy około 2 000 ciał.
- Wizualizacja za pomocą Raylib okazała się wydajna i estetyczna.

### 5.2 Problemy i wyzwania:\*\*

- **Zarządzanie pamięcią:** Implementacja drzewa opartego na wskaźnikach była prosta, ale mało wydajna cache'owo. Przepisanie drzewa na strukturę liniową (`std::vector` węzłów) znacząco poprawiło wydajność.
- **Dobór parametrów:** Zbyt duża wartość kroku czasowego lub zbyt małe _softening_ powodowały "wyrzucanie" ciał z układu z niefizyczną prędkością (błędy numeryczne integratora Eulera). Zastosowanie semi-implicit Euler częściowo rozwiązało ten problem.

### 5.3 Dalsze kierunki rozwoju:\*\*

1. **Równoległość:** Implementacja wielowątkowości (np. przy użyciu OpenMP lub `std::thread`) do budowy drzewa i obliczania sił.
2. **Akceleracja GPU:** Przeniesienie obliczeń na kartę graficzną przy użyciu Compute Shaders (OpenGL/Vulkan) lub CUDA, co pozwoliłoby na symulację milionów ciał.
3. **Przestrzeń 3D:** Rozszerzenie algorytmu na drzewo oktalne (Octree) dla symulacji trójwymiarowych.
