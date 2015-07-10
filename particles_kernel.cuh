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
<<<<<<< HEAD
  /** \file particles_kernel.cuh
  * \brief Definicja struktury przechowuj¹cej parametry symulacji.
  *
  */

=======
>>>>>>> master

#ifndef PARTICLES_KERNEL_H
#define PARTICLES_KERNEL_H

#define USE_TEX 0

#if USE_TEX
#define FETCH(t, i) tex1Dfetch(t##Tex, i)
#else
#define FETCH(t, i) t[i]
#endif

#include "vector_types.h"
typedef unsigned int uint;

<<<<<<< HEAD

=======
>>>>>>> master
// simulation parameters
struct SimParams
{
    float3 colliderPos;
    float  colliderRadius;

    float3 gravity;
    float globalDamping;
    float particleRadius;

<<<<<<< HEAD
	float particleMass;

=======
>>>>>>> master
    uint3 gridSize;
    uint numCells;
    float3 worldOrigin;
    float3 cellSize;

    uint numBodies;
    uint maxParticlesPerCell;

    float spring;
    float damping;
    float shear;
    float attraction;
    float boundaryDamping;

	float bigradius;
<<<<<<< HEAD
	float bigradius0;
	bool boundaries;
	float epsi;
	float brown;

	int particleTypesNum;
	unsigned long long int brownQuality;
=======
>>>>>>> master
};

#endif
