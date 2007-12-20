#pragma once

class RenderableTemplate
{
public:
	// draw list
	GLuint mDraw;

public:
	RenderableTemplate(void);
	~RenderableTemplate();

	// configure
	bool Configure(TiXmlElement *element);
};

class Renderable
{
private:
	// list of all renderables
	typedef std::list<Renderable *> List;
	static List sAll;

	// identifier
	unsigned int id;

	// list entry
	List::iterator entry;
	bool show;

protected:
	// time offset
	static float sOffset;

	// draw list
	GLuint mDraw;

public:
	Renderable(void);
	Renderable(const RenderableTemplate &aTemplate, unsigned int aId);
	~Renderable(void);

	// visibility
	void Show(void);
	void Hide(void);

	// render
	static void RenderAll(float aRatio, float aStep);
	virtual void Render(const Matrix2 &transform);
};

namespace Database
{
	extern Typed<RenderableTemplate> renderabletemplate;
	extern Typed<Renderable *> renderable;
	extern Typed<GLuint> drawlist;
}