#include "StdAfx.h"
#include "Aimer.h"
#include "Entity.h"
#include "Collidable.h"
#include "Link.h"
#include "Weapon.h"
#include "Damagable.h"
#include "Team.h"

#include "Ship.h"

#include "Behavior/Behavior.h"


#ifdef USE_POOL_ALLOCATOR
// aimer pool
static MemoryPool sPool(sizeof(Aimer));
void *Aimer::operator new(size_t aSize)
{
	return sPool.Alloc();
}
void Aimer::operator delete(void *aPtr)
{
	sPool.Free(aPtr);
}
#endif


namespace Database
{
	Typed<AimerTemplate> aimertemplate(0x9bde0ae7 /* "aimertemplate" */);
	Typed<Aimer *> aimer(0x2ea90881 /* "aimer" */);

	namespace Loader
	{
		static void AimerConfigure(unsigned int aId, const tinyxml2::XMLElement *element)
		{
			AimerTemplate &aimer = Database::aimertemplate.Open(aId);
			aimer.Configure(element, aId);
			Database::aimertemplate.Close(aId);
		}
		Configure aimerconfigure(0x2ea90881 /* "aimer" */, AimerConfigure);
	}

	namespace Initializer
	{
		static void AimerActivate(unsigned int aId)
		{
			const AimerTemplate &aimertemplate = Database::aimertemplate.Get(aId);
			Aimer *aimer = new Aimer(aimertemplate, aId);
			Database::aimer.Put(aId, aimer);
			Database::controller.Put(aId, aimer);
			aimer->Activate();
		}
		Activate aimeractivate(0x9bde0ae7 /* "aimertemplate" */, AimerActivate);

		static void AimerDeactivate(unsigned int aId)
		{
			if (Aimer *aimer = Database::aimer.Get(aId))
			{
				delete aimer;
				Database::aimer.Delete(aId);
				Database::controller.Delete(aId);
			}
		}
		Deactivate aimerdeactivate(0x9bde0ae7 /* "aimertemplate" */, AimerDeactivate);
	}
}

AimerTemplate::AimerTemplate(void)
: mSide(0.0f)
, mFront(0.0f)
, mTurn(0.0f)
{
}

AimerTemplate::~AimerTemplate(void)
{
}

bool AimerTemplate::Configure(const tinyxml2::XMLElement *element, unsigned int aId)
{
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		unsigned int hash = Hash(child->Value());
		switch (hash)
		{
		case 0x2e87eea4 /* "drift" */:
			{
				// backwards compatibility
				child->QueryFloatAttribute("strength", &mFront);

				// drift controls
				child->QueryFloatAttribute("turn", &mTurn);
				child->QueryFloatAttribute("side", &mSide);
				child->QueryFloatAttribute("front", &mFront);
			}
			break;

		default:
			{
				const BehaviorDatabase::Loader::Entry &configure = BehaviorDatabase::Loader::Configure::Get(hash);
				if (configure)
				{
					unsigned int initializer = configure(aId, child);
					if (std::find(mBehaviors.begin(), mBehaviors.end(), initializer) == mBehaviors.end())
						mBehaviors.push_back(initializer);
				}
				else
				{
					DebugPrint("\"%s\" aimer skipping item \"%s\"\n", child->Value());
				}
			}
			break;
		}
	}
	return true;
}


Aimer::Aimer(const AimerTemplate &aTemplate, unsigned int aId)
: Controller(aId)
, Brain()
{
	SetAction(Action(this, &Aimer::Control));

	for (std::vector<unsigned int>::const_iterator itor = aTemplate.mBehaviors.begin(); itor != aTemplate.mBehaviors.end(); ++itor)
	{
		const BehaviorDatabase::Initializer::Activate::Entry &activate = BehaviorDatabase::Initializer::Activate::Get(*itor);
		if (activate)
		{
			Behavior *behavior = activate(mId, this);
			mScheduler.Run(*behavior);
		}
	}
}

Aimer::~Aimer(void)
{
	mScheduler.Stop();

	// get aimer template
	const AimerTemplate &aimer = Database::aimertemplate.Get(mId);
	for (std::vector<unsigned int>::const_iterator itor = aimer.mBehaviors.begin(); itor != aimer.mBehaviors.end(); ++itor)
	{
		const BehaviorDatabase::Initializer::Deactivate::Entry &deactivate = BehaviorDatabase::Initializer::Deactivate::Get(*itor);
		if (deactivate)
		{
			deactivate(mId);
		}
	}
}

// Aimer Control
void Aimer::Control(float aStep)
{
	// get parent entity
	Entity *entity = Database::entity.Get(mId);

	// get transform
	const Transform2 &transform = entity->GetTransform();

	// get aimer template
	const AimerTemplate &aimer = Database::aimertemplate.Get(mId);

	// set default controls
	mMove = transform.Rotate(Vector2(aimer.mSide, aimer.mFront));
	mAim = Vector2(0, 0);
	mTurn = aimer.mTurn;
	memset(mFire, 0, sizeof(mFire));

	// think
	Think(aStep);

#ifdef AIMER_OBSTACLE_AVOIDANCE
	// obstacle avoidance
	if (b2Body *body = Database::collidablebody.Get(mId))
	{
		b2Fixture *shapelist = body->GetShapeList();
		const b2Filter &filter = shapelist->GetFilterData();

		// collision probe
		b2Vec2 start = transform.p;
		b2Vec2 end = transform.p + 32 * mMove;

		// perform a segment test
		float lambda = 1.0f;
		b2Vec2 normal(0, 0);
		b2Fixture *shape = NULL;
		Collidable::TestSegment(start, end, shapelist->GetSweepRadius() * 0.5f, mId, filter.categoryBits, filter.maskBits, lambda, normal, shape);
		if (lambda < 1.0f)
		{
			float push = 1.0f - lambda;
			mMove += push * normal;
			mTurn += transform.y.Cross(normal) > 0.0f ? push : -push;
		}
	}
#endif

	// limit move to 100%
	float moveLengthSq = mMove.LengthSq();
	if (moveLengthSq > 1.0f)
		mMove *= InvSqrt(moveLengthSq);

#ifdef AIMER_DEBUG_DRAW_CONTROLS
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2f(transform.p.x, transform.p.y);
	glVertex2f(transform.p.x + 16 * mMove.x, transform.p.y + 16 * mMove.y);
	glEnd();
#endif

	// convert to local coordinates
	mMove = transform.Unrotate(mMove);

	// limit turn to 100%
	mTurn = Clamp(mTurn, -1.0f, 1.0f);

#ifdef AIMER_DEBUG_DRAW_CONTROLS
	glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex2f(transform.p.x, transform.p.y);
	int steps = xs_CeilToInt(mTurn * 16);
	for(int i = 0; i < steps; i++)
	{
		float angle = float(M_PI) * i * mTurn / steps;
		Vector2 dir = transform.Transform(Vector2(-16*sinf(angle), 16*cosf(angle)));
		glVertex2f(dir.x, dir.y);
	}
	glEnd();
#endif
}
