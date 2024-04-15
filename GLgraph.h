/** \file GLgraph.h
 * \author Tomasz Jakubczyk
 * \brief Plik nagłówkowy klasy GLgraph - (EN) GLgraph class header file
 */

#include<vector>
/** \brief Klasa rysująca wykres - (EN) Class drawing a graph
 */
class GLgraph
{
public:
    GLgraph():dataVectorPtr(NULL){};
    ~GLgraph(){};
    void setDataVector(std::vector<float>* srcV);
    void render(float,float);
private:
    std::vector<float>* dataVectorPtr;
};
