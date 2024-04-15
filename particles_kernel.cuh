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
  /** \file particles_kernel.cuh
  * \brief Definicja struktury przechowującej parametry symulacji. - (EN) Definition 
  * of a structure storing simulation parameters.
  *
  */


#ifndef PARTICLES_KERNEL_H
#define PARTICLES_KERNEL_H

#define USE_TEX 0

#if USE_TEX
#define FETCH(t, i) tex1Dfetch(t##Tex, i)
#else
#define FETCH(t, i) t[i]
#endif

#include "vector_types.h"

#define MAX_TYPE_NUM 5
#define MAX_EPSI_NUM 15

typedef unsigned int uint;

// simulation parameters
/** \brief Struktura z parametrami symulacji przekazywanymi z host do device.
 * - (EN) Structure with simulation parameters passed from host to device.
 */
struct SimParams
{
    float3 colliderPos;
    float  colliderRadius;

    float3 gravity;
    float globalDamping;
	float rotation;/**< droplet (random) rotation [Hz] */

    float particleRadius[MAX_TYPE_NUM];

	float particleMass[MAX_TYPE_NUM];

	float normalizedCharge[MAX_TYPE_NUM];

	float normalizeEpsilon[MAX_EPSI_NUM];
	
	float surfaceTensionFactor[MAX_TYPE_NUM];

    uint3 gridSize;
    uint numCells;
    float3 worldOrigin;
    float3 cellSize;

    uint numBodies;
    uint maxParticlesPerCell;

    float spring;
    float damping;
    float shear;
    float attraction;/**< mnożnik sił Coulombowskich - (EN) Coulomb force multiplier */
    float boundaryDamping;

	float bigradius;/**< promien dużej kuli w mikrometrach - (EN) radius of the large sphere in micrometres */
	float bigradius0;
	float epsi;/**< mnożnik epsilonów - (EN) the multiplier of epsilons */
	bool boundaries;
	float brown;

	int particleTypesNum;
	unsigned long long int brownQuality;

	bool calcSurfacePreasure;

	float surfaceVel;/**< r0/(t+dt)-A/sqrt(t+dt) popraw - (EN) correct */
	float viscosity;/**< 0,89 mPa·s sprawdź - (EN) check */

	bool autoDt;
};

#endif
