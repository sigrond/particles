	Autor: Tomasz Jakubczyk
Plan dodania obs³ugi wielu rodzajów cz¹stek na raz


	Koncepcja:

¯eby oszczêdzaæ wyko¿ystywan¹ pamiêæ karty graficznej zauwa¿am, ¿e ca³e float4 jest ³adowane do pamieci.
4 pozycja zawsze zawiera 1, które jest potrzebne do transformacji GL w postaci znormalizowanej.
Oryginalnie kopiowana jest ca³a tablica float4 do karty graficznej mimo, ¿e co 4 float jest tam zbêdny,
jest to spowodowane tym, ¿e kod kopiowania miêdzy pamiêciami jest prostszy i nie trzeba wykonywaæ dodatkowych
operacji zsuwania i rozsuwania pamiêci.
W efekcie w pamiêci karty sê niewyko¿ystywane pozycje zawieraj¹ce 1.
Chcê to wyko¿ystaæ;
ponumerujê rodzaje cz¹stek i przed skopiowaniem do karty bêdê przypisyw³ na 4 pozycjê ka¿dej cz¹stce
numer jej rodzaju, a przy wczytywaniu do g³ównej pamiêci RAM dla GL'a bêdê te numery zapisywa³
do osobnej tablicy i przed wykonywaniem przekszta³ceñ GL 4 pozycje zape³nie 1.


	Modyfikacje funkcji:

Plik particles_kernel_impl.cuh

operator() integrate_functor
Informacja o rodzaju cz¹stki mo¿e byæ przechowywana w float4 posData
w metodzie bêdzie trzeba uwzglêdniæ, ¿e cz¹stki maj¹ ró¿ne promienie (params.particleRadius).

collideSpheres
Argumenty funkcji float3 nale¿y zamieniæ na float4 poniewa¿ w tej funkcji obliczane s¹ si³y dzia³aj¹ce
na cz¹stki podczas zderzenia.

collideCell
Argument pos funkcji zmieniæ z float3 na float4.
Potrzebne s¹ informacje o rodzaju obu cz¹stek i nale¿y je uwzglêdniæ; szczególnie ró¿ne promienie.
Odpowiednio zmodyfikowaæ wywo³anie collideSpheres.

collideD
Odpowiednio zmodyfikowaæ wywo³anie collideCell.
Uwzglêdniæ masê cz¹stki zale¿n¹ od jej rodzaju przy wyliczaniu nowej prêdkoœci.

particles_kernel.cuh

SimParams
Zmieniæ parametry cz¹stek float (particleRadius, particleMass) na tablice float i odwo³ywaæ siê do nich
przez numer typu cz¹stki.

particleSystem.cpp

Chyba gdzieœ w tym pliku trzeba zadbaæ o zamiany 1 z numerem typu cz¹stki.

ParticleSystem::ParticleSystem
Ustaliæ parametry cz¹stek dla ró¿nych typów.
Dobrze by by³o wczeœniej mieæ vector z parametrami typów cz¹stek.

ParticleSystem::initGrid, ParticleSystem::reset
Losowaæ typ cz¹stki z dostêpnych.
Powinna tu wkroczyæ tablica z przypisanymi typami do cz¹stek.

ParticleSystem::particleType
Ustaliæ, czy to dobry pomys³ i zintegrowaæ z reszt¹.

ParticleSystem::update
Tu jest po³¹czenie sterowania CUDA i GL.
Nale¿y zadbaæ, ¿eby prze³¹czanie 1 do normalizacji z numerem typu cz¹stki przebiega³o w w³aœciwej kolejnoœci.

particles.cpp

Zadbaæ o ustalenie na pocz¹tku symulacji ile bêdzie typów cz¹stek i jakie one bêd¹.

