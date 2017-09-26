/** \file GLgraph.h
 * \author Tomasz Jakubczyk
 * \brief Plik nagłówkowy klasy GLgraph
 */

#include<vector>
/** \brief Klasa rysująca wykres
 */
class GLgraph
{
public:
    GLgraph():dataVectorPtr(NULL){};
    ~GLgraph(){};
    void setDataVector(std::vector<double>* srcV);
    void render(double, double);
private:
    std::vector<double>* dataVectorPtr;
};
