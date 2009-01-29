#pragma once

#include "Controller.h"

// forward declaration
class Entity;

// wander behavior
class WanderBehaviorTemplate
{
public:
	float mSide;		// side strength
	float mSideRate;	// side frequency
	float mFront;		// front strength
	float mFrontRate;	// front frequency
	float mTurn;		// turn strength
	float mTurnRate;	// turn frequency

public:
	WanderBehaviorTemplate();
};

// target behavior
class TargetBehaviorTemplate
{
public:
	float mPeriod;		// time between scans
	float mRange;		// maximum range
	float mDirection;	// direction angle
	float mAngle;		// cone angle
	float mFocus;		// weight factor for current target
	float mAlign;		// weight factor for angle alignment
	b2FilterData mFilter;	// collision filtering

public:
	TargetBehaviorTemplate();
};

// pursue behavior
class PursueBehaviorTemplate
{
public:
	float mStrength;	// pursuit strength
	float mLeading;		// leading speed
	Transform2 mOffset;	// leading offset

public:
	PursueBehaviorTemplate();
};

// aim behavior
class AimBehaviorTemplate
{
public:
	float mStrength;	// aim strength
	float mLeading;		// leading speed

public:
	AimBehaviorTemplate();
};

// fire behavior
class FireBehaviorTemplate
{
public:
	int mChannel;		// fire channel
	float mRange;		// maximum range
	float mDirection;	// direction angle
	float mAngle;		// cone angle

public:
	FireBehaviorTemplate();
};

// evade behavior
class EvadeBehaviorTemplate
{
public:
	float mStrength;	// evasion strength

public:
	EvadeBehaviorTemplate();
};

// close behavior
class CloseBehaviorTemplate
{
public:
	float mRange;		// target distance
	float mScaleDist;	// proportional gain
	float mScaleSpeed;	// derivative gain

public:
	CloseBehaviorTemplate();
};

// far behavior
class FarBehaviorTemplate
{
public:
	float mRange;		// target distance
	float mScaleDist;	// proportional gain
	float mScaleSpeed;	// derivative gain

public:
	FarBehaviorTemplate();
};

// aimer template
class AimerTemplate
{
public:
	float mDrift;
	WanderBehaviorTemplate mWander;
	TargetBehaviorTemplate mTarget;
	PursueBehaviorTemplate mPursue;
	AimBehaviorTemplate mAim;
	FireBehaviorTemplate *mFire;
	int mFireCount;
	EvadeBehaviorTemplate mEvade;
	CloseBehaviorTemplate mClose;
	FarBehaviorTemplate mFar;

public:
	AimerTemplate(void);
	~AimerTemplate(void);

	// configure
	bool Configure(const TiXmlElement *element);
};

// aimer controller
class Aimer :
	public Controller
{
protected:
	unsigned int mTarget;
	Vector2 mOffset;
	float mDelay;
	float mWanderSidePhase;
	float mWanderFrontPhase;
	float mWanderTurnPhase;

public:
#ifdef USE_POOL_ALLOCATOR
	// allocation
	void *operator new(size_t aSize);
	void operator delete(void *aPtr);
#endif

	Aimer(const AimerTemplate &aTemplate, unsigned int aId = 0);
	virtual ~Aimer(void);

	// control
	void Control(float aStep);

protected:
	Vector2 Intercept(float aLeading, const Vector2 &aPosition, const Vector2 &aVelocity);
	Vector2 TargetDir(float aLeading, const Entity *aEntity, const Entity *aTargetEntity);
	void Wander(float aStep, Entity *entity, const AimerTemplate &aimer);
	void Target(float aStep, Entity *entity, const AimerTemplate &aimer);
	void Pursue(float aStep, Entity *entity, const AimerTemplate &aimer, Entity *targetEntity);
	void Aim(float aStep, Entity *entity, const AimerTemplate &aimer, Entity *targetEntity);
	void Evade(float aStep, Entity *entity, const AimerTemplate &aimer, Entity *targetEntity);
	void Range(float aStep, Entity *entity, const AimerTemplate &aimer, Entity *targetEntity);
	void Edge(float aStep, Entity *entity, const AimerTemplate &aimer);
};

namespace Database
{
	extern Typed<AimerTemplate> aimertemplate;
	extern Typed<Aimer *> aimer;
}