#ifndef PARTICLE_H
#define PARTICLE_H

#include "Vector3.h"

enum ParticleObject_TYPE
{
	P_SNOW = 0, 
	P_TOTAL
};

class ParticleObject
{
public:
	ParticleObject(ParticleObject_TYPE = P_SNOW);
	~ParticleObject(void);

	ParticleObject_TYPE type;

	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	float rotation;
	float rotationSpeed;
	bool isActive;
};

#endif