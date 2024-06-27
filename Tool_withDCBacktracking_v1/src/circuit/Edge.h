#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <queue>
#include "EdgePointer.h"
#include "Node.h"
#include "Circuit.h"
#include "../simVectors/SimVector.h"
#include "../simVectors/Random.h"

using namespace std;

class SimVector;
	
namespace vp
{

	class EdgePointer;
	class Circuit;
	class Node;
	class Edge
	{
	public:

		Edge();
		Edge(string, Circuit*);
		Edge& operator=(const Edge&);
		Edge(const Edge&);
		virtual ~Edge();

		unsigned int getSource() const;
		unsigned int getDestination(unsigned int) const;
		vector <unsigned int>* getDestinations();
		unsigned int nDestinations() const;

		void setSourceNode(unsigned int);
		void addDestinationNode(unsigned int);
		void disconnect();
		
		bool operator>(const Edge &e1) const;
		bool operator<(const Edge &e1) const;
		bool operator==(const Edge &e1) const;
		bool operator!=(const Edge &e1) const;
		
		aigpp::SimVector& sim();
		void setSimVector(aigpp::SimVector& vec);

	public:
		friend class Circuit;
		friend class Node;
		string name;
		int eIndex;
		unsigned int source;
		vector<unsigned int> destinations;
		Circuit* circuit;
		
		aigpp::SimVector _sim;  // this is where the actual simVector is stored.
		
		Edge* _nEqualSimHash;  // This are all pointers used by the simTable class for getting lists of equal simValues.
    		Edge* _pEqualSimHash;
    		Edge* _nEqualSim;
    		Edge* _pEqualSim;
    		Edge* _invertedSimClass;
		
		
	};

}
