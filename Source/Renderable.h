#pragma once

class RenderableTemplate
{
public:
	// bounding radius
	float mRadius;

	// period
	float mPeriod;

public:
	RenderableTemplate(void);
	~RenderableTemplate();

	// configure
	bool Configure(const TiXmlElement *element, unsigned int aId);
};

class Renderable
{
public:
	typedef fastdelegate::FastDelegate<void (unsigned int, float, float, float, float)> Action;

protected:
	// identifier
	unsigned int mId;

private:
	// list of all renderables
	static Renderable *sHead;
	static Renderable *sTail;

	// linked list
	Renderable *mNext;
	Renderable *mPrev;
	bool mActive;

	// action
	Action mAction;

	// bounding radius
	float mRadius;

protected:
	// creation turn
	unsigned int mStart;
	float mFraction;

public:
#ifdef USE_POOL_ALLOCATOR
	// allocation
	void *operator new(size_t aSize);
	void operator delete(void *aPtr);
#endif

	Renderable(void);
	Renderable(const RenderableTemplate &aTemplate, unsigned int aId);
	~Renderable(void);

	// set action
	void SetAction(Action aAction)
	{
		mAction = aAction;
	}

	// visibility
	void Show(void);
	void Hide(void);

	// set fraction
	void SetFraction(float aFraction)
	{
		mFraction = aFraction;
	}
	float GetFraction(void) const
	{
		return mFraction;
	}

	// render
	static void RenderAll(const AlignedBox2 &aView);
};

// render geometry
void RenderGeometry(float aTime, unsigned int aId, float aPosX, float aPosY, float aAngle);

namespace Database
{
	extern Typed<RenderableTemplate> renderabletemplate;
	extern Typed<Renderable *> renderable;
}
