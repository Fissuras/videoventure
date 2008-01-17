#pragma once

class Simulatable
{
private:
	// list of all simulatables
	typedef fastdelegate::FastDelegate<void (float)> Entry;
	typedef std::list<Entry> List;
	static List sAll;
	typedef std::deque<List::iterator> Remove;
	static Remove sRemove;

	// list entry
	List::iterator entry;

protected:
	// identifier
	unsigned int id;

public:
	Simulatable(unsigned int aId);
	virtual ~Simulatable(void);

	// activate
	void Activate(void);
	void Deactivate(void);

	// configure
	virtual bool Configure(const TiXmlElement *element) { return false; }

	// simulate
	static void SimulateAll(float aStep);
	virtual void Simulate(float aStep) = 0;
};