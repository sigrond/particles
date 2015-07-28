/** \file particleTypesLoader.h
 * \author Tomasz Jakubczyk
 * \brief Zawiera klasę która ma wczytywać informacje
 * o typach cząstek z pliku
 */

#include "particleType.h"
#include <vector>
#include <fstream>

/** \class particleTypesLoader
 * \brief Klasa ładowarki ustawień typów cząstek z pliku
 */
class particleTypesLoader
{
public:
    std::fstream configFile;
    std::string fileName;
    particleTypesLoader(){};
    particleTypesLoader(std::string name) : fileName(name){};
    void loadTypes(std::vector<particleType> &dstV);
};
