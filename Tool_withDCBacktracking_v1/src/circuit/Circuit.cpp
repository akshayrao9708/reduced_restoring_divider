#include "Circuit.h"
#include "EdgePointer.h"
#include "ParseError.h"

namespace vp
{
	Circuit::Circuit()
	{
		emptyNodesCounter = 0;
		maxEdgeIndex = 0;
	}

//_____________________________________________________________________________________________________________________________________
	Circuit::Circuit(const string& filename)
	{
		emptyNodesCounter = 0;
		maxEdgeIndex = 0;
		parseFile(filename);
	}

//_____________________________________________________________________________________________________________________________________
	bool Circuit::verifyFile(const string& fileName)
	{
		ifstream in(fileName.c_str());
		if (in.fail())
			return false;
		in.close();
		return true;
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::insertNode(const string& port, int revLevel)
	{
		if (nodeIndex.count(port)) throw ParseError("gate " + port + " already declared");
		nodeIndex[port] = nodes.size();
		int newNodeIndex = nodes.size();
		nodes.push_back(Node(port, this, revLevel, newNodeIndex));
		/*if (delays.count(nodes[nodes.size() - 1].getType())){
			nodes[nodes.size() - 1].setTRise(delays[nodes[nodes.size() - 1].getType()].first);
			nodes[nodes.size() - 1].setTFall(delays[nodes[nodes.size() - 1].getType()].second);
		}*/
	}


//_____________________________________________________________________________________________________________________________________
	void Circuit::parseFile(const string& fileName)
	{
		double time1, time2, time3, time4, tstart;
		time1 = 0.0;
		tstart = clock(); // Zeitmessung beginnt.
		
		if (!verifyFile(fileName)) throw ParseError("File not found");
		string line;
		ifstream inputFile(fileName.c_str());

		cout << "Parsing file has started." << endl;		
		
		inputFile >> line;
		while (line != "module") inputFile >> line;
		getline(inputFile, line);
		int openBracketIndex = line.find("(");
		moduleName = line.substr(1, openBracketIndex - 1);

		string osp = "[\\t\\n\\r ]*";
		string sp = "[\\t\\n\\r ]+";
		string delayPart = "((?:#\\(" + osp + "([0-9.]+)" + osp + "," + osp + "([0-9.]+)" + osp + "\\))?)";
		regex wirePattern("(input|output|inout|reg|wire)" + osp + "(\\[\\d+\\:\\d+\\])?" + sp + "(\\S+)");
		//regex gatePattern("([A-Za-z0-9]+)" + sp + delayPart + osp + "([_0-9a-zA-z]+)?" + osp + "\\(([^]+)\\)");
		//regex assignPattern("assign (.+) = 1'b(0|1);");
		regex bufferPattern("(assign)" + osp + "(.+)" + osp + "=" + osp + "(.+)" + "/\\*" + "(\\d+)" + "\\*/");
//		regex assignPattern2("(assign)" + osp + "(.+)" + osp + "=" + "(.+)" + "(&|\\||\\^|~)" + "(.+)");
		regex assignPattern2("(assign)" + osp + "(.+)" + osp + "=" + "(.+)" + "(&|\\||\\^|~)" + "(.+)" + "/\\*" + "(\\d+)" + "\\*/");
		smatch match;

		bool manualLevels = true;

		while (getline(inputFile, line, ';')){

			int commentIndex = line.find("//");
			int nwlnIndex = line.find("\n");
			if (nwlnIndex != -1 && commentIndex != -1 && nwlnIndex > commentIndex){
				line.erase(commentIndex, nwlnIndex - commentIndex + 1);
			}

			if (regex_search(line, match, wirePattern)){
				// parsing the array operator[7:0]
				string arr = match[2].str();
				if (arr.size()){
					int colonIndex = arr.find(":");
					int firstNumber = atoi(arr.substr(1, colonIndex - 1).c_str());
					int secondNumber = atoi(arr.substr(colonIndex + 1, arr.size() - 1 - colonIndex).c_str());
					if (firstNumber < secondNumber) swap(firstNumber, secondNumber);
					for (int i = firstNumber; i >= secondNumber; i--)
						insertNewEdge(match[3].str() + "[" + to_string(i) + "]", match[1].str());
				}
				else
					insertNewEdge(match[3].str(), match[1].str());
			}
			else if (regex_search(line, match, assignPattern2)){
//				cout << "Assign Pattern gefunden:" << match.str(0) << endl;
//				cout << match.str(4) << endl;
				//cout << "Level:" << match.str(6) << "!" << endl;
				if (match.str(4) == "&") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "A" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					if (manualLevels) insertNewNode(nodeName, Node::AND, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::AND, portDescription, "", "", -1);
				} else if (match.str(4) == "|") {
//					cout << "or found" << endl;
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "O" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					} 
					if (manualLevels) insertNewNode(nodeName, Node::OR, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::OR, portDescription, "", "", -1);
				} else if (match.str(4) == "^") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "X" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					} 
					if (manualLevels) insertNewNode(nodeName, Node::XOR, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::XOR, portDescription, "", "", -1);
				} else if (match.str(4) == "~") {
//					cout << "not found" << endl;
					string portDescription = "(" + match.str(2) + ", " + match.str(5) + ")";
					string nodeName = "N" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					} 
					if (manualLevels) insertNewNode(nodeName, Node::NOT, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::NOT, portDescription, "", "", -1);
				}
			} 
			else if (regex_search(line, match, bufferPattern)){  // buffer pattern.
				//std::cout << "2: !" << match.str(2) << "!" << "3: !" << match.str(3) << "!" << std::endl;
				if (match.str(2) == "zeroWire " && match.str(3) == "1'b0 ") {
					//cout << "zeroWire match gefunden! " << endl;
					insertNode("zeroNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("zeroWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["zeroWire"].setSourceNode(nodeIndex);
					continue;
				}
				if (match.str(2) == "oneWire " && match.str(3) == "1'b1 ") {
					//cout << "oneWire match gefunden! " << endl;
					insertNode("oneNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("oneWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["oneWire"].setSourceNode(nodeIndex);
					continue;
				}
				string portDescription = "(" + match.str(2) + ", " + match.str(3) + ")";
				string nodeName = "B" + match.str(2) ;
				while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					} 
				if (manualLevels) insertNewNode(nodeName, Node::BUFFER, portDescription, "", "", std::stoi(match.str(4)));
				else insertNewNode(nodeName, Node::BUFFER, portDescription, "", "", -1);
				
			}
		}
		inputFile.close();
		
		std::cout << "Parse file is completed." << std::endl; 
		
		
		this->foundMuxs = this->findMuxs();
		this->foundAdders = this->findFAs();
		//cout<<"foundMuxs.size():"<<foundMuxs.size()+foundAdders.size()<<endl;
		check_atomic_block_flag = sqrt(foundMuxs.size());
		//cout <<"check_atomic_block_flag"<<check_atomic_block_flag<<endl;
		number_of_stages = check_atomic_block_flag*2;
		//cout <<"number of stages "<<number_of_stages<<endl;

		// New function to already create DC candidates before constant propagation.
		this->createDCCandidatesFromAtomics();


		const0Prop();
		const1Prop();
		
		// New function to manage signals which were replaced during constant propagation.
		this->applyTransitivityForReplacedSignals();
		
		//cout << "const propagation done." << endl;
		
		// Remove dead edges.
		int j = removeDeadEdges();
		//cout << j << " dead edges removed." << endl;
		//*/

//		this->revLevelizeCircuit();

		//foundAdders = this->findFAs();
		
		//time4 += clock() - tstart; // Zeitmessung endet.
		//time4 = time4/CLOCKS_PER_SEC;
		//cout << "Simplifying circuit needed time in sec.: " << time4 << endl;
		
		// Sort node list.
		//time3 = 0.0;
		//tstart = clock();
		
		sortNodesByLevel();

		//time3 += clock() - tstart; // Zeitmessung endet.
		//time3 = time3/CLOCKS_PER_SEC;
		//cout << "SortNodesByLevel() needed time in sec.: " << time3 << endl;
		//*/
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::parseFileSimple(const string& fileName)
	{
		double time1, time2, time3, time4, tstart;
		time1 = 0.0;
		tstart = clock(); // Zeitmessung beginnt.

		if (!verifyFile(fileName)) throw ParseError("File not found");
		string line;
		ifstream inputFile(fileName.c_str());

		cout << "Parsing file simple has started." << endl;

		inputFile >> line;
		while (line != "module") inputFile >> line;
		getline(inputFile, line);
		int openBracketIndex = line.find("(");
		moduleName = line.substr(1, openBracketIndex - 1);

		string osp = "[\\t\\n\\r ]*";
		string sp = "[\\t\\n\\r ]+";
		string delayPart = "((?:#\\(" + osp + "([0-9.]+)" + osp + "," + osp + "([0-9.]+)" + osp + "\\))?)";
		regex wirePattern("(input|output|inout|reg|wire)" + osp + "(\\[\\d+\\:\\d+\\])?" + sp + "(\\S+)");
		//regex gatePattern("([A-Za-z0-9]+)" + sp + delayPart + osp + "([_0-9a-zA-z]+)?" + osp + "\\(([^]+)\\)");
		//regex assignPattern("assign (.+) = 1'b(0|1);");
		regex bufferPattern("(assign)" + osp + "(.+)" + osp + "=" + osp + "(.+)");
		regex bufferPatternLevel("(assign)" + osp + "(.+)" + osp + "=" + osp + "(.+)" + "/\\*" + "(\\d+)" + "\\*/");
		regex assignPattern2("(assign)" + osp + "(.+)" + osp + "=" + "(.+)" + "(&|\\||\\^|~)" + "(.+)");
		regex assignPattern2Level("(assign)" + osp + "(.+)" + osp + "=" + "(.+)" + "(&|\\||\\^|~)" + "(.+)" + "/\\*" + "(\\d+)" + "\\*/");
		smatch match;

		bool manualLevels = false;

		while (getline(inputFile, line, ';')){

			int commentIndex = line.find("//");
			int nwlnIndex = line.find("\n");
			if (nwlnIndex != -1 && commentIndex != -1 && nwlnIndex > commentIndex){
				line.erase(commentIndex, nwlnIndex - commentIndex + 1);
			}

			if (regex_search(line, match, wirePattern)){
				// parsing the array operator[7:0]
				string arr = match[2].str();
				if (arr.size()){
					int colonIndex = arr.find(":");
					int firstNumber = atoi(arr.substr(1, colonIndex - 1).c_str());
					int secondNumber = atoi(arr.substr(colonIndex + 1, arr.size() - 1 - colonIndex).c_str());
					if (firstNumber < secondNumber) swap(firstNumber, secondNumber);
					for (int i = firstNumber; i >= secondNumber; i--)
						insertNewEdge(match[3].str() + "[" + to_string(i) + "]", match[1].str());
				}
				else
					insertNewEdge(match[3].str(), match[1].str());
			}
			else if (regex_search(line, match, assignPattern2Level)){
				//cout << "Assign Pattern gefunden:" << match.str(0) << endl;
				//cout << match.str(4) << endl;
				//cout << "Level:" << match.str(6) << "!" << endl;
				if (match.str(4) == "&") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "A" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					if (manualLevels) insertNewNode(nodeName, Node::AND, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::AND, portDescription, "", "", -1);
				} else if (match.str(4) == "|") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "O" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					if (manualLevels) insertNewNode(nodeName, Node::OR, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::OR, portDescription, "", "", -1);
				} else if (match.str(4) == "^") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "X" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					if (manualLevels) insertNewNode(nodeName, Node::XOR, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::XOR, portDescription, "", "", -1);
				} else if (match.str(4) == "~") {
					string portDescription = "(" + match.str(2) + ", " + match.str(5) + ")";
					string nodeName = "N" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					if (manualLevels) insertNewNode(nodeName, Node::NOT, portDescription, "", "", std::stoi(match.str(6)));
					else insertNewNode(nodeName, Node::NOT, portDescription, "", "", -1);
				}
			} else if (regex_search(line, match, assignPattern2)){
				//cout << "Assign Pattern gefunden:" << match.str(0) << endl;
				//cout << match.str(4) << endl;
				//cout << "Level:" << match.str(6) << "!" << endl;
				if (match.str(4) == "&") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "A" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					insertNewNode(nodeName, Node::AND, portDescription, "", "", -1);
				} else if (match.str(4) == "|") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "O" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					insertNewNode(nodeName, Node::OR, portDescription, "", "", -1);
				} else if (match.str(4) == "^") {
					string portDescription = "(" + match.str(2) + ", " + match.str(3) + ", " + match.str(5) + ")";
					string nodeName = "X" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					insertNewNode(nodeName, Node::XOR, portDescription, "", "", -1);
				} else if (match.str(4) == "~") {
					string portDescription = "(" + match.str(2) + ", " + match.str(5) + ")";
					string nodeName = "N" + match.str(2);
					while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
					insertNewNode(nodeName, Node::NOT, portDescription, "", "", -1);
				}
			}
			else if (regex_search(line, match, bufferPatternLevel)){  // buffer pattern.
				//std::cout << "2: !" << match.str(2) << "!" << "3: !" << match.str(3) << "!" << std::endl;
				if (match.str(2) == "zeroWire " && match.str(3) == "1'b0 ") {
					//cout << "zeroWire match gefunden! " << endl;
					insertNode("zeroNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("zeroWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["zeroWire"].setSourceNode(nodeIndex);
					continue;
				}
				if (match.str(2) == "oneWire " && match.str(3) == "1'b1 ") {
					//cout << "oneWire match gefunden! " << endl;
					insertNode("oneNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("oneWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["oneWire"].setSourceNode(nodeIndex);
					continue;
				}
				string portDescription = "(" + match.str(2) + ", " + match.str(3) + ")";
				string nodeName = "B" + match.str(2) ;
				while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
				if (manualLevels) insertNewNode(nodeName, Node::BUFFER, portDescription, "", "", std::stoi(match.str(4)));
				else insertNewNode(nodeName, Node::BUFFER, portDescription, "", "", -1);

			} else if (regex_search(line, match, bufferPattern)){  // buffer pattern.
				//std::cout << "2: !" << match.str(2) << "!" << "3: !" << match.str(3) << "!" << std::endl;
				if (match.str(2) == "zeroWire " && match.str(3) == "1'b0 ") {
					//cout << "zeroWire match gefunden! " << endl;
					insertNode("zeroNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("zeroWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["zeroWire"].setSourceNode(nodeIndex);
					continue;
				}
				if (match.str(2) == "oneWire " && match.str(3) == "1'b1 ") {
					//cout << "oneWire match gefunden! " << endl;
					insertNode("oneNode", 0);
					int nodeIndex = nodes.size() - 1;
					inputNodes.push_back(nodeIndex);
					nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
					nodes[nodeIndex].addOutput(edges.find("oneWire"));
					nodes[nodeIndex].nIndex = nodeIndex;
					edges["oneWire"].setSourceNode(nodeIndex);
					continue;
				}
				string portDescription = "(" + match.str(2) + ", " + match.str(3) + ")";
				string nodeName = "B" + match.str(2) ;
				while (nodeName.find(" ") != string::npos) {	// Delete spaces from nodeName string.
						nodeName.erase(nodeName.find(" "), 1);
					}
				insertNewNode(nodeName, Node::BUFFER, portDescription, "", "", -1);

			}
		}
		inputFile.close();

		std::cout << "Parse file is completed." << std::endl;

		// TODO: Don't use simplifications and all here, but call them all single from Verifizierer instance.

		//time1 += clock() - tstart; // Zeitmessung endet.
		//time1 = time1/CLOCKS_PER_SEC;
		//cout << "Parsing file needed time in sec.: " << time1 << endl;
		//buildAdjacencyMatrix();

//		this->foundAdders = this->findFAs();

		// Find for all FAs previous and next FAs and get next and prev. indices.
//		this->setFanInsOutsOfFAs();

//		this->foundExtBlocks = this->initExtBlocksFromAdders();

//		this->extendBlocksWithRemainingGates();

//		this->addSingleNodesToExtBlocks();

//		for (size_t i=0; i < foundExtBlocks.size(); ++i) {
////			cout << "pos " << i << " full adder is: " << nodes.at(foundAdders.at(i).and1).name << "|" <<
////					nodes.at(foundAdders.at(i).and2).name << "|" << nodes.at(foundAdders.at(i).xor1).name << "|" <<
////					nodes.at(foundAdders.at(i).xor2).name << "|" << nodes.at(foundAdders.at(i).or1).name << "|" << endl;
//			cout << "pos " << i << " EAB is: ";
//			for (auto& elem: foundExtBlocks.at(i).memNodes) {
//				cout << nodes.at(elem).name << "|";
//			}
//			cout << endl;
//		}

//		if (!manualLevels) this->orderAllNodesAdvanced();

//		for (auto& currNode: this->nodes) {
//			cout << " ,name: " << currNode.name << " ,block: " << currNode.block << " ,revLevel: " << currNode.revLevel << endl;
//		}

		//this->orderAllNodes(foundAdders);

		// Constant propagation BEFORE sorting of nodes.
		//*
		time4 = 0.0;
		tstart = clock();

		//const0Prop();
		//const1Prop();

		cout << "const propagation done." << endl;

		// Remove dead edges.
		//int j = removeDeadEdges();
		//cout << j << " dead edges removed." << endl;
		//*/

		if (!manualLevels) this->revLevelizeCircuit();
		cout << "revLevelize done." << endl;

		//foundAdders = this->findFAs();

		//time4 += clock() - tstart; // Zeitmessung endet.
		//time4 = time4/CLOCKS_PER_SEC;
		//cout << "Simplifying circuit needed time in sec.: " << time4 << endl;

		// Sort node list.
		//time3 = 0.0;
		//tstart = clock();

		cout << "SortNodes started." << endl;

		sortNodesByLevel();

		cout << "SortNodes done." << endl;

		//time3 += clock() - tstart; // Zeitmessung endet.
		//time3 = time3/CLOCKS_PER_SEC;
		//cout << "SortNodesByLevel() needed time in sec.: " << time3 << endl;
		//*/
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::insertNewEdge(const string& edgeName, const string& edgeType)
	{
		if (edges.count(edgeName)) throw ParseError("wire \"" + edgeName + "\" already declared");
		edges[edgeName] = Edge(edgeName, this); 
		//int currEdgeSize = edgeIndex.size();
		//edgeIndex[Edge(edgeName, this)] = currEdgeSize;
		edges.at(edgeName).eIndex = this->maxEdgeIndex + 1;
		cout<<"edge name "<< edgeName<<endl;
		cout<<"edge index "<< edges.at(edgeName).eIndex<<endl;
		this->maxEdgeIndex++;
		EdgePointer pointer = edges.find(edgeName);
		if (edgeType == "input"){
			//inputEdgeIndex.push_back(edges.size() -1);  // Add edge index to input index list.
			//primaryEdgeIndex[edges.size() - 1] = Edge(edgeName, this);
			insertNode(edgeName, 0);
			int nodeIndex = nodes.size() - 1;
			inputNodes.push_back(nodeIndex);
			nodes[nodeIndex].setNodeType(Node::INPUT_PORT);
			nodes[nodeIndex].addOutput(pointer);
			nodes[nodeIndex].nIndex = nodeIndex;
			edges[edgeName].setSourceNode(nodeIndex);
		}
		else if (edgeType == "output"){
			//outputEdgeIndex.push_back(edges.size() -1);  // Add edge index to output index list.
			//primaryEdgeIndex[edges.size() - 1] = Edge(edgeName, this);
			insertNode(edgeName, 0);
			int nodeIndex = nodes.size() - 1;
			outputNodes.push_back(nodeIndex);
			nodes[nodeIndex].setNodeType(Node::OUTPUT_PORT);
			nodes[nodeIndex].addInput(pointer);
			nodes[nodeIndex].nIndex = nodeIndex;
			edges[edgeName].addDestinationNode(nodeIndex);
		}
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::wireReplace(EdgePointer oldEdge, EdgePointer newEdge, int nodeNIndex) {
		//cout << "wireReplace entered with: " << oldEdge->name << ", " << newEdge->name << endl;
		int prevSize = newEdge->nDestinations();
		int prevNodeIndex = nodeNIndex;
		// Remove the node of oldEdge from the list of destinations of newEdge.
		for (int i = 0; i < prevSize; i++) {
			if (newEdge->getDestination(i) == prevNodeIndex) {
				newEdge->getDestinations()->erase(newEdge->getDestinations()->begin() + i);
				break;
			}	
		} 
		// Add destinations of oldEdge to newEdge since it replaces the oldEdge. Change inputs of the new nodes of newEdge.
		for (int i = 0; i < oldEdge->nDestinations(); i++) {
			newEdge->addDestinationNode(oldEdge->getDestination(i));
			for (int j = 0; j < this->nodes.at(oldEdge->getDestination(i)).getInputsCount(); j++) {
				if (this->nodes.at(oldEdge->getDestination(i)).inputs.at(j)->name == oldEdge->name) {
					this->nodes.at(oldEdge->getDestination(i)).inputs.at(j) = newEdge;
				}	
			}
		}
		
		// Add replaced signals information to map to remember it for later use.
		this->replacedSignals.emplace(oldEdge->name, newEdge->name);
		
		// Remove old edge and delete contents of old node. Old node cannot be deleted from node list due to indexing.
		this->removeNode(&this->nodes.at(prevNodeIndex));
		this->removeEdge(&*oldEdge);
		//cout <<" new edge eindex"<<newEdge->eIndex<<endl;
	}

//_____________________________________________________________________________________________________________________________________
	bool Circuit::const0Prop() {
		cout<<"inside const0Prop "<<endl;
		Node* zeroNode = NULL;
		for (int i = 0; i < this->getInputNodesCount(); i++) {  // Get the pointer to the constant zero Node.
			if (this->inputNode(i).name == "zeroNode") {
				zeroNode = &this->inputNode(i);
				break;
			}
		} 
		EdgePointer zeroWire;
		if (zeroNode != NULL) zeroWire = zeroNode->outputs.at(0);
		else return false;
		//cout << "ZeroWire: " << zeroWire->name << endl;
		// Traverse circuit and replace constant 0.
		Node* currentNode;
		for (int i = 0; i < zeroNode->getOutputsCount(); i++) {
			currentNode = &zeroNode->outputNode(i);
			//cout << "currentNode name:" << currentNode->name << endl;
			if (currentNode->type == Node::DELETED) {
				zeroNode->outputs.at(0)->getDestinations()->erase(zeroNode->outputs.at(0)->getDestinations()->begin() + i);
				i--;
				continue;
			}  
			// Case destinction NOT: Ignore for now. Later make it a const1 node. 
			if (currentNode->type == Node::NOT) {
				std::cout << "WARNING: Negation of constant 0 is not allowed so far. Replace it with constant 1 and run again." << endl;
				exit(EXIT_FAILURE);
			} 
			// Case destinctions. OR: 
			if (currentNode->type == Node::OR) {
				//cout << "in1: " << currentNode->inputs.at(0)->name << ", in2: " << currentNode->inputs.at(1)->name << endl;
				if (currentNode->inputs.at(0)->name == "zeroWire") {  // Input detected. Replace output by other input.
					this->wireReplace(currentNode->outputs.at(0) , currentNode->inputs.at(1), currentNode->nIndex);
				} else if (currentNode->inputs.at(1)->name == "zeroWire") {
					this->wireReplace(currentNode->outputs.at(0) ,currentNode->inputs.at(0), currentNode->nIndex);
				} else cout << "Something went wrong. No zeroWire input in " << currentNode->name << " found." << endl; 
				i = -1;
				continue;
			}
			// Case destinctions. AND:
			if (currentNode->type == Node::AND) {
				if (currentNode->inputs.at(0)->name == "zeroWire") {  // Input detected. Replace output by zeroWire.
					this->wireReplace(currentNode->outputs.at(0) , zeroWire, currentNode->nIndex);
				} else if (currentNode->inputs.at(1)->name == "zeroWire") {
					this->wireReplace(currentNode->outputs.at(0) , zeroWire, currentNode->nIndex);
				} else cout << "Something went wrong. No zeroWire input in " << currentNode->name << " found." << endl;
				i = -1;
				continue;
			}
			// Case destinctions. XOR:
			if (currentNode->type == Node::XOR) {
				if (currentNode->inputs.at(0)->name == "zeroWire") {  // Input detected. Replace output by other input.
					this->wireReplace(currentNode->outputs.at(0) ,currentNode->inputs.at(1), currentNode->nIndex);
				} else if (currentNode->inputs.at(1)->name == "zeroWire") {
					this->wireReplace(currentNode->outputs.at(0) ,currentNode->inputs.at(0), currentNode->nIndex);
				} else cout << "Something went wrong. No zeroWire input in " << currentNode->name << " found." << endl; 
				i = -1;
				continue;	
			}
			// Case destinctions. BUFFER:
			if (currentNode->type == Node::BUFFER) {
				if (currentNode->inputs.at(0)->name == "zeroWire") {  // Input detected. Replace output by zeroWire.
					this->wireReplace(currentNode->outputs.at(0) , zeroWire, currentNode->nIndex);
				} else cout << "Something went wrong. No zeroWire input in " << currentNode->name << " found." << endl; 
				i = -1;
				continue;	
			}
			
		} 
		if (zeroNode == NULL) return false;
		else return true; 
	}

//_____________________________________________________________________________________________________________________________________
	bool Circuit::const1Prop() {
		cout<<"inside const1 prop"<<endl;
		Node* oneNode = NULL;
		for (int i = 0; i < this->getInputNodesCount(); i++) {  // Get the pointer to the constant zero Node.
			if (this->inputNode(i).name == "oneNode") {
				oneNode = &this->inputNode(i);
				break;
			}
		}
		EdgePointer oneWire; 
		if (oneNode != NULL) oneWire = oneNode->outputs.at(0);
		else return false;
		// Traverse circuit and replace constant 0.
		Node* currentNode;		
		for (int i = 0; i < oneNode->getOutputsCount(); i++) {
			currentNode = &oneNode->outputNode(i);
			if (currentNode->type == Node::DELETED) {
				oneNode->outputs.at(0)->getDestinations()->erase(oneNode->outputs.at(0)->getDestinations()->begin() + i);
				i--;
				continue;
			}  
			// Case destinction NOT: Not allowed. Give a warning. 
			if (currentNode->type == Node::NOT) {
				cout << "WARNING: Negation of constant 1 is not allowed so far. Replace it with constant 0." << endl;
				exit(EXIT_FAILURE);
			} 
			// Case destinctions. AND: This node is constant 0. Replace its appearances by zeroWire. 
			if (currentNode->type == Node::OR) {
				if (currentNode->inputs.at(0)->name == "oneWire") {  // Input detected. Replace output by oneWire.
					this->wireReplace(currentNode->outputs.at(0) , oneWire, currentNode->nIndex);
				} else if (currentNode->inputs.at(1)->name == "oneWire") {
					this->wireReplace(currentNode->outputs.at(0) , oneWire, currentNode->nIndex);
				} else cout << "Something went wrong. No oneWire input in " << currentNode->name << " found." << endl;
				i = -1;
				continue;
			}
			
			if (currentNode->type == Node::AND) {
				if (currentNode->inputs.at(0)->name == "oneWire") {  // Input detected. Replace output by other input.
					this->wireReplace(currentNode->outputs.at(0) ,currentNode->inputs.at(1), currentNode->nIndex);
				} else if (currentNode->inputs.at(1)->name == "oneWire") {
					this->wireReplace(currentNode->outputs.at(0) ,currentNode->inputs.at(0), currentNode->nIndex);
				} else cout << "Something went wrong. No oneWire input in " << currentNode->name << " found." << endl; 
				i = -1;
				continue;	
			}
			
			if (currentNode->type == Node::XOR) {
				if (currentNode->inputs.at(0)->name == "oneWire") {  // Constant One Input detected. Replace output by negation of other input.
					// 1 XOR A = NOT A.
					currentNode->type = Node::NOT;
					currentNode->inputs.at(0) = currentNode->inputs.at(1); 
				
					currentNode->inputs.erase(currentNode->inputs.begin() + 1);
					oneNode->outputs.at(0)->getDestinations()->erase(oneNode->outputs.at(0)->getDestinations()->begin() + i);
					notNodes.push_back(currentNode->nIndex);
					//checkForDoubleNot(currentNode);				
				} else if (currentNode->inputs.at(1)->name == "oneWire") {
					// A XOR 1 = NOT A.
					currentNode->type = Node::NOT;
					currentNode->inputs.erase(currentNode->inputs.begin() + 1);
					oneNode->outputs.at(0)->getDestinations()->erase(oneNode->outputs.at(0)->getDestinations()->begin() + i);
					notNodes.push_back(currentNode->nIndex);
					//checkForDoubleNot(currentNode);
				} else cout << "Something went wrong. No oneWire input in " << currentNode->name << " found." << endl;
				i = -1;
				continue;
			}
			
			if (currentNode->type == Node::BUFFER) {
				if (currentNode->inputs.at(0)->name == "oneWire") {  // Input detected. Replace output by oneWire.
					this->wireReplace(currentNode->outputs.at(0) , oneWire, currentNode->nIndex);
				} else cout << "Something went wrong. No oneWire input in " << currentNode->name << " found." << endl; 
				i = -1;
				continue;	
			}
			
		} 
		
		if (oneNode == NULL) return false;
		else return true; 
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::removeEdge(Edge* oldEdge) {
		// Remove old edge which has no use anymore.
		oldEdge->disconnect();
		

		this->edges.erase(oldEdge->name);
		//this->edgeIndex.erase(*oldEdge); This causes memory access error. Leave it out for now.
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::removeNode(Node* oldNode) {
		// Note: The node will not really be removed from the nodes and sortedNodes list because this would disqualify correct access of edges to the nodes (edges use index of node positions in nodes list).
		oldNode->setNodeType(Node::DELETED);
		oldNode->inputs.clear();
		oldNode->outputs.clear();
		this->nodeIndex.erase(oldNode->name);
	}

//_____________________________________________________________________________________________________________________________________
	int Circuit::removeDeadEdges(){
		// Edge is dead if it has no destinations.
		int removed = 0;
		map<string,Edge>::iterator it_prev;
		//bool first = true;
		for (map<string,Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
			//cout << "curr wire name is " << it->first << endl;
			//if (!first) cout << "prev wire name is " << it_prev->first << endl;
			//else first = false;
			if (it->second.destinations.size() == 0) {
				//cout << "removing: " << it->first << endl;
				recursivelyRemoveEdge(&it->second);
				removed++;
				if(this->edges.count(it_prev->first)>0){
					it = it_prev;
				} else{
					it = edges.begin();
				}
				//cout << "test" << endl;
				//it = it_prev;
				//cout << "test2" << endl;
			} else {
				it_prev = it;
			}

		}
		return removed;
	}


//_____________________________________________________________________________________________________________________________________
	void Circuit::recursivelyRemoveEdge(Edge* deletedEdge) {
		//cout << " recursive entered. " << endl;
		Edge* input1 = NULL;
		Edge* input2 = NULL;
		//cout << "edge name: " << deletedEdge->name << " source: " << nodes.at(deletedEdge->getSource()).name << endl;
		if (deletedEdge->getSource() == -1) {  // Edge source or destinations were not specified. Just delete this edge.
			//cout << "Wire " << deletedEdge->name << " gets removed." << endl;
			removeEdge(deletedEdge);
			return;
		}
		Node* deletedNode = &nodes.at(deletedEdge->getSource());
		//cout << "node name: " << deletedNode->name << endl;
		//if (deletedNode.type == "DELETED") return;
		if (deletedNode->inputs.size() > 0) {
			input1 = &*deletedNode->inputs.at(0);
			if (deletedNode->inputs.size() > 1) input2 = &*deletedNode->inputs.at(1); 
		}
		if (input1) {
			for (int i = 0; i < input1->nDestinations(); i++) {  // Erase deletedNode from input edge.
				if (nodes.at(input1->getDestination(i)).name == deletedNode->name) 
				input1->getDestinations()->erase(input1->getDestinations()->begin() + i);
			}
			if (input1->nDestinations() == 0) recursivelyRemoveEdge(input1);
		}
		if (input2) {
			for (int i = 0; i < input2->nDestinations(); i++) {  // Erase deletedNode from input edge.
				if (nodes.at(input2->getDestination(i)).name == deletedNode->name) 
				input2->getDestinations()->erase(input2->getDestinations()->begin() + i);
			}
			if (input2->nDestinations() == 0) recursivelyRemoveEdge(input2);
		}
		removeNode(deletedNode);
		//cout << "Wire " << deletedEdge->name << " gets removed." << endl;
		deleted_wires.push_back(deletedEdge->name);
		removeEdge(deletedEdge);
	}

//_____________________________________________________________________________________________________________________________________	
	void Circuit::wireReplaceAfterSAT(Edge* oldEdge, Edge* newEdge, bool equivalence) {
		
		if (equivalence) {
			// Add destinations of oldEdge to newEdge since it replaces the oldEdge. Change inputs of the new nodes of newEdge.
			int destiNode = 0;
			//vp::EdgePointer oldEP(this->edges.find(oldEdge->name));
			vp::EdgePointer newEP(this->edges.find(newEdge->name));
			for (int i = 0; i < oldEdge->nDestinations(); i++) {
				destiNode = oldEdge->getDestination(i);
				newEdge->addDestinationNode(destiNode);
				for (int j = 0; j < this->nodes.at(destiNode).getInputsCount(); j++) {
					if (this->nodes.at(destiNode).inputs.at(j)->name == oldEdge->name) {
						this->nodes.at(destiNode).inputs.at(j) = newEP;
					}	
				}
			}
			//cout << "Wire " << oldEdge->name << " gets removed." << endl;
			this->recursivelyRemoveEdge(oldEdge);
		} else {  // Antivalence case.
			int oldNodeIndex = oldEdge->getSource();
			int newNodeIndex;
			vp::EdgePointer oldEP(this->edges.find(oldEdge->name));
			vp::EdgePointer newEP(this->edges.find(newEdge->name));
			// Create new NOT Node, which is the negation of newEdge.	
			string nodeName = "nN_";
			nodeName = nodeName + oldEdge->name;
			//cout << "Neuer not node with name : " << nodeName << endl;
			if (nodeIndex.count(nodeName) == 0) {
				insertNode(nodeName, nodes.at(oldEdge->getSource()).revLevel);
				newNodeIndex = nodes.size() - 1;
				nodes[newNodeIndex].setNodeType(Node::NOT);
				nodes[newNodeIndex].addOutput(oldEP);
				nodes[newNodeIndex].addInput(newEP);
				nodes[newNodeIndex].nIndex = newNodeIndex; 
				notNodes.push_back(newNodeIndex);
				oldEP->setSourceNode(newNodeIndex);
				newEP->addDestinationNode(newNodeIndex);
			} else {
				//cout << "Existiert schon. Skip" << endl;
				return;	
			}
			// Delete old node of oldEdge.
			vp::Node* oldNode = &nodes.at(oldNodeIndex);
			for (int i = 0; i < oldNode->getInputsCount(); i++) {
				if (oldNode->inputs.at(i)->nDestinations() == 1) {
					//cout << "Wire "<< i << oldNode->inputs.at(i)->name << " gets removed." << endl;
					this->recursivelyRemoveEdge(&*oldNode->inputs.at(i));
					continue;
				}
				for (int j = 0; j < oldNode->inputs.at(i)->nDestinations(); j++) {
					if (oldNode->inputs.at(i)->getDestination(j) == oldNodeIndex) {
						oldNode->inputs.at(i)->getDestinations()->erase(oldNode->inputs.at(i)->getDestinations()->begin()+j);
					}	
				}
			}
			this->removeNode(oldNode);
			//checkForDoubleNot(&nodes.at(newNodeIndex));
		}
	}

//_____________________________________________________________________________________________________________________________________
	int Circuit::checkForDoubleNot() {
		//return;
		int count = 0;
		cout << "checkForDoubleNot started." << endl;
		Node* notNode;
		for(size_t i=0; i < this->notNodes.size(); i++) {
			notNode = &nodes.at(notNodes.at(i));
			if (notNode->type != Node::NOT) continue;
			// Check for a NOT before this node.
			for (size_t i = 0; i < notNode->getInputsCount(); i++) {
				if (notNode->inputNode(i).type == Node::NOT) {
					//cout << "Type: " << notNode->type << endl;
					//cout << "Double not prev. discovered:" << notNode->name << " and " << notNode->inputNode(i).name << endl;
					// Double NOT encountered. Replace output of notNode with input of prev. node.
					wireReplaceAfterSAT(&*notNode->outputs.at(0), &*notNode->inputNode(i).inputs.at(0), true);
					count++;
				}
			}
			// Check for a NOT after this node.
			for (size_t i = 0; i < notNode->getOutputsCount(); i++) {
				if (notNode->outputNode(i).type == Node::NOT) {
					//cout << "Type: " << notNode->type << endl;
					//cout << "Double not after discovered:" << notNode->name << " and " << notNode->outputNode(i).name <<endl;
					// Double NOT encountered. Replace output of next node with input of notNode.
					wireReplaceAfterSAT(&*notNode->outputNode(i).outputs.at(0), &*notNode->inputs.at(0), true);
					count++;
				}
			}
		}
		return count;
	}
//_____________________________________________________________________________________________________________________________________
	void Circuit::pushNotFanouts() {
		int notCount = 0;
		double time3 = 0.0;
		double tstart = clock();
		
		//std::vector<Node*> changeNodes;
		std::vector<int> changeNodes;
		Node* currNode;
		//cout << "Not amount: " << this->notNodes.size() << endl;
		for(size_t i=0; i < this->notNodes.size(); i++) {
			changeNodes.clear();
			assert(changeNodes.size() == 0);
			currNode = &nodes.at(notNodes.at(i));
			//cout << "currNode: " << currNode->name << endl;
			if (currNode->type == Node::NOT) {
				if (currNode->getOutputsCount() > 1) {
					//cout << "CurrNOde: " << currNode->name << endl;
					//if (currNode->name != "N_29_") continue; 
					for (size_t i = 0; i < currNode->getOutputsCount(); ++i) {
						if (currNode->outputNode(i).type == Node::DELETED) continue;
 						changeNodes.push_back(currNode->outputNode(i).nIndex);
					}
					// Push NOT behind the fanout.
					Edge* prevInput = &*currNode->inputs.at(0);
					Edge* prevOutput = &*currNode->outputs.at(0);
					prevOutput->getDestinations()->clear();
					//Node* helpNode = *changeNodes.begin();
					//string firstNodeName = helpNode->name;
					//prevOutput->getDestinations()->push_back(nodeIndex.at(firstNodeName));
					//cout << "test" << endl;
					for (size_t i = 0; i < changeNodes.size(); i++) {
						//if (i == 0) {
						//	currNode->revLevel = changeNodes.at(i)->revLevel + 1;  // Just change level of preserved not node.
						//	continue;  
						//}
						insertNot(prevInput, prevOutput, changeNodes.at(i));
						notCount++;
					}
					recursivelyRemoveEdge(prevOutput);  // Remove previous output of original not Node.
					/*cout << "Curr node: " << currNode->name << " has lvl : " << currNode->revLevel << endl; 
					for (size_t i = 0; i < prevInput->nDestinations(); i++) {
						cout << "Node " << i << " has name: " << nodes.at(prevInput->getDestination(i)).name << endl;
						if (nodes.at(prevInput->getDestination(i)).type == "not") {
							//cout << "not detected." << endl;
							cout << "new not goes into: " << nodes.at(prevInput->getDestination(i)).outputNode(0).name << endl;
							cout << "and has output count: " << nodes.at(prevInput->getDestination(i)).getOutputsCount() << endl;
						}
					}//*/
				} else {  // NOT gate has only one successor. Check level of both.
					if (currNode->revLevel - currNode->outputNode(0).revLevel != 1) {  // NOT gate is not direct predecessor of next node.
						//cout << "Found: " << currNode->name << endl;
						// Correct level of NOT gate to successor level +1.
						currNode->revLevel = currNode->outputNode(0).revLevel + 1; 
					}
				}
			}
		}
		cout << "PushNotFanouts successful." << endl;
		time3 += clock() - tstart; // Zeitmessung endet.
		time3 = time3/CLOCKS_PER_SEC;
		cout << "PushNotFanouts() needed time in sec.: " << time3 << endl;
		cout << "Nots pushed: " << notCount << endl;
	}
	
//_____________________________________________________________________________________________________________________________________
	void Circuit::insertNot(Edge* newInput, Edge* oldOutput, int changeNodeIndex) {
		//cout << "Not in." << endl;
		//int changeNodeIndex = nodeIndex.at(changeNodeName);
		int newNodeIndex;
		int oldLvl = nodes.at(changeNodeIndex).revLevel;
		EdgePointer newEP(this->edges.find(newInput->name));
		//cout << "Test1" << endl;
		// Create new NOT Node and new output edge for this node.	
		string edgeName = "_n";
		edgeName = edgeName + to_string(this->maxEdgeIndex) + "_";
		string nodeName = "nF";
		nodeName = nodeName + edgeName;
		insertNewEdge(edgeName, "wire");  // Add edge as wire.
		EdgePointer oldEP(this->edges.find(edgeName));
		//cout << "Test2" << endl;
		//if (nodeIndex.count(nodeName) == 0) {
		insertNode(nodeName, oldLvl + 1);  // Insert Node.
		newNodeIndex = nodes.size() - 1;
		//cout << "Test3" << endl;
		nodes[newNodeIndex].setNodeType(Node::NOT);
		nodes[newNodeIndex].addOutput(EdgePointer(this->edges.find(edgeName)));
		nodes[newNodeIndex].addInput(EdgePointer(this->edges.find(newInput->name)));
		nodes[newNodeIndex].nIndex = newNodeIndex;
		notNodes.push_back(newNodeIndex);
		//cout << "Test4" << endl;
		oldEP->setSourceNode(newNodeIndex);
		oldEP->addDestinationNode(changeNodeIndex);
		newEP->addDestinationNode(newNodeIndex);
		//cout << "Test5" << endl;
		// Replace edge of changeNode by new edge.
		Node* changeNode = &nodes.at(changeNodeIndex);
		for (vector<EdgePointer>::iterator it = changeNode->inputs.begin(); it != changeNode->inputs.end(); it++) {
			if ((*it)->name == oldOutput->name) {
				//cout << "Changed at " << changeNode->name <<" from " << (*it)->name << " to " << edgeName << endl;
				it = changeNode->inputs.erase(it);
				changeNode->inputs.push_back(EdgePointer(this->edges.find(edgeName)));
				break;
			}
		}
		//cout << "Test6" << endl;
		//for (size_t i = 0; i < changeNode->getInputsCount(); i++) {
		//	cout << "Input " << i << " is " << changeNode->inputs.at(i)->name << endl;
		//}
		//checkForDoubleNot(&nodes.at(newNodeIndex));
		//cout << "Edge: " << oldOutput->name << " ,source: " << oldOutput->getSource() << endl;
		//recursivelyRemoveEdge(oldOutput);
		//cout << "Not out." << endl;
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::insertNewNode(const string& nodeName, const Node::NodeType nodeType, const string& portDescription, const string& trise, const string& tfall, int revLevel)
	{
		insertNode(nodeName == "" ? (string("GATE_") + to_string(emptyNodesCounter++)) : nodeName, revLevel);
		nodes[nodes.size() - 1].assignTask(nodeType, portDescription, trise, tfall);
		if (nodeType == Node::NOT) notNodes.push_back(nodes.size() - 1);
	}

//_____________________________________________________________________________________________________________________________________
	std::string Circuit::getModuleName() const
	{
		return moduleName;
	}

//_____________________________________________________________________________________________________________________________________
	Node& Circuit::node(size_t index)
	{
		if (index >= getNodesCount()) throw out_of_range(" (Circuit) Node index out of range");
		return nodes[index];
	}

//_____________________________________________________________________________________________________________________________________
	Node& Circuit::node(const string& nodeName)
	{
		if (!nodeIndex.count(nodeName)) throw invalid_argument("Node \"" + nodeName + "\" not found");
		return node(nodeIndex[nodeName]);
	}

//_____________________________________________________________________________________________________________________________________
	size_t Circuit::getNodesCount() const
	{
		return nodes.size();
	}

//_____________________________________________________________________________________________________________________________________
	size_t Circuit::getInputNodesCount() const
	{
		return inputNodes.size();
	}

//_____________________________________________________________________________________________________________________________________
	Node& Circuit::inputNode(size_t index)
	{
		if (index >= getInputNodesCount()) throw out_of_range("(Input) Node index out of range");
		return node(inputNodes[index]);
	}

//_____________________________________________________________________________________________________________________________________	
	vector<int> Circuit::getInputNodes() const
	{
		return inputNodes;
	}
	
//_____________________________________________________________________________________________________________________________________	
	vector<int> Circuit::getOutputNodes() const
	{
		return outputNodes;
	}

//_____________________________________________________________________________________________________________________________________
	size_t Circuit::getOutputNodesCount() const
	{
		return outputNodes.size();
	}

//_____________________________________________________________________________________________________________________________________
	Node& Circuit::outputNode(size_t index)
	{
		if (index >= getOutputNodesCount()) throw out_of_range("(Output) Node index out of range");
		return node(outputNodes[index]);
	}

//_____________________________________________________________________________________________________________________________________
	size_t Circuit::getGatesCount() const {
		return getNodesCount() - getInputNodesCount() - getOutputNodesCount();
	}

//_____________________________________________________________________________________________________________________________________
	size_t Circuit::getNodeIndex(const string& nodeName) const {
		return nodeIndex.at(nodeName);
	}
	
//_____________________________________________________________________________________________________________________________________	
void Circuit::nodesInWindowRange(Node* startNode, int range, set<int>& allNodesInWindow) {
	if (range < 0) return;
	if (range > 1) {
		for (size_t i = 0; i < startNode->getInputsCount(); i++) {
			nodesInWindowRange(&(startNode->inputNode(i)), range - 1, allNodesInWindow);
			allNodesInWindow.insert(startNode->inputNode(i).nIndex);
		}
	} else if (range == 1) {
		for (size_t i = 0; i < startNode->getInputsCount(); i++) {
			allNodesInWindow.insert(startNode->inputNode(i).nIndex);
		}
	} 
	allNodesInWindow.insert(startNode->nIndex);
}	

//_____________________________________________________________________________________________________________________________________
void Circuit::improvedWindow(Node* startNode, int range, set<int>& windowNodes) {
	if (range < 0) return;
	vector<Node*> frontier;
	vector<Node*> tempFront;
	bool inserted;
	if (range > 1) {
		frontier.push_back(startNode);
		for (int step = range; step > 0; step--) {
			for (vector<Node*>::iterator it = frontier.begin(); it != frontier.end(); ++it) {
				for (size_t i=0; i < (*it)->getInputsCount(); i++) {
					inserted = (windowNodes.insert((*it)->inputNode(i).nIndex)).second;
					if (inserted && (step > 1)) tempFront.push_back(&((*it)->inputNode(i)));
				}
			}
			if (step > 1) {
				frontier = tempFront;
				tempFront.clear();
			}
		}
	} else if (range == 1) {
		for (size_t i = 0; i < startNode->getInputsCount(); i++) {
			windowNodes.insert(startNode->inputNode(i).nIndex);
		}
	}
	windowNodes.insert(startNode->nIndex);  // Dont forget to add startNode to window as well.
}

//_____________________________________________________________________________________________________________________________________
void Circuit::improvedWindow(vector<Node*> frontier, int range, set<int>& windowNodes) {
	if (range < 0) return;
	vector<Node*> tempFront;
	bool inserted;
	for (auto& elem: frontier) {  // At first, add frontier nodes to set of windowNodes.
		windowNodes.insert(elem->nIndex);
	}
	if (range > 0) {
		for (int step = range; step > 0; step--) {
			if (frontier.empty()) return;
			for (vector<Node*>::iterator it = frontier.begin(); it != frontier.end(); ++it) {
				for (size_t i=0; i < (*it)->getInputsCount(); i++) {
					inserted = (windowNodes.insert((*it)->inputNode(i).nIndex)).second;
					if (inserted && (step > 1)) tempFront.push_back(&((*it)->inputNode(i)));
				}
			}
			if (step > 1) {
				frontier = tempFront;
				tempFront.clear();
			}
		}
	}
}
	
//_____________________________________________________________________________________________________________________________________	
std::vector<fullAdder> Circuit::findFAs() {  // Define reversed topological order of all nodes based on FA finding and ordering.
		std::vector<fullAdder> adders;
		if (this->getGatesCount() == 0) {
			std::cout << "Number of gate nodes is 0. End reverse Order method. " << std::endl;
			return adders;
		}
		std::set<std::string> alreadyDoneSignals;
		int andFound = -1;
		int xorFound = -1;
		Node* andPointer = 0;
		Node* xorPointer = 0;
		Node* and2Pointer = 0;
		Node* xor2Pointer = 0;
		for (auto& elem: this->edges) {
			andFound = -1;
			xorFound = -1;
			andPointer = 0;
			xorPointer = 0;
			and2Pointer = 0;
			xor2Pointer = 0;
			if (elem.second.nDestinations() < 2) continue;
			if (alreadyDoneSignals.count(elem.second.name) > 0) continue;
			for (size_t i = 0; i < elem.second.nDestinations(); i++) {
				if (nodes.at(elem.second.getDestination(i)).type == Node::AND) andFound = i;
				else if (nodes.at(elem.second.getDestination(i)).type == Node::XOR) xorFound = i;
			}
			if (andFound != -1 && xorFound != -1) {  // Candidate for FA signal. Find out if it is input of an FA.
				andPointer = &nodes.at(elem.second.getDestination(andFound));
				xorPointer = &nodes.at(elem.second.getDestination(xorFound));
				if (xorPointer->getOutputsCount() == 2) {  // Possible xor1 candidate.
					if (xorPointer->outputNode(0).type == Node::XOR && xorPointer->outputNode(1).type == Node::AND) {
						xor2Pointer = &xorPointer->outputNode(0);
						and2Pointer = &xorPointer->outputNode(1);
					}
					else  if (xorPointer->outputNode(0).type == Node::AND && xorPointer->outputNode(1).type == Node::XOR) {
						xor2Pointer = &xorPointer->outputNode(1);
						and2Pointer = &xorPointer->outputNode(0);
					}

				} else continue;
				if (and2Pointer != 0) {  // Signal is likely to be input of a FA.  Check whether first and second AND go into same or.
					if (andPointer->getOutputsCount() == 1 && and2Pointer->getOutputsCount() == 1)  {
						if (andPointer->outputNode(0).type == Node::OR && and2Pointer->outputNode(0).type == Node::OR) {
							if (andPointer->outputNode(0).name == and2Pointer->outputNode(0).name) {
								// Signal is with high probability input of an FA!
							} else continue;
						} else continue;
					} else continue;
				} else continue;
				// Check for cin signal ?. Maybe not neccesary.
			} else continue;
			// All checks past. Add this FA to the list.
			adders.push_back(fullAdder(andPointer->outputNode(0).nIndex, andPointer->nIndex, and2Pointer->nIndex, xorPointer->nIndex, xor2Pointer->nIndex));
			// Add index of this FA to the nodes of it.
			andPointer->outputNode(0).adder = adders.size() - 1;
			andPointer->adder = adders.size() - 1;
			and2Pointer->adder = adders.size() - 1;
			xorPointer->adder = adders.size() - 1;
			xor2Pointer->adder = adders.size() - 1;
			// Add second input to alreadyDone list, so this FA is not doubled in list.
			if (andPointer->inputs.at(0)->name == elem.second.name) {
				if (andPointer->inputs.at(1)->name != "zeroWire" && andPointer->inputs.at(1)->name != "oneWire") {
					alreadyDoneSignals.insert(andPointer->inputs.at(1)->name);
				}
			}
			else if (andPointer->inputs.at(1)->name == elem.second.name) {
				if (andPointer->inputs.at(0)->name != "zeroWire" && andPointer->inputs.at(0)->name != "oneWire") {
					alreadyDoneSignals.insert(andPointer->inputs.at(0)->name);
				}
			}
			else cout << "FA finden: ETWAS IST SCHIEF GELAUFEN!." << endl;
		}

		std::cout << "findFAs() successfull. Number of found FAs is: " << adders.size() << std::endl;

		return adders;
	}

//_____________________________________________________________________________________________________________________________________
void Circuit::setFanInsOutsOfFAs() {
//	Node* currNode;
//	for (auto& elem: this->foundAdders) {
//		// Check in1 and in2.
//		currNode = &nodes.at(elem.and1);
//		elem.prevIn1 = currNode->inputNode(0).nIndex;
//		elem.prevIn2 = currNode->inputNode(1).nIndex;
//		if (currNode->inputNode(0).adder != -1) elem.prevIn1IsFA = true;
//		if (currNode->inputNode(1).adder != -1) elem.prevIn2IsFA = true;
//		// Check cin.
//		currNode = &nodes.at(elem.and2);
//		if (currNode->inputNode(0).name == nodes.at(elem.xor1).name) {
//			elem.prevCin = currNode->inputNode(1).nIndex;
//			if (currNode->inputNode(1).adder != -1) elem.prevCinIsFA = true;
//		} else if (currNode->inputNode(1).name == nodes.at(elem.xor1).name) {
//			elem.prevCin = currNode->inputNode(0).nIndex;
//			if (currNode->inputNode(0).adder != -1) elem.prevCinIsFA = true;
//		}
//		// Check  nextOr.
//		currNode = &nodes.at(elem.or1);
//		for (size_t i = 0; i < currNode->getOutputsCount(); i++) {
//			elem.nextOr.push_back(currNode->outputNode(i).nIndex);
//			if (currNode->outputNode(i).adder != -1) elem.nextOrIsFA.push_back(true);
//			else elem.nextOrIsFA.push_back(false);
//		}
//		// Check nextXOR.
//		currNode = &nodes.at(elem.xor2);
//		for (size_t i = 0; i < currNode->getOutputsCount(); i++) {
//			elem.nextXor.push_back(currNode->outputNode(i).nIndex);
//			if (currNode->outputNode(i).adder != -1) elem.nextXorIsFA.push_back(true);
//			else elem.nextXorIsFA.push_back(false);
//		}
//	}
}

//_____________________________________________________________________________________________________________________________________
void Circuit::addSingleNodesToExtBlocks()  {
	for (auto& elem: this->nodes) {
		if (elem.type == Node::INPUT_PORT || elem.type == Node::OUTPUT_PORT || elem.type == Node::DELETED) continue;
		if (elem.block != -1) continue;
		this->foundExtBlocks.emplace_back(elem.nIndex);
		elem.block = foundExtBlocks.size() -1;
		for (size_t i=0; i < elem.getInputsCount(); ++i) foundExtBlocks.back().fanIns.insert(elem.inputNode(i).nIndex);
		for (size_t i=0; i < elem.getOutputsCount(); ++i) foundExtBlocks.back().fanOuts.insert(elem.outputNode(i).nIndex);
	}
}

//_____________________________________________________________________________________________________________________________________
std::vector<extBlock> Circuit::initExtBlocksFromAdders()  {
	std::vector<extBlock> blocks;
	for (auto& elem: this->foundAdders) {
		blocks.emplace(blocks.end(), elem);  // Create extBlock from fullAdder.
		int currBlockNum = blocks.size() - 1;
		this->nodes.at(elem.and1).block = currBlockNum;
		this->nodes.at(elem.and2).block = currBlockNum;
		this->nodes.at(elem.xor1).block = currBlockNum;
		this->nodes.at(elem.xor2).block = currBlockNum;
		this->nodes.at(elem.or1).block = currBlockNum;
	}
	return blocks;
}

//_____________________________________________________________________________________________________________________________________
void Circuit::extendBlocksWithRemainingGates()  {
	if (foundExtBlocks.empty()) return;
	//vector<int> extendedNodes;
	unordered_set<int> tempFanInNodes;
	vp::Node* currNode;
	vector<int> currNodes;
	vector<int> tempNodes;
	for (size_t i=0; i < this->foundExtBlocks.size(); ++i) {
			tempFanInNodes.clear();
			currNodes.clear();
			currNodes.insert(currNodes.begin(), foundExtBlocks.at(i).fanIns.begin(), foundExtBlocks.at(i).fanIns.end());
			//processedSignals.insert(it->signals.at(i));
			while (currNodes.size() > 0) {
				tempNodes.clear();
				for (auto& elem: currNodes) {
					currNode = &this->node(elem);
					//cout << "currNode: " << currNode->name << endl;
					bool same = true;
					if (currNode->block != -1) same = false;  // If currNode is already part of a full adder continue.
					if (currNode->type == vp::Node::INPUT_PORT) same = false; // If currNode is an input stop traversing here.
					if (currNode->getOutputsCount() > 1) {
						for (size_t k=0; k < currNode->getOutputsCount(); ++k) {
							if (currNode->outputNode(k).block == -1) {
								same = false; // preNode has fanout > 1 with at least 1 which is not part of an adder.
								break;
							}
							for (size_t l=k+1; l < currNode->getOutputsCount(); ++l) {
								if (currNode->outputNode(k).block != currNode->outputNode(l).block) same = false;
							}
							if (!same) break;
						}
					}
					//cout << "same is " << same << endl;
					if (same) {  // same==true means currNode can be added to fanout free cone.
						foundExtBlocks.at(i).addNode(elem);
						currNode->block = i;
						for (size_t j=0; j < currNode->getInputsCount(); ++j) {  // currNode is part of fanout free cone. Add
							tempNodes.push_back(currNode->inputNode(j).nIndex);
							//assert(currNode->adder == -1);
							//cout << "currNode adder is " << currNode->adder << endl;
							//currNode->adder = adderNum;
							//extendedAtomics.at(adderNum).push_back(currNode->nIndex);
							//cout << "added to temp: " << currNode->inputs.at(j)->name << endl;
						}
					} else {  // same=false means currNode can not be added, meaning this is a fanIn node of the block.
						tempFanInNodes.insert(elem);
//						if (processedSignals.count(elem) == 0) {
//							extendedSignals.push_back(elem);
//							processedSignals.insert(elem);  // Remember that this signal was already added to avoid duplicates.
//							//cout << "added to extended: " << elem << endl;
//						}
					}
				}
				currNodes = tempNodes;
			}
			// One extended block has been processed. Set new fanIns of the block before continuing with next block.
			foundExtBlocks.at(i).fanIns = tempFanInNodes;
		}
//		for (auto& elem: extendedSignals) {
//			cout << "|" << elem;
//		}
}

std::vector<Multiplexer> Circuit::findMuxs() {  // Define reversed topological order of all nodes based on multiplexer finding and ordering.
		std::vector<Multiplexer> multiplexer;
		if (this->getGatesCount() == 0) {
			std::cout << "Number of gate nodes is 0. End reverse Order method. " << std::endl;	
			return multiplexer;
		}
		std::set<std::string> alreadyDoneSignals;
		int orFound = -1;
		int and1Found = -1;
		int and2Found = -1;
		int negFound = -1;
		Node* poss_neg = 0;
		Node* orPointer = 0;
		Node* and1Pointer = 0;
		Node* and2Pointer = 0;
		Node* negPointer = 0;
		for (auto& elem: this->edges) {
			orFound = -1;
			and1Found = -1;
			and2Found = -1;
			negFound = -1;
			orPointer = 0;
			and1Pointer = 0;
			and2Pointer = 0;
			negPointer = 0;
			poss_neg = 0;
		
			if (elem.second.nDestinations() != 1){ continue;}
			if (alreadyDoneSignals.count(elem.second.name) > 0){ continue;}
			if (nodes.at(elem.second.getDestination(0)).type == Node::OR){ orFound = 0;} else continue;
			if (orFound != -1) {  // Candidate for Mux signal. Find out if it is insinde of an Mux.
				orPointer = &nodes.at(elem.second.getDestination(0));
				if (orPointer->getInputsCount() == 2) {
					if (orPointer->inputNode(0).type == Node::AND && orPointer->inputNode(1).type == Node::AND) {
						and1Pointer = &orPointer->inputNode(0);
						and2Pointer = &orPointer->inputNode(1);
						and1Found = 0;
						and2Found = 1;
					} else continue;
				} else continue;
				if (and2Found != -1 && and1Found != -1 ) {  // Signal is likely to be inside of a Mux.
					if (and2Pointer->getInputsCount() != 2 || and1Pointer->getInputsCount() != 2) continue;
					if (and1Pointer->inputNode(0).type == Node::NOT) {
						poss_neg = &and1Pointer->inputNode(0);
						if (poss_neg->inputNode(0).name == and2Pointer->inputNode(0).name) {
							negPointer = &and1Pointer->inputNode(0);
						} else if (poss_neg->inputNode(0).name == and2Pointer->inputNode(1).name) {
							negPointer = &and1Pointer->inputNode(0);
						} else continue;
					} else if (and1Pointer->inputNode(1).type == Node::NOT) {
						poss_neg = &and1Pointer->inputNode(1);
						if (poss_neg->inputNode(0).name == and2Pointer->inputNode(0).name) {
							negPointer = &and1Pointer->inputNode(1);
						} else if (poss_neg->inputNode(0).name == and2Pointer->inputNode(1).name) {
							negPointer = &and1Pointer->inputNode(1);
						} else continue;
					} else if (and2Pointer->inputNode(0).type == Node::NOT) {
						poss_neg = &and2Pointer->inputNode(0);
						if (poss_neg->inputNode(0).name == and1Pointer->inputNode(0).name) {
							negPointer = &and2Pointer->inputNode(0);
						} else if (poss_neg->inputNode(0).name == and1Pointer->inputNode(1).name) {
							negPointer = &and2Pointer->inputNode(0);
						} else continue;
					} else if (and2Pointer->inputNode(1).type == Node::NOT) {
						poss_neg = &and2Pointer->inputNode(1);
						if (poss_neg->inputNode(0).name == and1Pointer->inputNode(0).name) {
							negPointer = &and2Pointer->inputNode(1);
						} else if (poss_neg->inputNode(0).name == and1Pointer->inputNode(1).name) {
							negPointer = &and2Pointer->inputNode(1);
						} else continue;
					} else continue;
				} else continue;
			} else continue;
			
			// All checks passed. Add this Mux to the list.
			multiplexer.push_back(Multiplexer(orPointer->nIndex, and1Pointer->nIndex, and2Pointer->nIndex, negPointer->nIndex));
			
			// Add index of this Mux to the nodes of it.
			
			orPointer->mux = multiplexer.size()- 1;
			and1Pointer->mux = multiplexer.size()-1 ;
			and2Pointer->mux = multiplexer.size()-1;
			negPointer->mux = multiplexer.size()-1;

			// Add second input to alreadyDone list, so this UL is not doubled in list.
			if (orPointer->inputs.at(0)->name == elem.second.name) {
				if (orPointer->inputs.at(1)->name != "zeroWire" && orPointer->inputs.at(1)->name != "oneWire") {
					alreadyDoneSignals.insert(orPointer->inputs.at(1)->name);
				}
			} 
			else if (orPointer->inputs.at(1)->name == elem.second.name) {
				if (orPointer->inputs.at(0)->name != "zeroWire" && orPointer->inputs.at(0)->name != "oneWire") {
					alreadyDoneSignals.insert(orPointer->inputs.at(0)->name);
				}
			}
			else cout << "UL finden: ETWAS IST SCHIEF GELAUFEN!." << endl;
		}
		mux_size = std::sqrt(multiplexer.size()) -1;
		std::cout << "findMuxs() successfull. Number of found Muxs is: " << multiplexer.size() << std::endl;
		std::cout<<"mux last stage"<< mux_size<<endl;
		
		return multiplexer;
}


//_____________________________________________________________________________________________________________________________________
void Circuit::orderAllNodesAdvanced() {
//	cout << "Ordering nodes started. " << endl;
//	if (this->foundExtBlocks.empty()) { cout << "Circuit has no EABs. Abort ordering of nodes." << endl; }
//	vector<int> candidateEABs;  // Save candidates for leveling next here.
//	vector<int> readyEABs;  // Save EABs which are ready to be leveled here.
//	int currMaxLevel = 0;
//	// Give all outputNodes level 0.
//	Node* currNode;
//	for(size_t i=0; i < this->getOutputNodesCount(); ++i) {
//		currNode = &this->outputNode(i);
//		currNode->revLevel = 0;
////		cout << "outputNode is: " << currNode->name << endl;
//		for (size_t j=0; j < currNode->getInputsCount(); ++j) {
//			for (size_t k=0; k < currNode->inputNode(j).getOutputsCount(); ++k) {
//				if (currNode->inputNode(j).outputNode(k).revLevel < 0) break;
//				candidateEABs.push_back(currNode->inputNode(j).block);
//			}
//		}
//	}
////	cout << "candidates are: " << endl;
////	for (auto& elem: candidateEABs) {
////		 cout << this->node(*this->foundExtBlocks.at(elem).memNodes.begin()).name << "|";
////	}
////	cout << endl;
//	// Check which of candidateEABs can be leveled already and push it to readyEABs vector.
//	extBlock* candBlock;
//	bool stopFanOuts;
//	assert(readyEABs.empty());
//	while (!candidateEABs.empty() || !readyEABs.empty()) {
//		for (auto& elem: candidateEABs) {
////			cout << "current candidate has num: " << elem << endl;
//			stopFanOuts = false;
//			candBlock = &this->foundExtBlocks.at(elem);
//			for (auto& fanOutIte: candBlock->fanOuts) {  // Check if all fanOut blocks of the candidate are already set with a level.
//				if (this->node(fanOutIte).type == Node::OUTPUT_PORT) continue;
//				if (!this->foundExtBlocks.at(this->node(fanOutIte).block).levelIsSet) stopFanOuts = true;
//				if (stopFanOuts) break;
//			}
//			if (!stopFanOuts) readyEABs.push_back(elem);
//
//		}
////		cout << "readyEABs are: " << endl;
////		for (auto& inner: readyEABs) {
////			cout << inner << "|";
////		}
////		cout << endl;
//		candidateEABs.clear();  // Ready blocks have been determined. Clear candidate list for next iteration.
//		// Choose one of the readyEABs and level it. Afterwards push its predecessor blocks to the candidate blocks.
//		if (!readyEABs.empty()) {
////			cout << "ready EAB has number " << readyEABs.back() << endl;
//			currMaxLevel = setLevelsOfEAB(readyEABs.back(), currMaxLevel);
//			candBlock = &this->foundExtBlocks.at(readyEABs.back());
//			for (auto& fanInIte: candBlock->fanIns) {
//				if (this->node(fanInIte).type != Node::INPUT_PORT) candidateEABs.push_back(this->node(fanInIte).block);
//			}
//			readyEABs.pop_back();
//		}
//	}
//

}

//_____________________________________________________________________________________________________________________________________
int Circuit::setLevelsOfEAB(int blockIndex, int prevMaxLevel) {
//	//TODO: Noch richtig leveln.
////	cout << "leveling of block has started: " << blockIndex << endl;
	int currMaxLevel = prevMaxLevel;
//	extBlock* currBlock = &this->foundExtBlocks.at(blockIndex);
//	vector<Node*> candNodes; // vector of candidate nodes to set level for inside the EAB.
//	Node* currNode = NULL;
//	for (auto& elem: currBlock->memNodes) {  // Find first candidates for leveling which are the predecessors of the fanout nodes.
//		currNode = &this->node(elem);
//		bool isLastNode = true;
//		for (size_t i=0; i < currNode->getOutputsCount(); ++i) {
//			if (currNode->outputNode(i).block == blockIndex) isLastNode = false;
//			if (!isLastNode) break;
//		}
//		if (isLastNode) candNodes.push_back(currNode);
////		for (size_t i=0; i < currNode->getInputsCount(); ++i) {
////			if (currNode->inputNode(i).block == blockIndex) candNodes.push_back(&currNode->inputNode(i));
////		}
//	}
////	vector<Node*> copyNodes = candNodes;
////	for (size_t i=0; i < copyNodes.size(); ++i) {
////		candNodes.at(i) = copyNodes.at(copyNodes.size() - 1 - i);
////	}
//	for (auto& elem: candNodes) {
//		elem->revLevel = ++currMaxLevel;
//	}
//	// Now start from candNodes and reverse level all nodes of the EAB.
//	for (int i = 0; i < candNodes.size(); i++) {
////			std::cout << "Frontier node i name: " << candNodes.at(i)->name << std::endl;
//			for (int j = 0; j < candNodes.at(i)->getInputsCount(); j++) {
////				std::cout << "Frontier node j name: " << candNodes.at(i)->inputNode(j).name << std::endl;
//				if (candNodes.at(i)->inputNode(j).block != blockIndex) continue;
//				if (candNodes.at(i)->inputNode(j).revLevel > 0)	continue;
//
//				for (int k = 0; k < candNodes.at(i)->inputNode(j).getOutputsCount(); k++) {
//					if (candNodes.at(i)->inputNode(j).outputNode(k).revLevel == -1) {
//						break;
//					}
////					if (currMaxLevel < (candNodes.at(i)->inputNode(j).outputNode(k).revLevel + 1)) {
////						currMaxLevel = candNodes.at(i)->inputNode(j).outputNode(k).revLevel + 1;
////					}
//					if (k == candNodes.at(i)->inputNode(j).getOutputsCount() - 1) {
//						//std::cout << "revLevel set for: " << frontierNodes.at(i)->inputNode(j).name << std::endl;
//						candNodes.at(i)->inputNode(j).revLevel = ++currMaxLevel;
//						candNodes.push_back(&candNodes.at(i)->inputNode(j));
//					}
//				}
//			}
//	}
//
//	/*
//	for (auto& elem: this->foundExtBlocks.at(blockIndex).memNodes) {
//		this->node(elem).revLevel = prevMaxLevel + 1;
//	}
//	this->foundExtBlocks.at(blockIndex).levelIsSet = true;
//	return prevMaxLevel + 1;
//	*/
//	this->foundExtBlocks.at(blockIndex).levelIsSet = true;
	return currMaxLevel;
}

//_____________________________________________________________________________________________________________________________________
void Circuit::orderAllNodes(std::vector<fullAdder>& adders) {
//	cout << "orderallNodes() started. " << endl;
//	// Give all outputNodes level 0.
//	for(int i = 0; i < this->getOutputNodesCount(); i++) {
//			this->outputNode(i).revLevel = 0;
//			cout << "outputNode is: " << this->outputNode(i).name << endl;
//	}
//	// Find for all FAs previous and next FAs and get next and prev. indices.
//	Node* currNode;
//	for (auto& elem: adders) {
//		// Check in1 and in2.
//		currNode = &nodes.at(elem.and1);
//		elem.prevIn1 = currNode->inputNode(0).nIndex;
//		elem.prevIn2 = currNode->inputNode(1).nIndex;
//		if (currNode->inputNode(0).adder != -1) elem.prevIn1IsFA = true;
//		if (currNode->inputNode(1).adder != -1) elem.prevIn2IsFA = true;
//		// Check cin.
//		currNode = &nodes.at(elem.and2);
//		if (currNode->inputNode(0).name == nodes.at(elem.xor1).name) {
//			elem.prevCin = currNode->inputNode(1).nIndex;
//			if (currNode->inputNode(1).adder != -1) elem.prevCinIsFA = true;
//		} else if (currNode->inputNode(1).name == nodes.at(elem.xor1).name) {
//			elem.prevCin = currNode->inputNode(0).nIndex;
//			if (currNode->inputNode(0).adder != -1) elem.prevCinIsFA = true;
//		}
//		// Check  nextOr.
//		currNode = &nodes.at(elem.or1);
//		for (size_t i = 0; i < currNode->getOutputsCount(); i++) {
//			elem.nextOr.push_back(currNode->outputNode(i).nIndex);
//			if (currNode->outputNode(i).adder != -1) elem.nextOrIsFA.push_back(true);
//			else elem.nextOrIsFA.push_back(false);
//		}
//		// Check nextXOR.
//		currNode = &nodes.at(elem.xor2);
//		for (size_t i = 0; i < currNode->getOutputsCount(); i++) {
//			elem.nextXor.push_back(currNode->outputNode(i).nIndex);
//			if (currNode->outputNode(i).adder != -1) elem.nextXorIsFA.push_back(true);
//			else elem.nextXorIsFA.push_back(false);
//		}
//	}
//
//	//cout << "Fnding first started. " << endl;
//	int firstFA = -1;
//	int step = 0;
//	bool one, two;
//	// Find first FA and set his levels.
//	for (auto& elem: adders) {
//		one = false;
//		two = false;
//		if (elem.nextOrIsFA.size() == 0) one = true;
//		for (size_t i=0; i < elem.nextOrIsFA.size(); i++) {
//			if (elem.nextOrIsFA.at(i) == true) break;
//			if (i == elem.nextOrIsFA.size()-1) one = true;
//		}
//		if (!one) {
//			step++;
//			continue;
//		}
//		if (elem.nextXorIsFA.size() == 0) two = true;
//		for (size_t i=0; i < elem.nextXorIsFA.size(); i++) {
//			if (elem.nextXorIsFA.at(i) == true) break;
//			if (i == elem.nextXorIsFA.size()-1) two = true;
//		}
//		if (one && two) {
//			firstFA = step;
//			break;
//		}
//		//cout << "First FA has number: " << firstFA << endl;
//		step++;
//	}
//	//cout << "set level 0: " << setLevelsOfFA(adders, firstFA) << endl;
//	int firstAlternative = -1;
//	int test = 0;
//	int prevMaxLevel = -1;
//
//	//*
//	// Follow chain of FAs backwards and set all levels.
//	prevMaxLevel = setLevelsOfFA(adders, firstFA, prevMaxLevel);
//	if (adders.size() > 0) {
//		while (test < adders.size() - 1) {
//			if (adders.at(firstFA).prevCinIsFA) {
//				//cout << "Cin: " << nodes.at(adders.at(firstFA).prevCin).adder << endl;
//				prevMaxLevel = setLevelsOfFA(adders, nodes.at(adders.at(firstFA).prevCin).adder, prevMaxLevel);
//				//cout << "prevMaxLevel:" << prevMaxLevel << endl;
//				if (firstAlternative == -1 && adders.at(firstFA).prevIn1IsFA) firstAlternative = nodes.at(adders.at(firstFA).prevIn1).adder;
//				if (firstAlternative == -1 && adders.at(firstFA).prevIn2IsFA) firstAlternative = nodes.at(adders.at(firstFA).prevIn2).adder;
//				firstFA = nodes.at(adders.at(firstFA).prevCin).adder;
//			} else {
//				if (firstAlternative != -1) {
//					//cout << "first alternative: " << firstAlternative << endl;
//					prevMaxLevel = setLevelsOfFA(adders, firstAlternative, prevMaxLevel);
//					//cout << "prevMaxLevel:" << prevMaxLevel << endl;
//					firstFA = firstAlternative;
//					firstAlternative = -1;
//				}
//			}
//			test++;
//		}
//	}
//	// All FA levels set. Set now all levels of nodes which are not contained in FAs.
//	for (size_t i =0; i < this->nodes.size(); i++) {
//		if (nodes.at(i).revLevel == -1) {
//			setLevelOfNode(adders, i);
//		}
//	}
//	cout << "orderallNodes() finished." << endl;
}	

//_____________________________________________________________________________________________________________________________________
int Circuit::setLevelsOfFA(std::vector<fullAdder>& adders, int adderIndex, int prevMaxLevel) {
//	//cout << "Test 1: " << adderIndex <<endl;
//	if (adders.size() == 0) {
//		cout << "No adders found." << endl;
//		return 0;
//	}
//	fullAdder& curr = adders.at(adderIndex);
//	// Now max previous level is obtained. Give all nodes of this adder their level.
//	//*
//	nodes.at(curr.or1).revLevel = prevMaxLevel + 12;
//	nodes.at(curr.and1).revLevel = prevMaxLevel + 13;
//	nodes.at(curr.and2).revLevel = prevMaxLevel + 14;
//	nodes.at(curr.xor2).revLevel = prevMaxLevel + 15;
//	nodes.at(curr.xor1).revLevel = prevMaxLevel + 16;
//	//*/
//	/*
//	nodes.at(curr.xor2).revLevel = prevMaxLevel + 12;
//	nodes.at(curr.or1).revLevel = prevMaxLevel + 13;
//	nodes.at(curr.and2).revLevel = prevMaxLevel + 14;
//	nodes.at(curr.xor1).revLevel = prevMaxLevel + 15;
//	nodes.at(curr.and1).revLevel = prevMaxLevel + 16;
//	//*/
//	curr.maxLevel = prevMaxLevel + 16;
//	return curr.maxLevel;
	return 0;
}

//_____________________________________________________________________________________________________________________________________
int Circuit::setLevelOfNode(std::vector<fullAdder>& adders, int nodeIx) {
//	Node* currNode = &nodes.at(nodeIx);
//	cout << "set level of node: " << currNode->name <<endl;
	int prevMaxLevel = -1;
//	int tmp = -1;
//	for (size_t i = 0; i < currNode->getOutputsCount(); i++) {
//		if (currNode->outputNode(i).adder != -1) {  // Outputnode is part of FA. Use maxLevel of this FA.
//			 if (prevMaxLevel < adders.at(currNode->outputNode(i).adder).maxLevel) prevMaxLevel = adders.at(currNode->outputNode(i).adder).maxLevel;
//		} else {  // Outputnode is not part of FA. Get just his revLevel. If it is not set yet, set his revLevel and use return value.
//			if (currNode->outputNode(i).revLevel > -1) {
//				if (prevMaxLevel < currNode->outputNode(i).revLevel) prevMaxLevel = currNode->outputNode(i).revLevel;
//			} else { // revLevel not set yet. Set level of this node first.
//				tmp = setLevelOfNode(adders, currNode->outputNode(i).nIndex);
//				if (prevMaxLevel < tmp) prevMaxLevel = tmp;
//			}
//		}
//	}
//	// Set prevMaxLevel.
//	currNode->revLevel = prevMaxLevel + 1;
//	return currNode->revLevel;
	return prevMaxLevel;
}

//_____________________________________________________________________________________________________________________________________
int Circuit::getSmallestLevel(std::vector<string> signals) {
	if (signals.size() == 0) return 0;
	int result = this->node(this->sortedNodes.back()).revLevel;
	int tmp = 0;
	for (auto& elem: signals) {
		if (this->node(this->edges.at(elem).source).type == vp::Node::INPUT_PORT) continue;
		tmp = this->node(this->edges.at(elem).source).revLevel;
		if (tmp < result) result = tmp;
	}
	return result;
}

//_____________________________________________________________________________________________________________________________________
int Circuit::getBiggestLevel(std::vector<string> signals) {
	if (signals.size() == 0) return 0;
	int result = 0;
	int tmp = 0;
	for (auto& elem: signals) {
		tmp = this->node(this->edges.at(elem).source).revLevel;
		if (tmp > result) result = tmp;
	}
	return result;
}

//_____________________________________________________________________________________________________________________________________
	void Circuit::revLevelizeCircuit() {	// Define reversed topological order of all nodes.
		if (this->getGatesCount() == 0) {
			std::cout << "Number of gate nodes is 0. End revLevel method. " << std::endl;	
			return;
		} 
		int maxLevel = 0;
		std::vector<vp::Node*> frontierNodes;
		for(int i = 0; i < this->getOutputNodesCount(); i++) {
			this->outputNode(i).revLevel = 0;
			frontierNodes.push_back(&this->outputNode(i));
		}
		for (int i = 0; i < frontierNodes.size(); i++) {
			//std::cout << "Frontier node i name: " << frontierNodes.at(i)->name << std::endl;
			for (int j = 0; j < frontierNodes.at(i)->getInputsCount(); j++) {
				//std::cout << "Frontier node j name: " << frontierNodes.at(i)->inputNode(j).name << std::endl;
				if (frontierNodes.at(i)->inputNode(j).revLevel > 0) {
					continue;
				}
				maxLevel = 0;
			
				for(int k = 0; k < frontierNodes.at(i)->inputNode(j).getOutputsCount(); k++) {
					if (frontierNodes.at(i)->inputNode(j).outputNode(k).revLevel == -1) {
						break;
					}
					if (maxLevel < (frontierNodes.at(i)->inputNode(j).outputNode(k).revLevel + 1)) {
						maxLevel = frontierNodes.at(i)->inputNode(j).outputNode(k).revLevel + 1;
					}
					if (k == frontierNodes.at(i)->inputNode(j).getOutputsCount() - 1) {
						//std::cout << "revLevel set for: " << frontierNodes.at(i)->inputNode(j).name << std::endl;
						frontierNodes.at(i)->inputNode(j).revLevel = maxLevel; 
						frontierNodes.push_back(&frontierNodes.at(i)->inputNode(j));
					}
				}
			}
		}
		std::cout << "revLevelizeCircuit() successfull" << std::endl;
	}
	
//_____________________________________________________________________________________________________________________________________	
	void Circuit::swapNodes(int& a, int& b) {
		int tmp = a; 
    	a = b; 
    	b = tmp; 
	}

//_____________________________________________________________________________________________________________________________________
	int Circuit::partition(int low, int high) {
		int pivot = this->sortedNodes.at(high);    // pivot 
//    		int i = (low - 1);  // Index of smaller element
			int i = (low-1);  // Index of smaller element
  
    		for (int j = low; j <= high - 1; j++) {
        		// If current element is smaller than or 
        		// equal to pivot 
       		 	if (this->node(this->sortedNodes.at(j)).revLevel <= this->node(pivot).revLevel) { 
        		    i++;    // increment index of smaller element 
        		    swapNodes(this->sortedNodes.at(i), this->sortedNodes.at(j)); 
        		} 
    		} 
   		swapNodes(this->sortedNodes.at(i+1), this->sortedNodes.at(high)); 
    		return (i + 1); 
	}

//_____________________________________________________________________________________________________________________________________
	void Circuit::quickSort(int low, int high) {
		//cout << "in quicksort now:" << endl;
		if (low < high) { 
        		/* pi is partitioning index, arr[p] is now 
        		   at right place */
				int pi = partition(low, high);
  		
        		// Separately sort elements before 
        		// partition and after partition 
        		this->quickSort(low, pi - 1); 
        		this->quickSort(pi + 1, high); 
    	} 	 
	}
	
//_____________________________________________________________________________________________________________________________________	
	void Circuit::sortNodesByLevel() {
		// Sort nodes by level with quick sort. Save indices in seperate vector sortedNodes.
		//std::cout << "sortNodesByLevel() successfull" <<std::endl;
		this->sortedNodes.clear();
		//assert(sortedNodes.size() == 0);
		// Start with reverse nodes vector, so many nodes are already sorted correct.
		for (std::vector<Node>::reverse_iterator rit = this->nodes.rbegin(); rit != this->nodes.rend(); ++rit) {
			if (rit->type == Node::DELETED /*|| rit->revLevel == -1*/) continue;
			else this->sortedNodes.push_back(rit->nIndex);
//			cout << rit->nIndex << endl;
		}
		size_t start = 0;
		size_t end = this->sortedNodes.size() - 1;
		std::cout << "before quicksort" <<std::endl;
		this->quickSort(start, end);
		std::cout << "sortNodesByLevel() successfull" <<std::endl;
	}

//_____________________________________________________________________________________________________________________________________
void Circuit::createAllOrderPermutations(std::vector<std::vector<int>>& allOrderPermutations) {
	vector<Node*> frontierNodes;
	cout << "orderallNodesAIG() started. " << endl;
	// Get all output nodes. Those are startpoints for the permutations.
	for (int i = 0; i < this->getOutputNodesCount(); i++) {
			frontierNodes.push_back(&this->outputNode(i));
	}
	// Create all permutations from starting point.
	unordered_set<int> readyNodes;
	vector<int> tempPermu;
	if (frontierNodes.size() == 1) {
		createPermutationsFromNode(allOrderPermutations, tempPermu, frontierNodes[0]->nIndex, readyNodes);
	} else {
		//TODO: Create permutations for situations with more than one output node.
	}
}

//_____________________________________________________________________________________________________________________________________
void Circuit::createPermutationsFromNode(std::vector<std::vector<int>>& allOrderPermutations, vector<int> prevPermu, int chosenNode, unordered_set<int> readyNodes) {
//	if (allOrderPermutations.size() > 10000000) return;
	vector<int> returnVec = prevPermu;
	returnVec.push_back(chosenNode);
	readyNodes.erase(chosenNode);
//	cout << "returnVec = ";
//	for (auto& elem: returnVec) {
//		cout << elem << "|";
//	}
//	cout << endl;
//	cout << "readyNodesBefore = ";
//	for (auto& elem: readyNodes) {
//		cout << elem << "|";
//	}
//	cout << endl;
	Node* nodeP = &this->node(chosenNode);
	for (size_t i=0; i < nodeP->getInputsCount(); ++i) {
		if (nodeP->inputNode(i).type == vp::Node::INPUT_PORT || nodeP->inputNode(i).type == vp::Node::OUTPUT_PORT || nodeP->inputNode(i).type == vp::Node::DELETED) continue;
		readyNodes.insert(nodeP->inputNode(i).nIndex);
	}
//	cout << "readyNodesAfter = ";
//	for (auto& elem: readyNodes) {
//		cout << elem << "|";
//	}
//	cout << endl;
	if (readyNodes.empty()) {
		allOrderPermutations.push_back(returnVec); return;
	}
//	int nextNode;
	for (auto& elem: readyNodes) {
//		nextNode = *readyNodes.begin();
		//readyNodes.erase(nextNode);
//		cout << "next node chosen is " << elem << endl;
		createPermutationsFromNode(allOrderPermutations, returnVec, elem, readyNodes);
	}
}


//_____________________________________________________________________________________________________________________________________
void Circuit::changeSortedNodePosManually(int oldPos, int newPos) {
	int currIndex = this->sortedNodes.at(oldPos);
	std::vector<int>::iterator it = this->sortedNodes.begin();
	if (oldPos < newPos) { // First delete oldPos and insert at newPos -1.
		std::advance(it, oldPos);
		this->sortedNodes.erase(it);
		it = this->sortedNodes.begin();
		std::advance(it, newPos - 1);
		this->sortedNodes.insert(it, currIndex);
	} else {
		std::advance(it, newPos);
		this->sortedNodes.insert(it, currIndex);
		it = this->sortedNodes.begin();
		std::advance(it, oldPos + 1);
		this->sortedNodes.erase(it);
	}
}

//_____________________________________________________________________________________________________________________________________
void Circuit::createDCCandidatesFromAtomics() {
	std::vector<std::string> atomicInputSignals;
	for (auto& elem: this->foundAdders) {
		atomicInputSignals.clear();
		atomicInputSignals.emplace_back(this->node(elem.xor1).inputs.at(0)->name);
		atomicInputSignals.emplace_back(this->node(elem.xor1).inputs.at(1)->name);
		string internXorName = this->node(elem.xor1).outputs.at(0)->name;
		if (internXorName != this->node(elem.xor2).inputs.at(0)->name) atomicInputSignals.emplace_back(this->node(elem.xor2).inputs.at(0)->name);
		else atomicInputSignals.emplace_back(this->node(elem.xor2).inputs.at(1)->name);
		this->dcCandidatesInit.emplace_back(atomicInputSignals);
	}
	for (auto& elem: this->foundMuxs) {
		atomicInputSignals.clear();
		string selector = this->node(elem.neg).inputs.at(0)->name;
		atomicInputSignals.emplace_back(selector);
		string selectorNegated = this->node(elem.neg).outputs.at(0)->name;
		if (this->node(elem.and1).inputs.at(0)->name == selector || this->node(elem.and1).inputs.at(0)->name == selectorNegated) {
			atomicInputSignals.emplace_back(this->node(elem.and1).inputs.at(1)->name);
		} else {
			atomicInputSignals.emplace_back(this->node(elem.and1).inputs.at(0)->name);
		}
		if (this->node(elem.and2).inputs.at(0)->name == selector || this->node(elem.and2).inputs.at(0)->name == selectorNegated) {
			atomicInputSignals.emplace_back(this->node(elem.and2).inputs.at(1)->name);
		} else {
			atomicInputSignals.emplace_back(this->node(elem.and2).inputs.at(0)->name);
		}
		this->dcCandidatesInit.emplace_back(atomicInputSignals);
	}
}

//_____________________________________________________________________________________________________________________________________
void Circuit::applyTransitivityForReplacedSignals() {
	bool changed = true;
	string new1;
	string new2;
	std::unordered_map<string, string>::iterator nextIt;
	while (changed) {
		changed = false;
		for (std::unordered_map<string, string>::iterator it = this->replacedSignals.begin(); it != this->replacedSignals.end(); ++it) {
			if (replacedSignals.find(it->second) != this->replacedSignals.end()) {
				replacedSignals[it->first] = replacedSignals.find(it->second)->second;
				//new1 = it->first;
				//new2 = replacedSignals.find(it->second);
				//nextIt = replacedSignals.erase(it);
				//replacedSignals.emplace(new1, new2);
				changed = true;
			}
		}
	}
}

//_____________________________________________________________________________________________________________________________________
	// Write all valid circuit nodes to a file.
	void Circuit::writeCircuitToFile(const string& fileName) {
		ofstream outputFile(fileName);
		sortNodesByLevel();
		outputFile << "module rdivider(R_0, D, Q, R_n1);" << endl;
		int countR0=0, countD=0, countQ=0, countRn1=0;
		// First count all inputs and write them down as vectors.
		for (size_t i=0; i < this->inputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->inputNode(i).name.find("R_0") != string::npos) countR0++;
			if (this->inputNode(i).name.find("D") != string::npos) countD++;
		}
		outputFile << "input [" << countR0 - 1 << ":0] R_0;" << endl;
		outputFile << "input [" << countD - 1 << ":0] D;" << endl;

		// Next count all outputs and write them down as vectors.
		for (size_t i=0; i < this->outputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->outputNode(i).name.find("Q") != string::npos) countQ++;
			if (this->outputNode(i).name.find("R_n1") != string::npos) countRn1++;
		}
		outputFile << "output [" << countQ - 1 << ":0] Q;" << endl;
		outputFile << "output [" << countRn1 - 1 << ":0] R_n1;" << endl;

		// Next write all wire declarations.
		for (auto& elem: this->edges) {
			if (elem.first.find("R_0") != string::npos || elem.first.find("D") != string::npos || elem.first.find("Q") != string::npos || elem.first.find("R_n1") != string::npos) continue;
			outputFile << "wire " << elem.first << ";" << endl;
		}
		Node* currNode;
		// Last write all assignments of the signals.
		for (std::vector<int>::reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend(); ++it ) {
			currNode = &this->node(*it);
			if (currNode->type == Node::DELETED || currNode->type == Node::INPUT_PORT || currNode->type == Node::OUTPUT_PORT) continue;
			outputFile << "assign " << currNode->outputs.at(0)->name << " = ";
			if (currNode->type == Node::NOT) {
				outputFile << "~" << currNode->inputs.at(0)->name << " /*" << currNode->revLevel << "*/;" << endl;
				continue;
			} else {
				outputFile << currNode->inputs.at(0)->name;
				if (currNode->type == Node::BUFFER) outputFile << " /*" << currNode->revLevel << "*/;" << endl;
				if (currNode->type == Node::AND) outputFile << " & " << currNode->inputs.at(1)->name << " /*" << currNode->revLevel << "*/;" << endl;
				else if (currNode->type == Node::OR) outputFile << " | " << currNode->inputs.at(1)->name << " /*" << currNode->revLevel << "*/;" << endl;
				else if (currNode->type == Node::XOR) outputFile << " ^ " << currNode->inputs.at(1)->name << " /*" << currNode->revLevel << "*/;" << endl;
			}  
		}
		outputFile << "endmodule";
		outputFile.close();
	}	

//_____________________________________________________________________________________________________________________________________
// Write all valid circuit nodes to a file.
	void Circuit::writeCircuitToFileNoLvls(const string& fileName) {
		ofstream outputFile(fileName);
		sortNodesByLevel();
		outputFile << "module nrdivider(R_0, D, Q, R_n1);" << endl;
		int countR0=0, countD=0, countQ=0, countRn1=0;
		// First count all inputs and write them down as vectors.
		for (size_t i=0; i < this->inputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->inputNode(i).name.find("R_0") != string::npos) countR0++;
			if (this->inputNode(i).name.find("D") != string::npos) countD++;
		}
		outputFile << "input [" << countR0 - 1 << ":0] R_0;" << endl;
		outputFile << "input [" << countD - 1 << ":0] D;" << endl;
		
		cout << "test" << endl;

		// Next count all outputs and write them down as vectors.
		for (size_t i=0; i < this->outputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->outputNode(i).name.find("Q") != string::npos) countQ++;
			if (this->outputNode(i).name.find("R_n1") != string::npos) countRn1++;
		}
		outputFile << "output [" << countQ - 1 << ":0] Q;" << endl;
		outputFile << "output [" << countRn1 - 1 << ":0] R_n1;" << endl;
		
		cout << "test2" << endl;

		// Next write all wire declarations.
		for (auto& elem: this->edges) {
			if (elem.first.find("R_0") != string::npos || elem.first.find("D") != string::npos || elem.first.find("Q") != string::npos || elem.first.find("R_n1") != string::npos) continue;
			outputFile << "wire " << elem.first << ";" << endl;
		}
		cout << "test3" << endl;
		Node* currNode;
		// Last write all assignments of the signals.
		for (std::vector<int>::reverse_iterator it = sortedNodes.rbegin(); it != sortedNodes.rend(); ++it ) {
			currNode = &this->node(*it);
			if (currNode->type == Node::DELETED || currNode->type == Node::INPUT_PORT || currNode->type == Node::OUTPUT_PORT) continue;
			outputFile << "assign " << currNode->outputs.at(0)->name << " = ";
			if (currNode->type == Node::NOT) {
				outputFile << "~" << currNode->inputs.at(0)->name << ";" <<  endl;
				continue;
			} else {
				outputFile << currNode->inputs.at(0)->name;
				if (currNode->type == Node::BUFFER) outputFile << ";" << endl;
				if (currNode->type == Node::AND) outputFile << " & " << currNode->inputs.at(1)->name << ";" << endl;
				else if (currNode->type == Node::OR) outputFile << " | " << currNode->inputs.at(1)->name << ";" << endl;
				else if (currNode->type == Node::XOR) outputFile << " ^ " << currNode->inputs.at(1)->name << ";" << endl;
			}
		}
		outputFile << "endmodule";
		outputFile.close();
		cout << "File was written." << endl;
	}
	
//_____________________________________________________________________________________________________________________________________
// Write all valid circuit nodes to a file.
	void Circuit::writeCircuitSimple(const string& fileName) {
		ofstream outputFile(fileName);
		//sortNodesByLevel();
		outputFile << "module nrdivider(R_0, D, Q, R_n1);" << endl;
		// First count all inputs and write them down as vectors.
		for (size_t i=0; i < this->inputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->inputNode(i).type != Node::INPUT_PORT) continue;
			outputFile << "input " << nodes.at(inputNodes.at(i)).outputs.at(0)->name << ";" << endl;
		}
		
		//cout << "test" << endl;

		// Next count all outputs and write them down as vectors.
		for (size_t i=0; i < this->outputNodes.size(); i++) {
			//if (this->inputNode(i).type == "DELETED") continue;
			if (this->outputNode(i).type != Node::OUTPUT_PORT) continue;
			outputFile << "output " << nodes.at(outputNodes.at(i)).inputs.at(0)->name << ";" << endl;
		}
		
		//cout << "test2" << endl;

		// Next write all wire declarations.
		for (auto& elem: this->edges) {
			if (elem.first.find("R_0") != string::npos || elem.first.find("D") != string::npos || elem.first.find("Q") != string::npos || elem.first.find("R_n1") != string::npos) continue;
			outputFile << "wire " << elem.first << ";" << endl;
		}
		//cout << "test3" << endl;
		Node* currNode;
		//cout << "node size: " << nodes.size() << endl;
		// Last write all assignments of the signals.
		this->sortNodesByLevel();
		for (std::vector<int>::iterator it = sortedNodes.begin(); it != sortedNodes.end(); ++it ) {
			//cout << "first" << endl; 
			currNode = &nodes.at(*it);
			if (currNode->name == "") continue;
			//cout << currNode->name << endl;
			if (currNode->type == Node::DELETED || currNode->type == Node::INPUT_PORT || currNode->type == Node::OUTPUT_PORT) continue;
			outputFile << "assign " << currNode->outputs.at(0)->name << " = ";
			if (currNode->type == Node::NOT) {
				outputFile << "~" << currNode->inputs.at(0)->name << ";" << " /*" << currNode->revLevel << "*/;" <<  endl;
				continue;
			} else {
				outputFile << currNode->inputs.at(0)->name;
				if (currNode->type == Node::BUFFER) outputFile << ";" << " /*" << currNode->revLevel << "*/;" << endl;
				if (currNode->type == Node::AND) outputFile << " & " << currNode->inputs.at(1)->name << ";" << " /*" << currNode->revLevel << "*/;" << endl;
				else if (currNode->type == Node::OR) outputFile << " | " << currNode->inputs.at(1)->name << ";" << " /*" << currNode->revLevel << "*/;" << endl;
				else if (currNode->type == Node::XOR) outputFile << " ^ " << currNode->inputs.at(1)->name << ";" << " /*" << currNode->revLevel << "*/;" << endl;
			}
		}
		outputFile << "endmodule";
		outputFile.close();
		cout << "File was written." << endl;
	}	
}
