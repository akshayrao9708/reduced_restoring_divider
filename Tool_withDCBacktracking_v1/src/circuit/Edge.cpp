#include "Edge.h"
#include "EdgePointer.h"
#include "Node.h"
#include "Circuit.h"

namespace vp
{
	Edge::Edge()
	{
		this->circuit = NULL;
		source = 0;
		//_sim = 0; 
		_nEqualSimHash = 0;
    		_pEqualSimHash = 0;
    		_nEqualSim = 0;
    		_pEqualSim = 0;
    		_invertedSimClass = 0;
	}

	Edge::Edge(string name, Circuit* circuit)
	{
		this->name = name;
		this->circuit = circuit;
		source = -1;
		//_sim = 0; 
		_nEqualSimHash = 0;
    	_pEqualSimHash = 0;
    	_nEqualSim = 0;
    	_pEqualSim = 0;
    	_invertedSimClass = 0;
    	//lrabs::Random rnd( /*seed = */32 );
    	//aigpp::SimVector vec(rnd, _sim->size());
    	//_sim = &vec;
	}
	
	Edge::Edge(const Edge& rhs)
	{
		name = rhs.name;
		circuit = rhs.circuit;
		source = rhs.source;
		destinations = rhs.destinations;
		_sim = rhs._sim; 
		_nEqualSimHash = rhs._nEqualSimHash;
    		_pEqualSimHash = rhs._pEqualSimHash;
    		_nEqualSim = rhs._nEqualSim;
    		_pEqualSim = rhs._pEqualSim;
    		_invertedSimClass = rhs._invertedSimClass;
	}
	//*
	Edge::~Edge()
	{
		//if (_sim != 0) delete[] _sim->getBins();
		//if (_sim != 0) delete _sim;
	}
	//*/

	Edge& Edge::operator=(const Edge& rhs)
	{
		name = rhs.name;
		circuit = rhs.circuit;
		source = rhs.source;
		destinations = rhs.destinations;
		_sim = rhs._sim; 
		_nEqualSimHash = rhs._nEqualSimHash;
    		_pEqualSimHash = rhs._pEqualSimHash;
    		_nEqualSim = rhs._nEqualSim;
    		_pEqualSim = rhs._pEqualSim;
    		_invertedSimClass = rhs._invertedSimClass;
    		return *this;
	}
	
	aigpp::SimVector& Edge::sim() 
	{
		return _sim;
	}
	
	void Edge::setSimVector(aigpp::SimVector& vec) 
	{
		this->_sim = vec;
	}

	void Edge::setSourceNode(unsigned int index)
	{
		source = index;
	}

	void Edge::addDestinationNode(unsigned int index)
	{
		destinations.push_back(index);
	}

	void Edge::disconnect()
	{
		source = -1;
		destinations.clear();
	}

	unsigned int Edge::getSource() const
	{
		return source;
	}

	unsigned int Edge::getDestination(unsigned int i) const
	{
		return destinations[i];
	}

	vector <unsigned int>* Edge::getDestinations()
	{
		return &destinations;
	}

	unsigned int Edge::nDestinations() const
	{
		return destinations.size();
	}
	
	bool Edge::operator>(const Edge &e1) const {
		return this->name > e1.name;	
	}

	bool Edge::operator<(const Edge &e1) const {
		return this->name < e1.name;
	}

	bool Edge::operator==(const Edge &e1) const {
		return this->name == e1.name;
	}

	bool Edge::operator!=(const Edge &e1) const {
		return !(this->name == e1.name);
	}
}
