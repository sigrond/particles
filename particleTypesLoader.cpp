/** \file particleTypesLoader.cpp
 * \brief Definicje metod klasy particleTypesLoader - (EN) Method definitions of the particleTypesLoader class
 * \author Tomasz Jakubczyk
 */
#include "particleTypesLoader.h"
#include <cstdlib>
#include <sstream>
#include <iostream>

/** \todo przydała by się jakaś ładna implementacja ltrim, rtrim i
 * trim żeby łatwo usuwać nadmiarowe białe znaki
 * - (EN) It would be useful to have a nice implementation of ltrim,
 * rtrim and trim to easily remove redundant whitespace characters.
 */

/** \brief Załaduje dane z pliku do wektora.
 * Plik będzie na nowo otwarty i przeczytany,
 * a wektor wyczyszczony i wypełniony od nowa.
 * - (EN) Loads data from a file into a vector.
 * The file will be reopened and read, and the vector will be cleared and filled anew.
 * \param dstV std::vector<particleType>&
 * \return void
 * \todo Potrzebny jest opis struktury pliku - (EN) A description of the file structure is needed
 */
void particleTypesLoader::loadTypes(std::vector<particleType> &dstV)
{
    bool pMassSet=false,pRadiusSet=false,pDensitySet=false;
    particlesNumber=0;
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
        if(dataStr.compare("particleType")==0)/**< rozpoczęcie opisu typu cząstki - (EN) start of particle type description */
        {/**< nie przewiduję żeby było tu więcej białych znaków niż jedna spacja - (EN) I do not anticipate that there will be more whitespace characters than one space */
            std::getline(configFile,dataStr,'{');/**< nazwa typu. parametry w {} - (EN) type name. parameters in {} */
            dataStr.erase(dataStr.find_last_not_of(" \f\n\r\t\v")+1);/**< rtrim */
            dstV.push_back(*(new particleType()));
            lastPos=dstV.size()-1;
            dstV[lastPos].setParticleName(dataStr);
#ifdef _DEBUG
			std::clog<<"particleType "<<dataStr<<"\n{\n";
#endif
            std::getline(configFile,dataStr,'}');
            iss<<dataStr;
            pMassSet=false;
            pRadiusSet=false;
            pDensitySet=false;
            while(iss.good())/**< wypełnianie parametrów - (EN) filling in the parameters */
            {
                std::getline(iss,dataStr,'=');/**< wysypie się jeśli nie będzie '=' - (EN) will be blown up if there is no '=' */
                dataStr.erase(0,dataStr.find_first_not_of(" \f\n\r\t\v"));/**< ltrim */
                dataStr.erase(dataStr.find_last_not_of(" \f\n\r\t\v")+1);/**< rtrim */
                if(dataStr.compare("particleRadius")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleRadius=(float)atof(dataStr.c_str());
                    pRadiusSet=true;
#ifdef _DEBUG
					std::clog<<"\tparticleRadius="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleMass")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleMass=(float)atof(dataStr.c_str());
                    pMassSet=true;
#ifdef _DEBUG
					std::clog<<"\tparticleMass="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleNoOfType")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleNoOfType=(unsigned int)atoi(dataStr.c_str());
                    particlesNumber+=dstV[lastPos].particleNoOfType;
#ifdef _DEBUG
					std::clog<<"\tparticleNoOfType="<<(unsigned int)atoi(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleDensity")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleDensity=(float)atof(dataStr.c_str());
                    pDensitySet=true;
#ifdef _DEBUG
					std::clog<<"\tparticleDensity="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleCharge")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleCharge=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleCharge="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleColorR")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleColorR=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleColorR="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleColorG")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleColorG=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleColorG="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
                else if(dataStr.compare("particleColorB")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].particleColorB=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\tparticleColorB="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
				else if(dataStr.compare("surfaceTensionFactor")==0)
                {
                    std::getline(iss,dataStr,';');
                    dstV[lastPos].surfaceTensionFactor=(float)atof(dataStr.c_str());
#ifdef _DEBUG
					std::clog<<"\surfaceTensionFactor="<<(float)atof(dataStr.c_str())<<";\n";
#endif
                }
            }
            dstV[lastPos].calcParams(pMassSet,pRadiusSet,pDensitySet);
            iss.clear();
#ifdef _DEBUG
			std::clog<<"}\n";
#endif
        }
        if(dataStr.compare("normalizedEpsilon")==0)/**< specjalna skrócona tablica znormalizowanych epsilonów jak w particleType - (EN) special abbreviated array of normalised epsilons as in particleType */
        {
            std::getline(configFile,dataStr,'{');/**< nawias otwierający - (EN) opening bracket */
#ifdef _DEBUG
			std::clog<<"normalizedEpsilon "<<dataStr<<"\n{\n";
#endif
            std::getline(configFile,dataStr,'}');
            iss<<dataStr;
            particleType::normalizedEpsilon.clear();
            while(iss.good())/**< wypełnianie parametrów - (EN) filling in the parameters */
            {
                std::getline(iss,dataStr,',');
                dataStr.erase(0,dataStr.find_first_not_of(" \f\n\r\t\v"));/**< ltrim */
                dataStr.erase(dataStr.find_last_not_of(" \f\n\r\t\v")+1);/**< rtrim */
                particleType::normalizedEpsilon.push_back((float)atof(dataStr.c_str()));
            }

            iss.clear();
        }
    }
	if(particlesNumber==0)
	{
		throw "loading error";
	}
    configFile.close();
#ifdef _DEBUG
	std::clog<<"dstV.size() "<<dstV.size()<<"\n";
#endif
}

/** \brief Zwraca sumę cząstek ze wszystkich podanych typów - (EN) Returns the sum of particles of all specified types
 *
 * \return unsigned int
 *
 */
unsigned int particleTypesLoader::getParticlesNumber()
{
    return particlesNumber;
}

















