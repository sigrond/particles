/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

 /** \file render_particles.cpp
  * \brief Zawiera implementacje funkcji związanych z wyświetlaniem cząstek w GL.
  * Generalnie metody klasy ParticleRenderer.
  */


#include <GL/glew.h>

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "render_particles.h"
#include "shaders.h"

#include "particleType.h"

#ifndef M_PI
#define M_PI    3.1415926535897932384626433832795
#endif

extern bool multiColor;
extern std::vector<particleType> typyCzastek;

ParticleRenderer::ParticleRenderer()
    : m_pos(0),
      m_numParticles(0),
      m_pointSize(1.0f),
      m_particleRadius(0.125f * 0.5f),
      m_program(0),
      m_vbo(0),
      m_colorVBO(0)
{
    _initGL();
}

ParticleRenderer::~ParticleRenderer()
{
    m_pos = 0;
}

void ParticleRenderer::setPositions(double *pos, int numParticles)
{
    m_pos = pos;
    m_numParticles = numParticles;
}

void ParticleRenderer::setVertexBuffer(unsigned int vbo, int numParticles, float zoom)
{
    m_vbo = vbo;
    m_numParticles = numParticles;
	this->zoom=zoom;
}

void ParticleRenderer::_drawPoints()
{
    if (!m_vbo)/**< zarejestrowanie miejsc w pamięci gdzie są współżędne punktów */
    {
        glBegin(GL_POINTS);
        {
            int k = 0;

            for (int i = 0; i < m_numParticles; ++i)
            {
                //glVertex3fv(&m_pos[k]);
				glVertex3f(m_pos[k], m_pos[k+1], m_pos[k+2]);
                k += 4;
            }
        }
        glEnd();
    }
    else
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
        glVertexPointer(4, GL_FLOAT, 0, 0);
        glEnableClientState(GL_VERTEX_ARRAY);

        if (m_colorVBO)
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_colorVBO);
            glColorPointer(4, GL_FLOAT, 0, 0);
            glEnableClientState(GL_COLOR_ARRAY);
        }

        glDrawArrays(GL_POINTS, 0, m_numParticles);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

/** \brief Funkcja odpowiedzialna za rysowanie cząstek.
 * Została utworzona na podstawie oryginalnej _drawPoints().
 * Rysuje tylko zadany fragment bufora współżędnych.
 * Dzieki tej funkcji można rysować grupy cząstek o różnych rozmiarach i kolorach.
 * \author Tomasz Jakubczyk
 * \param pFrom int Numer pierwszej cząstki z zakresu do odrysowania.
 * \param pTo int Numer pierwszej cząstki poza zakresem do odrysowania.
 * \return void
 *
 */
void ParticleRenderer::_drawPoints(int pFrom, int pTo)
{
    if (!m_vbo)/**< zarejestrowanie miejsc w pamięci gdzie są współżędne punktów */
    {
        glBegin(GL_POINTS);
        {
            int k = pFrom*4;

            for (int i = pFrom; i < pTo; ++i)
            {
                //glVertex3fv(&m_pos[k]);
				glVertex3f(m_pos[k], m_pos[k + 1], m_pos[k + 2]);
                k += 4;
            }
        }
        glEnd();
    }
    else
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vbo);
        glVertexPointer(4, GL_FLOAT, 0, 0);
        glEnableClientState(GL_VERTEX_ARRAY);

        if (m_colorVBO && multiColor)/**< nie używamy bufora kolorów jeśli wszystkie cząstki z grupy mają być w jednym kolorze. */
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_colorVBO);
            glColorPointer(4, GL_FLOAT, 0, 0);
            glEnableClientState(GL_COLOR_ARRAY);
        }

        glDrawArrays(GL_POINTS, pFrom, (pTo-pFrom));/**< rysowanie fragmentu bufora */

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

/** \brief Funkcja ustawiająca sposób rysowania i parametry oraz
 * wywołująca funkcję odrysowującą _drawPoints
 * \param DisplayMode mode  = PARTICLE_POINTS
 * \return void
 *
 */
void ParticleRenderer::display(DisplayMode mode /* = PARTICLE_POINTS */)
{
/*#ifdef _DEBUG
	system("pause");
	std::clog<<"typyCzastek.size() "<<typyCzastek.size()<<"\n";
#endif*/
    int nextType=0;
    int typeBegin=0;
    switch (mode)
    {
        case PARTICLE_POINTS:
            glColor3f(1, 1, 1);
            glPointSize(m_pointSize);
            _drawPoints();
            break;

        default:
        case PARTICLE_SPHERES:
            glEnable(GL_POINT_SPRITE_ARB);
            glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_NV);
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);

			glUseProgram(m_program);
            for(int i=0;i<typyCzastek.size();i++)/**< rysujemy cząstki grupami. Wszystkie cząstki z jednego typu za raz ze wspólnymi cechami (promień i kolor) */
            {

                glUniform1f(glGetUniformLocation(m_program, "pointScale"), zoom*m_window_h / tanf(m_fov*0.5f*(float)M_PI/180.0f));/**< skalowanie rozmiaru cząstki do ekranu */
                glUniform1f(glGetUniformLocation(m_program, "pointRadius"), typyCzastek[i].particleRadius);/**< nadanie grupie cząstek jednego promienia */

                glColor3f(typyCzastek[i].particleColorR, typyCzastek[i].particleColorG, typyCzastek[i].particleColorB);/**< ustalenie jednego koloru dla grupy cząstek */

				nextType+=typyCzastek[i].particleNoOfType;
                _drawPoints(typeBegin, nextType);/**< wywołanie rysowania grupy cząstek */
                typeBegin=nextType;

			}
			glUseProgram(0);

            glDisable(GL_POINT_SPRITE_ARB);
            break;
    }
}

GLuint
ParticleRenderer::_compileProgram(const char *vsource, const char *fsource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vsource, 0);
    glShaderSource(fragmentShader, 1, &fsource, 0);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    // check if program linked
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char temp[256];
        glGetProgramInfoLog(program, 256, 0, temp);
        printf("Failed to link program:\n%s\n", temp);
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

void ParticleRenderer::_initGL()
{
    m_program = _compileProgram(vertexShader, spherePixelShader);

#if !defined(__APPLE__) && !defined(MACOSX)
    glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
    glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
#endif
}
