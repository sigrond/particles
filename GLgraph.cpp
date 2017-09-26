/** \file GLgraph.cpp
 * \author Tomasz Jakubczyk
 * \brief Plik z metodami klasy GLgraph
 */
#include "GLgraph.h"
#include <GL/freeglut.h>

/** \brief Metoda przypisuje klasie wskaźnik do wektora
 * na podstawie którego ma byc rysowany wykres.
 * \param srcV std::vector<float>*
 * \return void
 *
 */
void GLgraph::setDataVector(std::vector<double>* srcV)
{
    dataVectorPtr=srcV;
}

inline void beginWinCoords(void)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0, (GLfloat)(glutGet(GLUT_WINDOW_HEIGHT) - 1.0), 0.0);
    glScalef(1.0, -1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);

    glMatrixMode(GL_MODELVIEW);
}

inline void endWinCoords(void)
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/** \brief Rysowanie wykresu
 *
 * \param xStartPos float
 * \param yStartPos float
 * \return void
 *
 */
void GLgraph::render(double xStartPos, double yStartPos)
{
    beginWinCoords();
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0,1,0);
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(xStartPos, yStartPos);
    float x = xStartPos;
	float width=(float)glutGet(GLUT_WINDOW_WIDTH);
	float hight=(float)glutGet(GLUT_WINDOW_HEIGHT);
	int pomniejszenie=1;
    for (int i=0; i<dataVectorPtr->size(); i++)
    {
		pomniejszenie=(int)((float)dataVectorPtr->size()/(float)width+1.2f);
        x+=1.0f;//1000.0f/((float)dataVectorPtr->size());
        glVertex2f(x/(float)pomniejszenie, -(*dataVectorPtr)[i]*5.0f*hight+(float)yStartPos);
    }
    //glVertex2f(width+xStartPos, yStartPos);
    glEnd();
	glLineWidth(1.0);
    endWinCoords();
}
