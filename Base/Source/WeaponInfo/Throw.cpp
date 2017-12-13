#include "Throw.h"
#include "../Projectile/Present.h"

CThrow::CThrow()
{
}

CThrow::~CThrow()
{
}

void CThrow::Init(void)
{
	// Call the parent's Init method
	CWeaponInfo::Init();

	// The number of ammunition in a magazine for this weapon
	magRounds = 50;
	// The maximum number of ammunition for this magazine for this weapon
	maxMagRounds = 50;
	// The current total number of rounds currently carried by this player
	totalRounds = 50;
	// The max total number of rounds currently carried by this player
	maxTotalRounds = 50;

	// The time between shots
	timeBetweenShots = 0.1;
	// The elapsed time (between shots)
	elapsedTime = 0.0;
	// Boolean flag to indicate if weapon can fire now
	bFire = true;
}

void CThrow::Discharge(Vector3 position, Vector3 target, CPlayerInfo * _source, int multiplier)
{
	if (bFire)
	{
		// If there is still ammo in the magazine, then fire
		if (magRounds > 0)
		{
			// Create a projectile with a cube mesh. Its position and direction is same as the player.
			// It will last for 3.0 seconds and travel at 500 units per second
			CProjectile* aProjectile = Create::Present("cube",
				position,
				(target - position).Normalized(),
				7.0f,
				multiplier,
				_source);
			aProjectile->SetCollider(true);
			aProjectile->SetAABB(Vector3(0.5f, 0.5f, 0.5f), Vector3(-0.5f, -0.5f, -0.5f));
			bFire = false;
			magRounds--;
		}
	}
	std::cout << "Present thrown" << std::endl;
}
