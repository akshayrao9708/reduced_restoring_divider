#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include<cmath>
#include <set>
#include <regex>
#include <queue>
#include "time.h"
#include "Node.h"
#include "Edge.h"
//#include "../Verifizierer.h"
#include "../helperStructs/fullAdder.hpp"
#include "../helperStructs/extBlock.hpp"

using namespace std;
//Added multiplexer class 
struct Multiplexer {
	int or1;
	int and1;
	int and2;
	int neg;
	int maxLevel;
	int previn1 = -1;
	int previn2 = -1;
	int prevS = -1;

	Multiplexer()
	  : or1(-1), and1(-1), and2(-1), neg(-1), maxLevel(-1)
	{}
	
	Multiplexer(int or1, int and1, int and2, int neg)
	  : or1(or1), and1(and1), and2(and2), neg(neg), maxLevel(-1)
	{}
};

namespace vp
{

	class Circuit
	{

	public:

		bool verifyFile(const string&);
		void insertNode(const string&, int);
		void insertNewEdge(const string&, const string&);
		void insertNewNode(const string&, const Node::NodeType, const string&, const string&, const string&, int);
		vector<int> outputNodes;
		vector<std::string> deleted_wires;
		vector<int> inputNodes;
		vector<Node> nodes;
		vector<int> sortedNodes;
		vector<int> notNodes;
		string moduleName;
		int mux_size =0;
		map<string, int> nodeIndex;  // Only used for checking if a node already exists or not. Get nodeIndex directly from node objects.
		map<string, Edge> edges;
		vector<fullAdder> foundAdders;
		vector<Multiplexer> foundMuxs;// added container for multiplxer 
		vector<extBlock> foundExtBlocks;
		//vector<Atomic> aigAtomics;
		//map<Edge, int> edgeIndex;  // DONE: edgeIndex is now simply a member variable of edge objects.
		//map<int, Edge> primaryEdgeIndex;
		int emptyNodesCounter;
		int maxEdgeIndex;
	public:
		friend class Node;
		friend class Edge;
		friend class Verifizierer;

		Circuit();
		Circuit(const string&);
		Circuit(const string&, const string&);
		void parseFile(const string&);						// Gets the path to the Verilog gate level netlist file
		void parseFileSimple(const string&);				// Gets the path to the Verilog gate level netlist file.
		
//		void createCircuitFromAIG(mockturtle::aig_network& aig);
//		void namePrimarySignals();
//		void setAtomicNumOfNodes(std::vector<Atomic>& atomics);
//		void orderAllNodesAIG(std::vector<Atomic>& atomics);
//		bool setLevelsOfAtomic(std::vector<Atomic>& atomics, int atomicIndex, map<int, int>& atomicMaxLevel);
//		bool setLevelOfNodeAIG(std::vector<Atomic>& atomics, int nodeIndex, map<int, int>& atomicMaxLevel);
//		void createDoubleNegation(int sourceIndex, int destinationIndex, std::vector<Atomic>& atomics);
//		void correctFAOutputNegation(std::vector<Atomic>& atomics);
		
		std::vector<fullAdder> findFAs();
		std::vector<Multiplexer> findMuxs();
		void setFanInsOutsOfFAs();
		std::vector<extBlock> initExtBlocksFromAdders();
		void extendBlocksWithRemainingGates();
		void addSingleNodesToExtBlocks();
		void orderAllNodesAdvanced();
		int setLevelsOfEAB(int blockIndex, int prevMaxLevel);

		void createAllOrderPermutations(std::vector<std::vector<int>>& allOrderPermutations);
		void createPermutationsFromNode(std::vector<std::vector<int>>& allOrderPermutations, vector<int> prevPermu, int chosenNode, unordered_set<int> readyNodes);

		void orderAllNodes(std::vector<fullAdder>& adders);
		int setLevelsOfFA(std::vector<fullAdder>& adder, int adderIndex, int prevMaxLevel);
		int setLevelOfNode(std::vector<fullAdder>& adders, int nodeIx);
		int getSmallestLevel(std::vector<string> signals);
		int getBiggestLevel(std::vector<string> signals);
		
		void writeCircuitToFile(const string&);
		void writeCircuitToFileNoLvls(const string&);
		void writeCircuitSimple(const string&);
		
		void revLevelizeCircuit();
		void swapNodes(int& a, int& b); // Used for quick sort. 
		int partition (int low, int high);  // Used for quick sort.
		void quickSort(int low, int high);
		void sortNodesByLevel();  // Sort nodes by their revLevel using quicksort algorithm.
		void changeSortedNodePosManually(int oldPos, int newPos);
		
		bool const0Prop();  // Propagate the constant zero wire.
		bool const1Prop();  // Propagate the constant one wire.
		
		void wireReplace(EdgePointer oldEdge, EdgePointer newEdge, int nodeNIndex);
		void wireReplaceAfterSAT(Edge* oldEdge, Edge* newEdge, bool equivalence);
		void removeEdge(Edge* oldEdge);
		void removeNode(Node* oldNode);
		int removeDeadEdges();
		void recursivelyRemoveEdge(Edge* deletedEdge);
		int checkForDoubleNot();
		void pushNotFanouts();
		void insertNot(Edge* newInput, Edge* oldOutput, int changeNodeIndex);
		void nodesInWindowRange(Node* startNode, int range, set<int>& windowNodes);
		void improvedWindow(Node* startNode, int range, set<int>& windowNodes);
		void improvedWindow(vector<Node*> startNodes, int range, set<int>& windowNodes);

		//vector<string> getInputPorts() const;				// Returns a vector containing the names of the input ports
		//vector<string> getOutputPorts() const;				// Returns a vector containing the names of the output ports

		vector<int> getInputNodes() const;				// Returns a vector containing input nodes.
		vector<int> getOutputNodes() const;

		string getModuleName() const;						// Returns the Verilog module name

		Node& node(size_t index);
		Node& node(const string& name);
		size_t getNodesCount() const;
		size_t getNodeIndex(const string& nodeName) const;	// Returns the index of a node

		Node& outputNode(size_t index);
		size_t getOutputNodesCount() const;

		Node& inputNode(size_t index);
		size_t getInputNodesCount() const;

		size_t getGatesCount() const;
	};

}
