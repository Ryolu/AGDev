#pragma once
#include "WeaponInfo.h"

class CThrow :
	public CWeaponInfo
{
public:
	CThrow();
	virtual ~CThrow();

	// Initialise this instance to default values
	void Init(void);
	// Discharge this weapon
	void Discharge(Vector3 position, Vector3 target, CPlayerInfo* _source, float multiplier);
};