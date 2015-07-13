/** \file paricleType.h
 * \author Tomasz Jakubczyk
 * \brief Zawiera klase opisującą typ cząstki
 *
 */

/** \brief Klasa przechowująca informacje o rodzajach cząstek.
 */
class particleType
{
public:
    /**< mogło by być śliczne get i set, ale na razie są niepotrzebne */
    float particleRadius;
    float particleMass;
    float particleDensity;
    float particleCharge;
    unsigned short typeNumber;
    static unsigned short typeCounter;
    particleTypes() : typeNumber(typeCounter)
    {
        typeCounter++;
    }
    ~particleTypes()
    {
        typeCounter--;
    }
};

particleType::typeCounter=0;
