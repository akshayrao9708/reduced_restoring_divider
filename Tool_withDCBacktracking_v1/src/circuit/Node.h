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
using namespace std;

struct fullAdder;
struct Multiplexer;
namespace vp
{
	class Circuit;
	class Edge;
	/*
		Node class represents an input/output port or a gate
		*/

	class Node
	{
	public:

		enum NodeType {NONE, AND, OR, XOR, XNOR, NOT, BUFFER, INPUT_PORT, OUTPUT_PORT, DELETED};

		Circuit* circuit;
		double tRise, tFall;
		int adder = -1;
		int mux=-1;
		int block = -1;
		int nIndex;

		NodeType type;
		string name;
		int revLevel;
		void setNodeType(NodeType);
		void addOutput(EdgePointer);
		void addInput(EdgePointer);
		void assignTask(const NodeType, const string&, const string&, const string&);

		Node();
		Node(string, Circuit*);
		Node(string, Circuit*, int, int);

	public:
		friend class Circuit;
		friend class Edge;
		friend struct fullAdder;
		friend struct Multiplexer;

		vector<EdgePointer> inputs;
		vector<EdgePointer> outputs;

		Node(const Node&);
		Node& operator=(const Node&);

		string getName() const;				// Returns the input or gate name
		NodeType getType() const;				// Returns the type of the gate

		bool isInputPort() const;
		bool isOutputPort() const;
		bool isGate() const;

		Node& inputNode(size_t index);
		Node& outputNode(size_t index);

		size_t getInputsCount();
		size_t getOutputsCount();

		double getTRise() const;
		double getTFall() const;
		void setTRise(double d);
		void setTFall(double d);
	};

}
