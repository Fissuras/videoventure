#pragma once

#include "Updatable.h"

class WeaponTemplate
{
public:
	// offset
	Matrix2 mOffset;
	Vector2 mInherit;
	Vector2 mVelocity;

	// ordnance
	unsigned int mOrdnance;

	// fire channel
	int mChannel;

	// fire delay
	float mDelay;
	int mPhase;
	int mCycle;

public:
	WeaponTemplate(void);
	~WeaponTemplate(void);

	// configure
	bool Configure(const TiXmlElement *element);
};

class Weapon
	: public Updatable
{
protected:
	// controller
	unsigned int mControlId;

	// fire channel
	int mChannel;

	// fire timer
	float mTimer;
	int mPhase;

public:
#ifdef USE_POOL_ALLOCATOR
	// allocation
	void *operator new(size_t aSize);
	void operator delete(void *aPtr);
#endif

	Weapon(void);
	Weapon(const WeaponTemplate &aTemplate, unsigned int aId);
	virtual ~Weapon(void);

	// set control
	void SetControl(unsigned int aControlId)
	{
		mControlId = aControlId;
	}

	// configure
	bool Configure(const TiXmlElement *element);

	// update
	virtual void Update(float aStep);
};

namespace Database
{
	extern Typed<WeaponTemplate> weapontemplate;
	extern Typed<Weapon *> weapon;
}