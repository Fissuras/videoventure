#include "StdAfx.h"
#include "Resource.h"
#include "Entity.h"
#include "Updatable.h"
#include "Link.h"


#ifdef USE_POOL_ALLOCATOR
// resource pool
static boost::pool<boost::default_user_allocator_malloc_free> pool(sizeof(Resource));
void *Resource::operator new(size_t aSize)
{
	return pool.malloc();
}
void Resource::operator delete(void *aPtr)
{
	pool.free(aPtr);
}
#endif


namespace Database
{
	Typed<Typed<ResourceTemplate> > resourcetemplate(0x79aa609b /* "resourcetemplate" */);
	Typed<Typed<Resource *> > resource(0x29df7ff5 /* "resource" */);
	Typed<Typed<Typed<std::vector<unsigned int> > > > resourceaction(0x7013b58f /* "resourceaction" */);
	Typed<Typed<Typed<Resource::ChangeListener> > > resourcechangelistener(0x80d66669 /* "resourcechangelistener" */);
	Typed<Typed<Typed<Resource::EmptyListener> > > resourceemptylistener(0xc5325c82 /* "resourceemptylistener" */);
	Typed<Typed<Typed<Resource::FullListener> > > resourcefulllistener(0xa4f2734c /* "resourcefulllistener" */);

	namespace Loader
	{
		class ResourceLoader
		{
		public:
			ResourceLoader()
			{
				AddConfigure(0x29df7ff5 /* "resource" */, Entry(this, &ResourceLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
			{
				Database::Typed<ResourceTemplate> &resources = Database::resourcetemplate.Open(aId);
				unsigned int aSubId = Hash(element->Attribute("name"));
				Database::name.Put(aSubId, element->Attribute("name"));
				ResourceTemplate &resource = resources.Open(aSubId);
				resource.Configure(element, aId, aSubId);
				resources.Close(aSubId);
				Database::resourcetemplate.Close(aId);
			}
		}
		resourceloader;
	}

	namespace Initializer
	{
		class ResourceInitializer
		{
		public:
			ResourceInitializer()
			{
				AddActivate(0x79aa609b /* "resourcetemplate" */, Entry(this, &ResourceInitializer::Activate));
				AddDeactivate(0x79aa609b /* "resourcetemplate" */, Entry(this, &ResourceInitializer::Deactivate));
			}

			void Activate(unsigned int aId)
			{
				Typed<Resource *> &resources = Database::resource.Open(aId);
				for (Typed<ResourceTemplate>::Iterator itor(Database::resourcetemplate.Find(aId)); itor.IsValid(); ++itor)
				{
					const ResourceTemplate &resourcetemplate = itor.GetValue();
					Resource *resource = new Resource(resourcetemplate, aId);
					resources.Put(itor.GetKey(), resource);
				}
				resources.Close(aId);
			}

			void Deactivate(unsigned int aId)
			{
				for (Typed<Resource *>::Iterator itor(Database::resource.Find(aId)); itor.IsValid(); ++itor)
				{
					Resource *resource = itor.GetValue();
					delete resource;
				}
				Database::resource.Delete(aId);
				Database::resourcechangelistener.Delete(aId);
				Database::resourceemptylistener.Delete(aId);
				Database::resourcefulllistener.Delete(aId);
			}
		}
		resourceinitializer;
	}
}


ResourceTemplate::ResourceTemplate(void)
: mSubId(0)
, mInitial(0)
, mMaximum(FLT_MAX)
, mDelay(0)
, mCycle(0)
, mAdd(0)
{
}

ResourceTemplate::~ResourceTemplate(void)
{
}

#if 0
void ResourceChangeActions(unsigned int aId, unsigned int aSubId, unsigned int aSourceId, float aValue)
{
	const std::vector<unsigned int> &action = Database::resourceaction.Get(aId).Get(aSubId).Get(0x729d01bd /* "change" */);
	if (action.size() > 0)
		ExecuteActionItems(&action[0], action.size(), aId);
}

void ResourceEmptyActions(unsigned int aId, unsigned int aSubId, unsigned int aSourceId)
{
	const std::vector<unsigned int> &action = Database::resourceaction.Get(aId).Get(aSubId).Get(0x18a7beee /* "empty" */);
	if (action.size() > 0)
		ExecuteActionItems(&action[0], action.size(), aId);
}

void ResourceFullActions(unsigned int aId, unsigned int aSubId, unsigned int aSourceId)
{
	const std::vector<unsigned int> &action = Database::resourceaction.Get(aId).Get(aSubId).Get(0xff79b33c /* "full" */);
	if (action.size() > 0)
		ExecuteActionItems(&action[0], action.size(), aId);
}
#endif

void ProcessResourceChange(const TiXmlElement *element, unsigned int aId, unsigned int aSubId)
{
#if 0
	ProcessActions(element, aId, aSubId, 0x729d01bd /* "change" */);

	Database::Typed<Database::Typed<Resource::ChangeListener> > &listenerss = Database::resourcechangelistener.Open(aId);
	Database::Typed<Resource::ChangeListener> &listeners = listenerss.Open(aSubId);

	Resource::ChangeListener &listener = listeners.Open(0xb5b54664 /* "actions" */);
	listener.bind(ResourceChangeActions);
	listeners.Close(0xb5b54664 /* "actions" */);

	listenerss.Close(aSubId);
	Database::resourcechangelistener.Close(aId);
#endif
}

void ProcessResourceEmpty(const TiXmlElement *element, unsigned int aId, unsigned int aSubId)
{
#if 0
	ProcessActions(element, aId, aSubId, 0x18a7beee /* "empty" */);

	Database::Typed<Database::Typed<Resource::EmptyListener> > &listenerss = Database::resourceemptylistener.Open(aId);
	Database::Typed<Resource::EmptyListener> &listeners = listenerss.Open(aSubId);

	Resource::EmptyListener &listener = listeners.Open(0xb5b54664 /* "actions" */);
	listener.bind(ResourceEmptyActions);
	listeners.Close(0xb5b54664 /* "actions" */);

	listenerss.Close(aSubId);
	Database::resourceemptylistener.Close(aId);
#endif
}

void ProcessResourceFull(const TiXmlElement *element, unsigned int aId, unsigned int aSubId)
{
#if 0
	ProcessActions(element, aId, aSubId, 0xff79b33c /* "full" */);

	Database::Typed<Database::Typed<Resource::FullListener> > &listenerss = Database::resourcefulllistener.Open(aId);
	Database::Typed<Resource::FullListener> &listeners = listenerss.Open(aSubId);

	Resource::FullListener &listener = listeners.Open(0xb5b54664 /* "actions" */);
	listener.bind(ResourceFullActions);
	listeners.Close(0xb5b54664 /* "actions" */);

	listenerss.Close(aSubId);
	Database::resourcefulllistener.Close(aId);
#endif
}

bool ResourceTemplate::Configure(const TiXmlElement *element, unsigned int aId, unsigned int aSubId)
{
	mSubId = aSubId;

	element->QueryFloatAttribute("initial", &mInitial);
	element->QueryFloatAttribute("maximum", &mMaximum);
	element->QueryFloatAttribute("delay", &mDelay);
	element->QueryFloatAttribute("cycle", &mCycle);
	element->QueryFloatAttribute("add", &mAdd);
	if (element->QueryFloatAttribute("rate", &mAdd) == TIXML_SUCCESS)
		mAdd *= mCycle;

	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
	{
		switch(Hash(child->Value()))
		{
		case 0xcba76728 /* "onchange" */:
			ProcessResourceChange(element, aId, aSubId);
			break;

		case 0xb9bd5041 /* "onempty" */:
			ProcessResourceEmpty(element, aId, aSubId);
			break;

		case 0x12a7aef5 /* "onfull" */:
			ProcessResourceFull(element, aId, aSubId);
			break;

		default:
			break;
		}
	}

	return true;
}


Resource::Resource(void)
: Updatable(0)
, mSubId(0)
, mValue(0)
, mTimer(0)
{
}

Resource::Resource(const ResourceTemplate &aTemplate, unsigned int aId)
: Updatable(aId)
, mSubId(aTemplate.mSubId)
, mValue(aTemplate.mInitial)
, mTimer(0)
{
	if (aTemplate.mAdd)
	{
		Updatable::SetAction(Updatable::Action(this, &Resource::Update));
	}
}

Resource::~Resource(void)
{
}

void Resource::Update(float aStep)
{
	mTimer -= aStep;
	if (mTimer <= 0)
	{
		const ResourceTemplate &resource = Database::resourcetemplate.Get(mId).Get(mSubId);
		Add(mId, resource.mAdd);
		mTimer += resource.mCycle;
	}
}

void Resource::Set(unsigned int aSourceId, float aValue)
{
	const ResourceTemplate &resource = Database::resourcetemplate.Get(mId).Get(mSubId);

	// if empty...
	if (aValue <= 0)
	{
		aValue = 0;

		// notify all empty listeners
		for (Database::Typed<EmptyListener>::Iterator itor(Database::resourceemptylistener.Get(mId).Find(mSubId)); itor.IsValid(); ++itor)
		{
			itor.GetValue()(mId, mSubId, aSourceId);
		}

		if (resource.mAdd < 0)
			Deactivate();
	}

	// if full...
	if (aValue >= resource.mMaximum)
	{
		aValue = resource.mMaximum;
		
		// notify all full listeners
		for (Database::Typed<FullListener>::Iterator itor(Database::resourcefulllistener.Get(mId).Find(mSubId)); itor.IsValid(); ++itor)
		{
			itor.GetValue()(mId, mSubId, aSourceId);
		}

		if (resource.mAdd > 0)
			Deactivate();
	}

	if (mValue != aValue)
	{
		// if something dropped the value...
		if ((mValue - aValue) * (resource.mAdd) > 0)
		{
			// reset timer
			mTimer = Database::resourcetemplate.Get(mId).Get(mSubId).mDelay;
			Activate();
		}

		DebugPrint("\"%s\" resource=\"%s\" value=%f->%f\n", Database::name.Get(mId).c_str(), Database::name.Get(mSubId).c_str(), mValue, aValue);

		// update the value
		mValue = aValue;

		// notify all change listeners
		for (Database::Typed<ChangeListener>::Iterator itor(Database::resourcechangelistener.Get(mId).Find(mSubId)); itor.IsValid(); ++itor)
		{
			itor.GetValue()(mId, mSubId, aSourceId, aValue);
		}
	}
}

void Resource::Add(unsigned int aSourceId, float aAdd)
{
	// set the value
	Set(aSourceId, mValue + aAdd);
}



unsigned int FindResource(unsigned int aId, unsigned int aSubId)
{
	// skip unnamed resource
	if (!aSubId)
		return 0;

	// for each entry in the backlink chain...
	for (unsigned int id = aId; id; id = Database::backlink.Get(id))
	{
		// if the entry has a matching resource...
		if (Database::resourcetemplate.Get(id).Find(aSubId))
		{
			// return the entry
			return id;
		}
	}

	// get the owner (player)
	unsigned int owner = Database::owner.Get(aId);

	// if the owner has a matching resource...
	if (Database::resourcetemplate.Get(owner).Find(aSubId))
	{
		// return the owner
		return owner;
	}

	// not found
	return 0;
}