#include "StdAfx.h"
#include "Cloud.h"
#include "Drawlist.h"


namespace Database
{
	namespace Loader
	{
		class CloudLoader
		{
		public:
			CloudLoader()
			{
				AddConfigure(0x74e9dbae /* "cloud" */, Entry(this, &CloudLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
			{
				int count = 1;
				element->QueryIntAttribute("count", &count);
				float mean = 256;
				element->QueryFloatAttribute("mean", &mean);
				float variance = 192;
				element->QueryFloatAttribute("variance", &variance);
				GLuint handle = CreateCloudDrawList(count, mean, variance);

				// get the list name
				const char *name = element->Attribute("name");
				if (name)
				{
					// register the draw list
					Database::drawlist.Put(Hash(name), handle);
				}
			}
		}
		cloudloader;
	}

}

inline float rand_float()
{
	return (float)rand() * (1.0f / (float)RAND_MAX);
}

GLuint CreateCloudDrawList(int aCount, float aMean, float aVariance)
{
	// create a new draw list
	GLuint handle = glGenLists(1);
	glNewList(handle, GL_COMPILE);

	// begin primitive
	glBegin( GL_QUADS );

	// for each cloud...
	for (int i = 0; i < aCount; i++)
	{
		// randomize position
		float x = ARENA_X_MIN + rand_float() * (ARENA_X_MAX - ARENA_X_MIN);
		float y = ARENA_Y_MIN + rand_float() * (ARENA_Y_MAX - ARENA_Y_MIN);
#ifdef DRAW_FRONT_TO_BACK
		float z = 256.0f + i * (768.0f / aCount);
#else
		float z = 1.0f + (aCount - 1 - i) * (3.0f / aCount);
#endif

		// randomize size
		float w = (rand_float() - rand_float()) * aVariance + aMean;
		float h = (rand_float() - rand_float()) * aVariance + aMean;

		// randomize color
		glColor4f(
			rand_float() * 0.25f + 0.25f,
			rand_float() * 0.25f + 0.25f,
			rand_float() * 0.25f + 0.25f,
			0.75f
			);

		// submit vertices
		glVertex3f( x - w, y - h, z );
		glVertex3f( x + w, y - h, z );
		glVertex3f( x + w, y + h, z );
		glVertex3f( x - w, y + h, z );
	}

	// end primitive
	glEnd();

	// finish the draw list
	glEndList();

	// return the draw list
	return handle;
}
