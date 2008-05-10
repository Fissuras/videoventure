#include "StdAfx.h"
#include "Collidable.h"
#include "Entity.h"
#include "Link.h"

#ifdef USE_POOL_ALLOCATOR
#include <boost/pool/pool.hpp>

// collidable pool
static boost::pool<boost::default_user_allocator_malloc_free> pool(sizeof(Collidable));
void *Collidable::operator new(size_t aSize)
{
	return pool.malloc();
}
void Collidable::operator delete(void *aPtr)
{
	pool.free(aPtr);
}

// joint pool
static const size_t jointsize = 
	std::max(sizeof(b2RevoluteJointDef),
	std::max(sizeof(b2PrismaticJointDef),
	std::max(sizeof(b2DistanceJointDef),
	std::max(sizeof(b2PulleyJointDef),
	sizeof(b2MouseJointDef)))));
static boost::pool<boost::default_user_allocator_malloc_free> jointpool(jointsize);
#endif


namespace Database
{
	Typed<CollidableTemplate> collidabletemplate(0xa7380c00 /* "collidabletemplate" */);
	Typed<Typed<b2CircleDef> > collidabletemplatecircle(0xa72cf124 /* "collidabletemplatecircle" */);
	Typed<Typed<b2PolygonDef> > collidabletemplatepolygon(0x8ce45056 /* "collidabletemplatepolygon" */);
	Typed<Collidable *> collidable(0x74e9dbae /* "collidable" */);
	Typed<Typed<Collidable::Listener> > collidablecontactadd(0x7cf2c45d /* "collidablecontactadd" */);
	Typed<Typed<Collidable::Listener> > collidablecontactremove(0x95ed5aba /* "collidablecontactremove" */);

	namespace Loader
	{
		class CollidableLoader
		{
		public:
			CollidableLoader()
			{
				AddConfigure(0x74e9dbae /* "collidable" */, Entry(this, &CollidableLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
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
				const CollidableTemplate &collidabletemplate = Database::collidabletemplate.Get(aId);
				Collidable *collidable = new Collidable(collidabletemplate, aId);
				Database::collidable.Put(aId, collidable);
				collidable->AddToWorld();
			}

			void Deactivate(unsigned int aId)
			{
				if (Collidable *collidable = Database::collidable.Get(aId))
				{
					collidable->RemoveFromWorld();
					delete collidable;
					Database::collidable.Delete(aId);
					Database::collidablecontactadd.Delete(aId);
				}
			}
		}
		collidableinitializer;
	}
}

CollidableTemplate::CollidableTemplate(void)
{
}

CollidableTemplate::CollidableTemplate(const CollidableTemplate &aTemplate)
{
	// deep-copy the joint list
	for (std::list<CollidableTemplate::JointTemplate>::const_iterator itor = aTemplate.joints.begin(); itor != aTemplate.joints.end(); ++itor)
	{
		const CollidableTemplate::JointTemplate &jointsource = *itor;
		joints.push_back(jointsource);
		JointTemplate &joint = joints.back();
		switch (jointsource.def->type)
		{
		case e_revoluteJoint:	joint.def = new(jointpool.malloc()) b2RevoluteJointDef(*static_cast<const b2RevoluteJointDef *>(jointsource.def)); break;
		case e_prismaticJoint:	joint.def = new(jointpool.malloc()) b2PrismaticJointDef(*static_cast<const b2PrismaticJointDef *>(jointsource.def)); break;
		case e_distanceJoint:	joint.def = new(jointpool.malloc()) b2DistanceJointDef(*static_cast<const b2DistanceJointDef *>(jointsource.def)); break;
		case e_pulleyJoint:		joint.def = new(jointpool.malloc()) b2PulleyJointDef(*static_cast<const b2PulleyJointDef *>(jointsource.def)); break;
		case e_mouseJoint:		joint.def = new(jointpool.malloc()) b2MouseJointDef(*static_cast<const b2MouseJointDef *>(jointsource.def)); break;
		default:				assert(false);
		}
		if (jointsource.def)
			jointsource.def->userData = &joint;
	}
}

CollidableTemplate::~CollidableTemplate(void)
{
	// free the joint list
	for (std::list<CollidableTemplate::JointTemplate>::const_iterator itor = joints.begin(); itor != joints.end(); ++itor)
	{
		const CollidableTemplate::JointTemplate &joint = *itor;
		jointpool.free(joint.def);
	}
}

bool CollidableTemplate::ProcessShapeItem(const TiXmlElement *element, b2ShapeDef &shape)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xa51be2bb /* "friction" */:
		element->QueryFloatAttribute("value", &shape.friction);
		return true;

	case 0xf59a4f8f /* "restitution" */:
		element->QueryFloatAttribute("value", &shape.restitution);
		return true;

	case 0x72b9059b /* "density" */:
		element->QueryFloatAttribute("value", &shape.density);
		return true;

	case 0xcf2f4271 /* "category" */:
		{
			int category = 0;
			element->QueryIntAttribute("value", &category);
			shape.filter.categoryBits = (category >= 0) ? (1<<category) : 0;
		}
		return true;

	case 0xe7774569 /* "mask" */:
		{
			char buf[16];
			for (int i = 0; i < 16; i++)
			{
				sprintf(buf, "bit%d", i);
				int bit = (shape.filter.maskBits & (1 << i)) != 0;
				element->QueryIntAttribute(buf, &bit);
				if (bit)
					shape.filter.maskBits |= (1 << i);
				else
					shape.filter.maskBits &= ~(1 << i);
			}
		}
		return true;

	case 0x5fb91e8c /* "group" */:
		{
			int group = shape.filter.groupIndex;
			element->QueryIntAttribute("value", &group);
			shape.filter.groupIndex = short(group);
		}
		return true;

	case 0x83b6367b /* "sensor" */:
		{
			int sensor = shape.isSensor;
			element->QueryIntAttribute("value", &sensor);
			shape.isSensor = sensor != 0;
		}
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ConfigureShape(const TiXmlElement *element, b2ShapeDef &shape)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessShapeItem(child, shape);
	}
	return true;
}

bool CollidableTemplate::ConfigureCircle(const TiXmlElement *element, b2CircleDef &shape)
{
	element->QueryFloatAttribute("radius", &shape.radius);
	ConfigureShape(element, shape);
	return true;
}

bool CollidableTemplate::ConfigureBox(const TiXmlElement *element, b2PolygonDef &shape)
{
	// half-width and half-height
	float w = 0, h = 0;
	element->QueryFloatAttribute("w", &w);
	element->QueryFloatAttribute("h", &h);

	// center and rotation
	b2Vec2 center(0.0f, 0.0f);
	float rotation = 0.0f;

	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		switch (Hash(child->Value()))
		{
		case 0x934f4e0a /* "position" */:
			element->QueryFloatAttribute("x", &center.x);
			element->QueryFloatAttribute("y", &center.y);
			if (element->QueryFloatAttribute("angle", &rotation) == TIXML_SUCCESS)
				 rotation *= float(M_PI)/180.0f;
			break;
		}
	}

	shape.SetAsBox(w, h, center, rotation);

	ConfigureShape(element, shape);
	return true;
}

bool CollidableTemplate::ProcessPolyItem(const TiXmlElement *element, b2PolygonDef &shape)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x945367a7 /* "vertex" */:
		element->QueryFloatAttribute("x", &shape.vertices[shape.vertexCount].x);
		element->QueryFloatAttribute("y", &shape.vertices[shape.vertexCount].y);
		++shape.vertexCount;
		return true;

	default:
		return ProcessShapeItem(element, shape);
	}
}

bool CollidableTemplate::ConfigurePoly(const TiXmlElement *element, b2PolygonDef &shape)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessPolyItem(child, shape);
	}
	return true;
}


bool CollidableTemplate::ProcessBodyItem(const TiXmlElement *element, b2BodyDef &body)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x934f4e0a /* "position" */:
		element->QueryFloatAttribute("x", &body.position.x);
		element->QueryFloatAttribute("y", &body.position.y);
		if (element->QueryFloatAttribute("angle", &body.angle) == TIXML_SUCCESS)
			 body.angle *= float(M_PI)/180.0f;
		return true;

	case 0xbb61b895 /* "damping" */:
		element->QueryFloatAttribute("linear", &body.linearDamping);
		element->QueryFloatAttribute("angular", &body.angularDamping);
		return true;

	case 0xac01f355 /* "allowsleep" */:
		{
			int allowsleep = body.allowSleep;
			element->QueryIntAttribute("value", &allowsleep);
			body.allowSleep = allowsleep != 0;
		}
		return true;

	case 0xa3ae0ca2 /* "startsleep" */:
		{
			int startsleep = body.isSleeping;
			element->QueryIntAttribute("value", &startsleep);
			body.isSleeping = startsleep != 0;
		}
		return true;

	case 0x7a04061b /* "fixedrotation" */:
		{
			int fixedrotation = body.fixedRotation;
			element->QueryIntAttribute("value", &fixedrotation);
			body.fixedRotation = fixedrotation != 0;
		}
		return true;

	case 0x029402af /* "fast" */:
		{
			int isBullet = body.isBullet;
			element->QueryIntAttribute("value", &isBullet);
			body.isBullet = isBullet != 0;
		}
		return true;

	case 0x28217089 /* "circle" */:
		{
			Database::Typed<b2CircleDef> &shapes = Database::collidabletemplatecircle.Open(id);
			int shapeid = shapes.GetCount()+1;
			b2CircleDef &shape = shapes.Open(shapeid);
			ConfigureCircle(element, shape);
			shapes.Close(shapeid);
			Database::collidabletemplatecircle.Close(id);
		}
		return true;

	case 0x70c67e32 /* "box" */:
		{
			Database::Typed<b2PolygonDef> &shapes = Database::collidabletemplatepolygon.Open(id);
			int shapeid = shapes.GetCount()+1;
			b2PolygonDef &shape = shapes.Open(shapeid);
			ConfigureBox(element, shape);
			Database::collidabletemplatepolygon.Close(id);
		}
		return true;

	case 0x84d6a947 /* "poly" */:
		{
			Database::Typed<b2PolygonDef> &shapes = Database::collidabletemplatepolygon.Open(id);
			int shapeid = shapes.GetCount()+1;
			b2PolygonDef &shape = shapes.Open(shapeid);
			ConfigurePoly(element, shape);
			Database::collidabletemplatepolygon.Close(id);
		}
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ConfigureBody(const TiXmlElement *element, b2BodyDef &body)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessBodyItem(child, body);
	}
	return true;
}

bool CollidableTemplate::ProcessJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2JointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x115ce60c /* "body1" */:
		{
			const char *name = element->Attribute("name");
			jointtemplate.name1 = name ? Hash(name) : 0;
		}
		return true;

	case 0x145ceac5 /* "body2" */:
		{
			const char *name = element->Attribute("name");
			jointtemplate.name2 = name ? Hash(name) : 0;
		}
		return true;

	case 0x2c5d8028 /* "collideconnected" */:
		{
			int collide = joint.collideConnected;
			element->QueryIntAttribute("value", &collide);
			joint.collideConnected = collide != 0;
		}
		return true;

	default:
		return false;
	}
}

bool CollidableTemplate::ProcessRevoluteJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2RevoluteJointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xe155cf5f /* "anchor1" */:
		element->QueryFloatAttribute("x", &joint.localAnchor1.x);
		element->QueryFloatAttribute("y", &joint.localAnchor1.y);
		return true;

	case 0xe255d0f2 /* "anchor2" */:
		element->QueryFloatAttribute("x", &joint.localAnchor2.x);
		element->QueryFloatAttribute("y", &joint.localAnchor2.y);
		return true;

	case 0xad544418 /* "angle" */:
		if (element->QueryFloatAttribute("value", &joint.referenceAngle) == TIXML_SUCCESS)
			joint.referenceAngle *= float(M_PI)/180.0f;
		return true;

	case 0x32dad934 /* "limit" */:
		if (element->QueryFloatAttribute("lower", &joint.lowerAngle) == TIXML_SUCCESS)
			joint.lowerAngle *= float(M_PI) / 180.0f;
		if (element->QueryFloatAttribute("upper", &joint.upperAngle) == TIXML_SUCCESS)
			joint.upperAngle *= float(M_PI) / 180.0f;
		joint.enableLimit = true;
		return true;

	case 0xcaf08472 /* "motor" */:
		element->QueryFloatAttribute("torque", &joint.maxMotorTorque);
		if (element->QueryFloatAttribute("speed", &joint.motorSpeed) == TIXML_SUCCESS)
			joint.motorSpeed *= float(M_PI) / 180.0f;
		joint.enableMotor = true;
		return true;

	default:
		return ProcessJointItem(element, jointtemplate, joint);
	}
}

bool CollidableTemplate::ConfigureRevoluteJoint(const TiXmlElement *element, JointTemplate &jointtemplate, b2RevoluteJointDef &joint)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessRevoluteJointItem(child, jointtemplate, joint);
	}
	return true;
}

bool CollidableTemplate::ProcessPrismaticJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2PrismaticJointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xe155cf5f /* "anchor1" */:
		element->QueryFloatAttribute("x", &joint.localAnchor1.x);
		element->QueryFloatAttribute("y", &joint.localAnchor1.y);
		return true;

	case 0xe255d0f2 /* "anchor2" */:
		element->QueryFloatAttribute("x", &joint.localAnchor2.x);
		element->QueryFloatAttribute("y", &joint.localAnchor2.y);
		return true;

	case 0x6d2badf4 /* "axis" */:
		element->QueryFloatAttribute("x", &joint.localAxis1.x);
		element->QueryFloatAttribute("y", &joint.localAxis1.y);
		return true;

	case 0xad544418 /* "angle" */:
		if (element->QueryFloatAttribute("value", &joint.referenceAngle) == TIXML_SUCCESS)
			joint.referenceAngle *= float(M_PI)/180.0f;
		return true;

	case 0x32dad934 /* "limit" */:
		element->QueryFloatAttribute("lower", &joint.lowerTranslation);
		element->QueryFloatAttribute("upper", &joint.upperTranslation);
		joint.enableLimit = true;
		return true;

	case 0xcaf08472 /* "motor" */:
		element->QueryFloatAttribute("force", &joint.maxMotorForce);
		element->QueryFloatAttribute("speed", &joint.motorSpeed);
		joint.enableMotor = true;
		return true;

	default:
		return ProcessJointItem(element, jointtemplate, joint);
	}
}

bool CollidableTemplate::ConfigurePrismaticJoint(const TiXmlElement *element, JointTemplate &jointtemplate, b2PrismaticJointDef &joint)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessPrismaticJointItem(child, jointtemplate, joint);
	}
	return true;
}

bool CollidableTemplate::ProcessDistanceJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2DistanceJointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xe155cf5f /* "anchor1" */:
		element->QueryFloatAttribute("x", &joint.localAnchor1.x);
		element->QueryFloatAttribute("y", &joint.localAnchor1.y);
		return true;

	case 0xe255d0f2 /* "anchor2" */:
		element->QueryFloatAttribute("x", &joint.localAnchor2.x);
		element->QueryFloatAttribute("y", &joint.localAnchor2.y);
		return true;

	default:
		return ProcessJointItem(element, jointtemplate, joint);
	}
}

bool CollidableTemplate::ConfigureDistanceJoint(const TiXmlElement *element, JointTemplate &jointtemplate, b2DistanceJointDef &joint)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessDistanceJointItem(child, jointtemplate, joint);
	}
	return true;
}

bool CollidableTemplate::ProcessPulleyJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2PulleyJointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0xe1acc15d /* "ground1" */:
		element->QueryFloatAttribute("x", &joint.groundAnchor1.x);
		element->QueryFloatAttribute("y", &joint.groundAnchor1.y);
		return true;

	case 0xdeacbca4 /* "ground2" */:
		element->QueryFloatAttribute("x", &joint.groundAnchor2.x);
		element->QueryFloatAttribute("y", &joint.groundAnchor2.y);
		return true;

	case 0xe155cf5f /* "anchor1" */:
		element->QueryFloatAttribute("x", &joint.localAnchor1.x);
		element->QueryFloatAttribute("y", &joint.localAnchor1.y);
		return true;

	case 0xe255d0f2 /* "anchor2" */:
		element->QueryFloatAttribute("x", &joint.localAnchor2.x);
		element->QueryFloatAttribute("y", &joint.localAnchor2.y);
		return true;

	case 0xa4c53aac /* "length1" */:
		element->QueryFloatAttribute("max", &joint.maxLength1);
		return true;

	case 0xa7c53f65 /* "length2" */:
		element->QueryFloatAttribute("max", &joint.maxLength2);
		return true;

	case 0xc1121e84 /* "ratio" */:
		element->QueryFloatAttribute("value", &joint.ratio);
		return true;

	default:
		return ProcessJointItem(element, jointtemplate, joint);
	}
}

bool CollidableTemplate::ConfigurePulleyJoint(const TiXmlElement *element, JointTemplate &jointtemplate, b2PulleyJointDef &joint)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessPulleyJointItem(child, jointtemplate, joint);
	}
	return true;
}

bool CollidableTemplate::ProcessMouseJointItem(const TiXmlElement *element, JointTemplate &jointtemplate, b2MouseJointDef &joint)
{
	const char *name = element->Value();
	switch (Hash(name))
	{
	case 0x32608848 /* "target" */:
		element->QueryFloatAttribute("x", &joint.target.x);
		element->QueryFloatAttribute("y", &joint.target.y);
		return true;

	case 0x79a98884 /* "force" */:
		element->QueryFloatAttribute("max", &joint.maxForce);
		return true;

	case 0x78e63274 /* "tuning" */:
		element->QueryFloatAttribute("frequency", &joint.frequencyHz);
		element->QueryFloatAttribute("damping", &joint.dampingRatio);
		element->QueryFloatAttribute("timestep", &joint.timeStep);
		return true;

	default:
		return ProcessJointItem(element, jointtemplate, joint);
	}
}

bool CollidableTemplate::ConfigureMouseJoint(const TiXmlElement *element, JointTemplate &jointtemplate, b2MouseJointDef &joint)
{
	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessMouseJointItem(child, jointtemplate, joint);
	}
	return true;
}


bool CollidableTemplate::Configure(const TiXmlElement *element, unsigned int id)
{
	if (Hash(element->Value()) != 0x74e9dbae /* "collidable" */)
		return false;

	// save identifier
	this->id = id;

	// process child elements
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		const char *name = child->Value();
		switch (Hash(name))
		{
		case 0xdbaa7975 /* "body" */:
			{
				// set up the collidable body
				CollidableTemplate::ConfigureBody(child, bodydef);
			}
			break;

		case 0xef2f9539 /* "revolutejoint" */:
			{
				b2RevoluteJointDef &joint = *new(jointpool.malloc()) b2RevoluteJointDef();
				joints.push_back(JointTemplate(&joint));
				CollidableTemplate::JointTemplate &jointtemplate = joints.back();
				joint.userData = &jointtemplate;
				CollidableTemplate::ConfigureRevoluteJoint(child, jointtemplate, joint);
			}
			break;

		case 0x4954853d /* "prismaticjoint" */:
			{
				b2PrismaticJointDef &joint = *new(jointpool.malloc()) b2PrismaticJointDef();
				joints.push_back(JointTemplate(&joint));
				CollidableTemplate::JointTemplate &jointtemplate = joints.back();
				joint.userData = &jointtemplate;
				CollidableTemplate::ConfigurePrismaticJoint(child, jointtemplate, joint);
			}
			break;

		case 0x6932d1ee /* "distancejoint" */:
			{
				b2DistanceJointDef &joint = *new(jointpool.malloc()) b2DistanceJointDef();
				joints.push_back(JointTemplate(&joint));
				CollidableTemplate::JointTemplate &jointtemplate = joints.back();
				joint.userData = &jointtemplate;
				CollidableTemplate::ConfigureDistanceJoint(child, jointtemplate, joint);
			}
			break;

		case 0xdd003dc4 /* "pulleyjoint" */:
			{
				b2PulleyJointDef &joint = *new(jointpool.malloc()) b2PulleyJointDef();
				joints.push_back(JointTemplate(&joint));
				CollidableTemplate::JointTemplate &jointtemplate = joints.back();
				joint.userData = &jointtemplate;
				CollidableTemplate::ConfigurePulleyJoint(child, jointtemplate, joint);
			}
			break;

		case 0xc3b5cf50 /* "mousejoint" */:
			{
				b2MouseJointDef &joint = *new(jointpool.malloc()) b2MouseJointDef();
				joints.push_back(JointTemplate(&joint));
				CollidableTemplate::JointTemplate &jointtemplate = joints.back();
				joint.userData = &jointtemplate;
				CollidableTemplate::ConfigureMouseJoint(child, jointtemplate, joint);
			}
			break;

		case 0x19a3586a /* "gearjoint" */:
			{
			}
			break;
		}
	}

	return true;
}

bool CollidableTemplate::SetupLinkJoint(const LinkTemplate &linktemplate, unsigned int aId, unsigned int aSecondary)
{
	// add a revolute joint to the linked template (HACK)
	b2RevoluteJointDef &joint = *new(jointpool.malloc()) b2RevoluteJointDef();
	joints.push_back(JointTemplate(&joint));
	JointTemplate &jointtemplate = joints.back();
	
	// configure the joint template
	jointtemplate.name1 = aId;
	jointtemplate.name2 = aSecondary;

	// configure the joint definition
	joint.userData = &jointtemplate;
	joint.localAnchor1.Set(linktemplate.mOffset.p.x, linktemplate.mOffset.p.y);
	joint.localAnchor2.Set(0, 0);
	joint.referenceAngle = linktemplate.mOffset.Angle();
	if (linktemplate.mUpdateAngle)
	{
		joint.lowerAngle = 0.0f;
		joint.upperAngle = 0.0f;
		joint.enableLimit = true;
	}
	return true;
}

bool CollidableTemplate::SetupJointDef(b2JointDef &joint, const JointTemplate &jointtemplate, unsigned int id) const
{
	unsigned int id1 = jointtemplate.name1 == 0xe3736e9a /* "backlink" */ ? Database::backlink.Get(id) : jointtemplate.name1;
	joint.userData = NULL;
	Collidable *coll1 = Database::collidable.Get(id1 ? id1 : id);
	if (!coll1)
		return false;
	joint.body1 = coll1->GetBody();
	if (!joint.body1)
		return false;
	unsigned int id2 = jointtemplate.name2 == 0xe3736e9a /* "backlink" */ ? Database::backlink.Get(id) : jointtemplate.name2;
	Collidable *coll2 = Database::collidable.Get(id2 ? id2 : id);
	if (!coll2)
		return false;
	joint.body2 = coll2->GetBody();
	if (!joint.body2)
		return false;
	return true;
}


b2World* Collidable::world;

/// Implement this class to get collision results. You can use these results for
/// things like sounds and game logic. You can also use this class to tweak contact 
/// settings. These tweaks persist until you tweak the settings again or the contact
/// is destroyed.
/// @warning You cannot create/destroy Box2D entities inside these callbacks.
class ContactListener : public b2ContactListener
{
public:

	/// Called when a contact point is added. This includes the geometry
	/// and the forces.
	virtual void Add(const b2ContactPoint* point)
	{
		b2Shape *shape1 = point->shape1;
		b2Shape *shape2 = point->shape2;
		Database::Key id1 = reinterpret_cast<Database::Key>(shape1->GetUserData());
		Database::Key id2 = reinterpret_cast<Database::Key>(shape2->GetUserData());
		for (Database::Typed<Collidable::Listener>::Iterator itor(Database::collidablecontactadd.Find(id1)); itor.IsValid(); ++itor)
			itor.GetValue()(id1, id2, 0.0f /*shape1->GetBody()->m_sweep.t0*/, *point);
		for (Database::Typed<Collidable::Listener>::Iterator itor(Database::collidablecontactadd.Find(id2)); itor.IsValid(); ++itor)
			itor.GetValue()(id2, id1, 0.0f /*shape2->GetBody()->m_sweep.t0*/, *point);
	};

	/// Called when a contact point persists. This includes the geometry
	/// and the forces.
	virtual void Persist(const b2ContactPoint* point)
	{
	}

	/// Called when a contact point is removed. This includes the last
	/// computed geometry and forces.
	virtual void Remove(const b2ContactPoint* point)
	{
		b2Shape *shape1 = point->shape1;
		b2Shape *shape2 = point->shape2;
		Database::Key id1 = reinterpret_cast<Database::Key>(shape1->GetUserData());
		Database::Key id2 = reinterpret_cast<Database::Key>(shape2->GetUserData());
		for (Database::Typed<Collidable::Listener>::Iterator itor(Database::collidablecontactremove.Find(id1)); itor.IsValid(); ++itor)
			itor.GetValue()(id1, id2, 0.0f /*shape1->GetBody()->m_sweep.t0*/, *point);
		for (Database::Typed<Collidable::Listener>::Iterator itor(Database::collidablecontactremove.Find(id2)); itor.IsValid(); ++itor)
			itor.GetValue()(id2, id1, 0.0f /*shape2->GetBody()->m_sweep.t0*/, *point);
	}
}
contactListener;


#ifdef COLLIDABLE_DEBUG_DRAW
// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
class DebugDraw : public b2DebugDraw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	void DrawPoint(const b2Vec2& p, const b2Color& color);
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawAxis(const b2Vec2& point, const b2Vec2& axis, const b2Color& color);
	void DrawXForm(const b2XForm& xf);
	void DrawForce(const b2Vec2& point, const b2Vec2& force, const b2Color& color);
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

void DebugDraw::DrawPoint(const b2Vec2& p, const b2Color& color)
{
	glColor3f(color.r, color.g, color.b);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glVertex2f(p.x, p.y);
	glEnd();
	glPointSize(1.0f);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	glColor3f(color.r, color.g, color.b);
	glBegin(GL_LINES);
	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);
	glEnd();
}

void DebugDraw::DrawAxis(const b2Vec2& point, const b2Vec2& axis, const b2Color& color)
{
	const float32 k_axisScale = 0.3f;
	b2Vec2 p1 = point;
	b2Vec2 p2 = point + k_axisScale	* axis;
	DrawSegment(p1, p2, color);
}

void DebugDraw::DrawXForm(const b2XForm& xf)
{
	b2Vec2 p1 = xf.position, p2;
	const float32 k_axisScale = 0.4f;
	glBegin(GL_LINES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.R.col1;
	glVertex2f(p2.x, p2.y);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(p1.x, p1.y);
	p2 = p1 + k_axisScale * xf.R.col2;
	glVertex2f(p2.x, p2.y);

	glEnd();
}

void DebugDraw::DrawForce(const b2Vec2& point, const b2Vec2& force, const b2Color& color)
{
	const float32 k_forceScale = 0.1f;
	b2Vec2 p1 = point;
	b2Vec2 p2 = point + k_forceScale * force;
	DrawSegment(p1, p2, color);
}
#endif


Collidable::Collidable(void)
: id(0), body(NULL) 
{
}

Collidable::Collidable(const CollidableTemplate &aTemplate, unsigned int aId)
: id(aId), body(NULL)
{
}

Collidable::~Collidable(void)
{
}

void Collidable::AddToWorld(void)
{
	const CollidableTemplate &collidable = Database::collidabletemplate.Get(id);

	// copy the body definition
	b2BodyDef def(collidable.bodydef);

	// set userdata identifier and body position to entity (HACK)
	def.userData = reinterpret_cast<void *>(id);
	const Entity *entity = Database::entity.Get(id);
	if (entity)
	{
		const Matrix2 &transform = entity->GetTransform();
		def.angle = transform.Angle();
		def.position = transform.p;
	}

	// create the body
	body = world->CreateBody(&def);

	// add shapes
	const Database::Typed<b2CircleDef> &circles = Database::collidabletemplatecircle.Get(id);
	for (Database::Typed<b2CircleDef>::Iterator circleitor(&circles); circleitor.IsValid(); ++circleitor)
	{
		b2CircleDef circle(circleitor.GetValue());
		circle.userData = reinterpret_cast<void *>(id);
		body->CreateShape(&circle);
	}
	const Database::Typed<b2PolygonDef> &polygons = Database::collidabletemplatepolygon.Get(id);
	for (Database::Typed<b2PolygonDef>::Iterator polygonitor(&polygons); polygonitor.IsValid(); ++polygonitor)
	{
		b2PolygonDef polygon(polygonitor.GetValue());
		polygon.userData = reinterpret_cast<void *>(id);
		body->CreateShape(&polygon);
	}

	// compute mass
	body->SetMassFromShapes();

	if (entity)
	{
		body->SetLinearVelocity(entity->GetVelocity());
		body->SetAngularVelocity(entity->GetOmega());
	}

	// for each joint
	for (std::list<CollidableTemplate::JointTemplate>::const_iterator itor = collidable.joints.begin(); itor != collidable.joints.end(); ++itor)
	{
		const CollidableTemplate::JointTemplate &joint = *itor;
		if (collidable.SetupJointDef(*joint.def, joint, id))
		{
			world->CreateJoint(joint.def);
		}
	}
}

void Collidable::RemoveFromWorld(void)
{
	world->DestroyBody(body);
	body = NULL;
}



// create collision world
void Collidable::WorldInit(void)
{
	// physics world
	b2AABB worldAABB;
	worldAABB.lowerBound.Set(ARENA_X_MIN - 32, ARENA_Y_MIN - 32);
	worldAABB.upperBound.Set(ARENA_X_MAX + 32, ARENA_Y_MAX + 32);
	b2Vec2 gravity;
	gravity.Set(0.0f, 0.0f);
	bool doSleep = true;
	world = new b2World(worldAABB, gravity, doSleep);

	// set contact listener
	world->SetContactListener(&contactListener);

#ifdef COLLIDABLE_DEBUG_DRAW
	// set debug render
	world->SetDebugDraw(&debugDraw);
	debugDraw.SetFlags(-1);
#endif

	// create perimeter walls
	b2BodyDef bodydef;
	b2Body *body = world->CreateBody(&bodydef);

	b2PolygonDef top;
	top.SetAsBox(0.5f * (ARENA_X_MAX - ARENA_X_MIN) + 32, 16, b2Vec2(0.5f * (ARENA_X_MAX + ARENA_X_MIN), ARENA_Y_MIN - 16), 0);
	body->CreateShape(&top);

	b2PolygonDef bottom;
	bottom.SetAsBox(0.5f * (ARENA_X_MAX - ARENA_X_MIN) + 32, 16, b2Vec2(0.5f * (ARENA_X_MAX + ARENA_X_MIN), ARENA_Y_MAX + 16), 0);
	body->CreateShape(&bottom);

	b2PolygonDef left;
	left.SetAsBox(16, 0.5f * (ARENA_Y_MAX - ARENA_Y_MIN) + 32, b2Vec2(ARENA_X_MIN - 16, 0.5f * (ARENA_Y_MAX + ARENA_Y_MIN)), 0);
	body->CreateShape(&left);

	b2PolygonDef right;
	right.SetAsBox(16, 0.5f * (ARENA_Y_MAX - ARENA_Y_MIN) + 32, b2Vec2(ARENA_X_MAX + 16, 0.5f * (ARENA_Y_MAX + ARENA_Y_MIN)), 0);
	body->CreateShape(&right);

	body->SetMassFromShapes();
}

void Collidable::WorldDone(void)
{
	delete world;
}

void Collidable::CollideAll(float aStep)
{
	world->Step(aStep, 16);
	world->Validate();

	// for each body...
	for (b2Body* body = world->GetBodyList(); body; body = body->GetNext())
	{
		// if the body is not sleeping or static...
		if (!body->IsSleeping() && !body->IsStatic())
		{
			// get the database key
			Database::Key id = reinterpret_cast<Database::Key>(body->GetUserData());

			// update the entity position (hack)
			Collidable *collidable = Database::collidable.Get(id);
			if (collidable)
			{
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
	}
}
