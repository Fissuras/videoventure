#include "StdAfx.h"
#include "World.h"
#include "Entity.h"

namespace Database
{
	namespace Loader
	{
		class ImportLoader
		{
		public:
			ImportLoader()
			{
				AddConfigure(0x112a90d4 /* "import" */, Entry(this, &ImportLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
			{
				// level configuration
				const char *name = element->Attribute("name");
				DebugPrint("Import %s\n", name);
				TiXmlDocument document(name);
				document.LoadFile();

				// process child elements
				if (const TiXmlElement *root = document.FirstChildElement())
					ProcessWorldItems(root);
			}
		}
		importloader;

		class TilemapLoader
		{
		public:
			TilemapLoader()
			{
				AddConfigure(0xbaf310c5 /* "tilemap" */, Entry(this, &TilemapLoader::Configure));
			}

			void Configure(unsigned int aid, const TiXmlElement *element)
			{
				// tilemap configuration
				float x = 0.0f, y = 0.0f;
				element->QueryFloatAttribute("x", &x);
				element->QueryFloatAttribute("y", &y);
				float dx = 1.0f, dy = 1.0f;
				element->QueryFloatAttribute("dx", &dx);
				element->QueryFloatAttribute("dy", &dy);

				// tiles
				struct Tile
				{
					unsigned int mSpawn;
					Vector2 mOffset;
					float mAngle;
				};
				Tile map[CHAR_MAX-CHAR_MIN+1];
				memset(map, 0, sizeof(map));

				// position value
				Vector2 pos(x, y);

				// process child elements
				for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
				{
					switch(Hash(child->Value()))
					{
					case 0x713a7cc9 /* "tile" */:
						{
							const char *name = child->Attribute("name");
							if (!name || !name[0])
								continue;
							Tile &tile = map[name[0]-CHAR_MIN];
							const char *spawn = child->Attribute("spawn");
							tile.mSpawn = Hash(spawn);
							child->QueryFloatAttribute("x", &tile.mOffset.x);
							child->QueryFloatAttribute("y", &tile.mOffset.y);
							if (child->QueryFloatAttribute("angle", &tile.mAngle) == TIXML_SUCCESS)
								tile.mAngle *= float(M_PI) / 180.0f;
						}
						break;

					case 0x440e1d7b /* "row" */:
						{
							pos.x = x;

							const char *text = child->Attribute("data");
							if (!text)
								text = child->GetText();
							if (!text)
								continue;

							for (const char *t = text; *t; ++t)
							{
								Tile &tile = map[*t-CHAR_MIN];
								if (tile.mSpawn)
								{
									Database::Instantiate(tile.mSpawn, 0, tile.mAngle, pos + tile.mOffset, Vector2(0, 0), 0);
								}

								pos.x += dx;
							}

							pos.y += dy;
						}
						break;

					default:
						break;
					}
				}
			}
		}
		tilemaploader;
	}
}

void ProcessWorldItem(const TiXmlElement *element)
{
	const char *value = element->Value();
	const char *name = element->Attribute("name");
	DebugPrint("Processing %s (%s)\n", element->Value(), name);

	// process world item
	const Database::Loader::Entry &configure = Database::Loader::GetConfigure(Hash(value));
	if (configure)
		configure(Hash(name), element);
	else
		DebugPrint("Unrecognized tag \"%s\"\n", value);
}

void ProcessWorldItems(const TiXmlElement *element)
{
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		ProcessWorldItem(child);
	}
}