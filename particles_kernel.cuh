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
  * \brief Definicja struktury przechowującej parametry symulacji.
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
 */
struct SimParams
{
    float3 colliderPos;
    float  colliderRadius;

    float3 gravity;
    double globalDamping;

	double particleRadius[MAX_TYPE_NUM];

	double particleMass[MAX_TYPE_NUM];

	double normalizedCharge[MAX_TYPE_NUM];

	double normalizeEpsilon[MAX_EPSI_NUM];

    uint3 gridSize;
    uint numCells;
    float3 worldOrigin;
    float3 cellSize;

    uint numBodies;
    uint maxParticlesPerCell;

	double spring;
	double damping;
	double shear;
	double attraction;/**< mnożnik sił Coulombowskich */
	double boundaryDamping;

	double bigradius;/**< promien duzej kuli w mikrometrach */
	double bigradius0;
	double epsi;/**< mnożnik epsilonów */
	bool boundaries;
	double brown;

	int particleTypesNum;
	unsigned long long int brownQuality;

	bool calcSurfacePreasure;

	double surfaceVel;/**< r0/(t+dt)-A/sqrt(t+dt) */
	double viscosity;/**< 0,89 mPa·s */

	bool autoDt;
};

#endif
