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
 /** \file particleSystem.cpp
  * \brief Definicje (implementacje) metod klasy ParticleSystem
  * oraz funkcji pomocniczych
  */


#include "particleSystem.h"
#include "particleSystem.cuh"
#include "particles_kernel.cuh"

#include <cuda_runtime.h>

#include <helper_functions.h>
#include <helper_cuda.h>

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <GL/glew.h>

#include <vector>
#include "particleType.h"

#ifndef CUDART_PI_F
#define CUDART_PI_F         3.141592654f
#endif

extern bool multiColor;
extern std::vector<particleType> typyCzastek;

ParticleSystem::ParticleSystem(uint numParticles, uint3 gridSize, bool bUseOpenGL) :
    m_bInitialized(false),
    m_bUseOpenGL(bUseOpenGL),
    m_numParticles(numParticles),
    m_hPos(0),
    m_hVel(0),
    m_dPos(0),
    m_dVel(0),
    m_gridSize(gridSize),
    m_timer(NULL),
    m_solverIterations(1)
{
    m_numGridCells = m_gridSize.x*m_gridSize.y*m_gridSize.z;
    float3 worldSize = make_float3(2.0f, 2.0f, 2.0f);

    m_gridSortBits = 18;    // increase this for larger grids

    // set simulation parameters
    m_params.gridSize = m_gridSize;
    m_params.numCells = m_numGridCells;
    m_params.numBodies = m_numParticles;

    int iloscTypow=typyCzastek.size();
    if(iloscTypow>0)
    {
        for(int i=0;i<iloscTypow;i++)
        {
            /**
            * promieñ w mikronach
            */
            m_params.particleRadius[i] = typyCzastek[i].particleRadius;
            m_params.particleMass[i]=typyCzastek[i].particleMass;
            m_params.normalizedCharge[i]=typyCzastek[i].particleCharge;
        }
        m_params.particleTypesNum=iloscTypow;
        for(int i=0;i<particleType::normalizedEpsilon.size();i++)
        {
            m_params.normalizeEpsilon[i]=particleType::normalizedEpsilon[i];
        }
    }
    else
    {
        m_params.particleRadius[0] = 0.225f;//1.0f / 32.0f;//32.0f / 64.0f;
        m_params.particleMass[0]=1.0f;
        m_params.particleTypesNum=1;
        m_params.normalizedCharge[0]=1.0f;
        m_params.normalizeEpsilon[0]=1.0f;
    }

    m_params.colliderPos = make_float3(-1.2f, -0.8f, 0.8f);
    m_params.colliderRadius = 0.2f;


    m_params.worldOrigin = make_float3(-1.0f, -1.0f, -1.0f);
    //    m_params.cellSize = make_float3(worldSize.x / m_gridSize.x, worldSize.y / m_gridSize.y, worldSize.z / m_gridSize.z);
	/**< żeby zachować strukturę składającą się z identycznych sześcianów trzeba wybrać największy typ cząstek */
	float maxRadius=0.0f;
	for(int i=0;i<m_params.particleTypesNum;i++)
	{
		if(maxRadius<=m_params.particleRadius[i])
		{
			maxRadius=m_params.particleRadius[i];
		}
	}
	float cellSize = maxRadius * 3.0f;  // cell size equal to particle diameter
    m_params.cellSize = make_float3(cellSize, cellSize, cellSize);

    m_params.spring = 0.5f;
    m_params.damping = 0.1f;
    m_params.shear = 0.1f;
    m_params.attraction = 0.1f;
    m_params.boundaryDamping = -1.0f;

    m_params.gravity = make_float3(0.0f, -0.0003f, 0.0f);
    m_params.globalDamping = 1.0f;

	m_params.bigradius=10.0f;//docelowo zmieny rozmiar zewnetrznej kuli
	m_params.bigradius0=10.0f;
	m_params.boundaries=true;
	m_params.epsi=0.1f;
	m_params.brown=0.00001f;
	m_params.brownQuality=10;


    _initialize(numParticles);
}

ParticleSystem::~ParticleSystem()
{
    _finalize();
    m_numParticles = 0;
}

uint
ParticleSystem::createVBO(uint size)
{
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vbo;
}

inline float lerp(float a, float b, float t)
{
    return a + t*(b-a);
}

// create a color ramp
void colorRamp(float t, float *r)
{
    const int ncolors = 7;
    float c[ncolors][3] =
    {
        { 1.0, 0.0, 0.0, },
        { 1.0, 0.5, 0.0, },
        { 1.0, 1.0, 0.0, },
        { 0.0, 1.0, 0.0, },
        { 0.0, 1.0, 1.0, },
        { 0.0, 0.0, 1.0, },
        { 1.0, 0.0, 1.0, },
    };
    t = t * (ncolors-1);
    int i = (int) t;
    float u = t - floor(t);
    r[0] = lerp(c[i][0], c[i+1][0], u);
    r[1] = lerp(c[i][1], c[i+1][1], u);
    r[2] = lerp(c[i][2], c[i+1][2], u);
}

void
ParticleSystem::_initialize(int numParticles)
{
    assert(!m_bInitialized);

    m_numParticles = numParticles;

    // allocate host storage
    m_hPos = new float[m_numParticles*4];
    m_hVel = new float[m_numParticles*4];

    memset(m_hPos, 0, m_numParticles*4*sizeof(float));
    memset(m_hVel, 0, m_numParticles*4*sizeof(float));

    m_hCellStart = new uint[m_numGridCells];
    memset(m_hCellStart, 0, m_numGridCells*sizeof(uint));

    m_hCellEnd = new uint[m_numGridCells];
    memset(m_hCellEnd, 0, m_numGridCells*sizeof(uint));

    // allocate GPU data
    unsigned int memSize = sizeof(float) * 4 * m_numParticles;

    if (m_bUseOpenGL)
    {
        m_posVbo = createVBO(memSize);
        registerGLBufferObject(m_posVbo, &m_cuda_posvbo_resource);
    }
    else
    {
        checkCudaErrors(cudaMalloc((void **)&m_cudaPosVBO, memSize)) ;
    }

    allocateArray((void **)&m_dVel, memSize);

    allocateArray((void **)&m_dSortedPos, memSize);
    allocateArray((void **)&m_dSortedVel, memSize);

    allocateArray((void **)&m_dGridParticleHash, m_numParticles*sizeof(uint));
    allocateArray((void **)&m_dGridParticleIndex, m_numParticles*sizeof(uint));

    allocateArray((void **)&m_dCellStart, m_numGridCells*sizeof(uint));
    allocateArray((void **)&m_dCellEnd, m_numGridCells*sizeof(uint));

    if (m_bUseOpenGL)
    {
        m_colorVBO = createVBO(m_numParticles*4*sizeof(float));
        registerGLBufferObject(m_colorVBO, &m_cuda_colorvbo_resource);

        // fill color buffer
        glBindBufferARB(GL_ARRAY_BUFFER, m_colorVBO);
        float *data = (float *) glMapBufferARB(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        float *ptr = data;

        for (uint i=0; i<m_numParticles; i++)
        {
            float t = i / (float) m_numParticles;
			if(multiColor)
			{
#if 0
            *ptr++ = rand() / (float) RAND_MAX;
            *ptr++ = rand() / (float) RAND_MAX;
            *ptr++ = rand() / (float) RAND_MAX;
#else
            colorRamp(t, ptr);
            ptr+=3;
#endif
			}
			else
			{
				*ptr++ = 0;
				*ptr++ = 1;
				*ptr++ = 0;
			}
            *ptr++ = 1.0f;
        }

        glUnmapBufferARB(GL_ARRAY_BUFFER);
    }
    else
    {
        checkCudaErrors(cudaMalloc((void **)&m_cudaColorVBO, sizeof(float)*numParticles*4));
    }

    sdkCreateTimer(&m_timer);

    setParameters(&m_params);

    m_bInitialized = true;
}

void
ParticleSystem::_finalize()
{
    assert(m_bInitialized);

    delete [] m_hPos;
    delete [] m_hVel;
    delete [] m_hCellStart;
    delete [] m_hCellEnd;

    freeArray(m_dVel);
    freeArray(m_dSortedPos);
    freeArray(m_dSortedVel);

    freeArray(m_dGridParticleHash);
    freeArray(m_dGridParticleIndex);
    freeArray(m_dCellStart);
    freeArray(m_dCellEnd);

    if (m_bUseOpenGL)
    {
        unregisterGLBufferObject(m_cuda_posvbo_resource);
        glDeleteBuffers(1, (const GLuint *)&m_posVbo);
        glDeleteBuffers(1, (const GLuint *)&m_colorVBO);
    }
    else
    {
        checkCudaErrors(cudaFree(m_cudaPosVBO));
        checkCudaErrors(cudaFree(m_cudaColorVBO));
    }
}

// step the simulation
void
ParticleSystem::update(float deltaTime)
{
    assert(m_bInitialized);

    float *dPos;

    if (m_bUseOpenGL)
    {
        dPos = (float *) mapGLBufferObject(&m_cuda_posvbo_resource);
    }
    else
    {
        dPos = (float *) m_cudaPosVBO;
    }

    // update constants
    setParameters(&m_params);

    // integrate
    integrateSystem(
        dPos,
        m_dVel,
        deltaTime,
        m_numParticles);

    // calculate grid hash
    calcHash(
        m_dGridParticleHash,
        m_dGridParticleIndex,
        dPos,
        m_numParticles);

    // sort particles based on hash
    sortParticles(m_dGridParticleHash, m_dGridParticleIndex, m_numParticles);

    // reorder particle arrays into sorted order and
    // find start and end of each cell
    reorderDataAndFindCellStart(
        m_dCellStart,
        m_dCellEnd,
        m_dSortedPos,
        m_dSortedVel,
        m_dGridParticleHash,
        m_dGridParticleIndex,
        dPos,
        m_dVel,
        m_numParticles,
        m_numGridCells);

    // process collisions
    collide(
        m_dVel,
        m_dSortedPos,
        m_dSortedVel,
        m_dGridParticleIndex,
        m_dCellStart,
        m_dCellEnd,
        m_numParticles,
        m_numGridCells,
		deltaTime);

    // note: do unmap at end here to avoid unnecessary graphics/CUDA context switch
    if (m_bUseOpenGL)
    {
        unmapGLBufferObject(m_cuda_posvbo_resource);
    }
}

void
ParticleSystem::dumpGrid()
{
    // dump grid information
    copyArrayFromDevice(m_hCellStart, m_dCellStart, 0, sizeof(uint)*m_numGridCells);
    copyArrayFromDevice(m_hCellEnd, m_dCellEnd, 0, sizeof(uint)*m_numGridCells);
    uint maxCellSize = 0;

    for (uint i=0; i<m_numGridCells; i++)
    {
        if (m_hCellStart[i] != 0xffffffff)
        {
            uint cellSize = m_hCellEnd[i] - m_hCellStart[i];

            //            printf("cell: %d, %d particles\n", i, cellSize);
            if (cellSize > maxCellSize)
            {
                maxCellSize = cellSize;
            }
        }
    }

    printf("maximum particles per cell = %d\n", maxCellSize);
}

void
ParticleSystem::dumpParticles(uint start, uint count)
{
    // debug
    copyArrayFromDevice(m_hPos, 0, &m_cuda_posvbo_resource, sizeof(float)*4*count);
    copyArrayFromDevice(m_hVel, m_dVel, 0, sizeof(float)*4*count);

    for (uint i=start; i<start+count; i++)
    {
        //        printf("%d: ", i);
        printf("pos: (%.4f, %.4f, %.4f, %.4f)\n", m_hPos[i*4+0], m_hPos[i*4+1], m_hPos[i*4+2], m_hPos[i*4+3]);
        printf("vel: (%.4f, %.4f, %.4f, %.4f)\n", m_hVel[i*4+0], m_hVel[i*4+1], m_hVel[i*4+2], m_hVel[i*4+3]);
    }
}

float *
ParticleSystem::getArray(ParticleArray array)
{
    assert(m_bInitialized);

    float *hdata = 0;
    float *ddata = 0;
    struct cudaGraphicsResource *cuda_vbo_resource = 0;

    switch (array)
    {
        default:
        case POSITION:
            hdata = m_hPos;
            ddata = m_dPos;
            cuda_vbo_resource = m_cuda_posvbo_resource;
            break;

        case VELOCITY:
            hdata = m_hVel;
            ddata = m_dVel;
            break;
    }

    copyArrayFromDevice(hdata, ddata, &cuda_vbo_resource, m_numParticles*4*sizeof(float));
    return hdata;
}

void
ParticleSystem::setArray(ParticleArray array, const float *data, int start, int count)
{
    assert(m_bInitialized);

    switch (array)
    {
        default:
        case POSITION:
            {
                if (m_bUseOpenGL)
                {
                    unregisterGLBufferObject(m_cuda_posvbo_resource);
                    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
                    glBufferSubData(GL_ARRAY_BUFFER, start*4*sizeof(float), count*4*sizeof(float), data);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    registerGLBufferObject(m_posVbo, &m_cuda_posvbo_resource);
                }
            }
            break;

        case VELOCITY:
            copyArrayToDevice(m_dVel, data, start*4*sizeof(float), count*4*sizeof(float));
            break;
    }
}

inline float frand()
{
    return rand() / (float) RAND_MAX;
}

void
ParticleSystem::initGrid(uint *size, float spacing, float jitter, uint numParticles)
{
    srand(1973);

    /*unsigned int tabOfTypesNumbers[10];
    unsigned int typeCounter=0;
    unsigned int nextType=0;
    nextType=typyCzastek[0].particleNoOfType;*/

    for (uint z=0; z<size[2]; z++)
    {
        for (uint y=0; y<size[1]; y++)
        {
            for (uint x=0; x<size[0]; x++)
            {
                uint i = (z*size[1]*size[0]) + (y*size[0]) + x;

                if (i < numParticles)
                {/**< losowanie typu */
					unsigned int tmpType=rand()%m_params.particleTypesNum;
					m_hPos[i*4] = (spacing * x) + m_params.particleRadius[tmpType] - 1.0f + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[i*4+1] = (spacing * y) + m_params.particleRadius[tmpType] - 1.0f + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[i*4+2] = (spacing * z) + m_params.particleRadius[tmpType] - 1.0f + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[i*4+3] = 1.0f;

                    m_hVel[i*4] = 0.0f;
                    m_hVel[i*4+1] = 0.0f;
                    m_hVel[i*4+2] = 0.0f;
                    if(typyCzastek.size()>0)
                    {
                        m_hVel[i*4+3]=tmpType;//rand()%typyCzastek.size();
                    }
                    else
                    {
                        m_hVel[i*4+3] = 0.0f;
                    }
                }
            }
        }
    }
}

/** \fn void ParticleSystem::reset(ParticleConfig config)
 * \brief rozłożenie cząstek
 * \param config ParticleConfig Typ wyliczeniowy. Wybór sposobu rozłożenia cząstek.
 * \return void
 */
void ParticleSystem::reset(ParticleConfig config)
{
/** \todo należy dodać właściwe losowanie typu cząstki i uwzględnić przy sprawdzaniu położenia
 * rozmiar wylosowanego typu oraz rozmiary 'sąsiadujących' cząstek tak, żeby ze sobą nie kolidowały
 */
	//unsigned int tmpType=rand()%m_params.particleTypesNum;

    unsigned int tmpType=0;
    unsigned int typeCounter=0;/**< ile cząstek już dodaliśmy */
    unsigned int nextType=0;/**< po ilu dodanych cząstkach zaczyna się następny typ */
    if(!typyCzastek.empty())
    {
        nextType=typyCzastek[0].particleNoOfType;
        typyCzastek[0].ofTypeParticleTrack.clear();
    }
    else
    {
        throw typyCzastek;
    }

    int p = 0, v = 0;
    float tmpbrad=m_params.bigradius-m_params.particleRadius[tmpType];
    float tmprad=tmpbrad/2;
    //printf("%f\n",tmprad);
    bool bo1=false;
    float point[3];
    int j4;
    float jitter;
    switch (config)
    {
        default:
        case CONFIG_RANDOM:
            {
                for (uint i=0; i < m_numParticles; /*i++*/)
                {
                    if(typeCounter>=nextType)
                    {
                        if(typyCzastek.size()<=(tmpType+1))
                        {
                            break;
                        }
                        nextType+=typyCzastek[++tmpType].particleNoOfType;
                        /**< \todo trzeba jeszcze uwzględnić rozmiar cząstki którą podejżewamy o przecinanie się z nową */
                        tmpbrad=m_params.bigradius-m_params.particleRadius[tmpType];
                        tmprad=tmpbrad/2;
                        typyCzastek[tmpType].ofTypeParticleTrack.clear();
                    }
                    //point[0] = frand();
                    //point[1] = frand();
                    //point[2] = frand();
					/*point[0]=tmpbrad*frand()-tmprad;//frand -> (0.0f,1.0f) 1.0f - promien kuli
					float my=sqrt(tmprad*tmprad-point[0]*point[0]);
					point[1]=rand() /( float ) RAND_MAX *( 2*my ) - my;
					float mz=sqrt(tmprad*tmprad-point[0]*point[0]-point[1]*point[1]);
					point[2]=rand() /( float ) RAND_MAX *( 2*mz ) - mz;*/

					point[0]=tmpbrad*frand()-tmprad;
					point[1]=tmpbrad*frand()-tmprad;
					point[2]=tmpbrad*frand()-tmprad;
                    /** \brief równomierne losowanie położeń metodą Monte Carlo
                     * \param point[0]*point[0]+point[1]*point[1]+point[2]*point[2]<=tmpbrad*tmpbrad x^2+y^2+z^2<=R^2
                     */
					if((point[0]*point[0]+point[1]*point[1]+point[2]*point[2])<(tmprad*tmprad))
                    {
                        bo1=true;
                        for(uint j=0;j<m_numParticles && j<i;j++)
                        {
							j4=4*j;
                            if( ( (m_hPos[j4]-point[0]*2)*(m_hPos[j4]-point[0]*2)+(m_hPos[j4+1]-point[1]*2)*(m_hPos[j4+1]-point[1]*2)+(m_hPos[j4+2]-point[2]*2)*(m_hPos[j4+2]-point[2]*2) )<4.0f*m_params.particleRadius[tmpType]*m_params.particleRadius[tmpType])
                            {
								//printf("%d %d %f %f %f\n",i ,j ,point[0],point[1],point[2]);
                                bo1=false;
                                break;
                            }
                        }
                        if(bo1)
						{
                            typeCounter++;

                            i++;
                            typyCzastek[tmpType].ofTypeParticleTrack.push_back(p);
                            m_hPos[p++] = 2 * (point[0] - 0.0f);
                            m_hPos[p++] = 2 * (point[1] - 0.0f);
                            m_hPos[p++] = 2 * (point[2] - 0.0f);
                            m_hPos[p++] = 1.0f; /**< 1 postaci znormalizowanej macierzy do trasformacji GL */
                            /*m_hVel[v++] = (rand() /( float ) RAND_MAX -0.5f)*m_params.brown;
                            m_hVel[v++] = (rand() /( float ) RAND_MAX -0.5f)*m_params.brown;;
                            m_hVel[v++] = (rand() /( float ) RAND_MAX -0.5f)*m_params.brown;;*/
                            m_hVel[v++] = 0.0f;
                            m_hVel[v++] = 0.0f;
                            m_hVel[v++] = 0.0f;
                            m_hVel[v++] = tmpType;
						}
                    }
                }
				/*for(int i=0;i<m_numParticles;i++)
				{
					printf("i: %d\n",i);
					for(int j=0;j<4;j++)
						printf("%f ",m_hPos[i*4+j]);
					printf("\n");
				}*/
            }
            break;

        case CONFIG_GRID:
            {
                jitter = m_params.particleRadius[tmpType]*0.01f;
                uint s = (int) ceilf(powf((float) m_numParticles, 1.0f / 3.0f));
                uint gridSize[3];
                gridSize[0] = gridSize[1] = gridSize[2] = s;
                initGrid(gridSize, m_params.particleRadius[tmpType]*2.0f, jitter, m_numParticles);
            }
            break;
    }

    setArray(POSITION, m_hPos, 0, m_numParticles);
    setArray(VELOCITY, m_hVel, 0, m_numParticles);
}

void
ParticleSystem::addSphere(int start, float *pos, float *vel, int r, float spacing)
{
	/** \todo tutaj też trzeba losować typ odpowiednio, ale można zastosować uproszczenia
	 * bo nie jest to domyślna metoda ustawiania cząstek w symulacji
	*/
	unsigned int tmpType=rand()%m_params.particleTypesNum;
    uint index = start;

    for (int z=-r; z<=r; z++)
    {
        for (int y=-r; y<=r; y++)
        {
            for (int x=-r; x<=r; x++)
            {
                float dx = x*spacing;
                float dy = y*spacing;
                float dz = z*spacing;
                float l = sqrtf(dx*dx + dy*dy + dz*dz);
                float jitter = m_params.particleRadius[tmpType]*0.01f;

                if ((l <= m_params.particleRadius[tmpType]*2.0f*r) && (index < m_numParticles))
                {
                    m_hPos[index*4]   = pos[0] + dx + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[index*4+1] = pos[1] + dy + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[index*4+2] = pos[2] + dz + (frand()*2.0f-1.0f)*jitter;
                    m_hPos[index*4+3] = pos[3];

                    m_hVel[index*4]   = vel[0];
                    m_hVel[index*4+1] = vel[1];
                    m_hVel[index*4+2] = vel[2];
                    m_hVel[index*4+3] = vel[3];
                    index++;
                }
            }
        }
    }

    setArray(POSITION, m_hPos, start, index);
    setArray(VELOCITY, m_hVel, start, index);
}


