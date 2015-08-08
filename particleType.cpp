/** \file particleType.cpp
 * \brief Definicje metod klasy particleType.
 * Na razie zawiera tylko inicjalizacjê licznika obiektów (pole statyczne)
 * \author Tomasz Jakubczyk
 */

#include "particleType.h"

std::vector<float> particleType::normalizedEpsilon;

unsigned short particleType::typeCounter=0;

/** \brief Ustawienie nazwy typu cząstki
 *
 * \param name std::string
 * \return void
 *
 */
void particleType::setParticleName(std::string name)
{
    particleName=name;
}

#ifndef M_PI
#define M_PI    3.1415926535897932384626433832795
#endif


/** \brief Ta metoda ustala czy zostały ustawione parametry
 * potrzebne do symulacji. Jeśli brakuje i jest wstanie wyliczyć z innych
 * to wylicza. Podstawowo ma wyliczać masę z promienia i gęstości.
 * \param pMassSet bool
 * \param pRadiusSet bool
 * \param pDensitySet bool
 * \return void
 *
 */
void particleType::calcParams(bool pMassSet,bool pRadiusSet,bool pDensitySet)
{
    float Volume=0.0f;
    if((!pMassSet)&&pRadiusSet&&pDensitySet)
    {
        Volume=(4.0f/3.0f)*M_PI*particleRadius*particleRadius*particleRadius;
        //particleDensity=particleMass/Volume;
        particleMass=particleDensity*Volume;
    }
}

















