/** \file particleType.h
 * \brief Zawiera klase opisującą typ cząstki
 * \author Tomasz Jakubczyk
 *
 */

#pragma once

 #include <string>
 #include <vector>

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
    /** \var particleNoOfType
     * \brief Ilość cząstek danego typu w symulacji
     */
    unsigned int particleNoOfType;
    /** \var ofTypeParticleTrack
     * \brief Tablica wskaźników na pozycje cząstek w pamięci hosta
     */
    std::vector<float*> ofTypeParticleTrack;
    std::string particleName;
    unsigned short typeNumber;
    static unsigned short typeCounter;
    particleType() :
        typeNumber(typeCounter), particleRadius(0.0f), particleMass(0.0f), particleNoOfType(0)
    {
        typeCounter++;
    }
    ~particleType()
    {
        typeCounter--;
    }
    void setParticleName(std::string name);
};


