/** \file particleType.h
 * \brief Zawiera klase opisującą typ cząstki
 * \author Tomasz Jakubczyk
 *
 */

#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

/** \class particleType
 * \brief Klasa przechowująca informacje o rodzajach cząstek.
 */
class particleType
{
public:
    /**< mogło by być śliczne get i set, ale na razie są niepotrzebne */
	double particleRadius;/**< \brief promień */
	double particleMass;/**< \brief masa */
	double particleDensity;/**< \brief gęstość */
	double particleCharge;/**< \brief ładunek */
    /**< można jednemu typowi cząstek nadać jeden kolor */
	double particleColorR;/**< \brief natężenie koloru czerwonego */
	double particleColorG;/**< \brief natężenie koloru zielonego */
	double particleColorB;/**< \brief natężenie koloru niebieskiego */
    /** \brief Vektor zawierający mnożniki epsilona do potencjału
     * Lennarda-Jonesa. Pozycja w wektorze odpowiada przeciciu
     * dwuch typów zgodnie ze wzorem:
     * \f[ i \left ( a, b \right ) = a \left ( n - \frac{ a - 1 }{2} \right ) + b \f]
     * gdzie x,y numery rodzajów cząstek i
     * \f[ a = min \left ( x , y \right ), b = max  \left ( x , y  \right ) - min \left ( x , y \right ) \f]
     * dwu wymiarowa wizualizacja jedno wymiarowego wektora
     * \verbatim
      a\b| 0  1  2  3  4 <-kolejne numery typów
      ---|--------------
       0 | 0  1  2  3  4 <-numery w wektorze
       1 |    5  6  7  8
       2 |       9  10 11
       3 |          12 13
       4 |             14
          ^
          |_-operacja wyliczania potencjału jest przemienny
            względem kolejności cząstek i można zaoszczędzić
            pamięć nie zapisując powtórzeń.
      \endverbatim
     * Zatem złożoność pamięciowa przechowywania epsilonów jest:
     * \f[ O \left ( m \right ) = \frac{1}{2} n^{2} + \frac{1}{2} n \f]
     */
    static std::vector<double> normalizedEpsilon;
    /** \var particleNoOfType
     * \brief Ilość cząstek danego typu w symulacji
     */
    unsigned int particleNoOfType;
    /** \var ofTypeParticleTrack
     * \brief Tablica pozycji cząstek w pamięci hosta
     * jest to pozycja pierwszej liczby z czwórki opisującej cząstkę
     * jeśli cząstki są uporządkowane po typach, to wystarczy sprawdzać
     * ile cząstek jest w typie i po numerze cząstki znać jej typ
     */
    std::vector<int> ofTypeParticleTrack;
    std::string particleName;/**< \brief nazwa rodzaju cząstki */
    unsigned short typeNumber;/**< \brief numer rodzaju cząstki */
    static unsigned short typeCounter;/**< \brief licznik typów */
    particleType() :
        typeNumber(typeCounter), particleRadius(0.0f), particleMass(0.0f), particleNoOfType(0),
        particleColorR(0), particleColorG(0), particleColorB(0)
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


