#pragma once

class Updatable
{
private:
	// list of all updatables
	typedef fastdelegate::FastDelegate<void (float)> Entry;
	typedef std::list<Entry> List;
	static List sAll;

	// list entry
	List::iterator entry;

protected:
	// identifier
	unsigned int id;

public:
	Updatable(unsigned int aId);
	virtual ~Updatable(void);

	// configure
	virtual bool Configure(const TiXmlElement *element) { return false; }

	// update
	static void UpdateAll(float aStep);
	virtual void Update(float aStep) = 0;
};
