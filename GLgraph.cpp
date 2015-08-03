/** \file GLgraph.cpp
 * \author Tomasz Jakubczyk
 * \brief Plik z metodami klasy GLgraph
 */


/** \brief Metoda przypisuje klasie wskaźnik do wektora
 * na podstawie którego ma byc rysowany wykres.
 * \param srcV std::vector<float>*
 * \return void
 *
 */
void GLgraph::setDataVector(std::vector<float>* srcV)
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
void GLgraph::render(float xStartPos, float yStartPos)
{
    beginWinCoords();
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0,1,0);
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(0f+xStartPos, 0f+yStartPos);
    float x = 0f+xStartPos;
    for (int i=0; x<100.0f+xStartPos && i<dataVectorPtr->size(); i++)
    {
        x+=100.0f/((float)dataVectorPtr->size();
        glVertex2f(x, (*dataVectorPtr)[i]+yStartPos);
    }
    glVertex2f(100f+xStartPos, 0f+yStartPos);
    glEnd();
    endWinCoords();
}
