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
  /** \file particleSystem.h
  * \brief Definicja klasy ParticleSystem i części jej prostych metod.
  * - (EN) Definition of the ParticleSystem class and some of its simpler methods.
  * 
  */

#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#define DEBUG_GRID 0
#define DO_TIMING 0

#include <helper_functions.h>
#include "particles_kernel.cuh"
#include "vector_functions.h"

// Particle system class
/** \brief Klasa odpowiadająca za parametry i ustawienie symulacji.
 * - (EN) The class responsible for the parameters and setting of the simulation.
 */
class ParticleSystem
{
    public:
        ParticleSystem(uint numParticles, uint3 gridSize, bool bUseOpenGL);
        ~ParticleSystem();

        enum ParticleConfig
        {
            CONFIG_RANDOM,
            CONFIG_GRID,
            _NUM_CONFIGS
        };

        enum ParticleArray
        {
            POSITION,
            VELOCITY,
        };

        void update(float deltaTime);
        void reset(ParticleConfig config);

        float *getArray(ParticleArray array);
        void   setArray(ParticleArray array, const float *data, int start, int count);

        int    getNumParticles() const
        {
            return m_numParticles;
        }

        unsigned int getCurrentReadBuffer() const
        {
            return m_posVbo;
        }
        unsigned int getColorBuffer()       const
        {
            return m_colorVBO;
        }

        void *getCudaPosVBO()              const
        {
            return (void *)m_cudaPosVBO;
        }
        void *getCudaColorVBO()            const
        {
            return (void *)m_cudaColorVBO;
        }

        void dumpGrid();
        void dumpParticles(uint start, uint count);

        void setIterations(int i)
        {
            m_solverIterations = i;
        }

        void setDamping(float x)
        {
            m_params.globalDamping = x;
        }
        void setGravity(float x)
        {
            m_params.gravity = make_float3(0.0f, x, 0.0f);
        }
		void setRotation(float x)
        {
            m_params.rotation = x;
        }

		void setBoundaryDamping(float x)// ustawienie wsp napiêcia powierzchniowego
        {
            m_params.boundaryDamping = x;
        }
		/** \brief ustawia masę cząstki - (EN) sets the mass of the particle
		 * \todo jakoś poprawić bo rożne typy cząstek mają różne masy - (EN) somehow 
		 * improve because different types of particles have different masses -- check - maybe it's already been done
		 */
		void setParticleMass(float x)
		{
			m_params.particleMass[0]=x;
		}
		void setEpsi(float x)
		{
			m_params.epsi=x;
		}

        void setCollideSpring(float x)
        {
            m_params.spring = x;
        }
        void setCollideDamping(float x)
        {
            m_params.damping = x;
        }
        void setCollideShear(float x)
        {
            m_params.shear = x;
        }
        void setCollideAttraction(float x)
        {
            m_params.attraction = x;
        }

		void setBigRadius(float x)//ustawienie promienia dużej kuli - (EN) setting the radius of the large sphere
		{
			m_params.bigradius=x;
		}
		void setBigRadius0(float x)//ustawienie początkowego promienia dużej kuli - (EN) setting the initial radius of the large sphere
		{
			m_params.bigradius0=x;
		}
		void setBrown(float x)
		{
			m_params.brown=x;
		}
		void setBrownQuality(unsigned long long int x)
		{
			m_params.brownQuality=x;
		}

		void setBoundaries(bool x)//ograniczenia zewnętrzne - (EN) external constraints
		{
			m_params.boundaries=x;
		}

		void setParticleTypesNum(int x)//ograniczenia zewnętrzne - (EN) external constraints
		{
			m_params.particleTypesNum=x;
		}

        void setColliderPos(float3 x)
        {
            m_params.colliderPos = x;
        }

        /** \brief ustawienie zmiennej logicznej czy chcemy
         * policzyć i zwrócić ciśnienie powierzchniowe kropli - (EN) setting of the logical 
		 * variable whether we want to calculate and return the surface pressure of the droplet
         * \param x bool
         * \return void
         *
         */
        void setCalcSurfacePreasure(bool x)
        {
            m_params.calcSurfacePreasure=x;
        }

        /** \brief ustawia chwilową prędkość zmniejszania się promienia kropli - (EN) sets the 
		 * instantaneous droplet radius decrease speed
         *
         * \param x float
         * \return void
         *
         */
        void setSurfaceVel(float x)
        {
            m_params.surfaceVel=x;
        }

		/** \brief pobiera promień cząstki - (EN) gets the radius of the particle
		 * \todo poprawić z uwzględnieniem różnych typów cząstek - (EN) improve for multiple particle types 
		 * -- check - maybe it's already been done 
		 */
        float getParticleRadius()
        {
            return m_params.particleRadius[0];
        }

        /** \brief Zwraca promień największej cząstki. - (EN) Returns the radius of the largest particle.
         *
         * \return float
         *
         */
        float getMaxParticleRadius()
        {
            float tmpMax=0.0f;
            for(int i=0; i<m_params.particleTypesNum;i++)
            {
                if(m_params.particleRadius[i]>tmpMax)
                {
                    tmpMax=m_params.particleRadius[i];
                }
            }
            return tmpMax;
        }

        /** \brief ustawia czy zmienny krok czasu, czy stały - (EN) sets whether the time step is variable or fixed
         *
         * \param x bool
         * \return void
         *
         */
        void setAutoDt(bool x)
        {
            m_params.autoDt=x;
        }

        float3 getColliderPos()
        {
            return m_params.colliderPos;
        }
        float getColliderRadius()
        {
            return m_params.colliderRadius;
        }
        uint3 getGridSize()
        {
            return m_params.gridSize;
        }
        float3 getWorldOrigin()
        {
            return m_params.worldOrigin;
        }
        float3 getCellSize()
        {
            return m_params.cellSize;
        }

        void addSphere(int index, float *pos, float *vel, int r, float spacing);

		//class particleType;

    protected: // methods
        ParticleSystem() {}
        uint createVBO(uint size);

        void _initialize(int numParticles);
        void _finalize();

        void initGrid(uint *size, float spacing, float jitter, uint numParticles);

    protected: // data
        bool m_bInitialized, m_bUseOpenGL;
        uint m_numParticles;

        // CPU data
        float *m_hPos;              // particle positions
        float *m_hVel;              // particle velocities

        uint  *m_hParticleHash;
        uint  *m_hCellStart;
        uint  *m_hCellEnd;

        // GPU data
        float *m_dPos;
        float *m_dVel;

        float *m_dForce;/**<  */

        float *m_dSortedPos;
        float *m_dSortedVel;

        // grid data for sorting method
        uint  *m_dGridParticleHash; // grid hash value for each particle
        uint  *m_dGridParticleIndex;// particle index for each particle
        uint  *m_dCellStart;        // index of start of each cell in sorted list
        uint  *m_dCellEnd;          // index of end of cell

        uint   m_gridSortBits;

        uint   m_posVbo;            // vertex buffer object for particle positions
        uint   m_colorVBO;          // vertex buffer object for colors

        float *m_cudaPosVBO;        // these are the CUDA deviceMem Pos
        float *m_cudaColorVBO;      // these are the CUDA deviceMem Color


        struct cudaGraphicsResource *m_cuda_posvbo_resource; // handles OpenGL-CUDA exchange
        struct cudaGraphicsResource *m_cuda_colorvbo_resource; // handles OpenGL-CUDA exchange

        // params
        SimParams m_params;
        uint3 m_gridSize;
        uint m_numGridCells;

        StopWatchInterface *m_timer;

        uint m_solverIterations;
};

#endif // __PARTICLESYSTEM_H__
