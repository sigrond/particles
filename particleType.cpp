/** \file particleType.cpp
 * \brief Definicje metod klasy particleType.
 * Na razie zawiera tylko inicjalizacjê licznika obiektów (pole statyczne)
 * \author Tomasz Jakubczyk
 */

#include "particleType.h"

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
