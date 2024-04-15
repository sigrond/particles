	Autor: Tomasz Jakubczyk
Plan dodania obsługi wielu rodzajów cząstek na raz; (EN) Plan to add support for multiple particle types all at once


	Koncepcja; (EN) Concept:

Żeby oszczędzać wykorzystywaną pamięć karty graficznej zauważam, że całe float4 jest ładowane do pamieci.
4ta pozycja zawsze zawiera 1, które jest potrzebne do transformacji GL w postaci znormalizowanej.
Oryginalnie kopiowana jest cała tablica float4 do karty graficznej mimo, że co 4 float jest tam zbędny,
jest to spowodowane tym, że kod kopiowania między pamięciami jest prostszy i nie trzeba wykonywać dodatkowych
operacji zsuwania i rozsuwania pamięci.
W efekcie w pamięci karty sę niewykorzystywane pozycje zawierające 1.
Chcę to wykorzystać;
ponumeruję rodzaje cząstek i przed skopiowaniem do karty będę przypisywł na 4 pozycję każdej cząstce
numer jej rodzaju, a przy wczytywaniu do głównej pamięci RAM dla GL'a będę te numery zapisywał
do osobnej tablicy i przed wykonywaniem przekształceń GL 4 pozycje zapełnie 1.
(EN)
To save the memory used by the graphics card, I notice that the entire float4 is loaded into memory.
The 4th position always contains a 1, which is needed for the GL transformation in normalized form.
Originally the whole float4 array is copied to the graphics card even though every 4th float is redundant there, 
this is because the code for copying between memories is simpler and no additional memory sliding in and out 
operations are needed. As a result, there are unused positions in the card's memory that contain 1. 
I want to take advantage of this; I will number the particle types and, before copying to the card, 
I will assign each particle's type number to the 4th position and, when loading into the main RAM for GL, 
I will store these numbers into a separate array and, before performing GL transformations, 
the 4th positions will be filled with 1.

EDIT:
Lepiej użyć wektora prędkości, bo tam 4 współrzędna zawsze może mieć dowolną wartość,
a nie będzie trzeba ciągle zamieniać numeru typu z 1 i niepełna implementacja
będzia miała mniejszy wpływ na działanie symulacji.
Zwiększy się też prostota rozwiązania.
(EN)
It is better to use a velocity vector, because there the 4th coordinate can always have an arbitrary value, 
and you won't have to keep swapping the type number with 1 and an incomplete implementation will have less 
impact on the performance of the simulation. The simplicity of the solution will also increase.

EDIT:
Następnym problemem wykrytym w trakcie pisania jest rysowanie cząstek w różnych rozmiarach.
Obecnie funkcja raz zadaje rozmiar i rysuje wszystkie cząstki jako punkty za raz jako
zbiór punktów. Zgodnie z sugestią dr hab. inż. D.J. ;-) przy losowaniu cząstek typy
przypisujemy pokolei. Po pierwsze likwiduje to problemy z proporcja i rozkładem ilości
typów cząstek. Po drugie nie trzeba losować typu. Po trzecie można łatwo rozdzielić cząstki
na tyle grup ile typów i zastosować obecny algorytm rysowania dla każdej z osobna.
(EN)
The next problem discovered at the time of writing is drawing particles in different sizes. 
Currently, the function asks for the size once and draws all particles as points at once as a set of points. 
As suggested by D.J., Ph.D. D.Sc. Eng. ;-) when drawing particles, we assign the types in sequence. Firstly, 
this eliminates problems with the proportion and distribution of the number of particle types. Secondly, 
you do not need to randomise the type. Thirdly, you can easily divide the particles into as many groups 
as types and apply the current drawing algorithm for each one separately.


	Modyfikacje funkcji; (EN) Function modifications:

Plik; (EN) file: particles_kernel_impl.cuh

operator() integrate_functor
Informacja o rodzaju cząstki może być przechowywana w float4 posData;
w metodzie będzie trzeba uwzględnić, że cząstki mają różne promienie (params.particleRadius).
(EN)
Information about the particle type can be stored in float4 posData; the method will need to take 
into account that particles have different radii (params.particleRadius).

collideSpheres
Argumenty funkcji float3 należy zamienić na float4 ponieważ w tej funkcji obliczane są siły działające
na cząstki podczas zderzenia.
(EN)
The arguments of the float3 function should be changed to float4 because in this function the forces 
acting on the particles during the collision are calculated.

collideCell
Argument pos funkcji zmienić z float3 na float4.
Potrzebne są informacje o rodzaju obu cząstek i należy je uwzględnić; szczególnie różne promienie.
Odpowiednio zmodyfikować wywołanie collideSpheres.
(EN)
Change the pos argument of the function from float3 to float4.
Information about the type of both particles is needed and should be taken into account; especially the different radii. 
Modify the collideSpheres call accordingly.

collideD
Odpowiednio zmodyfikować wywołanie collideCell.
Uwzględnić masę cząstki zależną od jej rodzaju przy wyliczaniu nowej prędkości.
(EN)
Modify the collideCell call accordingly.
Take into account the mass of the particle depending on its type when calculating the new velocity.

Plik; (EN) file: particles_kernel.cuh

SimParams
Zmienić parametry cząstek float (particleRadius, particleMass) na tablice float i odwoływać się do nich
przez numer typu cząstki.
(EN)
Change the float particle parameters (particleRadius, particleMass) to float arrays and refer to them 
by the particle type number.

Plik; (EN) file: particleSystem.cpp

Chyba gdzieś w tym pliku trzeba zadbać o zamiany 1 z numerem typu cząstki.
(EN)
I guess somewhere in this file you have to take care of the substitutions of 1 with the particle type number.

ParticleSystem::ParticleSystem
Ustalić parametry cząstek dla różnych typów.
Dobrze by było wcześniej mieć wektor z parametrami typów cząstek.
(EN)
Determine the particle parameters for the different types.
It would be good to have a vector with the parameters of the particle types beforehand.

ParticleSystem::initGrid, ParticleSystem::reset
Losować typ cząstki z dostępnych.
Powinna tu wkroczyć tablica z przypisanymi typami do cząstek.
(EN)
Randomise the particle type from those available.
An array with the assigned types to the particles should enter here.

ParticleSystem::particleType
Ustalić, czy to dobry pomysł i zintegrować z resztą.
(EN)
Determine whether this is a good idea and integrate with the rest.

ParticleSystem::update
Tu jest połączenie sterowania CUDA i GL.
Należy zadbać, żeby przełączanie 1 do normalizacji z numerem typu cząstki przebiegało w właściwej kolejności.
(EN)
Here is a combination of CUDA and GL control.
Care must be taken to ensure that the switching of 1 for normalisation with the particle type number is done in the correct order.

Plik; (EN) file: particles.cpp

Zadbać o ustalenie na początku symulacji ile będzie typów cząstek i jakie one będą.
(EN)
Take care to establish at the start of the simulation how many particle types there will be and what they will be.
