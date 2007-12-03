#include "StdAfx.h"
#include "Bullet.h"
#include "Explosion.h"

// bullet attributes
const float BULLET_RADIUS = 3;
const float BULLET_LIFE = 1.0f;

// bullet tail geometry
const float BULLET_TAIL_LENGTH = 0.0625f;
const float BULLET_TAIL_COLOR[2][4] =
{
	{ 1.0f, 1.0f, 1.0f, 0.5f },
	{ 0.0f, 0.0f, 1.0f, 0.0f }
};

// bullet pool
boost::object_pool<Bullet> Bullet::pool;

Bullet::Bullet(unsigned int aId, unsigned int aParentId)
: Entity(aId)
, Simulatable()
, Collidable(Database::collidabletemplate.Get(aParentId))
, Renderable(Database::renderabletemplate.Get(aParentId))
, mLife(BULLET_LIFE)
{
}

Bullet::~Bullet(void)
{
}

void Bullet::Simulate(float aStep)
{
	// count down life
	mLife -= aStep;
	if (mLife <= 0)
	{
		pool.destroy(this);
		return;
	}
}

void Bullet::Collide(Collidable &aRecipient, b2Manifold aManifold[], int aCount)
{
	// kill the bullet
//	mLife = 0.0f;
//	RemoveFromWorld();

	// create an explosion at the contact point
	Explosion *explosion = Explosion::pool.construct(0, 0x70f5d327 /* "playerbulletexplosion" */);
	b2Vec2 position(aManifold[0].points[0].position);
	b2Vec2 normal(aManifold[0].normal);
	float dist(body->m_shapeList[0].GetMaxRadius()-aManifold[0].points[0].separation);
	explosion->SetPosition(Vector2(position.x + normal.x * dist, position.y + normal.y * dist));
	explosion->Init();
}

void Bullet::Render(const Matrix2 &transform)
{
	// push a transform
	glPushMatrix();

	// load matrix
	float m[16] =
	{
		transform.x.x, transform.x.y, 0, 0,
		transform.y.x, transform.y.y, 0, 0,
		0, 0, 1, 0,
		transform.p.x, transform.p.y, 0, 1
	};
	glMultMatrixf( m );

	// draw bullet
	glCallList(mDraw);

	// reset the transform
	glPopMatrix();

	// if moving...
	if (vel.x != 0 && vel.y != 0)
	{
		// draw a tail
		float tail = std::min(1.0f - mLife, BULLET_TAIL_LENGTH);
		glBegin( GL_LINES );
		glColor4fv( BULLET_TAIL_COLOR[0] );
		glVertex2f( transform.p.x, transform.p.y );
		glColor4fv( BULLET_TAIL_COLOR[1] );
		glVertex2f( transform.p.x - vel.x * tail, transform.p.y - vel.y * tail );
		glEnd();
	}
}
