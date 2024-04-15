/** \file particleType.h
 * \brief Zawiera klasę opisującą typ cząstki - (EN) Contains a class describing the type of particle
 * \author Tomasz Jakubczyk
 *
 */

#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

/** \class particleType
 * \brief Klasa przechowująca informacje o rodzajach cząstek. - (EN) A class storing information about particle types.
 */
class particleType
{
public:
    /**< mogło by być śliczne get i set, ale na razie są niepotrzebne - (EN) could be a lovely "get" and "set", but for now they are unnecessary */
    float particleRadius;/**< \brief promień - (EN) radius */
    float particleMass;/**< \brief masa - (EN) mass */
    float particleDensity;/**< \brief gęstość - (EN) density */
    float particleCharge;/**< \brief ładunek - (EN) charge*/
    /**< można jednemu typowi cząstek nadać jeden kolor - (EN) one type of particle can be given one colour */
    float particleColorR;/**< \brief natężenie koloru czerwonego - (EN) red intensity */
    float particleColorG;/**< \brief natężenie koloru zielonego - (EN) green intensity */
    float particleColorB;/**< \brief natężenie koloru niebieskiego - (EN) blue intensity */
    /** \brief Vektor zawierający mnożniki epsilona do potencjału
     * Lennard-Jonesa. Pozycja w wektorze odpowiada przecięciu
     * dwóch typów zgodnie ze wzorem:
	 * - (EN)Vector containing the epsilon multipliers for the Lennard-Jones potential. The position in the vector 
	 * corresponds to the intersection of the two types according to the formula:
     * \f[ i \left ( a, b \right ) = a \left ( n - \frac{ a - 1 }{2} \right ) + b \f]
     * gdzie x,y numery rodzajów cząstek i - (EN) where x,y are the numbers of the particle types and
     * \f[ a = min \left ( x , y \right ), b = max  \left ( x , y  \right ) - min \left ( x , y \right ) \f]
     * dwuwymiarowa wizualizacja jednowymiarowego wektora - (EN) two-dimensional visualisation of a one-dimensional vector
     * \verbatim
      a\b| 0  1  2  3  4 <-kolejne numery typów - (EN) consecutive type numbers
      ---|--------------
       0 | 0  1  2  3  4 <-numery w wektorze - (EN) numbers in a vector
       1 |    5  6  7  8
       2 |       9  10 11
       3 |          12 13
       4 |             14
          ^
          |_-operacja wyliczania potencjału jest przemienna
            względem kolejności cząstek i można zaoszczędzić
            pamięć nie zapisując powtórzeń.
			- (EN) the operation of calculating the potential is 
			alternate with respect to the particle order 
			and memory can be saved by not storing repetitions.
      \endverbatim
     * Zatem złożoność pamięciowa przechowywania epsilonów jest:
	 * - (EN) Thus, the memory complexity of storing epsilons is:
     * \f[ O \left ( m \right ) = \frac{1}{2} n^{2} + \frac{1}{2} n \f]
     */
    static std::vector<float> normalizedEpsilon;
	float surfaceTensionFactor;/**< \brief współczynnik napięcia powierzchniowego - (EN) surface tension coefficient */
    /** \var particleNoOfType
     * \brief Ilość cząstek danego typu w symulacji - (EN) Number of particles of a given type in the simulation
     */
    unsigned int particleNoOfType;
    /** \var ofTypeParticleTrack
     * \brief Tablica pozycji cząstek w pamięci hosta
     * jest to pozycja pierwszej liczby z czwórki opisującej cząstkę,
     * jeśli cząstki są uporządkowane po typach, to wystarczy sprawdzać
     * ile cząstek jest w typie i po numerze cząstki znać jej typ
	 * - (EN) The array of particle positions in the host memory is the 
	 * position of the first number of the four describing the particle;
	 * if the particles are ordered by type, it is sufficient to check how 
	 *many particles are in type and to know the type of the particle by its number
     */
    std::vector<int> ofTypeParticleTrack;
    std::string particleName;/**< \brief nazwa rodzaju cząstki - (EN) name of the particle type */
    unsigned short typeNumber;/**< \brief numer rodzaju cząstki - (EN) particle type number */
    static unsigned short typeCounter;/**< \brief licznik typów - (EN) type counter */
    particleType() :
        typeNumber(typeCounter), particleRadius(0.0f), particleMass(0.0f), particleNoOfType(0),
        particleColorR(0), particleColorG(0), particleColorB(0),surfaceTensionFactor(1.0f)
    {
        typeCounter++;
#ifdef _DEBUG
		std::clog<<"particleType contructor "<<typeNumber<<"\n";
#endif
        if(typeNumber==0)particleColorG=1.0f;
        if(typeNumber==1)particleColorR=1.0f;
        if(typeNumber==2)particleColorB=1.0f;
        /*if(typeNumber>=3)
        {
            particleColorR=rand() / (float) RAND_MAX;
            particleColorG=rand() / (float) RAND_MAX;
            particleColorB=rand() / (float) RAND_MAX;
        }*/
    }
    ~particleType()
    {
        typeCounter--;
    }
    void setParticleName(std::string name);
    void calcParams(bool,bool,bool);
};


