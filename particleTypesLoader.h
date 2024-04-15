/** \file particleTypesLoader.h
 * \author Tomasz Jakubczyk
 * \brief Zawiera klasę która ma wczytywać informacje
 * o typach cząstek z pliku - (EN) Contains a class that 
 * is to load information about particle types from a file
 */

#pragma once

#include "particleType.h"
#include <vector>
#include <fstream>

/** \class particleTypesLoader
 * \brief Klasa ładowarki ustawień typów cząstek z pliku
 * - (EN) Class of particle type settings loader from file
 */
class particleTypesLoader
{
private:
    unsigned int particlesNumber;
public:
    std::fstream configFile;
    std::string fileName;
    particleTypesLoader():particlesNumber(0){};
    particleTypesLoader(std::string name) : fileName(name){};
    void loadTypes(std::vector<particleType> &dstV);
    unsigned int getParticlesNumber();
};
