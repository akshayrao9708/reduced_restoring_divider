#pragma once
#include <map>
#include <string>
using namespace std;

namespace vp
{
	class Node;
	class Circuit;
	class Edge;
	class EdgePointer
	{
	public:
		//friend class Connection;
		friend class Node;
		friend class Circuit;
		friend class main;
		typedef map<string, Edge>::iterator EdgeItr;
		EdgePointer(EdgeItr);
		EdgePointer();
		Edge& operator*() const;
		Edge* operator->() const;
		EdgeItr it;
	};
}
