// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2013-2015 NVIDIA Corporation. All rights reserved.


#ifndef FLEX_EXT_H
#define FLEX_EXT_H

//! \cond HIDDEN_SYMBOLS
#if _WIN32
#define FLEX_API __declspec(dllexport)
#else
#define FLEX_API
#endif

//! \endcond

extern "C" {

/** 
 * Represents a group of particles and constraints, each asset 
 * can be instanced into a simulation using flexExtCreateInstance
 */
struct FlexExtAsset
{	
	// particles
	float* mParticles;				//!< Local space particle positions, x,y,z,1/mass
	int mNumParticles;				//!< Number of particles

	// springs
	int* mSpringIndices;			//!< Spring indices
	float* mSpringCoefficients;		//!< Spring coefficients
	float* mSpringRestLengths;		//!< Spring rest-lengths
	int mNumSprings;				//!< Number of springs

	// rigid body stiffness
	float mRigidStiffness;			//!< Rigid body constraint stiffness		
	float mRigidCenter[3];			//!< Local space center of mass of the particles

	// faces for cloth
	int* mTriangleIndices;			//!< Indexed triangle mesh for clothing
	int mNumTriangles;				//!< Number of triangles

	// inflatable params
	bool mInflatable;				//!< Whether an inflatable constraint should be added
	float mInflatableVolume;		//!< The rest volume for the inflatable constraint
	float mInflatablePressure;		//!< How much over the rest volume the inflatable should attempt to maintain
	float mInflatableStiffness;		//!< How stiff the inflatable is
};

/** 
 * Represents an instance of a FlexAsset in a container
 */
struct FlexExtInstance
{
	int* mParticleIndices;			//!< Simulation particle indices
	int mNumParticles;				//!< Number of simulation particles
	
	int mTriangleIndex;				//!< Index in the container's triangle array
	int mRigidIndex;				//!< Index in the container's rigid body constraints array
	int mInflatableIndex;			//!< Index in the container's inflatables array
	
	const FlexExtAsset* mAsset;		//!< Source asset used to create this instance
	
	void* mUserData;				//!< User data pointer
};

/** 
 * Controls the way that force fields affect particles
 */
enum FlexForceExtMode
{
	//! Apply field value as a force. 
    eFlexExtModeForce			=      0,

	//! Apply field value as an impulse. 
    eFlexExtModeImpulse			=      1,

	//! Apply field value as a velocity change. 
    eFlexExtModeVelocityChange	=      2,
};

/** 
 * Force field data, currently just supports radial fields
 */
struct FlexExtForceField
{
	float mPosition[3];		//!< Center of force field
	float mRadius;			//!< Radius of the force field
	float mStrength;		//!< Strength of the force field
	FlexForceExtMode mMode;		//!< Mode of field application
	bool mLinearFalloff;	//!< Linear or no falloff 
};

/** 
 * Opaque type representing a simulation
 */
struct FlexExtContainer;

/**
 * Create an index buffer of unique vertices in the mesh 
 *
 * @param[in] vertices A pointer to an array of float3 positions
 * @param[in] numVertices The number of vertices in the mesh
 * @param[out] uniqueVerts A list of unique mesh vertex indices, should be numVertices in length (worst case all verts are unique)
 * @param[out] originalToUniqueMap Mapping from the original vertex index to the unique vertex index, should be numVertices in length
 * @param[in] threshold The distance below which two vertices are considered duplicates
 * @return The number of unique vertices in the mesh
 */
FLEX_API int flexExtCreateWeldedMeshIndices(const float* vertices, int numVertices, int* uniqueVerts, int* originalToUniqueMap, float threshold);

/**
 * Create a cloth asset consisting of stretch and bend distance constraints given an indexed triangle mesh. Stretch constraints will be placed along
 * triangle edges, while bending constraints are placed over two edges.
 *
 * @param[in] particles Positions and masses of the particles in the format [x, y, z, 1/m]
 * @param[in] numVertices The number of particles
 * @param[in] indices The triangle indices, these should be 'welded' using flexExtCreateWeldedMeshIndices() first
 * @param[in] numTriangles The number of triangles
 * @param[in] stretchStiffness The stiffness coefficient for stretch constraints
 * @param[in] bendStiffness The stiffness coefficient used for bending constraints
 * @param[in] tetherStiffness If > 0.0f then the function will create tether's attached to particles with zero inverse mass. These are unilateral, long-range attachments, which can greatly reduce stretching even at low iteration counts.
 * @param[in] tetherGive Because tether constraints are so effective at reducing stiffness, it can be useful to allow a small amount of extension before the constraint activates.
 * @param[in] pressure If > 0.0f then a volume (pressure) constraint will also be added to the asset, the rest volume and stiffness will be automatically computed by this function
 * @return A pointer to an asset structure holding the particles and constraints
 */
FLEX_API FlexExtAsset* flexExtCreateClothFromMesh(const float* particles, int numVertices, const int* indices, int numTriangles, float stretchStiffness, float bendStiffness, float tetherStiffness, float tetherGive, float pressure);

/**
 * Create a rigid body asset from a closed triangle mesh. The mesh is first voxelized at a spacing specified by the radius, and particles are placed at occupied voxels.
 *
 * @param[in] vertices Vertices of the triangle mesh
 * @param[in] numVertices The number of vertices
 * @param[in] indices The triangle indices
 * @param[in] numTriangleIndices The number of triangles indices (triangles*3)
 * @param[in] radius The spacing used for voxelization, note that the number of voxels grows proportional to the inverse cube of radius, currently this method limits construction to resolutions < 64^3
 * @return A pointer to an asset structure holding the particles and constraints
 */
FLEX_API FlexExtAsset* flexExtCreateRigidFromMesh(const float* vertices, int numVertices, const int* indices, int numTriangleIndices, float radius);

/**
 * Frees all memory associated with an asset created by one of the creation methods
 * param[in] asset The asset to destroy.
 */
FLEX_API void flexExtDestroyAsset(FlexExtAsset* asset);

/**
 * Creates a wrapper object around a Flex solver that can hold assets / instances
 * @param[in] solver The solver to wrap
 * @param[in] maxParticles The maximum number of particles to manage
 * @return A pointer to the new container
 */
FLEX_API FlexExtContainer* flexExtCreateContainer(FlexSolver* solver, int maxParticles);

/**
 * Frees all memory associated with a container
 * @param[in] container The container to destroy
 */
FLEX_API void flexExtDestroyContainer(FlexExtContainer* container);

/**
 * Allocates particles in the container.
 * @param[in] container The container to allocate out of
 * @param[in] n The number of particles to allocate
 * @param[out] indices An n-length array of ints that will store the indices to the allocated particles
 */
FLEX_API int  flexExtAllocParticles(FlexExtContainer* container, int n, int* indices);

/**
 * Free allocated particles
 * @param[in] container The container to free from
 * @param[in] n The number of particles to free
 * @param[in] indices The indices of the particles to free
 */
FLEX_API void flexExtFreeParticles(FlexExtContainer* container, int n, const int* indices);

/**
 * Retrives the indices of all active particles
 * @param[in] container The container to free from
 * @param[out] indices Returns the number of active particles
 * @return The number of active particles
 */
FLEX_API int flexExtGetActiveList(FlexExtContainer* container, int* indices);

/**
 * Creates an instance of an asset, the container will internally store a reference to the asset so it should remain valid for the instance lifetime. This
 * method will allocate particles for the asset, assign their initial positions, velocity and phase.
 *
 * @param[in] container The container to spawn into
 * @param[in] asset The asset to be spawned
 * @param[in] transform A pointer to a 4x4 column major, column vector transform that specifies the initial world space configuration of the particles
 * @param[in] vx The velocity of the particles along the x axis
 * @param[in] vy The velocity of the particles along the y axis
 * @param[in] vz The velocity of the particles along the z axis
 * @param[in] phase The phase used for the particles
 * @param[in] invMassScale A factor applied to the per particle inverse mass
 * @return A pointer to the instance of the asset
 */
FLEX_API FlexExtInstance* flexExtCreateInstance(FlexExtContainer* container, const FlexExtAsset* asset, const float* transform, float vx, float vy, float vz, int phase, float invMassScale);

/** Destoy an instance of an asset
 *
 * @param[in] container The container the instance belongs to
 * @param[in] instance The instance to destroy
 */
FLEX_API void flexExtDestroyInstance(FlexExtContainer* container, const FlexExtInstance* instance);

/** 
 * Returns pointers to the internal data stored by the container. These are host-memory pointers, and will 
 * remain valid until the container is destroyed. They can be used to read and write particle data, but *only*
 * after suitable synchronization. See flexExtTickContainer() for details.
 *
  @param container The container whose data should be accessed
  @param particles Receives a pointer to the particle position / mass data
  @param velocities Receives a pointer to the particle velocity data
  @param phases Receives a pointer to the particle phase data
  @param normals Receives a pointer to the particle normal data with 16 byte stride in format [nx, ny, nz, nw]
 */
FLEX_API void flexExtGetParticleData(FlexExtContainer* container, float** particles, float** velocities, int** phases, float** normals);

/** 
 * Access triangle constraint data, see flexExtGetParticleData() for notes on ownership.
 * @param container The container to retrive from
 * @param indices Receives a pointer to the array of triangle index data
 * @param normals Receives a pointer to an array of triangle normal data stored with 16 byte stride, i.e.: [nx, ny, nz]
 */
FLEX_API void flexExtGetTriangleData(FlexExtContainer* container, int** indices, float** normals);

/** 
 * Access rigid body constraint data, see flexExtGetParticleData() for notes on ownership.
 * @param container The container to retrive from
 * @param rotations Receives a pointer to the array 3x3 rotation matrix data
 * @param positions Receives a pointer to an array of rigid body translations in [x, y, z] format
 */
FLEX_API void flexExtGetRigidData(FlexExtContainer* container, float** rotations, float** positions);

/**
 * Updates the container, applies force fields, steps the solver forward in time, updates the host with the results synchronously.
 * This is a helper function which performs a synchronous update using the following flow.
 *
    \code{.c}
		// async update GPU data
		flexExtPushToDevice(container);
		flexExtApplyForceFields(container):

		// update solver
		flexUpdateSolver(container, dt, iterations);

		// async read data back to CPU
		flexExtPullFromDevice(container);

		// insert and wait on fence
		flexSetFence();
		flexWaitFence();

		// read / write particle data on CPU
		flexExtGetParticleData(container, particles, velocities, phases, normals);

  \endcode
  @param[in] container The container to update
  @param[in] dt The time-step in seconds
  @param[in] numSubsteps The number of substeps to perform
  @param[in] timers Pointer to a Flex profiling structure, see flexUpdateSolver()
 */
FLEX_API void flexExtTickContainer(FlexExtContainer* container, float dt, int numSubsteps, FlexTimers* timers=NULL);

/**
 * Updates the device asynchronously, transfers any particle and constraint changes to the flex solver, 
 * expected to be called in the following sequence: flexExtPushToDevice, [flexExtApplyForceFields,] flexUpdateSolver, flexExtPullFromDevice, flexSynchronize
 * @param[in] container The container to update
 */
FLEX_API void flexExtPushToDevice(FlexExtContainer* container);

/**
 * Updates the host asynchronously, transfers particle and constraint data back to he host, 
 * expected to be called in the following sequence: flexExtPushToDevice, [flexExtApplyForceFields,] flexUpdateSolver, flexExtPullFromDevice, flexSynchronize
 * @param[in] container The container to update
 */ 
FLEX_API void flexExtPullFromDevice(FlexExtContainer* container);

/*
 * Set force fields on the container
 * @param[in] container The container to update
 * @param[in] forceFields A pointer to an array of force field data, may be host or GPU memory
 * @param[in] numForceFields The number of force fields to send to the device
 * @param[in] source The memory space to copy the force fields from
 */
FLEX_API void flexExtSetForceFields(FlexExtContainer* container, const FlexExtForceField* forceFields, int numForceFields, FlexMemory source);

/*
 * Evaluate force fields and update particle velocities
 * expected to be called in the following sequence: flexExtPushToDevice, flexExtApplyForceFields, flexUpdateSolver, flexExtPullFromDevice, flexSynchronize
 * @param[in] container The container to update
 * @param[in] dt The time-step in seconds
 */
FLEX_API void flexExtApplyForceFields(FlexExtContainer* container, float dt);

} // extern "C"

#endif // FLEX_EXT_H

