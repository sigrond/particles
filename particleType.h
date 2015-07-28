/** \file particleType.h
 * \brief Zawiera klase opisującą typ cząstki
 * \author Tomasz Jakubczyk
 *
 */

#pragma once

 #include <string>
 #include <vector>
 #include <cstdlib>

/** \class particleType
 * \brief Klasa przechowująca informacje o rodzajach cząstek.
 */
class particleType
{
public:
    /**< mogło by być śliczne get i set, ale na razie są niepotrzebne */
    float particleRadius;
    float particleMass;
    float particleDensity;
    float particleCharge;
    /**< można jednemu typowi cząstek nadać jeden kolor */
    float particleColorR;
    float particleColorG;
    float particleColorB;
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
    std::string particleName;
    unsigned short typeNumber;
    static unsigned short typeCounter;
    particleType() :
        typeNumber(typeCounter), particleRadius(0.0f), particleMass(0.0f), particleNoOfType(0),
        particleColorR(0), particleColorG(0), particleColorB(0)
    {
        typeCounter++;
        if(typeNumber==0)particleColorG=1.0f;
        if(typeNumber==1)particleColorR=1.0f;
        if(typeNumber==2)particleColorB=1.0f;
        if(typeNumber>=3)
        {
            particleColorR=rand() / (float) RAND_MAX;
            particleColorG=rand() / (float) RAND_MAX;
            particleColorB=rand() / (float) RAND_MAX;
        }
    }
    ~particleType()
    {
        typeCounter--;
    }
    void setParticleName(std::string name);
};


