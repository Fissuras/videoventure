#include "StdAfx.h"

#include "AimBehavior.h"
#include "TargetBehavior.h"
#include "BotUtilities.h"
#include "..\Controller.h"
#include "..\Entity.h"
#include "..\Ship.h"

namespace Database
{
	Typed<AimBehaviorTemplate> aimbehaviortemplate(0x48ae5f7a /* "aimbehaviortemplate" */);
	Typed<Typed<FireConeTemplate> > fireconetemplate(0x00dbebf8 /* "fireconetemplate" */);
}

FireConeTemplate::FireConeTemplate()
: mRange(0.0f)
, mDirection(0.0f)
, mAngle(0.3f)
, mChannel(-1)
{
}

bool FireConeTemplate::Configure(const TiXmlElement *element, unsigned int aId)
{
	if (element->QueryIntAttribute("channel", &mChannel) == TIXML_SUCCESS)
		--mChannel;
	element->QueryFloatAttribute("range", &mRange);
	if (element->QueryFloatAttribute("direction", &mDirection) == TIXML_SUCCESS)
		mDirection *= float(M_PI) / 180.0f;
	if (element->QueryFloatAttribute("angle", &mAngle) == TIXML_SUCCESS)
		mAngle *= float(M_PI) / 180.0f;
	return true;
}


AimBehaviorTemplate::AimBehaviorTemplate()
: mStrength(0.0f)
, mLeading(0.0f)
{
}

bool AimBehaviorTemplate::Configure(const TiXmlElement *element, unsigned int aId)
{
	element->QueryFloatAttribute("strength", &mStrength);
	element->QueryFloatAttribute("leading", &mLeading);
	return true;
}

AimBehavior::AimBehavior(unsigned int aId, const AimBehaviorTemplate &aTemplate, Controller *aController)
: Behavior(aId, aController)
{
	bind(this, &AimBehavior::Execute);
}

// aim behavior
Status AimBehavior::Execute(void)
{
	// get target
	const TargetData &targetdata = Database::targetdata.Get(mId);

	// get target entity
	Entity *targetEntity = Database::entity.Get(targetdata.mTarget);
	if (!targetEntity)
		return runningTask;

	// get aim behavior template
	const AimBehaviorTemplate &aim = Database::aimbehaviortemplate.Get(mId);

	// get owner entity
	Entity *entity = Database::entity.Get(mId);

	// get transform
	const Transform2 &transform = entity->GetTransform();

	// direction to target
	Vector2 targetDir(TargetDir(aim.mLeading, entity, targetEntity, targetdata.mOffset));

	// save range
	float distSq = targetDir.LengthSq();

	// normalize direction
	targetDir *= InvSqrt(distSq);

	// local direction
	mController->mAim = transform.Unrotate(targetDir);

	// angle to target
	float aimAngle = -atan2f(mController->mAim.x, mController->mAim.y);

	// if aiming...
	if (aim.mStrength != 0)
	{
		// turn towards target direction
		const ShipTemplate &ship = Database::shiptemplate.Get(mId);	// <-- hack!
		if (ship.mMaxOmega != 0.0f)
		{
			mController->mTurn += aim.mStrength * Clamp(aimAngle / (ship.mMaxOmega * sim_step), -1.0f, 1.0f);
		}
	}

	// for each fire cone...
	for (Database::Typed<FireConeTemplate>::Iterator itor(Database::fireconetemplate.Find(mId)); itor.IsValid(); ++itor)
	{
		// get the fire cone properties
		const FireConeTemplate &fire = itor.GetValue();

		// if not set to fire and target is in range...
		if (!mController->mFire[fire.mChannel] &&
			distSq <= fire.mRange * fire.mRange)
		{
			// normalize direction
			targetDir *= InvSqrt(distSq);

			// local direction
			Vector2 localDir = transform.Unrotate(targetDir);

			// angle to target
			float aimAngle = -atan2f(localDir.x, localDir.y);

			float localAngle = aimAngle - fire.mDirection;
			if (localAngle > float(M_PI))
				localAngle -= float(M_PI)*2.0f;
			else if (localAngle < -float(M_PI))
				localAngle += float(M_PI)*2.0f;
			if (fabsf(localAngle) <= fire.mAngle)
				mController->mFire[fire.mChannel] = sqrtf(distSq) / fire.mRange; // encode range into fire (HACK)
		}
	}

	return runningTask;
}
