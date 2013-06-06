#include "StdAfx.h"
#include "Collidable.h"
#include "Entity.h"
#include "Link.h"
#include "Command.h"
#include "Console.h"

struct CollisionCircleDef 
{
	b2FixtureDef mFixture;
	b2CircleShape mShape;

	CollisionCircleDef()
	{
		mFixture.shape = &mShape;
	}
	CollisionCircleDef(const CollisionCircleDef &aSrc)
	{
		mFixture = aSrc.mFixture;
		mShape = aSrc.mShape;
		mFixture.shape = &mShape;
	}
	b2Fixture *Instantiate(b2Body *aBody) const
	{
		b2Fixture *fixture = aBody->CreateFixture(&mFixture);
		fixture->SetUserData(aBody->GetUserData());
		return fixture;
	}
};
struct CollisionPolygonDef 
{
	b2FixtureDef mFixture;
	b2PolygonShape mShape;

	CollisionPolygonDef()
	{
		mFixture.shape = &mShape;
	}
	CollisionPolygonDef(const CollisionPolygonDef &aSrc)
	{
		mFixture = aSrc.mFixture;
		mShape = aSrc.mShape;
		mFixture.shape = &mShape;
	}
	b2Fixture *Instantiate(b2Body *aBody) const
	{
		b2Fixture *fixture = aBody->CreateFixture(&mFixture);
		fixture->SetUserData(aBody->GetUserData());
		return fixture;
	}
};
struct CollisionEdgeDef 
{
	b2FixtureDef mFixture;
	b2EdgeShape mShape;

	CollisionEdgeDef()
	{
		mFixture.shape = &mShape;
	}
	CollisionEdgeDef(const CollisionEdgeDef &aSrc)
	{
		mFixture = aSrc.mFixture;
		mShape = aSrc.mShape;
		mFixture.shape = &mShape;
	}
	b2Fixture *Instantiate(b2Body *aBody) const
	{
		b2Fixture *fixture = aBody->CreateFixture(&mFixture);
		fixture->SetUserData(aBody->GetUserData());
		return fixture;
	}
};
struct CollisionChainDef 
{
	b2FixtureDef mFixture;
	b2ChainShape mShape;

	CollisionChainDef()
	{
		mFixture.shape = &mShape;
	}
	CollisionChainDef(const CollisionChainDef &aSrc)
	{
		mFixture = aSrc.mFixture;
		mShape = aSrc.mShape;
		mFixture.shape = &mShape;
	}
	b2Fixture *Instantiate(b2Body *aBody) const
	{
		b2Fixture *fixture = aBody->CreateFixture(&mFixture);
		fixture->SetUserData(aBody->GetUserData());
		return fixture;
	}
};

namespace Database
{
	Typed<b2Filter> collidablefilter(0x5224d988 /* "collidablefilter" */);
	Typed<CollidableTemplate> collidabletemplate(0xa7380c00 /* "collidabletemplate" */);
	Typed<Typed<CollisionCircleDef> > collidablecircles(0x67fa99ff /* "collidablecircles" */);
	Typed<Typed<CollisionPolygonDef> > collidablepolygons(0xab54c159 /* "collidablepolygons" */);
	Typed<Typed<CollisionEdgeDef> > collidableedges(0xae12a01c /* "collidableedges" */);
	Typed<Typed<CollisionChainDef> > collidablechains(0xd2bae418 /* "collidablechains" */);
	Typed<b2Body *> collidablebody(0x6ccc2b62 /* "collidablebody" */);
	Typed<Collidable::ContactSignal> collidablecontactadd(0x7cf2c45d /* "collidablecontactadd" */);
	Typed<Collidable::ContactSignal> collidablecontactremove(0x95ed5aba /* "collidablecontactremove" */);

	namespace Loader
	{
		class FilterLoader
		{
		public:
			FilterLoader()
			{
				AddConfigure(0x5224d988 /* "collidablefilter" */, Entry(this, &FilterLoader::Configure));

				b2Filter &filter = Database::collidablefilter.OpenDefault();
				filter = Collidable::GetDefaultFilter();
				Database::collidablefilter.CloseDefault();
			}

			void Configure(unsigned int aId, const tinyxml2::XMLElement *element)
			{
				if (!Database::collidablefilter.Find(aId))
					Database::collidablefilter.Put(aId, Collidable::GetDefaultFilter());
				b2Filter &filter = Database::collidablefilter.Open(aId);
				ConfigureFilterData(filter, element);
				Database::collidablefilter.Close(aId);
			}
		}
		filterloader;

		class CollidableLoader
		{
		public:
			CollidableLoader()
			{
				AddConfigure(0x74e9dbae /* "collidable" */, Entry(this, &CollidableLoader::Configure));
			}

			void Configure(unsigned int aId, const tinyxml2::XMLElement *element)
			{
				CollidableTemplate &collidable = Database::collidabletemplate.Open(aId);
				collidable.Configure(element, aId);
				Database::collidabletemplate.Close(aId);
			}
		}
		collidableloader;
	}

	namespace Initializer
	{
		class CollidableInitializer
		{
		public:
			CollidableInitializer()
			{
				AddActivate(0xa7380c00 /* "collidabletemplate" */, Entry(this, &CollidableInitializer::Activate));
				AddDeactivate(0xa7380c00 /* "collidabletemplate" */, Entry(this, &CollidableInitializer::Deactivate));
			}

			void Activate(unsigned int aId)
			{
				Collidable::AddToWorld(aId);
			}

			void Deactivate(unsigned int aId)
			{
				Collidable::RemoveFromWorld(aId);
			}
		}
		collidableinitializer;
	}
}

static void ConfigureFilterCategory(b2Filter &aFilter, const tinyxml2::XMLElement *element, const char *name)
{
	int category = 0;
	if (element->QueryIntAttribute(name, &category) == tinyxml2::XML_SUCCESS)
		aFilter.categoryBits = (category >= 0) ? (1<<category) : 0;
}

static void ConfigureFilterMask(b2Filter &aFilter, const tinyxml2::XMLElement *element)
{
	bool defvalue = true;
	if (element->QueryBoolAttribute("default", &defvalue) == tinyxml2::XML_SUCCESS)
		aFilter.maskBits = defvalue ? 0xFFFF : 0x0000;

	char buf[16];
	for (int i = 0; i < 16; i++)
	{
		sprintf(buf, "bit%d", i);
		bool bit = false;
		if (element->QueryBoolAttribute(buf, &bit) == tinyxml2::XML_SUCCESS)
		{
			if (bit)
				aFilter.maskBits |= (1 << i);
			else
				aFilter.maskBits &= ~(1 << i);
		}
	}
}

static void ConfigureFilterGroup(b2Filter &aFilter, const tinyxml2::XMLElement *element, const char *name)
{
	int group = aFilter.groupIndex;
	element->QueryIntAttribute(name, &group);
	aFilter.groupIndex = short(group);
}

void ConfigureFilterData(b2Filter &aFilter, const tinyxml2::XMLElement *element)
{
	if (const char *name = (Hash(element->Value()) == 0xc7e16877 /* "filter" */) ? element->Attribute("name") : element->Attribute("filter"))
		aFilter = Database::collidablefilter.Get(Hash(name));

	ConfigureFilterCategory(aFilter, element, "category");
	ConfigureFilterMask(aFilter, element);
	ConfigureFilterGroup(aFilter, element, "group");

	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		switch(Hash(child->Value()))
		{
		case 0xcf2f4271 /* "category" */:
			ConfigureFilterCategory(aFilter, child, "value");
			break;

		case 0xe7774569 /* "mask" */:
			ConfigureFilterMask(aFilter, child);
			break;

		case 0x5fb91e8c /* "group" */:
			ConfigureFilterGroup(aFilter, child, "value");
			break;
		}
	}
}

CollidableTemplate::CollidableTemplate(void)
: id(0)
{
	// default to automatic type selection
	bodydef.type = b2BodyType(-1);
}

CollidableTemplate::CollidableTemplate(const CollidableTemplate &aTemplate)
: id(aTemplate.id)
, bodydef(aTemplate.bodydef)
{
}

CollidableTemplate::~CollidableTemplate(void)
{
}

bool CollidableTemplate::ConfigureFixtureItem(const tinyxml2::XMLElement *element, b2FixtureDef &fixture)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xa51be2bb /* "friction" */:
		element->QueryFloatAttribute("value", &fixture.friction);
		return true;

	case 0xf59a4f8f /* "restitution" */:
		element->QueryFloatAttribute("value", &fixture.restitution);
		return true;

	case 0x72b9059b /* "density" */:
		element->QueryFloatAttribute("value", &fixture.density);
		return true;

	case 0xc7e16877 /* "filter" */:
		ConfigureFilterData(fixture.filter, element);
		return true;

	case 0xcf2f4271 /* "category" */:
		ConfigureFilterCategory(fixture.filter, element, "value");
		return true;

	case 0xe7774569 /* "mask" */:
		ConfigureFilterMask(fixture.filter, element);
		return true;

	case 0x5fb91e8c /* "group" */:
		ConfigureFilterGroup(fixture.filter, element, "value");
		return true;

	case 0x83b6367b /* "sensor" */:
		element->QueryBoolAttribute("value", &fixture.isSensor);
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ConfigureFixture(const tinyxml2::XMLElement *element, b2FixtureDef &fixture)
{
	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ConfigureFixtureItem(child, fixture);
	}
	return true;
}

bool CollidableTemplate::ConfigureCircle(const tinyxml2::XMLElement *element, b2CircleShape &shape)
{
	element->QueryFloatAttribute("radius", &shape.m_radius);
	return true;
}

bool CollidableTemplate::ConfigureBox(const tinyxml2::XMLElement *element, b2PolygonShape &shape)
{
	// half-width and half-height
	float w = 0, h = 0;
	element->QueryFloatAttribute("w", &w);
	element->QueryFloatAttribute("h", &h);

	// center and rotation
	b2Vec2 center(0.0f, 0.0f);
	float rotation = 0.0f;

	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		switch (Hash(child->Value()))
		{
		case 0x934f4e0a /* "position" */:
			element->QueryFloatAttribute("x", &center.x);
			element->QueryFloatAttribute("y", &center.y);
			if (element->QueryFloatAttribute("angle", &rotation) == tinyxml2::XML_SUCCESS)
				 rotation *= float(M_PI)/180.0f;
			break;
		}
	}

	shape.SetAsBox(w, h, center, rotation);
	return true;
}

bool CollidableTemplate::ConfigurePolyItem(const tinyxml2::XMLElement *element, b2PolygonShape &shape)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x945367a7 /* "vertex" */:
		element->QueryFloatAttribute("x", &shape.m_vertices[shape.m_count].x);
		element->QueryFloatAttribute("y", &shape.m_vertices[shape.m_count].y);
		++shape.m_count;
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ConfigurePoly(const tinyxml2::XMLElement *element, b2PolygonShape &shape)
{
	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ConfigurePolyItem(child, shape);
	}

	// update other properties
	shape.Set(shape.m_vertices, shape.m_count);

	return true;
}

bool CollidableTemplate::ConfigureEdge(const tinyxml2::XMLElement *element, b2EdgeShape &shape)
{
	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		const char *name = child->Value();
		switch (Hash(name))
		{
		case 0x154c1122 /* "vertex1" */:
			child->QueryFloatAttribute("x", &shape.m_vertex1.x);
			child->QueryFloatAttribute("y", &shape.m_vertex1.y);
			break;

		case 0x144c0f8f /* "vertex2" */:
			child->QueryFloatAttribute("x", &shape.m_vertex2.x);
			child->QueryFloatAttribute("y", &shape.m_vertex2.y);
			break;
		}
	}

	return true;
}

bool CollidableTemplate::ConfigureChain(const tinyxml2::XMLElement *element, b2ChainShape &shape)
{
	if (const tinyxml2::XMLElement *first = element->FirstChildElement("vertex"))
	{
		int count = 0;
		for (const tinyxml2::XMLElement *child = first; child != NULL; child = child->NextSiblingElement("vertex"))
			++count;

		if (count > 1)
		{
			bool loop = false;
			element->QueryBoolAttribute("loop", &loop);

			b2Vec2 *vertices = (b2Vec2 *)_alloca(count * sizeof(b2Vec2));

			int index = 0;
			for (const tinyxml2::XMLElement *child = first; child != NULL; child = child->NextSiblingElement("vertex"))
			{
				child->QueryFloatAttribute("x", &vertices[index].x);
				child->QueryFloatAttribute("y", &vertices[index].y);
				++index;
			}

			if (loop)
				shape.CreateLoop(vertices, count);
			else
				shape.CreateChain(vertices, count);
		}
	}

	return true;
}


bool CollidableTemplate::ConfigureBodyItem(const tinyxml2::XMLElement *element, b2BodyDef &body, unsigned int id)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x934f4e0a /* "position" */:
		element->QueryFloatAttribute("x", &body.position.x);
		element->QueryFloatAttribute("y", &body.position.y);
		if (element->QueryFloatAttribute("angle", &body.angle) == tinyxml2::XML_SUCCESS)
			 body.angle *= float(M_PI)/180.0f;
		return true;

	case 0xbb61b895 /* "damping" */:
		element->QueryFloatAttribute("linear", &body.linearDamping);
		element->QueryFloatAttribute("angular", &body.angularDamping);
		return true;

	case 0xac01f355 /* "allowsleep" */:
		element->QueryBoolAttribute("value", &body.allowSleep);
		return true;

	case 0xd71034dc /* "awake" */:
		element->QueryBoolAttribute("value", &body.awake);
		return true;

	case 0x7a04061b /* "fixedrotation" */:
		element->QueryBoolAttribute("value", &body.fixedRotation);
		return true;

	case 0x029402af /* "fast" */:
		element->QueryBoolAttribute("value", &body.bullet);
		return true;

	case 0xd975992f /* "active" */:
		element->QueryBoolAttribute("value", &body.active);
		return true;

	case 0x5127f14d /* "type" */:
		{
			switch(Hash(element->Attribute("value")))
			{
			case 0x923fa396 /* "auto" */:
				body.type = b2BodyType(-1);
				break;

			case 0xd290c23b /* "static" */:
				body.type = b2_staticBody;
				break;

			case 0xc4be0946 /* "kinematic" */:
				body.type = b2_kinematicBody;
				break;

			case 0x4f5296ae /* "dynamic" */:
				body.type = b2_dynamicBody;
				break;
			}
		}
		return true;

	case 0x28217089 /* "circle" */:
		{
			Database::Typed<CollisionCircleDef> &shapes = Database::collidablecircles.Open(id);
			unsigned int subid;
			if (const char *name = element->Attribute("name"))
				subid = Hash(name);
			else
				subid = shapes.GetCount() + 1;
			CollisionCircleDef &def = shapes.Open(subid);
			ConfigureFixture(element, def.mFixture);
			ConfigureCircle(element, def.mShape);
			shapes.Close(subid);
			Database::collidablecircles.Close(id);
		}
		return true;

	case 0x70c67e32 /* "box" */:
		{
			Database::Typed<CollisionPolygonDef> &shapes = Database::collidablepolygons.Open(id);
			unsigned int subid;
			if (const char *name = element->Attribute("name"))
				subid = Hash(name);
			else
				subid = shapes.GetCount() + 1;
			CollisionPolygonDef &def = shapes.Open(subid);
			ConfigureFixture(element, def.mFixture);
			ConfigureBox(element, def.mShape);
			shapes.Close(subid);
			Database::collidablepolygons.Close(id);
		}
		return true;

	case 0x84d6a947 /* "poly" */:
		{
			Database::Typed<CollisionPolygonDef> &shapes = Database::collidablepolygons.Open(id);
			unsigned int subid;
			if (const char *name = element->Attribute("name"))
				subid = Hash(name);
			else
				subid = shapes.GetCount() + 1;
			CollisionPolygonDef &def = shapes.Open(subid);
			ConfigureFixture(element, def.mFixture);
			ConfigurePoly(element, def.mShape);
			shapes.Close(subid);
			Database::collidablepolygons.Close(id);
		}
		return true;

	case 0x56f6d83c /* "edge" */:
		{
			Database::Typed<CollisionEdgeDef> &shapes = Database::collidableedges.Open(id);
			unsigned int subid;
			if (const char *name = element->Attribute("name"))
				subid = Hash(name);
			else
				subid = shapes.GetCount() + 1;
			CollisionEdgeDef &def = shapes.Open(subid);
			ConfigureFixture(element, def.mFixture);
			ConfigureEdge(element, def.mShape);
			shapes.Close(subid);
			Database::collidableedges.Close(id);
		}
		return true;

	case 0x620c0b45 /* "edgechain" */:
		{
			Database::Typed<CollisionChainDef> &shapes = Database::collidablechains.Open(id);
			unsigned int subid;
			if (const char *name = element->Attribute("name"))
				subid = Hash(name);
			else
				subid = shapes.GetCount() + 1;
			CollisionChainDef &def = shapes.Open(subid);
			ConfigureFixture(element, def.mFixture);
			ConfigureChain(element, def.mShape);
			shapes.Close(subid);
			Database::collidablechains.Close(id);
		}
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ConfigureBody(const tinyxml2::XMLElement *element, b2BodyDef &body, unsigned int id)
{
	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ConfigureBodyItem(child, body, id);
	}
	return true;
}

bool CollidableTemplate::Configure(const tinyxml2::XMLElement *element, unsigned int id)
{
	// save identifier
	this->id = id;

	// process child elements
	for (const tinyxml2::XMLElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		const char *name = child->Value();
		switch (Hash(name))
		{
		case 0xdbaa7975 /* "body" */:
			{
				// set up the collidable body
				CollidableTemplate::ConfigureBody(child, bodydef, id);
			}
			break;
		}
	}

	return true;
}


namespace Collidable
{
	b2World* world;
	b2AABB boundary;
}

/// Implement this class to get collision results. You can use these results for
/// things like sounds and game logic. You can also use this class to tweak contact 
/// settings. These tweaks persist until you tweak the settings again or the contact
/// is destroyed.
/// @warning You cannot create/destroy Box2D entities inside these callbacks.
class ContactListener : public b2ContactListener
{
public:

	virtual void BeginContact(b2Contact* contact)
	{
		b2Fixture *fixture1 = contact->GetFixtureA();
		b2Fixture *fixture2 = contact->GetFixtureB();
		Database::Key id1 = reinterpret_cast<Database::Key>(fixture1->GetUserData());
		Database::Key id2 = reinterpret_cast<Database::Key>(fixture2->GetUserData());
		Database::collidablecontactadd.Get(id1)(id1, id2, 0.0f /*fixture1->GetBody()->m_sweep.t0*/, *contact);
		Database::collidablecontactadd.Get(id2)(id2, id1, 0.0f /*fixture2->GetBody()->m_sweep.t0*/, *contact);
	}
	virtual void EndContact(b2Contact* contact)
	{
		b2Fixture *fixture1 = contact->GetFixtureA();
		b2Fixture *fixture2 = contact->GetFixtureB();
		Database::Key id1 = reinterpret_cast<Database::Key>(fixture1->GetUserData());
		Database::Key id2 = reinterpret_cast<Database::Key>(fixture2->GetUserData());
		Database::collidablecontactremove.Get(id1)(id1, id2, 0.0f /*fixture1->GetBody()->m_sweep.t0*/, *contact);
		Database::collidablecontactremove.Get(id2)(id2, id1, 0.0f /*fixture2->GetBody()->m_sweep.t0*/, *contact);
	}
}
contactListener;


#ifdef COLLIDABLE_DEBUG_DRAW
// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class DebugDraw : public b2Draw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawAxis(const b2Vec2& point, const b2Vec2& axis, const b2Color& color);
	void DrawForce(const b2Vec2& point, const b2Vec2& force, const b2Color& color);
	void DrawAABB(b2AABB* aabb, const b2Color& c);
	void DrawTransform(const b2Transform& xf);
}
debugDraw;

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINE_LOOP);
	for (int32 i = 0; i < vertexCount; ++i)
	{
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	glBegin(GL_TRIANGLE_FAN);
	for (int32 i = 0; i < vertexCount; ++i)
	{
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();

	glColor4f(color.r, color.g, color.b, 1.0f);
	glBegin(GL_LINE_LOOP);
	for (int32 i = 0; i < vertexCount; ++i)
	{
		glVertex2f(vertices[i].x, vertices[i].y);
	}
	glEnd();
}

void DebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	const float32 k_segments = 16.0f;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINE_LOOP);
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	const float32 k_segments = 16.0f;
	const float32 k_increment = 2.0f * b2_pi / k_segments;
	float32 theta = 0.0f;
	glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	glBegin(GL_TRIANGLE_FAN);
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();

	theta = 0.0f;
	glColor4f(color.r, color.g, color.b, 1.0f);
	glBegin(GL_LINE_LOOP);
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
		glVertex2f(v.x, v.y);
		theta += k_increment;
	}
	glEnd();

	b2Vec2 p = center + radius * axis;
	glBegin(GL_LINES);
	glVertex2f(center.x, center.y);
	glVertex2f(p.x, p.y);
	glEnd();
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINES);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	b2Vec2 p1 = xf.p, p2;
	const float32 k_axisScale = 0.4f;
	glBegin(GL_LINES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	glVertex2f(p2.x, p2.y);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	glVertex2f(p2.x, p2.y);

	glEnd();
}

void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{
	glPointSize(size);
	glBegin(GL_POINTS);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(p.x, p.y);
	glEnd();
	glPointSize(1.0f);
}

void DebugDraw::DrawAABB(b2AABB* aabb, const b2Color& c)
{
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_LINE_LOOP);
	glVertex2f(aabb->lowerBound.x, aabb->lowerBound.y);
	glVertex2f(aabb->upperBound.x, aabb->lowerBound.y);
	glVertex2f(aabb->upperBound.x, aabb->upperBound.y);
	glVertex2f(aabb->lowerBound.x, aabb->upperBound.y);
	glEnd();
}

void DebugDraw::DrawForce(const b2Vec2& point, const b2Vec2& force, const b2Color& color)
{
	const float32 k_forceScale = 0.1f;
	b2Vec2 p1 = point;
	b2Vec2 p2 = point + k_forceScale * force;
	DrawSegment(p1, p2, color);
}

// console
extern Console *console;

int CommandDrawCollidable(const char * const aParam[], int aCount)
{
	struct Option
	{
		unsigned int hash;
		const char * name;
		unsigned int flag;
	};
	const Option options[] =
	{
		{ 0x9dc3d926 /* "shape" */, "shape", b2Draw::e_shapeBit },
		{ 0xaeae0877 /* "joint" */, "joint", b2Draw::e_jointBit },
		{ 0x63e91357 /* "aabb" */, "aabb", b2Draw::e_aabbBit },
		{ 0x7c445ab1 /* "pair" */, "pair", b2Draw::e_pairBit },
		{ 0x058c4484 /* "center" */, "center", b2Draw::e_centerOfMassBit },
	};

	unsigned int hash = (aCount >= 1) ? Hash(aParam[0]) : 0x13254bc4 /* "all" */;
	for (int i = 0; i < SDL_arraysize(options); ++i)
	{
		const Option &option = options[i];
		if (hash == option.hash || hash == 0x13254bc4 /* "all" */)
		{
			if (aCount >= 2)
			{
				if (atoi(aParam[1]) != 0)
					debugDraw.AppendFlags(option.flag);
				else
					debugDraw.ClearFlags(option.flag);
			}
			else
			{
				console->Print("%s: %s\n", option.name, (debugDraw.GetFlags() & option.flag) == option.flag ? "on" : "off");
			}
		}
	}

	return std::min(aCount, 2);
}
Command::Auto commanddrawcollidable(0x38c5ac70 /* "drawcollidable" */, CommandDrawCollidable);

#endif

b2World *Collidable::GetWorld(void)
{
	return world;
}
const b2AABB &Collidable::GetBoundary(void)
{
	return boundary;
}

void Collidable::AddToWorld(unsigned int aId)
{
	const CollidableTemplate &collidable = Database::collidabletemplate.Get(aId);

	// copy the body definition
	b2BodyDef def(collidable.bodydef);

	// set userdata identifier and body position to entity (HACK)
	def.userData = reinterpret_cast<void *>(aId);
	if (const Entity *entity = Database::entity.Get(aId))
	{
		def.position = entity->GetPosition();
		def.angle = entity->GetAngle();
		def.linearVelocity = entity->GetVelocity();
		def.angularVelocity = entity->GetOmega();
	}

	// if using automatic type...
	if (def.type < 0)
	{
		// kinematic if the body is moving, static otherwise
		if (def.linearVelocity.LengthSquared() > 0.0f || fabsf(def.angularVelocity) > 0.0f)
			def.type = b2_kinematicBody;
		else
			def.type = b2_staticBody;

		// dynamic if the body has mass
		for (Database::Typed<CollisionCircleDef>::Iterator itor(Database::collidablecircles.Find(aId)); itor.IsValid(); ++itor)
			if (itor.GetValue().mFixture.density > 0.0f)
				def.type = b2_dynamicBody;
		for (Database::Typed<CollisionPolygonDef>::Iterator itor(Database::collidablepolygons.Find(aId)); itor.IsValid(); ++itor)
			if (itor.GetValue().mFixture.density > 0.0f)
				def.type = b2_dynamicBody;
	}

	// create the body
	b2Body *body = Collidable::world->CreateBody(&def);
	Database::collidablebody.Put(aId, body);

	// add shapes
	for (Database::Typed<CollisionCircleDef>::Iterator itor(Database::collidablecircles.Find(aId)); itor.IsValid(); ++itor)
		itor.GetValue().Instantiate(body);
	for (Database::Typed<CollisionPolygonDef>::Iterator itor(Database::collidablepolygons.Find(aId)); itor.IsValid(); ++itor)
		itor.GetValue().Instantiate(body);
	for (Database::Typed<CollisionEdgeDef>::Iterator itor(Database::collidableedges.Find(aId)); itor.IsValid(); ++itor)
		itor.GetValue().Instantiate(body);
	for (Database::Typed<CollisionChainDef>::Iterator itor(Database::collidablechains.Find(aId)); itor.IsValid(); ++itor)
		itor.GetValue().Instantiate(body);
}

void Collidable::RemoveFromWorld(unsigned int aId)
{
	if (b2Body *body = Database::collidablebody.Get(aId))
	{
		Collidable::world->DestroyBody(body);
	}
	Database::collidablebody.Delete(aId);
	Database::collidablecontactadd.Delete(aId);
	Database::collidablecontactremove.Delete(aId);
}



// create collision world
void Collidable::WorldInit(float aMinX, float aMinY, float aMaxX, float aMaxY, bool aWall)
{
	// save boundary extents
	boundary.lowerBound.Set(aMinX, aMinY);
	boundary.upperBound.Set(aMaxX, aMaxY);

	// create physics world
	b2Vec2 gravity;
	gravity.Set(0.0f, 0.0f);
	world = new b2World(gravity);

	// set contact listener
	world->SetContactListener(&contactListener);

#ifdef COLLIDABLE_DEBUG_DRAW
	// set debug render
	world->SetDebugDraw(&debugDraw);
#endif

	if (aWall)
	{
		// create perimeter wall
		b2BodyDef def;
		b2Body *body = world->CreateBody(&def);

		b2Vec2 vertex[4] =
		{
			b2Vec2(aMaxX, aMaxY),
			b2Vec2(aMaxX, aMinY),
			b2Vec2(aMinX, aMinY),
			b2Vec2(aMinX, aMaxY)
		};

		b2EdgeShape shape;
		b2FixtureDef fixture;
		fixture.shape = &shape;
		for (int i = 0; i < 4; ++i)
		{
			shape.m_vertex0 = vertex[(i-1)&3];
			shape.m_vertex1 = vertex[i];
			shape.m_vertex2 = vertex[(i+1)&3];
			shape.m_vertex3 = vertex[(i+2)&3];
			shape.m_hasVertex0 = true;
			shape.m_hasVertex3 = true;
			body->CreateFixture(&fixture);
		}
	}
}

void Collidable::WorldDone(void)
{
	delete world;
	world = NULL;
}

void Collidable::CollideAll(float aStep)
{
	// exit if no world
	if (!world)
		return;

	// step the physics world
	world->Step(aStep, 8, 3);

	// for each body...
	for (b2Body* body = world->GetBodyList(); body; body = body->GetNext())
	{
		// if the body is not sleeping or static...
		if (body->IsAwake() && body->GetType() != b2_staticBody)
		{
			// get the database key
			Database::Key id = reinterpret_cast<Database::Key>(body->GetUserData());

			// update the entity position (hack)
			Entity *entity = Database::entity.Get(id);
			if (entity)
			{
				entity->Step();
				entity->SetTransform(body->GetAngle(), Vector2(body->GetPosition()));
				entity->SetVelocity(Vector2(body->GetLinearVelocity()));
				entity->SetOmega(body->GetAngularVelocity());
			}
		}
	}

#ifdef COLLIDABLE_DEBUG_DRAW
	world->DrawDebugData();
#endif
}

class CollidableRayCast : public b2RayCastCallback
{
public:
	b2Filter mFilter;
	unsigned int mSkipId;

	unsigned int mHitId;
	b2Fixture *mHitFixture;
	Vector2 mHitPoint;
	Vector2 mHitNormal;
	float mHitFraction;

public:
	CollidableRayCast(const b2Filter &aFilter, unsigned int aSkipId)
		: mFilter(aFilter), mSkipId(aSkipId), mHitId(0), mHitFixture(NULL), mHitFraction(1.0f)
	{
	}

	virtual float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
									const b2Vec2& normal, float32 fraction)
	{
		// skip unhittable fixtures
		if (fixture->IsSensor())
			return 1.0f;
		if (!Collidable::CheckFilter(mFilter, fixture->GetFilterData()))
			return 1.0f;

		// get the parent body
		b2Body* body = fixture->GetBody();

		// get the collidable identifier
		unsigned int targetId = reinterpret_cast<unsigned int>(body->GetUserData());

		// skip self
		if (targetId == mSkipId)
			return 1.0f;

		// update hit entity identifier
		mHitId = reinterpret_cast<unsigned int>(body->GetUserData());

		// update hit fixture
		mHitFixture = fixture;

		// update hit location
		mHitPoint = point;
		mHitNormal = normal;
		mHitFraction = fraction;

		// get closest
		return fraction;
	}
};

unsigned int Collidable::TestSegment(const b2Vec2 &aStart, const b2Vec2 &aEnd, const b2Filter &aFilter, unsigned int aId,
									 float &aLambda, b2Vec2 &aNormal, b2Fixture *&aShape)
{
	// raycast
	CollidableRayCast raycast(aFilter, aId);
	world->RayCast(&raycast, aStart, aEnd);

	// return result
	aLambda = raycast.mHitFraction;
	aNormal = raycast.mHitNormal;
	return raycast.mHitId;
}

void Collidable::QueryAABB(b2QueryCallback* callback, const b2AABB& aabb)
{
	world->QueryAABB(callback, aabb);
}