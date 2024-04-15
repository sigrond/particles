/** \file particleType.cpp
 * \brief Definicje metod klasy particleType. - (EN) Method definitions of the particleType class.
 * Na razie zawiera tylko inicjalizację licznika obiektów (pole statyczne)
 * - (EN) For the time being, it only includes initialisation of the object counter (static field)
 * \author Tomasz Jakubczyk
 */

#include "particleType.h"

std::vector<float> particleType::normalizedEpsilon;

unsigned short particleType::typeCounter=0;

/** \brief Ustawienie nazwy typu cząstki - (EN) Setting the name of the particle type
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
 * - (EN) This method determines whether the parameters needed for the 
 * simulation have been set. If they are missing and it is able to calculate 
 * from others, it calculates. Basically, it has to calculate the mass from the 
 * radius and density.
 *
 * units: micrometre, nanogram, second
 *
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

















