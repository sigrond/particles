	Autor: Tomasz Jakubczyk
Plan dodania obsługi wielu rodzajów cząstek na raz


	Koncepcja:

Żeby oszczędzać wykożystywaną pamięć karty graficznej zauważam, że całe float4 jest ładowane do pamieci.
4 pozycja zawsze zawiera 1, które jest potrzebne do transformacji GL w postaci znormalizowanej.
Oryginalnie kopiowana jest cała tablica float4 do karty graficznej mimo, że co 4 float jest tam zbędny,
jest to spowodowane tym, że kod kopiowania między pamięciami jest prostszy i nie trzeba wykonywać dodatkowych
operacji zsuwania i rozsuwania pamięci.
W efekcie w pamięci karty sę niewykożystywane pozycje zawierające 1.
Chcę to wykożystać;
ponumeruję rodzaje cząstek i przed skopiowaniem do karty będę przypisywł na 4 pozycję każdej cząstce
numer jej rodzaju, a przy wczytywaniu do głównej pamięci RAM dla GL'a będę te numery zapisywał
do osobnej tablicy i przed wykonywaniem przekształceń GL 4 pozycje zapełnie 1.

EDIT:
Lepiej użyć wektora prędkości, bo tam 4 współżędna zawsze może mieć dowolną wartość,
a nie będzie trzeba ciągle zamieniać numeru typu z 1 i niepełna implementacja
będzia miała mniejszy wpływ na działanie symulacji.
Zwiększy się też prostota rozwiązania.

EDIT:
Następnym problemem wykrytym w trakcie pisania jest rysowanie cząstek w różnych rozmiarach.
Obecnie funkcja raz zadaje rozmiar i rysuje wszystkie cząstki jako punkty za raz jako
zbiór punktów. Zgodnie z sugestią dr hab. inż. D.J. ;-) przy losowaniu cząstek typy
przypisujemy pokolei. Po pierwsze likwiduje to problemy z proporcja i rozkładem ilości
typów cząstek. Po drugie nie trzeba losować typu. Po trzecie można łatwo rozdzielić cząstki
na tyle grup ile typów i zastosować obecny algorytm rysowania dla każdej z osobna.


	Modyfikacje funkcji:

Plik particles_kernel_impl.cuh

operator() integrate_functor
Informacja o rodzaju cząstki może być przechowywana w float4 posData
w metodzie będzie trzeba uwzględnić, że cząstki mają różne promienie (params.particleRadius).

collideSpheres
Argumenty funkcji float3 należy zamienić na float4 ponieważ w tej funkcji obliczane są siły działające
na cząstki podczas zderzenia.

collideCell
Argument pos funkcji zmienić z float3 na float4.
Potrzebne są informacje o rodzaju obu cząstek i należy je uwzględnić; szczególnie różne promienie.
Odpowiednio zmodyfikować wywołanie collideSpheres.

collideD
Odpowiednio zmodyfikować wywołanie collideCell.
Uwzględnić masę cząstki zależną od jej rodzaju przy wyliczaniu nowej prędkości.

particles_kernel.cuh

SimParams
Zmienić parametry cząstek float (particleRadius, particleMass) na tablice float i odwoływać się do nich
przez numer typu cząstki.

particleSystem.cpp

Chyba gdzieś w tym pliku trzeba zadbać o zamiany 1 z numerem typu cząstki.

ParticleSystem::ParticleSystem
Ustalić parametry cząstek dla różnych typów.
Dobrze by było wcześniej mieć vector z parametrami typów cząstek.

ParticleSystem::initGrid, ParticleSystem::reset
Losować typ cząstki z dostępnych.
Powinna tu wkroczyć tablica z przypisanymi typami do cząstek.

ParticleSystem::particleType
Ustalić, czy to dobry pomysł i zintegrować z resztą.

ParticleSystem::update
Tu jest połączenie sterowania CUDA i GL.
Należy zadbać, żeby przełączanie 1 do normalizacji z numerem typu cząstki przebiegało w właściwej kolejności.

particles.cpp

Zadbać o ustalenie na początku symulacji ile będzie typów cząstek i jakie one będą.

