/** \file particleTypesLoader.cpp
 * \brief Definicje metod klasy particleTypesLoader
 * \author Tomasz Jakubczyk
 */
#include "particleTypesLoader.h"
#include <cstdlib>
#include <sstream>
#include <iostream>

/** \todo przydała by się jakaś ładna implementacja ltrim, rtrim i
 * trim żeby łatwo usuwać nadmiarowe białe znaki
 */

/** \brief Załaduje dane z pliku do wektora.
 * Plik będzie na nowo otwarty i przeczytany,
 * a wektor wyczyszczony i wypełniony od nowa
 * \param dstV std::vector<particleType>&
 * \return void
 * \todo Potrzebny jest opis struktury pliku
 */
void particleTypesLoader::loadTypes(std::vector<particleType> &dstV)
{
    std::string dataStr;
    unsigned int lastPos=0;
    std::stringstream iss;
    configFile.open(fileName.c_str());
    if(!configFile.is_open())
    {
		std::cerr<<"can't open the config file\n";
        return;
    }
    dstV.clear();
    while(configFile.good())
    {
        std::getline(configFile,dataStr,' ');
        dataStr.erase(0,dataStr.find_first_not_of(" \f\n\r\t\v"));/**< ltrim */
        if(dataStr.compare("particleType")==0)/**< rozpoczęcie opisu typu cząstki */
        {/**< nie przewiduję żeby było tu więcej białych znaków niz jedna spacja */
            std::getline(configFile,dataStr,'{');/**< nazwa typu. parametry w {} */
            dataStr.erase(dataStr.find_last_not_of(" \f\n\r\t\v")+1);/**< rtrim */
            dstV.push_back(particleType());
            lastPos=dstV.size()-1;
            dstV[lastPos].setParticleName(dataStr);
#ifdef _DEBUG
			std::clog<<"particleType "<<dataStr<<"\n{\n";
#endif
            std::getline(configFile,dataStr,'}');
            iss<<dataStr;
            while(iss.good())/**< wypełnianie parametrów */
            {
                std::getline(iss,dataStr,'=');/**< wysypie się jeśli nie będzie '=' */
                dataStr.erase(0,dataStr.find_first_not_of(" \f\n\r\t\v"));/**< ltrim */
                dataStr.erase(dataStr.find_last_not_of(" \f\n\r\t\v")+1);/**< rtrim */
                if(dataStr.compare("particleRadius")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleRadius=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleRadius="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleMass")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleMass=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleMass="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleNoOfType")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleNoOfType=(unsigned int)atoi(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleNoOfType="<<(unsigned int)atoi(dataStr.c_str())<<";\n";
#endif
                }
            }
            iss.clear();
#ifdef _DEBUG
			std::clog<<"}\n";
#endif
        }
    }
    configFile.close();
}

















