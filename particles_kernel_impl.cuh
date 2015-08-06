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

/*
 * CUDA particle system kernel code.
 */
 /** \file particles_kernel_impl.cuh
  * \brief Implementacje (definicje) funkcji odpowiadających za fizykę.
  *
  */


#ifndef _PARTICLES_KERNEL_H_
#define _PARTICLES_KERNEL_H_

#include <stdio.h>
#include <math.h>
#include <float.h>
#include "helper_math.h"
#include "math_constants.h"
#include "particles_kernel.cuh"
#include <curand_kernel.h>/**< biblioteka zawierająca funkcje generujące liczby pseudolosowe i quasi-losowe na karcie */

#if USE_TEX
// textures for particle position and velocity
texture<float4, 1, cudaReadModeElementType> oldPosTex;
texture<float4, 1, cudaReadModeElementType> oldVelTex;

texture<uint, 1, cudaReadModeElementType> gridParticleHashTex;
texture<uint, 1, cudaReadModeElementType> cellStartTex;
texture<uint, 1, cudaReadModeElementType> cellEndTex;
#endif

// simulation parameters in constant memory
__constant__ SimParams params;

__device__ float surfacePreasure;

__device__ float globalDeltaTime;

__constant__ float* forcePtr;

__device__ static float atomicMin(float* address, float val)
{
    int* address_as_i = (int*) address;
    int old = *address_as_i, assumed;
    do {
        assumed = old;
        old = ::atomicCAS(address_as_i, assumed,
            __float_as_int(::fminf(val, __int_as_float(assumed))));
    } while (assumed != old);
    return __int_as_float(old);
}

/** \struct integrate_functor
 * \brief ta struktura inicjowana jest z krokiem delta_time dla danych
 * opisujących prędkość i położenie cząstki, te dane siedzą w wektorze thrust (w GPU)
 * jako para (tuple) float4 położenia i prędkości
 *
 */
struct integrate_functor
{
    float deltaTime;

    __host__ __device__
    integrate_functor(float delta_time) : deltaTime(delta_time) {}

    /** \brief nowe położenie
     * \param t para położenia i prędkości
     * wylicza nowe położenie i sprawdza czy położenie nie jest za brzegiem
     */
    template <typename Tuple>
    __device__
    void operator()(Tuple t)
    {
        volatile float4 posData = thrust::get<0>(t);
        volatile float4 velData = thrust::get<1>(t);
        volatile float4 forceData=thrust::get<2>(t);
        float3 pos = make_float3(posData.x, posData.y, posData.z);
        float3 vel = make_float3(velData.x, velData.y, velData.z);
        float3 force = make_float3(forceData.x, forceData.y, forceData.z);

		if(params.brown!=0.0f)/**< ruchy Browna */
        {
			unsigned int seed=threadIdx.x+(((gridDim.x*blockIdx.y)+blockIdx.x)*blockDim.x)+(unsigned int)floor((vel.x+vel.y+vel.z)*params.brownQuality);
			curandState s;
			curand_init(seed,0,0,&s);
			skipahead((unsigned long long int)floor(vel.z*params.brownQuality+pos.x+pos.y+pos.z),&s);
			vel.x+=(curand_normal(&s))*params.brown;
			skipahead((unsigned long long int)floor(vel.y*params.brownQuality),&s);
			vel.y+=(curand_normal(&s))*params.brown;
			skipahead((unsigned long long int)floor(vel.x*params.brownQuality),&s);
			vel.z+=(curand_normal(&s))*params.brown;
		}



        // set this to zero to disable collisions with cube sides
#if 0

        if (pos.x > 1.0f - params.particleRadius[(int)velData.w])
        {
            pos.x = 1.0f - params.particleRadius[(int)velData.w];
            vel.x *= params.boundaryDamping;
        }

        if (pos.x < -1.0f + params.particleRadius[(int)velData.w])
        {
            pos.x = -1.0f + params.particleRadius[(int)velData.w];
            vel.x *= params.boundaryDamping;
        }

        if (pos.y > 1.0f - params.particleRadius[(int)velData.w])
        {
            pos.y = 1.0f - params.particleRadius[(int)velData.w];
            vel.y *= params.boundaryDamping;
        }

        if (pos.z > 1.0f - params.particleRadius[(int)velData.w])
        {
            pos.z = 1.0f - params.particleRadius[(int)velData.w];
            vel.z *= params.boundaryDamping;
        }

        if (pos.z < -1.0f + params.particleRadius[(int)velData.w])
        {
            pos.z = -1.0f + params.particleRadius[(int)velData.w];
            vel.z *= params.boundaryDamping;
        }

#endif

		//odbijanie sie od kuli
		/**< odbijanie sie od kuli vel napiecie powierzchniowe */
#if 1
		float xk=pos.x*pos.x;
		float yk=pos.y*pos.y;
		float zk=pos.z*pos.z;
		float r0k=xk+yk+zk;
		float R=params.bigradius;//- params.particleRadius;
		float r0=sqrt(r0k);
		if(params.boundaries && r0>R-params.particleRadius[(int)velData.w] && r0<R+params.particleRadius[(int)velData.w])
		{
            float3 relPos = pos;
            float dist = length(relPos);
            float3 norm = relPos / dist;

            float3 relVel=make_float3(vel)*norm;
            float relVelS=sqrt(relVel.x*relVel.x+relVel.y*relVel.y+relVel.z*relVel.z);
            float DtMax=0.1f*params.particleRadius[(int)velData.w]/relVelS;
            if(DtMax<0.00001f)
            {
                DtMax=0.00001f;
            }
            atomicMin(&globalDeltaTime,DtMax);

			vel+=vel*norm*0.1f*params.globalDamping;/**< na powierchni zmniejszone tłumienie w kierunku radialnym */

			force-=params.boundaryDamping*(abs(r0-R)-(params.particleRadius[(int)velData.w]));/**< siła napięcia powierzchniowego */

		}
		__syncthreads();
        if(params.calcSurfacePreasure && params.boundaries && r0>R-params.particleRadius[(int)velData.w] && r0<R+params.particleRadius[(int)velData.w])
        {
            float momentum=forceF1*globalDeltaTime;/**< pęd */
            momentum=abs(momentum);
            atomicAdd(&surfacePreasure,momentum);
        }
#endif

//dolna plaszczyzna

#if 1
		if (pos.y < -2.0f*params.bigradius0 + params.particleRadius[(int)velData.w])/**< blat */
        {
            pos.y = -2.0f*params.bigradius0 + params.particleRadius[(int)velData.w];
            vel.y = 0;
        }
#endif
		vel += params.gravity * globalDeltaTime;/**< grawitacja */
		vel *= params.globalDamping;/**< lepkosc */
        vel += force*(globalDeltaTime/params.particleMass[(int)vel.w]);/**< siły oddziaływań między cząsteczkami */
        // new position = old position + velocity * deltaTime
        pos += vel * deltaTime;/**< przemieszczenie */

        // store new position and velocity
        thrust::get<0>(t) = make_float4(pos, posData.w);
        thrust::get<1>(t) = make_float4(vel, velData.w);
        thrust::get<2>(t) = make_float4(0.0f);/**< zerowanie tablicy sił na koniec kroku całkowania */
    }
};

// calculate position in uniform grid
/** \brief calculate position in uniform grid
 *
 * \param p float3
 * \return __device__ int3
 *
 */
__device__ int3 calcGridPos(float3 p)
{
    int3 gridPos;
    gridPos.x = floor((p.x - params.worldOrigin.x) / params.cellSize.x);
    gridPos.y = floor((p.y - params.worldOrigin.y) / params.cellSize.y);
    gridPos.z = floor((p.z - params.worldOrigin.z) / params.cellSize.z);
    return gridPos;
}

// calculate address in grid from position (clamping to edges)
/** \brief calculate address in grid from position (clamping to edges)
 *
 * \param gridPos int3
 * \return __device__ uint
 *
 */
__device__ uint calcGridHash(int3 gridPos)
{
    gridPos.x = gridPos.x & (params.gridSize.x-1);  // wrap grid, assumes size is power of 2
    gridPos.y = gridPos.y & (params.gridSize.y-1);
    gridPos.z = gridPos.z & (params.gridSize.z-1);
    return __umul24(__umul24(gridPos.z, params.gridSize.y), params.gridSize.x) + __umul24(gridPos.y, params.gridSize.x) + gridPos.x;
}

// calculate grid hash value for each particle
/** \brief calculate grid hash value for each particle
 *
 * \param gridParticleHash uint* output
 * \param *gridParticleIndex uint output
 * \param *pos float4 input: positions
 * \param  uint    numParticles
 * \return __global__ void
 *
 */
__global__
void calcHashD(uint   *gridParticleHash,  // output
               uint   *gridParticleIndex, // output
               float4 *pos,               // input: positions
               uint    numParticles)
{
    uint index = __umul24(blockIdx.x, blockDim.x) + threadIdx.x;

    if (index >= numParticles) return;

    volatile float4 p = pos[index];

    // get address in grid
    int3 gridPos = calcGridPos(make_float3(p.x, p.y, p.z));
    uint hash = calcGridHash(gridPos);

    // store grid hash and particle index
    gridParticleHash[index] = hash;
    gridParticleIndex[index] = index;
}

// rearrange particle data into sorted order, and find the start of each cell
// in the sorted hash array
/** \brief rearrange particle data into sorted order, and find the start of each cell in the sorted hash array
 *
 * \param cellStart uint* output: cell start index
 * \param uint   *cellEnd output: cell end index
 * \param float4 *sortedPos output: sorted positions
 * \param float4 *sortedVel output: sorted velocities
 * \param uint   *gridParticleHash input: sorted grid hashes
 * \param uint   *gridParticleIndex input: sorted particle indices
 * \param float4 *oldPos input: sorted position array
 * \param float4 *oldVel input: sorted velocity array
 * \param uint    numParticles
 * \return __global__ void
 *
 *
 */
__global__
void reorderDataAndFindCellStartD(uint   *cellStart,        // output: cell start index
                                  uint   *cellEnd,          // output: cell end index
                                  float4 *sortedPos,        // output: sorted positions
                                  float4 *sortedVel,        // output: sorted velocities
                                  uint   *gridParticleHash, // input: sorted grid hashes
                                  uint   *gridParticleIndex,// input: sorted particle indices
                                  float4 *oldPos,           // input: sorted position array
                                  float4 *oldVel,           // input: sorted velocity array
                                  uint    numParticles)
{
    extern __shared__ uint sharedHash[];    // blockSize + 1 elements
    uint index = __umul24(blockIdx.x,blockDim.x) + threadIdx.x;

    uint hash;

    // handle case when no. of particles not multiple of block size
    if (index < numParticles)
    {
        hash = gridParticleHash[index];

        // Load hash data into shared memory so that we can look
        // at neighboring particle's hash value without loading
        // two hash values per thread
        sharedHash[threadIdx.x+1] = hash;

        if (index > 0 && threadIdx.x == 0)
        {
            // first thread in block must load neighbor particle hash
            sharedHash[0] = gridParticleHash[index-1];
        }
    }

    __syncthreads();

    if (index < numParticles)
    {
        // If this particle has a different cell index to the previous
        // particle then it must be the first particle in the cell,
        // so store the index of this particle in the cell.
        // As it isn't the first particle, it must also be the cell end of
        // the previous particle's cell

        if (index == 0 || hash != sharedHash[threadIdx.x])
        {
            cellStart[hash] = index;

            if (index > 0)
                cellEnd[sharedHash[threadIdx.x]] = index;
        }

        if (index == numParticles - 1)
        {
            cellEnd[hash] = index + 1;
        }

        // Now use the sorted index to reorder the pos and vel data
        uint sortedIndex = gridParticleIndex[index];
        float4 pos = FETCH(oldPos, sortedIndex);       // macro does either global read or texture fetch
        float4 vel = FETCH(oldVel, sortedIndex);       // see particles_kernel.cuh

        sortedPos[index] = pos;
        sortedVel[index] = vel;
    }


}

// collide two spheres using DEM method
/** \brief collide two spheres using DEM method
 *
 * \param posA float3
 * \param posB float3
 * \param velA float3
 * \param velB float3
 * \param radiusA float
 * \param radiusB float
 * \param attraction float
 * \return __device__ float3
 *
 */
__device__
float3 collideSpheres(float3 posA, float3 posB,
                      float4 velA, float4 velB,
                      float radiusA, float radiusB,
                      float attraction)
{
    // calculate relative position
    float3 relPos = posB - posA;

    float dist = length(relPos);
    float collideDist = (radiusA + radiusB)*1.5f;//zasieg dzialania sil

    float3 force = make_float3(0.0f);

    if (dist < collideDist)
    {
        float3 norm = relPos / dist;

		float sigma=(params.particleRadius[(int)velA.w]+params.particleRadius[(int)velB.w])/(1.12246204f);
		float sd=sigma/dist;
		sd*=sd*sd*sd*sd*sd;
		float q1q2=params.normalizedCharge[(int)velA.w]*params.normalizedCharge[(int)velB.w];
		float a=!(velB.w<velA.w)?velA.w:velB.w;/**< min(x,y) */
		float b= (velA.w<velB.w)?velB.w:velA.w - a;/**< max(x,y)-min(x,y) */
		int epsilonIndex=(int)floor(a*(params.particleTypesNum-((a-1.0f)/2.0f))+b+0.5f);/**< wzór na index z zabezpieczeniem przeciwko niedokładności działań na float'ach */
		float epsilon=params.epsi*params.normalizeEpsilon[epsilonIndex];
		float foreScalar=48.0f*epsilon/dist*sd*(sd-0.5f)+attraction*q1q2/(dist*dist);
		force=-foreScalar*norm; //jest dobrze :-) Uwaga na kierunek wektora normalnego
/////////////////////////////////////////////////////////////////////////////////
/*	tu wpisywac rownania na sily dla czastek bedacywch w zasiegu	*/
/////////////////////////////////////////////////////////////////////////////////
		float3 relVel=make_float3(velB)*norm-make_float3(velA)*norm;
		float relVelS=sqrt(relVel.x*relVel.x+relVel.y*relVel.y+relVel.z*relVel.z);
		float DtMax=0.1f*dist/relVelS;
        if(DtMax<0.00001f)
        {
            DtMax=0.00001f;
        }
		atomicMin(&globalDeltaTime,DtMax);
	}

    return force;
}



// collide a particle against all other particles in a given cell
/** \brief collide a particle against all other particles in a given cell
 *
 * \param gridPos int3
 * \param index uint
 * \param pos float3
 * \param vel float3
 * \param oldPos float4*
 * \param oldVel float4*
 * \param cellStart uint*
 * \param cellEnd uint*
 * \return __device__ float3
 *
 */
__device__
float3 collideCell(int3    gridPos,
                   uint    index,
                   float3  pos,
                   float4  vel,
                   float4 *oldPos,
                   float4 *oldVel,
                   uint   *cellStart,
                   uint   *cellEnd)
{
    uint gridHash = calcGridHash(gridPos);

    // get start of bucket for this cell
    uint startIndex = FETCH(cellStart, gridHash);

    float3 force = make_float3(0.0f);

    if (startIndex != 0xffffffff)          // cell is not empty
    {
        // iterate over particles in this cell
        uint endIndex = FETCH(cellEnd, gridHash);

        for (uint j=startIndex; j<endIndex; j++)
        {
            if (j != index)                // check not colliding with self
            {
                float3 pos2 = make_float3(FETCH(oldPos, j));
                //float3 vel2 = make_float3(FETCH(oldVel, j));
                float4 vel2 = FETCH(oldVel, j);

                // collide two spheres
                force += collideSpheres(pos, pos2, vel, vel2, params.particleRadius[(int)vel.w], params.particleRadius[(int)vel2.w], params.attraction);
            }
        }
    }

    return force;
}

/** \brief wyliczenie wypadkowej siły zderzeń jednej cząstki ze wszystkimi w zasięgu
 *
 * \param newVel float4* output: new velocit
 * \param float4 *oldPos input: sorted positions
 * \param float4 *oldVel input: sorted velocities
 * \param uint   *gridParticleIndex input: sorted particle indices
 * \param uint   *cellStart
 * \param cellEnd uint*
 * \param numParticles uint
 * \return __global__ void
 *
 */
__global__
void collideD(float4 *newVel,               // output: new velocity
              float4 *oldPos,               // input: sorted positions
              float4 *oldVel,               // input: sorted velocities
              uint   *gridParticleIndex,    // input: sorted particle indices
              uint   *cellStart,
              uint   *cellEnd,
              uint    numParticles,
			  float deltaTime)
{
    uint index = __mul24(blockIdx.x,blockDim.x) + threadIdx.x;

    if (index >= numParticles) return;

    // read particle data from sorted arrays
    float3 pos = make_float3(FETCH(oldPos, index));
    //float3 vel = make_float3(FETCH(oldVel, index));
    float4 vel = FETCH(oldVel, index);

    // get address in grid
    int3 gridPos = calcGridPos(pos);

    // examine neighbouring cells
    float3 force = make_float3(0.0f);

    for (int z=-1; z<=1; z++)
    {
        for (int y=-1; y<=1; y++)
        {
            for (int x=-1; x<=1; x++)
            {
                int3 neighbourPos = gridPos + make_int3(x, y, z);
                force += collideCell(neighbourPos, index, pos, vel, oldPos, oldVel, cellStart, cellEnd);
            }
        }
    }

    // write new velocity back to original unsorted location
    uint originalIndex = gridParticleIndex[index];
    //__syncthreads();/**< wyrównanie globalDeltaTime */
    //newVel[originalIndex] = make_float4(make_float3(vel) + force*(globalDeltaTime/params.particleMass[(int)vel.w]), vel.w);
    forcePtr[originalIndex]=make_float4(force, 0.0f);
}

#endif
