/*
 * Verifizierer.cpp
 *
 *  Created on: 08.01.2019
 *      Author: Alexander Konrad
 */


#include "Verifizierer.h"
using std::cout;
std::string atomic_block;
std::map<gendc, int> map_1;
int  set_dc = 0;

int atomic_mux_count =0;
// ***************************************************************************************************************************************
Verifizierer::Verifizierer() {

}

// _______________________________________________________________________________________________________________________________________
Verifizierer::Verifizierer(const string& fileName){  // Construct from a parsed circuit file.
	// Initialize Circuit

	try {
		//Parse circuit file
		this->circuit.parseFile(fileName);

		
	}
	catch (vp::ParseError& e) {
		std::cout << "ParseError: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
		//return;
	}
	catch (exception& e) {
		std::cout << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	// Initialize polynom, read signature file and create signature polynom.
	//this->poly = Polynom(this->circuit.edgeIndex.size());
	//this->poly.addPolynom(createSignaturePolynom(readSignatureFile(sigFile)));
	//this->initRepresentatives();
	//represent = new repr[this->circuit.maxEdgeIndex+1]();

	this->pMap = createDividerPrimarySignalsMap();
}

// _______________________________________________________________________________________________________________________________________
Verifizierer::~Verifizierer() {
	// Clear simTable.
	this->simTable.clear();
	//delete[] this->represent;
	//delete this->solver;
	
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::startVerification(const string& sigFile) {
	
	double timeG = 0.0;
	double tstart = clock();
	
	this->initRepresentatives();

	// Initialize polynom from signature file.
	this->poly = Polynom(this->circuit.maxEdgeIndex + 1);
	this->poly.addPolynom(this->createSignaturePolynom(this->readSignatureFile(sigFile)));	
	
	cout << "Anfangspolynom hat die Größe: " << this->poly.polySet.size() << endl;
	cout << "Anfangspolynom ist: " << this->poly << endl;
	
	this->executeReplacements();
	
	if (this->poly.getSet()->size() < 20) cout << "Endpolynom ist: " << this->poly <<  endl;
	cout << "mit Anzahl Momonem: " << this->poly.polySet.size() << endl;
	
	if (this->poly.getSet()->size() == 0) cout << "VERIFICATION SUCCESSFUL." << endl;

	else  {
		cout << "Ending polynomial is not empty." << endl;
		cout << "Apply input constraint." << endl;
		bool successful = applyICOnPolynomial(this->poly);
		if (successful) cout << "VERIFICATION SUCCESSFUL." << endl;
		else cout << "ENDPOLYNOMIAL NOT EMPTY. VERIFICATION FAILED" << endl;
	}

	timeG += clock() - tstart; // Zeitmessung endet.
	timeG = timeG/CLOCKS_PER_SEC;
	cout << "Complete Verification process needed time: " << timeG << endl;

}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::startAdvancedVerification(const string& sigFile) {
	
	double timeG = 0.0;
	double tstart = clock();
	
	//this->runFirstSimulation();
	
	//return;
	cout << "startAdvancedVerification started." << endl;

	this->initRepresentatives();
	
	//cout << "init finished." << endl;

    //this->checkAllEquiAnti();// comment this if not using sbif
	
	this->changeDCsByRepresent();

	//return;

	//this->circuit.pushNotFanouts();
	
	//this->circuit.pushNotFanouts();

	timeG += clock() - tstart; // Zeitmessung endet.
	timeG = timeG/CLOCKS_PER_SEC;
	//cout << "SAT-based information forwarding needed time: " << timeG << endl;

	timeG = 0.0;
	tstart = clock();
	
	this->circuit.sortNodesByLevel();
	
	// Initialize polynomial from signature file.
	this->poly = Polynom(this->circuit.maxEdgeIndex + 1);
	this->poly.addPolynom(this->createSignaturePolynom(this->readSignatureFile(sigFile)));	
	
	cout << "Starting polynomial has size: " << this->poly.polySet.size() << endl;
	cout << "Anfangspolynom ist: " << this->poly << endl;
	
	this->executeReplacements();
	
	if (this->poly.getSet()->size() < 30) cout << "Endpolynom ist: " << this->poly <<  endl;
	cout << "with number monomials: " << this->poly.polySet.size() << endl;
	
	if (this->poly.getSet()->size() == 0) cout << "VERIFICATION SUCCESSFUL." << endl;
		else  {
			cout << "Ending polynomial is not empty." << this->poly.size()<<endl;
			cout<<"ending polynomial is "<<this->poly<<endl;
			cout << "Apply input constraint." << endl;
		
			bool successful = applyICOnPolynomial(this->poly);
			if (successful) cout << "VERIFICATION SUCCESSFUL." << endl;
			else cout << "ENDPOLYNOMIAL NOT EMPTY. VERIFICATION FAILED" << endl;
		}

	timeG += clock() - tstart; // Zeitmessung endet.
	timeG = timeG/CLOCKS_PER_SEC;
	cout << "Complete Verification process needed time: " << timeG << endl;

}

// _______________________________________________________________________________________________________________________________________
std::pair<std::map<std::string, std::vector<mpz_class>>, std::vector<string>> Verifizierer::readSignatureFile(const string& fileName) {
	// Verify file first.
	cout << "read signatureFile started" << endl;
	bool verify;
	ifstream in(fileName.c_str());
	if (!in) {
		verify = false;
	} else {
		verify = true;
	} 
	in.close();	
	if (!verify) throw vp::ParseError("File not found");
	
	// Create map to save signature coefficients and vector to save parts of the polynom description string.
	map<string, vector<mpz_class>> coff;
	vector<string> polyDescription;
	
	// Parse signature file line by line.
	string line;
	ifstream inputFile(fileName.c_str());
	inputFile >> line;
	while (line != "Signature:") inputFile >> line;
	getline(inputFile, line);	// Take next line, at the moment we are in the key word "Signature:" line.
	//while (line != "Polynom Description:") {
	// Help variables.
	size_t posEqual;
	string signalName;
	string coefficients;
	mpz_class currNum;
	size_t open;
	size_t close;
	vector<mpz_class> signalCoeff;
	// Filling the map with coefficients.
	while (getline(inputFile, line)) {
		if (line == "Polynom Description:") {  // Read in polynom description to build the polynom from it later on.
			getline(inputFile, line);  // Take next line with polynom description.
			while(line.find("+") != string::npos) {  // As long as where are plus signs, which are delimeters of polynom.
				size_t posPlus = line.find("+");
				string part = line.substr(0, posPlus);
				while (part.find(" ") != string::npos) {	// Delete spaces from part string.
					part.erase(part.find(" "), 1);
				}
				polyDescription.push_back(part);
				line.erase(0, posPlus+1);
				//cout << "Line:" << line << " , part:" << part << ";" << endl; 
			}
			while (line.find(" ") != string::npos) {	// Last summand.
				line.erase(line.find(" "), 1);
			}
			polyDescription.push_back(line);
			
			continue;
		}
		posEqual = line.find("="); 
		if (posEqual != string::npos) {
			// Get signal name.
			signalName = line.substr(0, posEqual);
			while (signalName.find(" ") != string::npos) {	// Delete spaces from signalName string.
				signalName.erase(signalName.find(" "), 1);
			} 
			//cout << "name:." << signalName << "." <<  endl;
		} else {
			//cout << "No equal sign found in this line. Ignore this line." << endl;
			continue;
		}
		// Get coeffiecients.
		coefficients = line.substr(posEqual+1);
		open = coefficients.find("("); 
		close = coefficients.find(")");
		coefficients = coefficients.substr(open+1, close - open - 1);
		while (coefficients.find(" ") != string::npos) {	// Delete spaces from coefficients string.
			coefficients.erase(coefficients.find(" "), 1);
		} 
		while (coefficients.find(",") != string::npos) {  // Find number. Erase comma. Find next number, and so on...
			//currNum = std::stoi(coefficients.substr(0, coefficients.find(",")));  // Need mpz_class conversion.
			try {  // Check if coefficient is correct number. Otherwise terminate program. 
    				bool negative = false;	
    				if (coefficients.substr(0, coefficients.find(",")).find("-") != string::npos) {
    					negative = true;
    					coefficients.erase(0, 1);
    				}
    				mpz_class base = 2;
   					int expo = stoi(coefficients.substr(0, coefficients.find(",")));
    				//currNum = pow(base, expo);
    				mpz_pow_ui(currNum.get_mpz_t(), base.get_mpz_t(), expo);
    				if (negative) currNum = currNum * -1; 
    				//currNum = mpz_class(coefficients.substr(0, coefficients.find(",")));
			}
			catch( const std::invalid_argument& e ) {
    				cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
    				cout << "Wrong format in signature file. Stop program." << endl;
    				exit(EXIT_FAILURE);	
			}
			//currNum = mpz_class(coefficients.substr(0, coefficients.find(",")));
			coefficients.erase(0, coefficients.find(",") + 1);
			signalCoeff.push_back(currNum);
		}
		try {  // Check if last coefficient is correct number. Otherwise terminate program. 
   			bool negative = false;	
    			if (coefficients.find("-") != string::npos) {
    				negative = true;
    				coefficients.erase(0, 1);
    			}
    			mpz_class base = 2;
   				int expo(stoi(coefficients));
    			mpz_pow_ui(currNum.get_mpz_t(), base.get_mpz_t(), expo);
    			if (negative) currNum = currNum * -1;
    			//currNum = mpz_class(coefficients);
		}
		catch( const std::invalid_argument& e ) {
    			cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
    			cout << "Wrong format in signature file. Stop program." << endl;
    			exit(EXIT_FAILURE);	
		}
		//currNum = mpz_class(coefficients); // Add last number.
		signalCoeff.push_back(currNum);
		// Add signalCoeff in coff map with string signalName and coefficients in signalCoeff. Clear signalCoeff afterwards.
		coff.insert(pair<string, vector<mpz_class>>(signalName, signalCoeff));
		signalCoeff.clear();
	}
	//for (std::map<std::string, std::vector<mpz_class>>::iterator it = coff.begin(); it != coff.end(); ++it) {
	//	cout << "Coff: " << it->first << endl;
	//	for (auto& elem: it->second) cout << elem << endl;
	//}
	cout << "read signatureFile ended" << endl;

	std::pair<std::map<std::string, std::vector<mpz_class>>, std::vector<string>> retValue;
	retValue.first = coff;
	retValue.second = polyDescription;
	return retValue;
}

// _______________________________________________________________________________________________________________________________________
Polynom Verifizierer::createSignaturePolynom(std::pair<std::map<std::string, std::vector<mpz_class>>, std::vector<string>> readValues) {
	cout << "create signature polynom started" << endl;
	
	Polynom p1(this->circuit.maxEdgeIndex + 1);  // Create polynom where signature polynom will be add and returned.
	
	std::map<std::string, std::vector<mpz_class>> coff = readValues.first;
	std::vector<string> description = readValues.second;
	
	// Make a list of all outputNode and inputNode names and at parallel a list of their edge indices.
	std::vector<string> primaryNames;
	std::vector<varIndex> primaryIndices;
	for (int i = 0; i < this->circuit.getOutputNodesCount(); i++) {  // Add all output node names and corresponding indices.
		 if (this->circuit.outputNode(i).type == vp::Node::OUTPUT_PORT) {
		 	primaryNames.push_back(this->circuit.outputNode(i).name);
		 	primaryIndices.push_back(this->circuit.outputNode(i).inputs.at(0)->eIndex);
		 } else {
		 	//cout << "Something went wrong. This node is not an output node." << endl;
		 } 
	}
	for (int i = 0; i < this->circuit.getInputNodesCount(); i++) {  // Add all input node names and corresponding indices.
		 if (this->circuit.inputNode(i).type == vp::Node::INPUT_PORT) {
		 	if (this->circuit.inputNode(i).name == "zeroNode") continue;
		 	if (this->circuit.inputNode(i).name == "oneNode") continue;
		 	//cout << "Input name: " << this->circuit.inputNode(i).name << endl;
		 	primaryNames.push_back(this->circuit.inputNode(i).name);
		 	//cout << "Input index: " << this->circuit.edgeIndex.at(*this->circuit.inputNode(i).outputs.at(0)) << endl;
		 	primaryIndices.push_back(this->circuit.inputNode(i).outputs.at(0)->eIndex);
		 } else {
		 	//cout << "Something went wrong. This node is not an input node." << endl;
		 }
	}
	
	if (primaryNames.size() != primaryIndices.size()) {  // Check whether both vectors have same size.
		cout << " Vector sizes do not match. Something went wrong. " << endl;
		return p1;
	}
	// Match all coefficients which were read in signature file and create the signature polynomial.
	map<string, Polynom> polynomials;
	Polynom currPolynom(this->circuit.maxEdgeIndex + 1);
	Polynom emptyPolynom(this->circuit.maxEdgeIndex + 1);
	string name = "";
	string lastName;
	for (int i = 0; i < primaryNames.size(); i++) {
		lastName = name;
		name = primaryNames.at(i).substr(0, primaryNames.at(i).find("[")); 
		if (coff.count(name) == 0) {  // Check primary signal name. If it is not given in signature file, continue with next.
			cout << "Primary signal with name: " << name << " not specified in signature." << endl;
			continue;
		}
		if (i > 0 && lastName != name) {
			polynomials.insert(pair<string, Polynom>(lastName, currPolynom));
			currPolynom = emptyPolynom;
		}
		string bitNumber = primaryNames.at(i).substr(primaryNames.at(i).find("[") + 1, primaryNames.at(i).find("]") - primaryNames.at(i).find("[") - 1);
		bool hasBitNumber = true;
		try {  // Check if signal has a bitNumber. 
    			std::stoi(bitNumber);
		}
		catch( const std::invalid_argument& e ) {
    			//cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
    			//cout << "Primary signal of bit length one." << endl;
    			hasBitNumber = false;
		}
		mpz_class factor = 0;
		if (hasBitNumber) {
			if (std::stoi(bitNumber) >= coff.at(name).size()) continue;  // This bit of signal was not specified in signature.
			factor = coff.at(name).at(std::stoi(bitNumber));
		} else {
			if (coff.at(name).size() == 0) continue;  // The one bit signal was not specified in signature. 
			factor = coff.at(name).at(0);
		}
		// Add monoms to currPolynom with factor given by signature file.
		if (factor == 0) continue;
		Monom2 temp(primaryIndices.at(i));
		temp.setFactor(factor);
		currPolynom.addMonom(temp);  // Add monoms with right indices and factor to temporary polynom.
	} 
	polynomials.insert(pair<string, Polynom>(name, currPolynom)); // Add last name and polynom part.
	//for (map<string, Polynom>::iterator it = polynomials.begin(); it!=polynomials.end(); ++it) {
	//	cout << it->first << " => " << it->second << endl;
	//}
	// Polynom just with factor -1. For negation if a minus sign is detected in signature description.
	Monom2 minusOne;
	minusOne.setFactor(-1);
	Polynom negation(1);
	negation.addMonom(minusOne);
	Polynom one(1); 
	minusOne.setFactor(1);
	one.addMonom(minusOne);
	// Check the polynom description string and add single or multiplied signals to the signature polynom.
	for (vector<string>::iterator it = description.begin(); it != description.end(); ++it) {
		//cout << "String part: !" << *it << "!" << endl;
		if ((*it).find("*") != string::npos) { 
			 // Found multiplication in polynom description.
			vector<string> parts; 
			if ((*it).find("-") != string::npos) {  // Negative version.
				string noMinus = (*it).erase((*it).find("-"), 1);
				
				while (noMinus.find("*") != string::npos) {  // Save every factor of multiplication.
					if (polynomials.count(noMinus.substr(0, noMinus.find("*"))) > 0) 
					parts.push_back(noMinus.substr(0, noMinus.find("*")));
					noMinus.erase(0, noMinus.find("*")+1);
				}
				if (polynomials.count(noMinus) > 0) parts.push_back(noMinus);  // Add last factor.
				// Create multiplied polynom and add it to p1.
				Polynom tempPol;
				tempPol = Polynom::multiplyPoly(negation, polynomials.at(parts.back()));
				parts.pop_back();
				for (vector<string>::iterator partIte = parts.begin(); partIte != parts.end(); ++partIte) {
					tempPol = Polynom::multiplyPoly(tempPol, polynomials.at(*partIte));
				}
				p1.addPolynom(tempPol);
			} else {  // Positive version.
				string noMinus = (*it);
				
				while (noMinus.find("*") != string::npos) { 
					cout<<"inside while"<<endl; // Save every factor of multiplication.
					if (polynomials.count(noMinus.substr(0, noMinus.find("*"))) > 0) parts.push_back(noMinus.substr(0, noMinus.find("*")));
					noMinus.erase(0, noMinus.find("*")+1);
				}
				if (polynomials.count(noMinus) > 0) parts.push_back(noMinus);  // Add last factor.
				// Create multiplied polynom and add it to p1.
				Polynom tempPol;
				tempPol = Polynom::multiplyPoly(one, polynomials.at(parts.back()));
				parts.pop_back();
				for (vector<string>::iterator partIte = parts.begin(); partIte != parts.end(); ++partIte) {
					tempPol = Polynom::multiplyPoly(tempPol, polynomials.at(*partIte));
				}
				p1.addPolynom(tempPol);
			}
		} else {  // No multiplication in this part of polynom description. Just add partial polynom to p1.
			if ((*it).find("-") != string::npos) {  // Negative version.
				string noMinus = (*it).erase((*it).find("-"), 1);
				if (polynomials.count(noMinus) > 0) p1.addPolynom(Polynom::multiplyPoly(negation, polynomials.at(noMinus)));
			} else {  // Positive version.
				if (polynomials.count(*it) > 0) p1.addPolynom(polynomials.at(*it));
			}
		}
	}
	
	if (coff.count("constant_monom") > 0) {  // Signature file gives a constant monom factor.
		if (coff.at("constant_monom").at(0) != 0) { 
			Monom2 empty;
			empty.setFactor(coff.at("constant_monom").at(0));
			p1.addMonom(empty);
		}
	}
	
	cout << "end createPolynom" << endl;
	return p1;
}

// _______________________________________________________________________________________________________________________________________
std::vector<vp::Node> Verifizierer::createReplaceOrder() {
	//TODO: Nur nodeIndex übergeben.
	// Save replace order of nodes in this vector.
	std::vector<vp::Node> order;
	// Consider all nodes except for output and input nodes.
	// TODO: Einfach nur sortedNodes übergeben, um nicht linearen Aufwand zu erhalten. IN und OUTPUTS beim erstellen der Replacements aussortieren.
	vp::Node* currNode;
	for (int i = 0; i < this->circuit.sortedNodes.size(); i++) {
		currNode = &this->circuit.node(this->circuit.sortedNodes.at(i));
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::OUTPUT_PORT || currNode->type == vp::Node::DELETED) {
			//std::cout << "Input or Output detected. Skip. " << std::endl;
		} else {
			order.push_back(*currNode);
		}
	}
	//std::cout << "createReplaceOrder successfull." << std::endl;
	return order;
}

// _______________________________________________________________________________________________________________________________________
std::vector<replacementOld> Verifizierer::makeReplaceList(std::vector<vp::Node> order) {
	//cout << "makeReplaceList start." << endl;
	// Use vector with already given length.
	std::vector<replacementOld> replaceList(order.size());
	//cout << "Order has size: " << order.size() << endl; 
	for (int i = 0; i < order.size(); i++) {
		//cout << "schritt " << i << endl;
		//cout << "Node name: " << order.at(i).name << "outputs: " << order.at(i).outputs.size() << " inputs: " << order.at(i).inputs.size() << " type: " << order.at(i).type << endl;
		//if (order.at(i).outputs.size() == 0 || order.at(i).inputs.size() == 0) continue;  // Skip dangling node. 
		replaceList.push_back(convertNodeToReplacementOld(order.at(i)));
	}
	//std::cout << "makeReplaceList successfull." << std::endl;
	return replaceList;
}

// _______________________________________________________________________________________________________________________________________
replacement Verifizierer::convertNodeToReplacement(vp::Node& node) {
	//cout << "replacement of " << node.name << endl;
	replacement replace;
	bool in1Inverted = false, in2Inverted = false;
	replace.sig = node.outputs.at(0)->eIndex;
	varIndex in1 = node.inputs.at(0)->eIndex, in2;
	if (this->represent[in1].sig != 0) {
		if (this->represent[in1].state == 1) in1 = this->represent[in1].sig->eIndex;
		else in1Inverted = true;
	}
	if (node.type == vp::Node::NOT || node.type == vp::Node::BUFFER) {
		in2 = in1; // does not care.
	} else {
		in2 = node.inputs.at(1)->eIndex;
		//cout << "in2 index is " << in2 << endl;
		if (this->represent[in2].sig != 0) {
				//cout << "represent is: " << this->represent[in2].index << " with state: " << this->represent[in2].state << endl;
				if (this->represent[in2].state == 1) in2 = this->represent[in2].sig->eIndex;
				else in2Inverted = true;
		}
	}
	// Now create a gate polynomial for this node.
	replace.pol = Polynom(this->circuit.maxEdgeIndex + 1);
	varIndex start = 0;
	Monom2 startMon(start);
	replace.pol.addMonom(startMon);
	switch (node.type) {
		case vp::Node::AND: replace.pol.replaceAND(start, in1, in2); break;
		case vp::Node::XOR: replace.pol.replaceXOR(start, in1, in2); break;
		case vp::Node::OR: replace.pol.replaceOR(start, in1, in2); break;
		case vp::Node::NOT: replace.pol.replaceNOT(start, in1); break;
		case vp::Node::BUFFER: replace.pol.replaceBUFFER(start, in1); break;
	}
	if (in1Inverted) {
		replace.pol.replaceNOT(in1, this->represent[in1].sig->eIndex);
	}
	if (in2Inverted) {
		replace.pol.replaceNOT(in2, this->represent[in2].sig->eIndex);
	}
	return replace;
}

// _______________________________________________________________________________________________________________________________________
replacementOld Verifizierer::convertNodeToReplacementOld(vp::Node& node) {
	//cout << "replacement of " << node.name << endl;
	replacementOld replace;
	replace.type = node.type;
	//cout << "Nodename: " << node.name << endl;
	//cout << "replacetype: " << node.type << endl;
	replace.output = node.outputs.at(0)->eIndex;
	//cout << "replace output: " << replace.output << endl; 
	//cout << "replace output: " << this->circuit.edgeIndex.at(*node.outputs.at(0)) << endl;
	replace.inputOne = node.inputs.at(0)->eIndex;
	//cout << "replace input1: " << replace.inputOne << endl;
	//cout << "replace input 1: " << this->circuit.edgeIndex.at(*node.inputs.at(0)) << endl;
	if (node.type == vp::Node::NOT || node.type == vp::Node::BUFFER) {
		replace.inputTwo = -1; // does not care.
	} else {
	replace.inputTwo = node.inputs.at(1)->eIndex;
	//cout << "replace input2: " << replace.inputTwo << endl;
	//cout << "replace input 2: " << endl;
	}
	return replace;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::replaceSingleNodeWithRepr(vp::Node& currNode, bool backTrackedRepr) {
	cout<<"inside replaceSingleNodeWithRepr"<<endl;
	varIndex out, in1, in2;
	in2 = 0;
	varIndex oldOut, oldIn1, oldIn2;
	bool in1Inverted = false, in2Inverted = false;
	//cout << "test: " << in1Inverted << "|" << in2Inverted << endl;
	out = currNode.outputs.at(0)->eIndex;
	in1 = currNode.inputs.at(0)->eIndex;
	cout<<"in1"<<in1<<endl;
	if (backTrackedRepr) {
		if (in1 != this->represent[in1].sig->eIndex) {
			this->represent[in1].activated = true;
			//cout << "in1: " << in1 << " activated to " << this->represent[in1].sig->eIndex << endl;
		}
		/*
		if (this->represent[out].sig->eIndex != out) {
			oldOut = out;
			out = this->represent[out].sig->eIndex;
			//if (this->represent[out].state) this->poly.replaceBUFFER(oldOut, out);
			//else this->poly.replaceNOT(oldOut, out);
			//cout << "this poly now: " << this->poly << endl;
		}
		*/
	}
	
	if (this->represent[in1].sig != 0 && this->represent[in1].activated == true) {
		if (this->represent[in1].state == 0) in1Inverted = true;
		//oldIn1 = in1;
		in1 = this->represent[in1].sig->eIndex;
		//if (in1Inverted) this->poly.replaceNOT(oldIn1, in1);
		//else this->poly.replaceBUFFER(oldIn1, in1);
	}
	
	if (!(currNode.type == vp::Node::NOT || currNode.type == vp::Node::BUFFER)) {
		in2 = currNode.inputs.at(1)->eIndex;
		if (backTrackedRepr) {
			if (in2 != this->represent[in2].sig->eIndex) {
				this->represent[in2].activated = true;
				cout << "in2: " << in2 << " activated to " << this->represent[in2].sig->eIndex << endl;
			}
		}
		//cout << "in2 index is " << in2 << endl;
		if (this->represent[in2].sig != 0 && this->represent[in2].activated == true) {
			//cout << "represent is: " << this->represent[in2].index << " with state: " << this->represent[in2].state << endl;
			if (this->represent[in2].state == 0) in2Inverted = true;
			//oldIn2 = in2;
			in2 = this->represent[in2].sig->eIndex;
			//if (in2Inverted) this->poly.replaceNOT(oldIn2, in2);
			//else this->poly.replaceBUFFER(oldIn2, in2);
		}
	}
	//cout << "out: " << out << " in1: " << in1  << " in1Inverted: " << in1Inverted << " in2: " << in2 << endl;
	// Now choose appropriate replace operation.
	switch (currNode.type) {
		case vp::Node::AND:
		cout<<"replace AND"<<endl;
			if (in1Inverted) {
				if (in2Inverted) this->poly.replaceANDDoubleNegation(out, in1, in2);  // Two negations.
				else this->poly.replaceANDOneNegation(out, in1, in2);  // Only in1 negated.
			} else {
				if (in2Inverted) this->poly.replaceANDOneNegation(out, in2, in1);  // Only in2 negated.
				else this->poly.replaceAND(out, in1, in2);  // No negations. Normal replacement operation for AND gate.
			}
			break;
		case vp::Node::XOR:
		cout<<"replace xor"<<endl;
			if (in1Inverted == in2Inverted)	this->poly.replaceXOR(out, in1, in2);
			else this->poly.replaceXOROneNegation(out, in1, in2);  // Symmetric polynom, so it does not matter which of signals is inverted.
			break;
		case vp::Node::OR:
		cout<<"replace or"<<endl;
			if (in1Inverted) {
				if (in2Inverted) this->poly.replaceORDoubleNegation(out, in1, in2);  // Two negations.
				else this->poly.replaceOROneNegation(out, in1, in2);  // Only in1 negated.
			} else {
				if (in2Inverted) this->poly.replaceOROneNegation(out, in2, in1);  // Only in2 negated.
				else this->poly.replaceOR(out, in1, in2);  // No negations. Normal replacement operation for OR gate.
			}
			break;
		case vp::Node::NOT:
		cout<<"replace not"<<endl;
			if (in1Inverted) this->poly.replaceBUFFER(out, in1);
			else this->poly.replaceNOT(out, in1);
			break;
		case vp::Node::BUFFER:
			if (in1Inverted) this->poly.replaceNOT(out, in1);
			else this->poly.replaceBUFFER(out, in1); cout<<"replacing buffer"<<endl;
			break;
		default:
			cout << "replaceSingleNodeWithRepr(): currNode type not specified." << endl;
			break;
	}
}


// _______________________________________________________________________________________________________________________________________
void Verifizierer::replaceSingleNodeSimple(vp::Node& currNode) {
	varIndex out, in1, in2;
	out = currNode.outputs.at(0)->eIndex;
	in1 = currNode.inputs.at(0)->eIndex;
	if (!(currNode.type == vp::Node::NOT || currNode.type == vp::Node::BUFFER)) {
		in2 = currNode.inputs.at(1)->eIndex;
	}
	// Now choose appropriate replace operation.
	switch (currNode.type) {
		case vp::Node::AND:
			this->poly.replaceAND(out, in1, in2);  // No negations. Normal replacement operation for AND gate.
			break;
		case vp::Node::XOR:
			this->poly.replaceXOR(out, in1, in2);
			break;
		case vp::Node::OR:
			this->poly.replaceOR(out, in1, in2);  // No negations. Normal replacement operation for OR gate.
			break;
		case vp::Node::NOT:
			this->poly.replaceNOT(out, in1);
			break;
		case vp::Node::BUFFER:
			this->poly.replaceBUFFER(out, in1);
			break;
		default:
			cout << "replaceSingleNodeSimple(): currNode type not specified." << endl;
			break;
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::replaceVariablesInPoly() {
	// Replace all variables in (starting) polynomial if they are represented by another variable.
	std::set<varIndex> currVars;
	for (std::set<Monom2>::iterator it= this->poly.polySet.begin(); it != this->poly.polySet.end(); ++it) {
		for (size_t i=0; i < it->getSize(); ++i) {
			currVars.insert(it->getVars()[i]);
		}
	}
//	cout << "set has variables " << currVars.size() << endl;
	for (std::set<varIndex>::iterator it= currVars.begin(); it != currVars.end(); ++it) {
//		cout << "var is: " << *it << endl;
		if (this->represent[*it].sig->eIndex != *it) {  // Representative is not identity.
			if (this->represent[*it].state) this->poly.replaceBUFFER(*it, this->represent[*it].sig->eIndex);
			else this->poly.replaceNOT(*it, this->represent[*it].sig->eIndex);
		}
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::executeReplacements() {
	int step = 0;
	size_t maxSize = 0;
	size_t maxSizeNoDCs = 0;
	double time1, tstart, tDCOp, sumtDCOp, tRewrite, sumtRewrite, tCopy, sumtCopy, t2, sumt2, t3, sumt3, t4, sumt4;
	time1 = 0.0;
	tDCOp = 0.0;
	sumtDCOp = 0.0;
	tRewrite = 0.0;
	sumtRewrite = 0.0;
	tCopy = 0.0;
	sumtCopy = 0.0;
	t2 = 0.0;
	sumt2 = 0.0;
	t3 = 0.0;
	sumt3 = 0.0;
	t4 = 0.0;
	sumt4 = 0.0;
	vp::Node* currNode, *currNode_next;
	size_t startPolySize = this->poly.size();
	int startMargin = 2 * startPolySize; //200
	int multiplyMargin = 2;//2
	int count_1 =0;
	int flag = 0;
	
//	cout << "Enter multiplicative margin for backtracking: " << endl;
//	cin >> multiplyMargin;
//	cout << endl;
//	cout << "Chosen multiplicative margin: " << multiplyMargin << endl;
	int additiveMargin = 0;//2
//	cout << "Enter additive margin for backtracking: " << endl;
//	cin >> additiveMargin;
//	cout << endl;
//	cout << "Chosen additive margin: " << additiveMargin << endl;
	vector<Polynom> backTrackDCPoly;
	vector<vector<int>::iterator> backTrackDCIterator;
	vector<int> marginDC;
	vector<Polynom> backTrackREPRPoly;
	vector<vector<int>::iterator> backTrackREPRIterator;
	vector<int> marginREPR;
	bool backTrackedDC = false;
	bool backTrackedRepr = false;
	int stopCount = 0;
	mpz_class carryCoef;
	bool stopBacktrack = false;
	bool betweenAtomics = false;
	
	cout << "Starting backward rewriting process. " << endl;

	//if (this->poly.size() < 100) cout << "Anfangspolynom ist: " << this->poly << endl;
	tstart = clock(); // Zeitmessung beginnt.
	//replaceVariablesInPoly(); // Before starting, check if variables in start polynomial can already be replaced by representatives.
	//cout << "Anfangspolynom danach ist: " << this->poly << endl;

	//Initialize save poly values for beginning.
	//* Start backward rewriting.
	for (vector<int>::iterator it= this->circuit.sortedNodes.begin(); it != this->circuit.sortedNodes.end(); ++it) {
		//cout << "next step" << endl;
		
		currNode = &this->circuit.node(*it);
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::OUTPUT_PORT || currNode->type == vp::Node::DELETED) continue;
		tRewrite = clock();
		step++;
//		if (this->poly.refList[currNode->outputs.at(0)->eIndex].getSize() == 0 && backTrackedDC == false) {
			//cout << "Skipped." << endl;
			//continue;
//		}
		//*
		
		cout << "Step: " << step << ", Replacement Typ: " << currNode->type << " with Vars: " << currNode->outputs.at(0)->name << ", " << currNode->inputs.at(0)->name;
		if (currNode->type != vp::Node::BUFFER && currNode->type != vp::Node::NOT) cout << ", " << currNode->inputs.at(1)->name;
		cout << " | ";
		cout << " Atomic Num: " << currNode->adder << " level: " << currNode->revLevel << endl;
		cout <<"Atomic Mux"<< currNode->mux<<endl;
		//|| currNode->mux == circuit.mux_size-1
		/*
		if (currNode->mux == circuit.mux_size || currNode->mux == circuit.mux_size-1 ){
			cout<<"circuit_mux:"<<circuit.mux_size<<endl;
			flag=1;
		}
		*/
		

		//*/
		// Check if we are between atomics by comparing currNode adder num with nextNode adder num.
		if (it+1 != this->circuit.sortedNodes.end()) if (currNode->adder != this->circuit.node(*(it+1)).adder || currNode->mux != this->circuit.node(*(it+1)).mux) betweenAtomics = true;
		else betweenAtomics = false;
		if (betweenAtomics)
			{cout << "Here we are between two atomics" << endl;
			if(currNode->mux != this->circuit.node(*(it+1)).mux) 
			{   atomic_block ="mux";
				cout<<"atomic block is mux"<<endl;
			}
			else if (currNode->adder != this->circuit.node(*(it+1)).adder){
				atomic_block ="adder";
				cout<<"atomic block is adder "<<endl;
			
						}			
						}
		
		//*
		// Check if representants are available. If so, create backTrack point of type 1.(
		//	
		//cout<<"currNode_next->outputs.at(0)->name"<<currNode_next->outputs.at(0)->name<<endl;
	
		if(currNode->inputs.at(0)->name == "r_2[0]"){
			break;
		}

		if (backTrackedRepr == false && !stopBacktrack) {
			varIndex out, in1, in2;
			out = currNode->outputs.at(0)->eIndex;
			in1 = currNode->inputs.at(0)->eIndex;
			bool alreadyTracked = false;
			if (this->represent[in1].sig != 0 && this->represent[in1].tracked == false) {
				if (!(in1 == this->represent[in1].sig->eIndex)) { // Save backtrack Point.
					backTrackREPRPoly.push_back(this->poly);
					backTrackREPRIterator.push_back(it);
					marginREPR.push_back(multiplyMargin * this->poly.size() + additiveMargin);
					//backTrackType.push_back(1);
					//cout << "backtrack point on Repr set" << endl;
					//cout << "in1: " << in1 << " represent by: " << this->represent[in1].sig->eIndex << endl;
					alreadyTracked = true;
					this->represent[in1].tracked = true;
				}
			}
			if (alreadyTracked == false) {
				if (!(currNode->type == vp::Node::NOT || currNode->type == vp::Node::BUFFER)) {
					in2 = currNode->inputs.at(1)->eIndex;
					if (this->represent[in2].sig != 0 && this->represent[in2].tracked == false) {
						if (!(in2 == this->represent[in2].sig->eIndex)) { // Save backtrack Point.
						  // Save backtrack Point.
							backTrackREPRPoly.push_back(this->poly);
							backTrackREPRIterator.push_back(it);
							marginREPR.push_back(multiplyMargin * this->poly.size() + additiveMargin);
							//backTrackType.push_back(1);
							//cout << "backtrack point on Repr set" << endl;
							//cout << "in2: " << in2 << " represent by: " << this->represent[in2].sig->eIndex << endl;
							this->represent[in2].tracked = true;
						}
					}
				}
			}
		}
		/*to skip quotient bits */
		
		
		std::string s1 = currNode->outputs.at(0)->name;
		
	
		if(s1[0] == 'Q' && s1[2] == '0')
		{cout<<"skipping quotient replacement"<<endl;
		//cout<<"the value of count"<<count_1<<endl;
			continue;} 
		
		

		this->replaceSingleNodeWithRepr(*currNode, backTrackedRepr);
//		this->replaceSingleNodeSimple(*currNode);
		sumtRewrite += clock() - tRewrite;
		backTrackedRepr = false;

		cout << "polynomial after replacement is: " << this->poly << endl;
	
		// Check whether a proven dc can be applied to the polynomial. If so, apply the DC optimization to the polynomial.
//		if (betweenAtomics) dcAdded = this->applyGeneralDCOnRewriting(stopBacktrack, it, backTrackedDC, backTrackDCPoly, backTrackDCIterator, marginDC, specialMargin, tDCOp);
//		else dcAdded = false;
//		backTrackedDC = false;
//		t2 = clock();
		if (backTrackedDC) betweenAtomics = true;
		//go to applyGeneralDCOnRewriting if rewriting is between atomic blocks
		if (betweenAtomics) this->applyGeneralDCOnRewriting(stopBacktrack, it, backTrackedDC, backTrackDCPoly, backTrackDCIterator, marginDC, multiplyMargin, additiveMargin, tDCOp,flag);
		cout<<"return from applyGeneralDCOnRewriting"<<endl;
		flag = 0;
		backTrackedDC = false;

		//*/
//		if (this->poly.size() > 10000000) {  // Exponential blowup detected. Cancel verification process.
//			cout << "Abort verification. Exponential blow up in size: " << this->poly.size() << " detected. " << endl;
//			break;
//		}
		//*
		
		int margin;
		if (marginDC.size() != 0) margin = marginDC.back();
		else if (marginREPR.size() != 0) margin = marginREPR.back();
		else margin = startMargin;
		if (this->poly.size() > margin && !stopBacktrack) {
			bool dcTracked = false;
			//cout << "Monomials causing backtrack are : " << this->poly.size() << endl;
//			cout << "Threshold size reached: Backtrack to last DC usage." << endl << endl << endl;
			if (maxSize < this->poly.size()) maxSize = this->poly.size();
			if (backTrackDCIterator.size() == 0) {
				//cout << "No more dc backtrack points availabe. Try repr backtrack." << endl;
			} else { // DC backtrack.
				it = backTrackDCIterator.back();
				--it;
				this->poly = backTrackDCPoly.back();
				backTrackDCIterator.pop_back();
				backTrackDCPoly.pop_back();
				stopCount++;
				backTrackedDC = true;
				dcTracked = true;
				cout << "backtrack type dc" << endl;
			}
			if (!dcTracked) {
				if (backTrackREPRIterator.size() == 0) {
//					cout << "No more backtrack points available. Run verification without backtracking from here." << endl;
//					return;
					stopBacktrack = true;
				} else { // REPR backtrack.
					// First reactivate all DCs.
					it = backTrackREPRIterator.back();
					--it;
					this->poly = backTrackREPRPoly.back();
					backTrackREPRIterator.pop_back();
					backTrackREPRPoly.pop_back();
					stopCount++; 
					backTrackedRepr = true; 
//					cout << "backtrack type repr" << endl;
//					maxActivatedDC = 0;
//					dcPolSave.clear();
//					dcPolSizeTracking.clear();
//					dcTracking.clear();
				}
			}
		}
		
		//comment from int margin till line 882 to skip back-tracking 
		//*/
		//if ((step % 1 == 0) && this->poly.size() < 100) cout << "Zwischenpolynom ist:" << this->poly << endl;
		if (maxSize < this->poly.size()) maxSize = this->poly.size();
		cout << "Poly size: " << this->poly.size() << endl;
//		if (this->poly.size() < 100 && step > 129) cout << "Zwischenpolynom ist: " << this->poly << endl;
//		if (this->poly.size() > 50000000) { cout << "POLYNOMIAL SIZE EXCEEDED THRESHOLD: ABORT REWRITING PROCESS." << endl; break;}

		/*
		if (backTrackDCPoly.size() >= 400) {
//			cout << "copy offset operation start" << endl;
			int copyAmount = 100;
			int copyOffset = backTrackDCPoly.size() - copyAmount;
			for (size_t i=0; i < copyAmount; ++i) {
				backTrackDCPoly.at(i) = backTrackDCPoly.at(copyOffset + i);
			}
			for (size_t i=0; i < copyAmount; ++i) {
				backTrackDCIterator.at(i) = backTrackDCIterator.at(copyOffset + i);
			}
			backTrackDCPoly.resize(copyAmount);
			backTrackDCIterator.resize(copyAmount);
//			cout << "copy offset operation end" << endl;
		}
		// */
		/*
		if (backTrackREPRPoly.size() >= 50) {
//			stopBacktrack = true;
//			cout << "copy offset operation start" << endl;
			//backTrackREPRIterator.clear();
			//backTrackREPRPoly.clear();
			//*
			int copyAmount = 5;
			int copyOffset = backTrackREPRPoly.size() - copyAmount;
			for (size_t i=0; i < copyAmount; ++i) {
				backTrackREPRPoly.at(i) = backTrackREPRPoly.at(copyOffset + i);
			}
			for (size_t i=0; i < copyAmount; ++i) {
				backTrackREPRIterator.at(i) = backTrackREPRIterator.at(copyOffset + i);
			}
			backTrackREPRPoly.resize(copyAmount);
			backTrackREPRIterator.resize(copyAmount);

//			cout << "copy offset operation end" << endl;
		}
		//*/
		// */
	}
	// Backward rewriting ended.
	// If after backward rewriting still DC variables left, solve the DC problem at the end.
	pair<vector<int>, mpz_class> dcSolEnd;
	vector<mpz_class> solEnd;

	//*/
	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	sumtDCOp = sumtDCOp/CLOCKS_PER_SEC;
	sumtRewrite = sumtRewrite/CLOCKS_PER_SEC;
	sumtCopy = sumtCopy/CLOCKS_PER_SEC;
	sumt2 = sumt2/CLOCKS_PER_SEC;
	sumt3 = sumt3/CLOCKS_PER_SEC;
	sumt4 = sumt4/CLOCKS_PER_SEC;
	cout << "Backward rewriting total all together time in sec. : " << time1 << endl;
	cout << "Start polynomial size was: " << startPolySize << endl;
	cout << "Maximum polynomial size was: " << maxSize << endl;
	//cout << "Maximum polynomial size without DCs was: " << maxSizeNoDCs << endl;
	cout << "Number of backtrackings: " << stopCount << endl;
//	cout << "Number of ILP optimizations: " << numILPOptimizations << endl;
//	cout << "Number of DCs dismissed: " << numDismissedDCs << endl;
}

// _______________________________________________________________________________________________________________________________________
bool Verifizierer::applyGeneralDCOnRewriting(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, vector<int>& marginDC, int& multMargin, int& additiveMargin, double& tDCOp, int flag_in =0) {
	bool ret = false;
	std::string s1;
	vp::Node* test_node  = &this->circuit.node(*it);
	std::cout<<"inside applyGeneralDCOnRewriting"<<endl;
	//cout<<"the value backTrackedDC"<<backTrackedDC<<endl;
	
	for (set<gendc>::iterator inner = this->generalDCs.begin(); inner != this->generalDCs.end(); ++inner) {

		//if (!inner->activated) continue;
		
		vector<varIndex> eIndices;
		//cout<<"inner:"<<*inner<<endl;
		//cout<<"inner size "<<inner->size()<<endl;
		for (size_t i=0; i < inner->size(); ++i) {
				eIndices.push_back(this->circuit.edges.at(inner->signals.at(i)).eIndex);
		}
		
		bool continueDCs = false;
//		for (size_t i=0; i < eIndices.size(); ++i) {
//			if (!this->poly.containsVar(eIndices.at(i))) { continueDCs = true; break;}
//		}
//		if (continueDCs && inner->signals.at(0) != "R_0[22]") continue;
		if (*(it+1) >= this->circuit.getNodesCount()){
			cout<<"inside if"<<endl; 
			cout<<"*(it+1) >= this->circuit.getNodesCount()"<<endl; 
			break;
			}

		int nextIndex = this->circuit.node(*(it+1)).outputs.at(0)->eIndex;
		
		/*
		if(flag_in==1){
			cout<<"inside mux_if"<<endl;
			cout<<"test_node->inputs.at(0)->eIndex "<< test_node->inputs.at(0)->eIndex<<endl;
			for (size_t i=0; i < eIndices.size(); ++i){
				if (eIndices.at(i) == test_node->inputs.at(0)->eIndex){
					cout<<"this dc is there"<<*inner<<endl;
				}

			}
		}
		*/
		continueDCs = true;
		for (size_t i=0; i < eIndices.size(); ++i) {
			if (eIndices.at(i) == nextIndex) {
				cout<<"nextIndex :"<<nextIndex<<endl;
				cout<< "eIndices.at(i)"<<eIndices.at(i)<<endl; 
			    ++atomic_mux_count; 
				continueDCs = false; 
				break; }
		}
		
		if (continueDCs) continue;
		
		cout << "This dc present: ";
		for (size_t i=0; i < eIndices.size(); ++i) {
			cout << eIndices.at(i) << "|";
		}

		
		cout <<"inner :"<< *inner << endl;
//		cout << "poly is: " << this->poly << endl;
//		this->addVariableMonomialsFromDC(*inner);
//		inner->activated = false;
//		cout << "poly after inserting is: " << this->poly << endl;
		
		ret = true;
//		break;  // This is needed so only one DC can be added in one step.
		//*
		// Save backtrack informations. Comment the below if block to skip back tracking
		/*
		if (backTrackedDC == false && atomic_mux_count > 1) {
			backTrackDCPoly.push_back(this->poly);
			backTrackDCIterator.push_back(it);
			marginDC.push_back((multMargin * this->poly.size()) + additiveMargin);
			//backTrackType.push_back(0);
			cout << "backtrack point on DC set" << endl;
			break;
		}
		*/
		// Add DCs and immediately solve ILP problem.


		++map_1[*inner];
		// created for version 1.0
		//to apply optimisation of adders only once and skip mux optimisation && atomic_block!="mux" && 
		if(map_1[*inner] <=1 &&flag_in ==0 ){ 
		pair<vector<int>, mpz_class> dcSol;
		vector<mpz_class> sol;
		this->addVariableMonomialsFromDC(*inner);
		std::cout<<"solve ILP"<<endl;
		dcSol = readDCPolynomialAndSolveILP(this->poly.maxDCNum);
		for (auto& elem: dcSol.first) {
			sol.push_back(elem * dcSol.second);
		}
		for (size_t i=0; i < sol.size(); ++i) {
			if (sol.at(i) != 0) 
			cout << "v" << i << " = " << sol.at(i) << endl;
		}
		cout << "poly before dc opt!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1: " << this->poly << endl;
		
		applyDCSolutionToPolynomial(sol, this->poly.maxDCNum);
		cout << "polynomial after  dc opt replacement is: " << this->poly << endl;
		break;
		}
		else{
		break;
		}
	}
	return ret;
}

// _______________________________________________________________________________________________________________________________________
bool Verifizierer::lookForwardForDCInsertion(vector<int>::iterator& it) {
	bool ret = false;
	vp::Node* currNode;
	int nextAdder = this->circuit.node(*(it+1)).adder;
	int afterNextAdder = nextAdder;
	size_t itAdd = 1;
	while (nextAdder == afterNextAdder) {
		++itAdd;
		if (*(it+itAdd) >= this->circuit.getNodesCount()) break;
		afterNextAdder = this->circuit.node(*(it+itAdd)).adder;
	}
//	--itAdd; // Reduce iterator advancing by one since we need the last step of nextAdder.
//	currNode = &this->circuit.node(*it);
		// Check if we are between atomics by comparing currNode adder num with nextNode adder num.
//		if (it+1 != this->circuit.sortedNodes.end()) if (currNode->adder != this->circuit.node(*(it+1)).adder) betweenAtomics = true;
//		else betweenAtomics = false;
//		if (!betweenAtomics) continue;
	for (set<gendc>::iterator inner = this->generalDCs.begin(); inner != this->generalDCs.end(); ++inner) {
		if (!inner->activated) continue;
		vector<varIndex> eIndices;
		for (size_t i=0; i < inner->size(); ++i) {
			eIndices.push_back(this->circuit.edges.at(inner->signals.at(i)).eIndex);
		}
		if (*(it+itAdd) >= this->circuit.getNodesCount()) break;
		int nextIndex = this->circuit.node(*(it+itAdd)).outputs.at(0)->eIndex;
		bool continueDCs = true;
//		continueDCs = true;
		for (size_t i=0; i < eIndices.size(); ++i) {
			if (eIndices.at(i) == nextIndex) { continueDCs = false; break; }
		}
		if (continueDCs) continue;
//		cout << "This dc will be inserted next time: ";
//		for (size_t i=0; i < eIndices.size(); ++i) {
//			cout << eIndices.at(i) << "|";
//		}
//		cout << *inner << endl;
		ret = true;
//		cout << "poly is: " << this->poly << endl;
		//this->addVariableMonomialsFromDC(*inner);
//		inner->activated = false;
//		cout << "poly after inserting is: " << this->poly << endl;
		break;  // This is needed so only one DC can be added in one step.
	}
	return ret;
}

// _______________________________________________________________________________________________________________________________________
bool Verifizierer::willDCBeInsertedThisStep(vector<int>::iterator& it) {
	bool ret = false;
	for (set<gendc>::iterator inner = this->generalDCs.begin(); inner != this->generalDCs.end(); ++inner) {
		if (!inner->activated) continue;
		vector<varIndex> eIndices;
		for (size_t i=0; i < inner->size(); ++i) {
			eIndices.push_back(this->circuit.edges.at(inner->signals.at(i)).eIndex);
		}
		bool continueDCs = false;
		if (*(it+1) >= this->circuit.getNodesCount()) break;
		int nextIndex = this->circuit.node(*(it+1)).outputs.at(0)->eIndex;
		continueDCs = true;
		for (size_t i=0; i < eIndices.size(); ++i) {
			if (eIndices.at(i) == nextIndex) { continueDCs = false; break; }
		}
		if (continueDCs) continue;
		ret = true;
	}
	return ret;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::checkStepsWithDCInsertionBeforehand() {
	bool betweenAtomics;
	vp::Node* currNode;
	int step = 0;
	for (vector<int>::iterator it= this->circuit.sortedNodes.begin(); it != this->circuit.sortedNodes.end(); ++it) {
		currNode = &this->circuit.node(*it);
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::OUTPUT_PORT || currNode->type == vp::Node::DELETED) continue;
		step++;
		// Check if we are between atomics by comparing currNode adder num with nextNode adder num.
		if (it+1 != this->circuit.sortedNodes.end()) if (currNode->adder != this->circuit.node(*(it+1)).adder) betweenAtomics = true;
		else betweenAtomics = false;
		if (!betweenAtomics) continue;
	for (set<gendc>::iterator inner = this->generalDCs.begin(); inner != this->generalDCs.end(); ++inner) {
		if (!inner->activated) continue;
		vector<varIndex> eIndices;
		for (size_t i=0; i < inner->size(); ++i) {
			eIndices.push_back(this->circuit.edges.at(inner->signals.at(i)).eIndex);
		}
		bool continueDCs = false;
		if (*(it+1) >= this->circuit.getNodesCount()) break;
		int nextIndex = this->circuit.node(*(it+1)).outputs.at(0)->eIndex;
		continueDCs = true;
		for (size_t i=0; i < eIndices.size(); ++i) {
			if (eIndices.at(i) == nextIndex) { continueDCs = false; break; }
		}
		if (continueDCs) continue;
		cout << "This dc present: ";
		for (size_t i=0; i < eIndices.size(); ++i) {
			cout << eIndices.at(i) << "|";
		}
		cout << *inner << endl;
//		cout << "poly is: " << this->poly << endl;
		//this->addVariableMonomialsFromDC(*inner);
		inner->activated = false;
//		cout << "poly after inserting is: " << this->poly << endl;
		break;  // This is needed so only one DC can be added in one step.
	}
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::applyDCOnRewriting(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, vector<int>& marginDC, int& specialMargin, double& tDCOp) {
	for (set<dc>::iterator inner = this->provenDCs.begin(); inner != this->provenDCs.end(); ++inner) {
		//if (this->poly.refList[this->circuit.edges.at(inner.sig1).eIndex].getSize() == 0) continue;
		if (stopBacktrack) break;
		//if (!inner->activated) continue;
		int e1Index = this->circuit.edges.at(inner->sig1).eIndex;
		int e2Index = this->circuit.edges.at(inner->sig2).eIndex;
		int e3Index = this->circuit.edges.at(inner->sig3).eIndex;
		if (!this->poly.containsVar(e1Index)) { continue; }
		if (!this->poly.containsVar(e2Index)) { continue; }
		if (!this->poly.containsVar(e3Index)) { continue; }
		int nextIndex = this->circuit.node(*(it+1)).outputs.at(0)->eIndex;
		if (e1Index != nextIndex && e2Index != nextIndex && e3Index != nextIndex) break;
		cout << "This dc present " << this->circuit.edges.at(inner->sig1).eIndex << "|" << this->circuit.edges.at(inner->sig2).eIndex << "|" << this->circuit.edges.at(inner->sig3).eIndex << "---" << inner->sig1 << "|" << inner->sig2 << "|" << inner->sig3 << endl;
		//if (inner->poss[0] == true || inner->poss[7] == true) continue;
		// Save backtrack informations.
		if (backTrackedDC == false) {
			backTrackDCPoly.push_back(this->poly);
			backTrackDCIterator.push_back(it);
			marginDC.push_back(specialMargin * this->poly.size());
			//backTrackType.push_back(0);
			cout << "backtrack point on DC set" << endl;
			break;
		}
		double optTime, tStartOp;
		optTime = 0.0;
		tStartOp = clock();
		// Read coefs from polynomial needed for ILP problem.
		vector<mpz_class> readCoefs = getCoefficientsForDC(inner->sig1, inner->sig2, inner->sig3);
		//for (auto& num: readCoefs) cout << num << endl;
		mpz_class ggt = findGCD(readCoefs);
		//cout << "ggt is " << ggt << endl;
		vector<long> reducedCoefs(readCoefs.size());
		mpz_class tempNum;
		if (ggt == 0) { cout << "ggt is 0!!!" << endl; break; }
		for (size_t j=0; j < readCoefs.size(); j++) {
			tempNum = readCoefs.at(j) / ggt;
			if (!tempNum.fits_slong_p()) cout << j << " < tempNum too big!" << endl;
			reducedCoefs.at(j) = tempNum.get_si();
		}
		//for (auto& num: reducedCoefs) cout << num << endl;
		vector<int> sol = this->solveILPforFA(*inner, reducedCoefs);
		//cout << "s1: " << s1Index << " s2: " << s2Index << " s3 " << s3Index << endl;
		//cout << "sol size: " << sol.size() << endl;
		if (sol.size() == 0) {  // No solution for ILP problem found.
			//this->provenDCs.erase(*inner);
			break;
		}
		//for (auto& elemSol: sol) cout << "Sol is " << elemSol << endl;
		vector<mpz_class> addCoefs(sol.size());
		for (size_t j=0; j < sol.size(); j++) {
			addCoefs.at(j) = sol.at(j) * ggt;
		}
		//for (auto& coef: addCoefs) { cout << coef << endl; }
		//cout << "Poly before: " << this->poly << endl;
		addDCMonomials(*inner, addCoefs);
		optTime += clock() - tStartOp; // Zeitmessung endet.
		optTime = optTime/CLOCKS_PER_SEC;
		tDCOp += optTime;
		//cout << "Poly after: " << this->poly << endl;
		//this->provenDCs.erase(inner);
		//inner->activated = false;
		break;
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::applyDCOnRewriting4(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, vector<int>& marginDC, int& specialMargin, double& tDCOp) {
	for (set<input4dc>::iterator inner = this->proven4DCs.begin(); inner != this->proven4DCs.end(); ++inner) {
		//if (this->poly.refList[this->circuit.edges.at(inner.sig1).eIndex].getSize() == 0) continue;
		if (stopBacktrack) break;
		//if (!inner->activated) continue;
		int e1Index = this->circuit.edges.at(inner->sig1).eIndex;
		int e2Index = this->circuit.edges.at(inner->sig2).eIndex;
		int e3Index = this->circuit.edges.at(inner->sig3).eIndex;
		int e4Index = this->circuit.edges.at(inner->sig4).eIndex;
		if (!this->poly.containsVar(e1Index)) { continue; }
		if (!this->poly.containsVar(e2Index)) { continue; }
		if (!this->poly.containsVar(e3Index)) { continue; }
		if (!this->poly.containsVar(e4Index)) { continue; }
		int nextIndex = this->circuit.node(*(it+1)).outputs.at(0)->eIndex;
		if (e1Index != nextIndex && e2Index != nextIndex && e3Index != nextIndex && e4Index != nextIndex) break;
		cout << "This dc present " << this->circuit.edges.at(inner->sig1).eIndex << "|" << this->circuit.edges.at(inner->sig2).eIndex << "|" << this->circuit.edges.at(inner->sig3).eIndex << "|" << this->circuit.edges.at(inner->sig4).eIndex << " --- " << inner->sig1 << "|" << inner->sig2 << "|" << inner->sig3 << "|" << inner->sig4 << endl;
		//if (inner->poss[0] == true || inner->poss[7] == true) continue;
		// Save backtrack informations.
		if (backTrackedDC == false) {
			backTrackDCPoly.push_back(this->poly);
			backTrackDCIterator.push_back(it);
			marginDC.push_back(specialMargin * this->poly.size());
			//backTrackType.push_back(0);
			cout << "backtrack point on DC set" << endl;
			break;
		}
		double optTime, tStartOp;
		optTime = 0.0;
		tStartOp = clock();
		// Read coefs from polynomial needed for ILP problem.
		vector<mpz_class> readCoefs = getCoefficientsForDC4(inner->sig1, inner->sig2, inner->sig3, inner->sig4);
		//for (auto& num: readCoefs) cout << num << endl;
		mpz_class ggt = findGCD(readCoefs);
		//cout << "ggt is " << ggt << endl;
		vector<long> reducedCoefs(readCoefs.size());
		mpz_class tempNum;
		if (ggt == 0) { cout << "ggt is 0!!!" << endl; break; }
		for (size_t j=0; j < readCoefs.size(); j++) {
			tempNum = readCoefs.at(j) / ggt;
			if (!tempNum.fits_slong_p()) cout << j << " < tempNum too big!" << endl;
			reducedCoefs.at(j) = tempNum.get_si();
		}
		//for (auto& num: reducedCoefs) cout << num << endl;
		vector<int> sol = this->solveILPforFA4(*inner, reducedCoefs); //TODO: Change for usage with 4 input DC.
		//cout << "s1: " << s1Index << " s2: " << s2Index << " s3 " << s3Index << endl;
		//cout << "sol size: " << sol.size() << endl;
		if (sol.size() == 0) {  // No solution for ILP problem found.
			//this->provenDCs.erase(*inner);
			break;
		}
		for (auto& elemSol: sol) cout << "Sol is " << elemSol << endl;
		vector<mpz_class> addCoefs(sol.size());
		for (size_t j=0; j < sol.size(); j++) {
			addCoefs.at(j) = sol.at(j) * ggt;
		}
		for (auto& coef: addCoefs) { cout << coef << endl; }
		//cout << "Poly before: " << this->poly << endl;
		addDCMonomials4(*inner, addCoefs); //TODO: Change for usage with 4 input DC.
		optTime += clock() - tStartOp; // Zeitmessung endet.
		optTime = optTime/CLOCKS_PER_SEC;
		tDCOp += optTime;
		//cout << "Poly after: " << this->poly << endl;
		//this->provenDCs.erase(inner);
		//inner->activated = false;
		break;
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::runFirstSimulation() {
	double time1 = 0.0, tstart;
	tstart = clock(); // Zeitmessung beginnt.	
	// Fill only part of SimVectors with random values. Others will be filled with counter examples of the Solver.
	std::size_t halfFull = aigpp::SimVector::BinCount;
	//halfFull = halfFull * 0.9;  
	//cout << "Bin COunt: " << halfFull << endl; 
	lrabs::Random rnd( /*seed = */42 );  // Create random seed to use for random SimVectors. Old:42
	vp::Edge* currEdge;
	aigpp::SimVector startVec;  // Empty SimVector every edge starts with.
	for (auto& elem: circuit.edges) elem.second._sim = startVec;  // Fill SimVectors of all edges with zeros.
	for (size_t i = 0; i < circuit.getInputNodesCount(); i++) {  // Give all valid input nodes a random SimVector.
		if (circuit.inputNode(i).outputs.size() > 0) {
			currEdge = &*circuit.inputNode(i).outputs.at(0);
    		currEdge->_sim.generateRandom(rnd, halfFull);
    		this->simTable.insert(currEdge);
    	}
	}
	// Check whether the just created SimVectors meets the input constraint: R^0 < D * 2^(n-1). Otherwise change not valid entries.
   	checkSimInputConstraint();

	vp::Node* currNode;
	// Simulate all nodes from top to bottom through the circuit. Use reverse substitution order.
	for (std::vector<int>::reverse_iterator rit = circuit.sortedNodes.rbegin(); rit != circuit.sortedNodes.rend(); ++rit) {
		currNode = &this->circuit.node(*rit);
		if (currNode->type != vp::Node::DELETED && currNode->type != vp::Node::INPUT_PORT && currNode->type != vp::Node::OUTPUT_PORT) {
			if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER) {
				currNode->outputs.at(0)->_sim.updateGate(currNode->inputs.at(0)->_sim, currNode->inputs.at(1)->_sim, currNode->type);
				this->simTable.insert(&*currNode->outputs.at(0));
			} else { 
				currNode->outputs.at(0)->_sim.updateGate(currNode->inputs.at(0)->_sim, currNode->inputs.at(0)->_sim, currNode->type);
				this->simTable.insert(&*currNode->outputs.at(0));
			}
		} 
	}
	
	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "Running first simulation needed time in sec. : " << time1 << endl;
	cout << "Simulation was executed with SimVectors of Size: " << SIMVECSIZE << endl;

//	for (auto& elem: circuit.edges) {
//		if (elem.first == "_2841_" || elem.first == "_2842_")
//		cout << "Edge: " << elem.first << " has SimVector: " << elem.second._sim << endl;
//	}

	/*
	for (auto& elem: circuit.edges) {
		if (elem.first == "D[0]" || elem.first == "_14_" || elem.first == "_12_" || elem.first == "_1_" || elem.first == "R_0[1]") {
			if (this->simTable.getClass(&elem.second) != 0) {
				cout << "Edge: " << elem.first << " has Sim class: " << this->simTable.getClass(&elem.second)->name << "and simClassSize: " << this->simTable.simClassSize(this->simTable.getClass(&elem.second)) << endl;
			} else {
				cout << "Edge: " << elem.first << " has no sim class. " << endl;
			}
			if (this->simTable.getInvertedClass(&elem.second) != 0) {
				cout << "Edge: " << elem.first << " has inverted class: " << this->simTable.getInvertedClass(&elem.second)->name << "and simClassSize: " << this->simTable.simClassSize(this->simTable.getInvertedClass(&elem.second)) << endl;
			} else {
				cout << "Edge: " << elem.first << " has no inverted class. " << endl;
			}
		}
	}
	//cout << "Max simClassSize: " << simTable.maxSimClassSize() << endl;
	//*/
	// Clear simTable at the end.
	//this->simTable.clear();
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::runFirstSimulationBeforeConstProp() {
	double time1 = 0.0, tstart;
	tstart = clock(); // Zeitmessung beginnt.	
	// Fill only part of SimVectors with random values. Others will be filled with counter examples of the Solver.
	std::size_t halfFull = aigpp::SimVector::BinCount;
	//halfFull = halfFull * 0.9;  
	//cout << "Bin COunt: " << halfFull << endl; 
	lrabs::Random rnd( /*seed = */42 );  // Create random seed to use for random SimVectors.
	vp::Edge* currEdge;
	aigpp::SimVector startVec;  // Empty SimVector every edge starts with.
	for (auto& elem: circuit.edges) elem.second._sim = startVec;  // Fill SimVectors of all edges with zeros.
	for (size_t i = 0; i < circuit.getInputNodesCount(); i++) {  // Give all valid input nodes a random SimVector.
		if (circuit.inputNode(i).outputs.size() > 0) {
			currEdge = &*circuit.inputNode(i).outputs.at(0);
    		if (circuit.inputNode(i).name == "zeroNode")  currEdge->_sim = startVec;  
    		else if (circuit.inputNode(i).name == "oneNode") currEdge->_sim = startVec.invert(); 
    		else currEdge->_sim.generateRandom(rnd, halfFull);
    		this->simTable.insert(currEdge);
    	}
	}
	// Check whether the just created SimVectors meets the input constraint: R^0 < D * 2^(n-1). Otherwise change not valid entries.
    checkSimInputConstraint();
	
	vp::Node* currNode;
	// Simulate all nodes from top to bottom through the circuit. Use reverse substitution order.
	for (std::vector<int>::reverse_iterator rit = circuit.sortedNodes.rbegin(); rit != circuit.sortedNodes.rend(); ++rit) {
		currNode = &this->circuit.node(*rit);
		if (currNode->type != vp::Node::DELETED && currNode->type != vp::Node::INPUT_PORT && currNode->type != vp::Node::OUTPUT_PORT) {
			if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER) {
				currNode->outputs.at(0)->_sim.updateGate(currNode->inputs.at(0)->_sim, currNode->inputs.at(1)->_sim, currNode->type);
				this->simTable.insert(&*currNode->outputs.at(0));
			} else { 
				currNode->outputs.at(0)->_sim.updateGate(currNode->inputs.at(0)->_sim, currNode->inputs.at(0)->_sim, currNode->type);
				this->simTable.insert(&*currNode->outputs.at(0));
			}
		} 
	}
	
	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "Running first simulation needed time in sec. : " << time1 << endl;
	cout << "Simulation was executed with SimVectors of Size: " << SIMVECSIZE << endl;
	
	//for (auto& elem: circuit.edges) {
	//	if (elem.first == "zeroWire" || elem.first == "oneWire")
	//	cout << "Edge: " << elem.first << " has SimVector: " << elem.second._sim << endl;
	//}
	
}

// ____________________________________________________________________________________________________________________________
map< string, vector<int> > Verifizierer::createDividerPrimarySignalsMap() {
	map<string, vector<int> > pMap;
	vector<int> dividend, divisor, quotient, remainder;
	for (size_t i=0; i < this->circuit.getInputNodesCount(); i++) {
		if (this->circuit.inputNode(i).name.find("R_0") != string::npos) dividend.push_back(this->circuit.inputNode(i).nIndex);
		else if (this->circuit.inputNode(i).name.find("D") != string::npos) divisor.push_back(this->circuit.inputNode(i).nIndex);
	}
	for (size_t i=0; i < this->circuit.getOutputNodesCount(); i++) {
			if (this->circuit.outputNode(i).name.find("R_n1") != string::npos) remainder.push_back(this->circuit.outputNode(i).nIndex);
			else if (this->circuit.outputNode(i).name.find("Q") != string::npos) quotient.push_back(this->circuit.outputNode(i).nIndex);
	}
	size_t* dividendIndex = new size_t[dividend.size()];
	size_t* divisorIndex = new size_t[divisor.size()];
	size_t* remainderIndex = new size_t[remainder.size()];
	size_t* quotientIndex = new size_t[quotient.size()];
	string nodeName;
	string bitNumberString;
	int bitNumber = -1;
	for (auto& elem: dividend) {
		nodeName = this->circuit.node(elem).name;
		bitNumberString = nodeName.substr(nodeName.find("[") + 1, nodeName.find("]") - nodeName.find("[") - 1);
		try {  // Check if signal has a bitNumber.
			bitNumber = std::stoi(bitNumberString);
		}
		catch( const std::invalid_argument& e ) {
			cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
		}
		if (bitNumber >= 0) dividendIndex[bitNumber] = elem;
	}
	for (auto& elem: divisor) {
		nodeName = this->circuit.node(elem).name;
		bitNumberString = nodeName.substr(nodeName.find("[") + 1, nodeName.find("]") - nodeName.find("[") - 1);
		try {  // Check if signal has a bitNumber.
			bitNumber = std::stoi(bitNumberString);
		}
		catch( const std::invalid_argument& e ) {
			cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
		}
		if (bitNumber >= 0) divisorIndex[bitNumber] = elem;
	}
	for (auto& elem: remainder) {
		nodeName = this->circuit.node(elem).name;
		bitNumberString = nodeName.substr(nodeName.find("[") + 1, nodeName.find("]") - nodeName.find("[") - 1);
		try {  // Check if signal has a bitNumber.
			bitNumber = std::stoi(bitNumberString);
		}
		catch( const std::invalid_argument& e ) {
			cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
		}
		if (bitNumber >= 0) remainderIndex[bitNumber] = elem;
	}
	for (auto& elem: quotient) {
		nodeName = this->circuit.node(elem).name;
		bitNumberString = nodeName.substr(nodeName.find("[") + 1, nodeName.find("]") - nodeName.find("[") - 1);
		try {  // Check if signal has a bitNumber.
			bitNumber = std::stoi(bitNumberString);
		}
		catch( const std::invalid_argument& e ) {
			cerr <<  "std::invalid_argument exception caught at: " << e.what() << endl;
		}
		if (bitNumber >= 0) quotientIndex[bitNumber] = elem;
	}

	std::vector<int> dividendFinal(dividendIndex, dividendIndex + dividend.size());
	std::vector<int> divisorFinal(divisorIndex, divisorIndex + divisor.size());
	std::vector<int> quotientFinal(quotientIndex, quotientIndex + quotient.size());
	std::vector<int> remainderFinal(remainderIndex, remainderIndex + remainder.size());
	pMap.insert(std::pair<string, vector<int>>("R0", dividendFinal));
	pMap.insert(std::pair<string, vector<int>>("D", divisorFinal));
	pMap.insert(std::pair<string, vector<int>>("Q", quotientFinal));
	pMap.insert(std::pair<string, vector<int>>("R", remainderFinal));

	return pMap;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::checkSimInputConstraint() {
	// Check whether the just created SimVectors meets the input constraint: 0 <= R^0 < D * 2^(n-1). Otherwise change not valid entries.
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divisor = this->pMap.at("D");

	//*
	// Check for all simulation vector entries whether they fulfill the input constraint or not.
	bool currR0 = 0, currD = 0;
	int currR0Pos = -1, currDPos = -1;
	for (size_t i=0; i < aigpp::SimVector::BinCount * aigpp::SimVector::BinSize; i++) {
		// At beginning, set currR0 to R^0_max and currD to D_max and compare them.
		currR0Pos = dividend.size() - 1;
		currDPos = divisor.size() - 1;
		for (size_t j=0; j < divisor.size(); j++) {
			currR0 = this->circuit.node(dividend.at(currR0Pos)).outputs.at(0)->_sim.getBit(i);
			currD = this->circuit.node(divisor.at(currDPos)).outputs.at(0)->_sim.getBit(i);
			if (currR0 == currD) {  // This bits are the same. Check next lower bits.
				currR0Pos--;
				currDPos--;
				if (j < (divisor.size() - 1)) continue;  // Not last bit to check. Check next ones.
				else {  // Last bits were checked. All bits are equal -> Simulation entry is wrong.
					correctSimVectorEntry(i, dividend.at(dividend.size()-1), divisor.at(divisor.size()-1));
//					cout << "Simulation entry " << i << " is wrong." << endl; // This simulation entry is wrong. Correct it.
					break;
				}
			} else 	if (currR0 < currD) break;  // This simulation entry is correct. Check next one.
					else  {
//						cout << "Simulation entry " << i << " is wrong." << endl; // This simulation entry is wrong. Correct it.
						correctSimVectorEntry(i, dividend.at(currR0Pos), divisor.at(currDPos));
						break;
					}
			// All bits were checked.
		}
	}
	//*/
}

// ______________________________________________________________________________________________________________________________________
void Verifizierer::correctSimVectorEntry(size_t pos, int R0Max, int DMax) {
	// Correct simVector entry at position pos of nodes with nodeInde R0Max and DMax.
	size_t binPos = pos / aigpp::SimVector::BinSize;
	aigpp::SimVector::BinType r0Bin = this->circuit.node(R0Max).outputs.at(0)->_sim.getBins()[binPos];
	aigpp::SimVector::BinType dBin = this->circuit.node(DMax).outputs.at(0)->_sim.getBins()[binPos];
//	cout << "pos " << pos << " and " << binPos << endl;
//	cout << "vorher dBin: " << std::bitset<64>(dBin) << endl;
	r0Bin = r0Bin & ~(1ul << (pos % aigpp::SimVector::BinSize)); // Assure a 0 at pos of this simVector.
	dBin = dBin | (1ul << (pos % aigpp::SimVector::BinSize));  // Assure a 1 at pos of this simVector.
//	cout << "nacher dBin: " << std::bitset<64>(dBin) << endl;

	this->circuit.node(R0Max).outputs.at(0)->_sim.setBin(binPos, r0Bin);
	this->circuit.node(DMax).outputs.at(0)->_sim.setBin(binPos, dBin);
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::calcSimulationValues() {
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divisor = this->pMap.at("D");
	vector<int> quotient = this->pMap.at("Q");
	vector<int> remainder = this->pMap.at("R");

	int simSize = aigpp::SimVector::BinSize * aigpp::SimVector::BinCount;
	cout << "sim size is: " << simSize << endl;
	mpz_class qValue = 0, dValue = 0, r0Value = 0, rValue = 0;
	int countWrong = 0;
	for (size_t i=0; i < simSize; i++) {
		qValue = 0; dValue = 0; r0Value = 0; rValue = 0;
		for (size_t j=0; j < dividend.size(); j++) {
			r0Value += pow(2, j) * this->circuit.node(dividend.at(j)).outputs.at(0)->_sim.getBit(i);
		}
		for (size_t j=0; j < divisor.size(); j++) {
			dValue += pow(2, j) * this->circuit.node(divisor.at(j)).outputs.at(0)->_sim.getBit(i);
		}
		for (size_t j=0; j < remainder.size(); j++) {
			rValue += pow(2, j) * this->circuit.node(remainder.at(j)).inputs.at(0)->_sim.getBit(i);
		}
		for (size_t j=0; j < quotient.size(); j++) {
			qValue += pow(2, j) * this->circuit.node(quotient.at(j)).inputs.at(0)->_sim.getBit(i);
		}
		cout << "step " << i << ": " << r0Value << " | " << dValue << " | " << qValue << " | " << rValue;
		if ((qValue * dValue) + rValue == r0Value) cout << " is correct. " << endl;
		else { cout << " is wrong!!!!!!!! " << endl; countWrong++;}
	}
	cout << "Number of wrong simulation entries: " << countWrong << endl;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::extendDCsFanoutFree(set<gendc>& dcCandidates, vector<vector<int>>& extendedAtomics) {
	vector<string> extendedSignals;
	set<string> processedSignals;
	vp::Node* currNode;
	set<gendc> retCandidates;
	int adderNum = -1;
	//vp::Node* preNode;  // use for checking predecessor node properties.
	for(set<gendc>::iterator it = dcCandidates.begin(); it != dcCandidates.end(); ++it) {
//		cout << *it << endl;
		vector<string> currSignals;
		vector<string> tempSignals;
		processedSignals.clear();
		adderNum = -1;
		int pos = 0;
		while (adderNum == -1 && pos < it->size()) {
//			cout << "currSignal: " << it->signals.at(pos) << endl;
			adderNum = this->circuit.node(this->circuit.edges.at(it->signals.at(pos)).getDestination(0)).adder;
//			cout << "adderNum: " << adderNum << endl;
			++pos;
		}
		assert(adderNum > -1);
		for (size_t i=0; i < it->size(); ++i) {
			currSignals.clear();
			currSignals.push_back(it->signals.at(i));
			//processedSignals.insert(it->signals.at(i));
			while (currSignals.size() > 0) {
				tempSignals.clear();
				for (auto& elem: currSignals) {
					currNode = &this->circuit.node(this->circuit.edges.at(elem).source);
					//cout << "currNode: " << currNode->name << endl;
					bool same = true;
					if (currNode->adder != -1) same = false;  // If currNode is already part of a full adder continue.
					if (currNode->type == vp::Node::INPUT_PORT) same = false; // If currNode is an input stop traversing here and add the input signal.
					if (currNode->getOutputsCount() > 1) {
						for (size_t k=0; k < currNode->getOutputsCount(); ++k) {
							if (currNode->outputNode(k).adder == -1) {
								same = false; // preNode has fanout > 1 with at least 1 which is not part of an adder.
								break;
							}
							for (size_t l=k+1; l < currNode->getOutputsCount(); ++l) {
								if (currNode->outputNode(k).adder != currNode->outputNode(l).adder) same = false;
							}
							if (!same) break;
						}
					}
					//cout << "same is " << same << endl;
					if (same) {  // same=true means currNode can be added to fanout free cone.
						for (size_t j=0; j < currNode->getInputsCount(); ++j) {  // currNode is part of fanout free cone.
							tempSignals.push_back(currNode->inputs.at(j)->name);
							//assert(currNode->adder == -1);
							//cout << "currNode adder is " << currNode->adder << endl;
							currNode->adder = adderNum;
							extendedAtomics.at(adderNum).push_back(currNode->nIndex);
							//cout << "added to temp: " << currNode->inputs.at(j)->name << endl;
						}
					} else {  // same=false means currNode can not be added, so currSignal will be input of new DC.
						if (processedSignals.count(elem) == 0) {
							extendedSignals.push_back(elem);
							processedSignals.insert(elem);  // Remember that this signal was already added to avoid duplicates.
							//cout << "added to extended: " << elem << endl;
						}
					}
				}
				currSignals = tempSignals;
			}
		}
//		for (auto& elem: extendedSignals) {
//			cout << "|" << elem;
//		}
//		cout << endl;
		int level = this->circuit.getSmallestLevel(extendedSignals);
		retCandidates.insert(gendc(extendedSignals, level));
		//dcCandidates.insert(gendc(extendedSignals, level));
		extendedSignals.clear();
	}
	dcCandidates = retCandidates;
}

// ____________________________________________________________________________________________________________________________
set<gendc> Verifizierer::checkDCCandidatesGeneral() {
	// First convert foundAdders list into possible DC candidates.
	cout << "check DCs started" << endl;

	double time1 = 0.0, tstart;
	tstart = clock(); // Zeitmessung beginnt.

	set<gendc> dcCandidates;
	//vector<vector<int>> extendedAtomicNodes;
	/*
	string s1, s2, s3;
	int level;
	for (auto& elem: atomics) {
		assert(elem.returnInputs().size() == 3); //continue;
		s1 = this->circuit.nodes.at(elem.returnInputs().at(0)).outputs.at(0)->name;
		s2 = this->circuit.nodes.at(elem.returnInputs().at(1)).outputs.at(0)->name;
		s3 = this->circuit.nodes.at(elem.returnInputs().at(2)).outputs.at(0)->name;
		level = this->circuit.nodes.at(elem.returnOutputs().at(0)).revLevel;
		dcCandidates.insert(dc(0, s1,s2,s3, level));
	}
	//*/
	//*
	vp::Node* currNode;
	vp::Node* currNode2;
	vp::Node* currNode3;
	vp::Node* currtest;
	vp::Node* currtest1;
	string s1, s2, s3;
	int level;
	//int sumIndex;
	assert(circuit.foundAdders.size() > 0);
	cout << circuit.foundAdders.size() << endl;
	for (auto& elem: circuit.foundAdders) {
		currNode = &circuit.node(elem.xor1);
		currtest =&circuit.node(elem.xor2);
//		cout << "xor name: " << circuit.node(elem.xor1).name << " adder number: " << currNode->adder << endl;
//		cout << "xor type: " << currNode->type << endl;
		//if (circuit.node(elem.xor1).name == "X_0_") continue;
		
		if (currtest->inputs.at(1)->name == "oneWire"){
		continue;
		}
		if (currNode->type == vp::Node::DELETED /*|| currNode->type == vp::Node::NOT*/) 
		{ 
			continue;
		}
		
	
		if ( currNode->inputs.at(1)->name == "r_0[6]"){
			//cout <<"skipping"<<endl;
			continue;
		}
		

        
		std::string  check = currNode->inputNode(0).getName();
		if(check[0] =='B'){

		//s1 = currNode->inputs.at(0)->name;
		//cout << "s1: " << s1 << endl;
		s2 = currNode->inputs.at(1)->name;
		//cout << "s2: " << s2 << endl;
		//if (s1 == s2){ cout<<"s1==s2 continue "<<endl; continue;}
		currNode2 = &circuit.node(elem.xor2);
		//cout << "CurrNode2 is: " << currNode2->name << endl;
		//cout << "CurrNode2 has type: " << currNode2->type << endl;
		if (currNode2->type == vp::Node::DELETED) {
			currNode2 = &circuit.node(elem.or1);
			//cout << "currNode2 changed to or1" << endl;
		}
		//cout << "after here" << endl;
		if (currNode2->type == vp::Node::DELETED) cout << "Both FA outputs are deleted" << endl;
		if (currNode2->type == vp::Node::XOR) {
			//cout << "currNode2 is xor" << endl;
			if (currNode2->inputs.at(0)->name == currNode->outputs.at(0)->name) {
				s3 = currNode2->inputs.at(1)->name;
			} else {
				s3 = currNode2->inputs.at(0)->name;
			}
		} else if (currNode2->type == vp::Node::OR){
			currNode3 = &circuit.node(elem.and2);
			if (currNode3->inputs.at(0)->name == currNode->outputs.at(0)->name) {
				s3 = currNode3->inputs.at(1)->name;
			} else {
				s3 = currNode3->inputs.at(0)->name;
			}
		} else {  //s3 was not defined.
			s3 = "notDefinied";
		}
		if (s3 == "notDefinied") {
			//cout<<"s3 is undefined"<<endl;
			level = this->circuit.getSmallestLevel({s2, s3});
			vector<string> twoInputsInsert = {s2, s3};
			dcCandidates.insert(gendc(twoInputsInsert, level));
			//cout << "inserted signals: " << s1 << "|" << s2<<endl;
		} 
		else {
		level = this->circuit.getSmallestLevel({s2, s3});
		//level = circuit.node(circuit.edges.at(s3).source).revLevel;
		//level = currNode2->revLevel;
		vector<string> twoInputsInsert = {s2, s3};
		dcCandidates.insert(gendc(twoInputsInsert, level));
		//cout << "inserted signals: "  << s2 << "|" << s3 << endl;
		}

		}

		if (currNode->type == vp::Node::NOT) {
			//cout << "Constant 1 input case." << endl;
			s1 = currNode->inputs.at(0)->name;
			//cout << "s1 in not type: " << s1 << endl;
			string s1_neg = currNode->outputs.at(0)->name;
			//cout << "s1_neg: " << s1_neg << endl;
			//cout << "and2: " << circuit.node(elem.and2).name << endl;
			//cout << "and2type: " << circuit.node(elem.and2).type << endl;
			if (circuit.node(elem.and2).type == vp::Node::DELETED) continue;
			s2 = circuit.node(elem.and2).inputs.at(0)->name;
			//cout << "s2 first: " << s2 << endl;
			if (s2 == s1_neg) s2 = circuit.node(elem.and2).inputs.at(1)->name;
			//cout << "s2 in not type : " << s2 << endl;
			level = this->circuit.getSmallestLevel({s1, s2});
			vector<string> twoInputsInsert = {s1, s2};
			dcCandidates.insert(gendc(twoInputsInsert, level));
			continue;
		}
		s1 = currNode->inputs.at(0)->name;
		//cout << "s1: " << s1 << endl;
		s2 = currNode->inputs.at(1)->name;
		//cout << "s2: " << s2 << endl;
		if (s1 == s2){ cout<<"s1==s2 continue "<<endl; continue;}
		currNode2 = &circuit.node(elem.xor2);
		//cout << "CurrNode2 is: " << currNode2->name << endl;
		//cout << "CurrNode2 has type: " << currNode2->type << endl;
		if (currNode2->type == vp::Node::DELETED) {
			currNode2 = &circuit.node(elem.or1);
		//	cout << "currNode2 changed to or1" << endl;
		}
		//cout << "after here" << endl;
		if (currNode2->type == vp::Node::DELETED) cout << "Both FA outputs are deleted" << endl;
		if (currNode2->type == vp::Node::XOR) {
		//	cout << "currNode2 is xor" << endl;
			if (currNode2->inputs.at(0)->name == currNode->outputs.at(0)->name) {
				s3 = currNode2->inputs.at(1)->name;
			} else {
				s3 = currNode2->inputs.at(0)->name;
			}
		} else if (currNode2->type == vp::Node::OR){
			currNode3 = &circuit.node(elem.and2);
			if (currNode3->inputs.at(0)->name == currNode->outputs.at(0)->name) {
				s3 = currNode3->inputs.at(1)->name;
			} else {
				s3 = currNode3->inputs.at(0)->name;
			}
		} else {  //s3 was not defined.
			s3 = "notDefinied";
		}
		if (s3 == "notDefinied") {
			//cout<<"s3 is undefined"<<endl;
			level = this->circuit.getSmallestLevel({s1, s2});
			vector<string> twoInputsInsert = {s1, s2};
			dcCandidates.insert(gendc(twoInputsInsert, level));
			//cout << "inserted signals: " << s1 << "|" << s2<<endl;
		} else {
		level = this->circuit.getSmallestLevel({s1, s2, s3});
		//level = circuit.node(circuit.edges.at(s3).source).revLevel;
		//level = currNode2->revLevel;
		dcCandidates.insert(gendc(s1,s2,s3, level));
		//cout << "inserted signals: " << s1 << "|" << s2 << "|" << s3 << endl;
		}
	}
	
		
	




	//insert Mux dc candidates
	for (auto& elem: circuit.foundMuxs) {
		currNode = &circuit.node(elem.and1);
		if (currNode->type == vp::Node::DELETED) continue;
		if (currNode->type == vp::Node::AND) {
			s1 = currNode->inputs.at(0)->name;
			s2 = currNode->inputs.at(1)->name;
			if (s1 == circuit.node(elem.neg).outputs.at(0)->name) {
				s1 = circuit.node(elem.neg).inputs.at(0)->name;
			} else if (s2 == circuit.node(elem.neg).outputs.at(0)->name) {
				s2 = circuit.node(elem.neg).inputs.at(0)->name;
			}
			if (circuit.node(elem.and2).inputs.at(0)->name == s1 || circuit.node(elem.and2).inputs.at(0)->name == s2) {
				s3 = circuit.node(elem.and2).inputs.at(1)->name;
			} else if (circuit.node(elem.and2).inputs.at(1)->name == s1 || circuit.node(elem.and2).inputs.at(1)->name == s2) {
				s3 = circuit.node(elem.and2).inputs.at(0)->name;
			}
		} else {
			cout << "test" << endl;
			continue;
		}
		level = this->circuit.getSmallestLevel({s1, s2, s3});
		dcCandidates.insert(gendc(s1,s2,s3, level));
		//cout << "inserted signals for mux : " << s1 << "|" << s2 << "|" << s3 << endl;
	}
	vector<string> twoInputsInsert1 = {"r_3[3]","Q[0]"};
	dcCandidates.insert(gendc(twoInputsInsert1, 65));



//	cout << "first DCs added. Next step is extending the DCs." << endl;

	/*
	for (size_t i=0; i < circuit.foundAdders.size(); ++i) {
		cout << "FoundAdders nodes are: |";
		cout << circuit.foundAdders.at(i).and1 << "|" << circuit.foundAdders.at(i).and2 << "|" << circuit.foundAdders.at(i).xor1 << "|" << circuit.foundAdders.at(i).xor2 << "|" << circuit.foundAdders.at(i).or1 << "|" << endl;
		cout << "extendedAto nodes are: |";
		for (auto& elem: extendedAtomicNodes.at(i)) {
			cout << elem << "|";
		}
		cout << endl;
	}
	//*/

	// Extending fulladder DC candidates to fanout-free cones starting from just created fulladder DCs.
	//extendDCsFanoutFree(dcCandidates, extendedAtomicNodes);

	// Next rule out most candidates by checking simulation vectors.
	
	cout << "SIMULATION >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;

	//this->runFirstSimulation();// runs simulation with random vectors taking IC into account?
	this->runFirstSimulationBeforeConstProp();
	
	
	
	checkDCsInSimulationGeneral(dcCandidates);//provides inputcombinations that can be Don't cares 
	cout<<"dc candidates before clean up"<<endl;
	for (auto& elem: dcCandidates) {
		cout << elem << endl << endl;
	} 

	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "Simulation for DCs needed time: " << time1 << endl;

	//dcCandidates.clear();
	//return dcCandidates;

	//cout << "imageComputation begins. >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	time1 = 0.0;
	tstart = clock(); // Zeitmessung beginnt.

	//imageComputationForDCsGeneral(dcCandidates); // For remaining dcCandidates use imageComputation.
	
	

	
	
	cleanDCcandidates(dcCandidates);  // Remove all candidates which have no DCs left.

	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	//cout << "imageComp needed time in sec: " << time1 << endl;

	cout << "Amount of DC cells found after clean up : " << dcCandidates.size() << endl;

	for (auto& elem: dcCandidates) {
		cout << elem << endl << endl;
		}

	circuit.const0Prop();
	cout<<"finished const0 propagation"<<endl;
	circuit.const1Prop();
	cout<<"finished const1 propagation"<<endl;
	
	circuit.removeDeadEdges();	
	cout<<"removed dead edges after const prop"<<endl;

	return dcCandidates;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::checkDCsInSimulationGeneral(set<gendc>& candidates) {
	// Traverse all simulation values and notice all accuring combinations of the dc candidates (which are obviously not really dc in this cases)
	cout << "check DCs in Simulation started" << endl;
	vector<vp::Edge*> signalPointers;
	vector<bool> bitsInSim;
	int position;
	set<gendc>::iterator saveIt;
	bool validItSave = false;  // Check whether a valid iterator save already exists.
	for (size_t i=0; i < aigpp::SimVector::BinCount * aigpp::SimVector::BinSize; i++) {
		//cout << "Simulation step: " << i << "/" << aigpp::SimVector::BinCount * aigpp::SimVector::BinSize << endl;
		validItSave = false;
		for (set<gendc>::iterator it = candidates.begin(); it != candidates.end();) {
			signalPointers.clear();
			bitsInSim.clear();
			position = 0;
			for (size_t j=0; j < it->signals.size(); j++) {
 				signalPointers.push_back(&circuit.edges.at(it->signals.at(j)));
			}

			for (size_t j=0; j < signalPointers.size(); j++) {
				bitsInSim.push_back(signalPointers.at(j)->_sim.getBit(i));
			}
			for (size_t j=0; j < bitsInSim.size(); j++) {
				position += pow(2,j) * bitsInSim.at(j);
			}
			if (it->poss[position] == false) {
				it->poss[position] = true;
				it->count++;
			}
			if (it->count == pow(2, it->signals.size())) {
				candidates.erase(it);
				if (validItSave) {
					it = saveIt;
					++it;
				} else {
					it = candidates.begin();
				}
			} else {
				saveIt = it;
				validItSave = true;
				++it;
			}
		}
	}
	cout << endl;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::checkDCsInSimulation(set<dc>& candidates) {
	// Traverse all simulation values and notice all accuring combinations of the dc candidates (which are obviously not really dc in this cases)
	vp::Edge* s1;
	vp::Edge* s2;
	vp::Edge* s3;
	bool b1, b2, b3;
	int position;
	set<dc>::iterator saveIt;
	for (size_t i=0; i < aigpp::SimVector::BinCount * aigpp::SimVector::BinSize; i++) {
		//cout << "Simulation step: " << i << "/" << aigpp::SimVector::BinCount * aigpp::SimVector::BinSize << endl;
		for (set<dc>::iterator it = candidates.begin(); it != candidates.end(); ++it) {
			s1 = &circuit.edges.at(it->sig1);
			s2 = &circuit.edges.at(it->sig2);
			s3 = &circuit.edges.at(it->sig3);
			b1 = s1->_sim.getBit(i);
			b2 = s2->_sim.getBit(i);
			b3 = s3->_sim.getBit(i);
			position = (b1 * 1) + (b2 * 2) + (b3 * 4);
			if (it->poss[position] == false) {
				it->poss[position] = true;
				it->count++;
			}
			if (it->count == 8) {
				candidates.erase(it);
				it = saveIt;
			}
			saveIt = it;
		}
	}
	cout << endl;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::cleanDCcandidates(set<gendc>& dcCandidates) {
	cout << "cleanDCcandidates started." << endl;
	int maxCount;
	set<gendc>::iterator saveIt;
	bool validItSave = false;  // Check whether a valid iterator save already exists.
	for (set<gendc>::iterator it = dcCandidates.begin(); it != dcCandidates.end();) {
		
		maxCount = pow(2, it->size());
		if (it->count == maxCount) {
			dcCandidates.erase(it);
			if (validItSave) {
				it = saveIt;
				it++;
			} else {
				it = dcCandidates.begin();
			}
		} else {
			saveIt = it;
			validItSave = true;
			it++;
		}
	}
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::changeDCsByRepresent() {
	/*
	int s1Index, s2Index, s3Index;
	for (auto& elem: this->provenDCs) {
		s1Index = circuit.edges.at(elem.sig1).eIndex; 
		s2Index = circuit.edges.at(elem.sig2).eIndex; 
		s3Index = circuit.edges.at(elem.sig3).eIndex;
		if (this->represent[s1Index].sig->name != elem.sig1) {
			if (this->represent[s1Index].state == true) {
				elem.sig1 = this->represent[s1Index].sig->name;
			} // TODO: Insert else case when representative is antivalent.
		}
		if (this->represent[s2Index].sig->name != elem.sig2) {
			if (this->represent[s2Index].state == true) {
				elem.sig2 = this->represent[s2Index].sig->name;
			} // TODO: Insert else case when representative is antivalent.
		}
		if (this->represent[s3Index].sig->name != elem.sig3) {
			if (this->represent[s3Index].state == true) {
				elem.sig3 = this->represent[s3Index].sig->name;
			} // TODO: Insert else case when representative is antivalent.
		}
		
	}//*/	
	/* Reverse: Change represents inside of FAs with DC signals back to itself.
	int adderNum;
	int nodeIndex;
	for (auto& elem: this->provenDCs) {
		nodeIndex = circuit.edges.at(elem.sig1).getDestination(0);
		cout << "node name is: " << circuit.node(nodeIndex).name << " with adderNum: " << circuit.node(nodeIndex).adder << endl; 
		adderNum = circuit.node(nodeIndex).adder;
		if (adderNum != -1) this->dcFAs.insert(adderNum);
	}
	*/
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::proveSpecific4DCSubset(string sig1, string sig2, string sig3, string sig4, bool check, int num) {
	input4dc currDC(0, sig1, sig2, sig3 , sig4, 1);
	bool n1 = 0, n2 = 0, n3 = 0, n4 = 0, n5 = 0, n6 = 0, n7 = 0, n8 = 0, n9 = 0;
	cout << "num is: " << num << endl;
	n1 = (num % 2) == 1;
//	cout << "n1: " << n1 << endl;
	num = num / 2;
	n2 = (num % 2) == 1;
//	cout << "n2: " << n2 << endl;
	num = num / 2;
	n3 = (num % 2) == 1;
//	cout << "n3: " << n3 << endl;
	num = num / 2;
	n4 = (num % 2) == 1;
//	cout << "n4: " << n4 << endl;
	num = num / 2;
	n5 = (num % 2) == 1;
//	cout << "n5: " << n5 << endl;
	num = num / 2;
	n6 = (num % 2) == 1;
//	cout << "n6: " << n6 << endl;
	num = num / 2;
	n7 = (num % 2) == 1;
//	cout << "n7: " << n7 << endl;
	num = num / 2;
	n8 = (num % 2) == 1;
//	cout << "n8: " << n8 << endl;
	num = num / 2;
	n9 = (num % 2) == 1;
//	cout << "n9: " << n9 << endl;

	currDC.poss[0] = true;
	currDC.poss[1] = !n1;  //n1
	currDC.poss[2] = !n2;  //n2
	currDC.poss[3] = !n3;  //n3
	currDC.poss[4] = true;
	currDC.poss[5] = !n4;  //n4
	currDC.poss[6] = true;
	currDC.poss[7] = true;
	currDC.poss[8] = !n5;  //n5
	currDC.poss[9] = !n6;  //n6
	currDC.poss[10] = !n7; //n7
	currDC.poss[11] = true;
	currDC.poss[12] = !n8; //n8
	currDC.poss[13] = true;
	currDC.poss[14] = !n9; //n9
	currDC.poss[15] = true;

	for (int i = 0; i < 16; i++) {
		cout << currDC.poss[i] << "|";
	}
	cout << endl;
	this->proven4DCs.insert(currDC);
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::proveSpecific4DC(string sig1, string sig2, string sig3, string sig4, bool check) {
	input4dc currDC(0, sig1, sig2, sig3 , sig4, 1);
	if (check) {
		currDC.poss[0] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 0, 0, 0, 1);
		currDC.poss[1] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 0, 0, 0, 1);
		currDC.poss[2] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 1, 0, 0, 1);
		currDC.poss[3] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 1, 0, 0, 1);
		currDC.poss[4] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 0, 1, 0, 1);
		currDC.poss[5] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 0, 1, 0, 1);
		currDC.poss[6] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 1, 1, 0, 1);
		currDC.poss[7] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 1, 1, 0, 1);
		currDC.poss[8] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 0, 0, 1, 1);
		currDC.poss[9] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 0, 0, 1, 1);
		currDC.poss[10] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 1, 0, 1, 1);
		currDC.poss[11] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 1, 0, 1, 1);
		currDC.poss[12] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 0, 1, 1, 1);
		currDC.poss[13] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 0, 1, 1, 1);
		currDC.poss[14] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 0, 1, 1, 1, 1);
		currDC.poss[15] = this->proveDCwithSAT4(sig1, sig2, sig3, sig4, 1, 1, 1, 1, 1);
	} else {
		currDC.poss[0] = true;
		currDC.poss[1] = false;
		currDC.poss[2] = false;
		currDC.poss[3] = false;
		currDC.poss[4] = true;
		currDC.poss[5] = false;
		currDC.poss[6] = true;
		currDC.poss[7] = true;
		currDC.poss[8] = false;
		currDC.poss[9] = false;
		currDC.poss[10] = false;
		currDC.poss[11] = true;
		currDC.poss[12] = false;
		currDC.poss[13] = true;
		currDC.poss[14] = false;
		currDC.poss[15] = true;
	}
	for (int i = 0; i < 16; i++) {
		cout << currDC.poss[i] << "|";
	}
	cout << endl;
	this->proven4DCs.insert(currDC);
}


// ____________________________________________________________________________________________________________________________
bool Verifizierer::proveDCwithSAT4(string sig1, string sig2, string sig3, string sig4, bool b1, bool b2, bool b3, bool b4, int window) {
	// Prove with SAT whether the Don't Care candidate is a real one or not.

	// Use eIndex instead of edge names.
	int s1Index = circuit.edges.at(sig1).eIndex;
	int s2Index = circuit.edges.at(sig2).eIndex;
	int s3Index = circuit.edges.at(sig3).eIndex;
	int s4Index = circuit.edges.at(sig4).eIndex;

	// Create solver and map containing SAT variables corresponding to circuits edge indices.
	Minisat::Solver solver;
	std::map<int, Minisat::Var> varMap;

	// Insert clauses for input constraint into the solver.
	int icVar = addInputConstraintToSolver(solver, varMap);

	// Create circuit for checking Don't Care candidate with SAT.
	int outputVar = solver.newVar();
	solver.addClause(Minisat::mkLit(outputVar));  // Output of whole circuit shall be 1 for finding satisfying assignment.

	// OutputVar = Circuit description & Input Constraint. Both have to hold for a valid assignment.
	int circuitVar = solver.newVar();  // Variable for output of "tseitin circuit description".
	addTseitinClauses(solver, circuitVar, icVar, outputVar, vp::Node::AND);

	// Circuit description: (Sig1 * b1) & (Sig2 * b2) & (Sig3 * b3) -> if b_i is 0 add inverter after Sig_i.
	varMap.insert(std::pair<int, Minisat::Var>(s1Index, solver.newVar()));  // Insert SAT variable for sig1.2
	varMap.insert(std::pair<int, Minisat::Var>(s2Index, solver.newVar()));  // Insert SAT variable for sig2.
	varMap.insert(std::pair<int, Minisat::Var>(s3Index, solver.newVar()));  // Insert SAT variable for sig3.
	varMap.insert(std::pair<int, Minisat::Var>(s4Index, solver.newVar()));  // Insert SAT variable for sig4.

	// Apply needed negations of all signals.
	int s1Inv, s2Inv, s3Inv, s4Inv;
	if (!b1) {
		s1Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s1Index), 0, s1Inv, vp::Node::NOT);
	} else {
		s1Inv = varMap.at(s1Index);
	}
	if (!b2) {
		s2Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s2Index), 0, s2Inv, vp::Node::NOT);
	} else {
		s2Inv = varMap.at(s2Index);
	}
	if (!b3) {
		s3Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s3Index), 0, s3Inv, vp::Node::NOT);
	} else {
		s3Inv = varMap.at(s3Index);
	}
	if (!b4) {
		s4Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s4Index), 0, s4Inv, vp::Node::NOT);
	} else {
		s4Inv = varMap.at(s4Index);
	}
	// Add s1Inv & s2Inv & s3Inv & s4Inv = circuitVar to solver.
	int tempVar1 = solver.newVar();
	int tempVar2 = solver.newVar();
	addTseitinClauses(solver, s1Inv, s2Inv, tempVar1, vp::Node::AND);
	addTseitinClauses(solver, tempVar1, s3Inv, tempVar2, vp::Node::AND);
	addTseitinClauses(solver, tempVar2, s4Inv, circuitVar, vp::Node::AND);

	// Add previous found DCs as conflict clauses to the SAT problem.
	Minisat::Var f1, f2, f3;
	for (auto& elem: this->foundDCs) {
		//continue;
//		cout << "s1: " << elem.sig1 << " s2: " << elem.sig2 << " s3: " << elem.sig3 << " poss: " << elem.lvl << endl;
		f1 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig1).eIndex, solver.newVar()))).first).second;
		f2 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig2).eIndex, solver.newVar()))).first).second;
		f3 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig3).eIndex, solver.newVar()))).first).second;
		if (elem.lvl == 0) {
			solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 1) {
			solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 2) {
			solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 3) {
			solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 4) {
			solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 5) {
			solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 6) {
			solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 7) {
			solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
	}

	// Add fanin of all starting signals to solver.
	// Traverse the circuit backwards to primary inputs (or until end of window) and add tseitin clauses of every node.
	std::vector<vp::Node*> frontierNodes;
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig1).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig2).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig3).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig4).getSource()));
	vp::Node* currNode;
	vp::Node* nextNode1 = NULL;
	vp::Node* nextNode2 = NULL;
	int outputIndex, input1Index, input2Index;
	int in1, in2, outNum;
	std::set<int> insertedNodes;


	//cout << "sig1: " << sig1 << " sig2: " << sig2 << " sig3: " << sig3 << " sig4: " << sig4 << endl;
	//cout << b1 << "|" << b2 << "|" << b3 << "|" << b4 << endl;
	// Before building tseitin, traverse circuit from both starting nodes up to window width with breadth first search. Safe all nodes in the given window in frontierNodes, which will be all put in tseitin transformation in next step.
	//std::set<int> windowNodes;
	//circuit.nodesInWindowRange(frontierNodes.at(0), window, windowNodes);
	//circuit.nodesInWindowRange(frontierNodes.at(1), window, windowNodes);
	//circuit.nodesInWindowRange(frontierNodes.at(2), window, windowNodes);

	//std::set<int> bigWindow;
	//circuit.improvedWindow(frontierNodes, 1000, bigWindow);

//	std::set<int> allFanInNodes;
//	circuit.nodesInWindowRange(frontierNodes.at(0), 1000, allFanInNodes);
//	circuit.nodesInWindowRange(frontierNodes.at(1), 1000, allFanInNodes);
//	circuit.nodesInWindowRange(frontierNodes.at(2), 1000, allFanInNodes);

	//cout << "Nodes in Window / bigWindow: " << windowNodes.size() << " / " << bigWindow.size()  << endl;

//	for (auto& elem: bigWindow) {
//		if (windowNodes.count(elem) == 0)
//			cout << "Node " << circuit.node(elem).name << " not in window range." << endl;
//	}


	//Add tseitin clauses of all nodes in window range. Use representative if they are given for specific nodes.
	for (size_t i = 0; i < frontierNodes.size(); i++) {
		currNode = frontierNodes.at(i);
		// if (windowNodes.count(currNode->nIndex) == 0) continue;  // Considered node is not in window range. Skip it.
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::DELETED) continue;
		outputIndex = currNode->outputs.at(0)->eIndex;
		outNum = varMap.at(outputIndex);
		if (insertedNodes.count(outNum) > 0) continue;	// Node output already in map -> Node already added to solver. Skip it.
		// New node. Add inputs to map and add tseitin clauses to solver.
		input1Index = currNode->inputs.at(0)->eIndex;
		in1 = (*(varMap.insert(std::pair<int, Minisat::Var>(input1Index, solver.newVar()))).first).second;
		nextNode1 = &currNode->inputNode(0);

		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER) {
			input2Index = currNode->inputs.at(1)->eIndex;
			in2 = (*(varMap.insert(std::pair<int, Minisat::Var>(input2Index, solver.newVar()))).first).second;
			nextNode2 = &currNode->inputNode(1);
		} else {
			in2 = in1;
		}

		addTseitinClauses(solver, in1, in2, outNum, currNode->type);  // Add tseitin clauses of the current node to the solver.

		// Add inputNodes of current node to frontierNodes vector if the node with this output was not already inserted.
		if (insertedNodes.count(in1) == 0 && nextNode1->type != vp::Node::INPUT_PORT) frontierNodes.push_back(nextNode1);
		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER && nextNode2->type != vp::Node::INPUT_PORT && insertedNodes.count(in2) == 0) frontierNodes.push_back(nextNode2);
		insertedNodes.insert(outNum);  // Add to set to remember that this node is already inserted.
	}

	// Solve SAT problem.
	bool sat = solver.solve();
	if (sat) {
		//cout << "DIESER KANDIDAT WURDE NICHT BESTÄTIGT." << endl;
		/*
		for (std::map<int, Minisat::Var>::iterator it = varMap.begin(); it != varMap.end(); it++) {
			std::cout << "Edge index: " << it->first << " := " << (solver.modelValue(it->second) == l_True) << std::endl;
		}//*/
	} else {
		cout << "sig1: " << sig1 << " sig2: " << sig2 << " sig3: " << sig3 << " sig4: " << sig4 << endl;
		cout << b1 << "|" << b2 << "|" << b3 << "|" << b4 << endl;
		cout << "DIESER KANDIDAT IST BESTÄTIGTER DC!!!!" << endl;
	}
	return sat;
}


// ____________________________________________________________________________________________________________________________
bool Verifizierer::proveDCwithSAT(string sig1, string sig2, string sig3, bool b1, bool b2, bool b3, int window) {
	// Prove with SAT whether the Don't Care candidate is a real one or not.

	// Use eIndex instead of edge names.
	int s1Index = circuit.edges.at(sig1).eIndex;
	int s2Index = circuit.edges.at(sig2).eIndex;
	int s3Index = circuit.edges.at(sig3).eIndex;

	// Create solver and map containing SAT variables corresponding to circuits edge indices.
	Minisat::Solver solver;
	std::map<int, Minisat::Var> varMap;

	// Insert clauses for input constraint into the solver.
	int icVar = addInputConstraintToSolver(solver, varMap);

	// Create circuit for checking Don't Care candidate with SAT.
	int outputVar = solver.newVar();
	solver.addClause(Minisat::mkLit(outputVar));  // Output of whole circuit shall be 1 for finding satisfying assignment.

	// OutputVar = Circuit description & Input Constraint. Both have to hold for a valid assignment.
	int circuitVar = solver.newVar();  // Variable for output of "tseitin circuit description".
	addTseitinClauses(solver, circuitVar, icVar, outputVar, vp::Node::AND);

	// Circuit description: (Sig1 * b1) & (Sig2 * b2) & (Sig3 * b3) -> if b_i is 0 add inverter after Sig_i.
	varMap.insert(std::pair<int, Minisat::Var>(s1Index, solver.newVar()));  // Insert SAT variable for sig1.
	varMap.insert(std::pair<int, Minisat::Var>(s2Index, solver.newVar()));  // Insert SAT variable for sig2.
	varMap.insert(std::pair<int, Minisat::Var>(s3Index, solver.newVar()));  // Insert SAT variable for sig3.
	// Apply needed negations of sig1, sig2 and sig3.
	int s1Inv, s2Inv, s3Inv;
	if (!b1) {
		s1Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s1Index), 0, s1Inv, vp::Node::NOT);
	} else {
		s1Inv = varMap.at(s1Index);
	}
	if (!b2) {
		s2Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s2Index), 0, s2Inv, vp::Node::NOT);
	} else {
		s2Inv = varMap.at(s2Index);
	}
	if (!b3) {
		s3Inv = solver.newVar();
		addTseitinClauses(solver, varMap.at(s3Index), 0, s3Inv, vp::Node::NOT);
	} else {
		s3Inv = varMap.at(s3Index);
	}
	// Add s1Inv & s2Inv & s3Inv = circuitVar to solver.
	int tempVar = solver.newVar();
	addTseitinClauses(solver, s1Inv, s2Inv, tempVar, vp::Node::AND);
	addTseitinClauses(solver, tempVar, s3Inv, circuitVar, vp::Node::AND);

	// Add previous found DCs as conflict clauses to the SAT problem.
	Minisat::Var f1, f2, f3;
	for (auto& elem: this->foundDCs) {
		//continue;
//		cout << "s1: " << elem.sig1 << " s2: " << elem.sig2 << " s3: " << elem.sig3 << " poss: " << elem.lvl << endl;
		f1 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig1).eIndex, solver.newVar()))).first).second;
		f2 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig2).eIndex, solver.newVar()))).first).second;
		f3 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.sig3).eIndex, solver.newVar()))).first).second;
		if (elem.lvl == 0) {
			solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 1) {
			solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 2) {
			solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 3) {
			solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
		}
		if (elem.lvl == 4) {
			solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 5) {
			solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 6) {
			solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
		if (elem.lvl == 7) {
			solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
		}
	}

	// Add fanin of sig1, sig2 and sig3 to solver.
	// Traverse the circuit backwards to primary inputs (or until end of window) and add tseitin clauses of every node.
	std::vector<vp::Node*> frontierNodes;
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig1).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig2).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(sig3).getSource()));
	vp::Node* currNode;
	vp::Node* nextNode1 = NULL;
	vp::Node* nextNode2 = NULL;
	int outputIndex, input1Index, input2Index;
	int in1, in2, outNum;
	std::set<int> insertedNodes;


	//cout << "sig1: " << sig1 << " sig2: " << sig2 << " sig3: " << sig3 << endl;
	//cout << b1 << "|" << b2 << "|" << b3 << endl;
	// Before building tseitin, traverse circuit from both starting nodes up to window width with breadth first search. Safe all nodes in the given window in frontierNodes, which will be all put in tseitin transformation in next step.
	//std::set<int> windowNodes;
	//circuit.nodesInWindowRange(frontierNodes.at(0), window, windowNodes);
	//circuit.nodesInWindowRange(frontierNodes.at(1), window, windowNodes);
	//circuit.nodesInWindowRange(frontierNodes.at(2), window, windowNodes);

	//std::set<int> bigWindow;
	//circuit.improvedWindow(frontierNodes, 1000, bigWindow);

//	std::set<int> allFanInNodes;
//	circuit.nodesInWindowRange(frontierNodes.at(0), 1000, allFanInNodes);
//	circuit.nodesInWindowRange(frontierNodes.at(1), 1000, allFanInNodes);
//	circuit.nodesInWindowRange(frontierNodes.at(2), 1000, allFanInNodes);

	//cout << "Nodes in Window / bigWindow: " << windowNodes.size() << " / " << bigWindow.size()  << endl;

//	for (auto& elem: bigWindow) {
//		if (windowNodes.count(elem) == 0)
//			cout << "Node " << circuit.node(elem).name << " not in window range." << endl;
//	}


	//Add tseitin clauses of all nodes in window range. Use representative if they are given for specific nodes.
	for (size_t i = 0; i < frontierNodes.size(); i++) {
		currNode = frontierNodes.at(i);
		// if (windowNodes.count(currNode->nIndex) == 0) continue;  // Considered node is not in window range. Skip it.
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::DELETED) continue;
		outputIndex = currNode->outputs.at(0)->eIndex;
		outNum = varMap.at(outputIndex);
		if (insertedNodes.count(outNum) > 0) continue;	// Node output already in map -> Node already added to solver. Skip it.
		// New node. Add inputs to map and add tseitin clauses to solver.
		input1Index = currNode->inputs.at(0)->eIndex;
		in1 = (*(varMap.insert(std::pair<int, Minisat::Var>(input1Index, solver.newVar()))).first).second;
		nextNode1 = &currNode->inputNode(0);

		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER) {
			input2Index = currNode->inputs.at(1)->eIndex;
			in2 = (*(varMap.insert(std::pair<int, Minisat::Var>(input2Index, solver.newVar()))).first).second;
			nextNode2 = &currNode->inputNode(1);
		} else {
			in2 = in1;
		}

		addTseitinClauses(solver, in1, in2, outNum, currNode->type);  // Add tseitin clauses of the current node to the solver.

		// Add inputNodes of current node to frontierNodes vector if the node with this output was not already inserted.
		if (insertedNodes.count(in1) == 0 && nextNode1->type != vp::Node::INPUT_PORT) frontierNodes.push_back(nextNode1);
		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER && nextNode2->type != vp::Node::INPUT_PORT && insertedNodes.count(in2) == 0) frontierNodes.push_back(nextNode2);
		insertedNodes.insert(outNum);  // Add to set to remember that this node is already inserted.
	}

	// Solve SAT problem.
	bool sat = solver.solve();
	if (sat) {
		//cout << "DIESER KANDIDAT WURDE NICHT BESTÄTIGT." << endl;
		/*
		for (std::map<int, Minisat::Var>::iterator it = varMap.begin(); it != varMap.end(); it++) {
			std::cout << "Edge index: " << it->first << " := " << (solver.modelValue(it->second) == l_True) << std::endl;
		}//*/
	}
	return sat;
}

// ____________________________________________________________________________________________________________________________
mpz_class Verifizierer::gcd(mpz_class a, mpz_class b) {
	if (a == 0) return b;
	return gcd(b % a, a);
}

// ____________________________________________________________________________________________________________________________
mpz_class Verifizierer::findGCD(vector<mpz_class>& nums) {
	if (nums.size() == 0) return 0;
	mpz_class result = nums.at(0);
	for (size_t i=1; i < nums.size(); i++) {
		result = Verifizierer::gcd(nums.at(i), result);
		if (result == 1) return 1;
	}
	return result;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::addDCMonomials(dc dontCare, vector<mpz_class> coefs) {
	int s1Index = this->circuit.edges.at(dontCare.sig1).eIndex;
	int s2Index = this->circuit.edges.at(dontCare.sig2).eIndex;
	int s3Index = this->circuit.edges.at(dontCare.sig3).eIndex;
	assert(s1Index != 0 && s1Index != 2 && s1Index != 3);
	assert(s2Index != 0 && s2Index != 2 && s2Index != 3);
	assert(s3Index != 0 && s3Index != 2 && s3Index != 3);
	Polynom pTemp(this->poly.getVarSize());
	int count = 0;
	for (size_t i=0; i < 8; i++) {
		//cout << i << " is " << dontCare.poss[i] << endl;
		if (dontCare.poss[i] == true) continue; 
		switch(i) {	 
			case 0: { // Add monomials for (0,0,0) dc.
				int prev0[3] = {0, 3, 2};
				Monom2 prevMon0(prev0, 3);
				prevMon0.setFactor(coefs.at(count++));
				//cout << "Mon0 is: " << prevMon0 << endl;
				pTemp.addMonom(prevMon0);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(3, s2Index);
				pTemp.replaceNOT(2, s3Index);
				//cout << "pTemp0: " << pTemp << endl;
				}
				break;
			case 1: { // Add monomials for (1,0,0) dc.
				int prev1[3] = {0, 3, 2};
				Monom2 prevMon1(prev1, 3);
				prevMon1.setFactor(coefs.at(count++));
				//cout << "Mon1 is: " << prevMon1 << endl;
				pTemp.addMonom(prevMon1);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(3, s2Index);
				pTemp.replaceNOT(2, s3Index);
				//cout << "pTemp1: " << pTemp << endl;
				}
				break;
			case 2: { // Add monomials for (0,1,0) dc.
				//break;
				int prev2[3] = {0, 3, 2};
				Monom2 prevMon2(prev2, 3);
				prevMon2.setFactor(coefs.at(count++));
				//cout << "Mon2 is: " << prevMon2 << endl;
				pTemp.addMonom(prevMon2);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(3, s2Index);
				pTemp.replaceNOT(2, s3Index);
				//cout << "pTemp2: " << pTemp << endl;
				}
				break;
			case 3: { // Add monomials for (1,1,0) dc.
				int prev3[3] = {0, 3, 2};
				Monom2 prevMon3(prev3, 3);
				prevMon3.setFactor(coefs.at(count++));
				//cout << "Mon3 is: " << prevMon3 << endl;
				pTemp.addMonom(prevMon3);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceBUFFER(3, s2Index);
				pTemp.replaceNOT(2, s3Index);
				//cout << "pTemp3: " << pTemp << endl;
				}
				break;
			case 4: {// Add monomials for (0,0,1) dc.
				int prev4[3] = {0, 3, 2};
				Monom2 prevMon4(prev4, 3);
				prevMon4.setFactor(coefs.at(count++));
				//cout << "Mon4 is: " << prevMon4 << endl;
				pTemp.addMonom(prevMon4);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(3, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				//cout << "pTemp4: " << pTemp << endl;
				}
				break;
			case 5: {// Add monomials for (1,0,1) dc.
				int prev5[3] = {0, 3, 2};
				Monom2 prevMon5(prev5, 3);
				prevMon5.setFactor(coefs.at(count++));
				//cout << "Mon5 is: " << prevMon5 << endl;
				pTemp.addMonom(prevMon5);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(3, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				//cout << "pTemp5: " << pTemp << endl;
				}
				break;
			case 6: {// Add monomials for (0,1,1) dc.
				int prev6[3] = {0, 3, 2};
				Monom2 prevMon6(prev6, 3);
				prevMon6.setFactor(coefs.at(count++));
				//cout << "Mon6 is: " << prevMon6 << endl;
				pTemp.addMonom(prevMon6);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(3, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				//cout << "pTemp6: " << pTemp << endl;
				}
				break;
			case 7:	{// Add monomials for (1,1,1) dc.
				int prev7[3] = {s1Index, s2Index, s3Index}; 
				Monom2 secMon(prev7, 3);  
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon7 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				//cout << "pTemp7: " << pTemp << endl;
				}
				break;
			default: 
				cout << "switch in addDCMonomials(): value was not found." << endl;
				break;
		}
	}
	cout << "added poynomial after ILP optimization: " << pTemp << endl;
	this->poly.addPolynom(pTemp);
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::addDCMonomials4(input4dc dontCare, vector<mpz_class> coefs) {
	int s1Index = this->circuit.edges.at(dontCare.sig1).eIndex;
	int s2Index = this->circuit.edges.at(dontCare.sig2).eIndex;
	int s3Index = this->circuit.edges.at(dontCare.sig3).eIndex;
	int s4Index = this->circuit.edges.at(dontCare.sig4).eIndex;
	assert(s1Index != 0 && s1Index != 1 && s1Index != 2 && s1Index != 3);
	assert(s2Index != 0 && s2Index != 1 && s2Index != 2 && s2Index != 3);
	assert(s3Index != 0 && s3Index != 1 && s3Index != 2 && s3Index != 3);
	assert(s4Index != 0 && s4Index != 1 && s4Index != 2 && s4Index != 3);
	Polynom pTemp(this->poly.getVarSize());
	int count = 0;
	for (size_t i=0; i < 16; i++) {
		//cout << i << " is " << dontCare.poss[i] << endl;
		if (dontCare.poss[i] == true) continue;
		switch(i) {
			case 0: { // Add monomials for (0,0,0,0) dc.
				int prev0[4] = {0, 1, 2, 3};
				Monom2 prevMon0(prev0, 4);
				prevMon0.setFactor(coefs.at(count++));
				//cout << "Mon0 is: " << prevMon0 << endl;
				pTemp.addMonom(prevMon0);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp0: " << pTemp << endl;
				}
				break;
			case 1: { // Add monomials for (1,0,0,0) dc.
				int prev1[4] = {0, 1, 2, 3};
				Monom2 prevMon1(prev1, 4);
				prevMon1.setFactor(coefs.at(count++));
				//cout << "Mon1 is: " << prevMon1 << endl;
				pTemp.addMonom(prevMon1);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp1: " << pTemp << endl;
				}
				break;
			case 2: { // Add monomials for (0,1,0,0) dc.
				//break;
				int prev2[4] = {0, 1, 2, 3};
				Monom2 prevMon2(prev2, 4);
				prevMon2.setFactor(coefs.at(count++));
				//cout << "Mon2 is: " << prevMon2 << endl;
				pTemp.addMonom(prevMon2);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp2: " << pTemp << endl;
				}
				break;
			case 3: { // Add monomials for (1,1,0,0) dc.
				int prev3[4] = {0, 1, 2, 3};
				Monom2 prevMon3(prev3, 4);
				prevMon3.setFactor(coefs.at(count++));
				//cout << "Mon3 is: " << prevMon3 << endl;
				pTemp.addMonom(prevMon3);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp3: " << pTemp << endl;
				}
				break;
			case 4: {// Add monomials for (0,0,1,0) dc.
				int prev4[4] = {0, 1, 2, 3};
				Monom2 prevMon4(prev4, 4);
				prevMon4.setFactor(coefs.at(count++));
				//cout << "Mon4 is: " << prevMon4 << endl;
				pTemp.addMonom(prevMon4);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp4: " << pTemp << endl;
				}
				break;
			case 5: {// Add monomials for (1,0,1,0) dc.
				int prev5[4] = {0, 1, 2, 3};
				Monom2 prevMon5(prev5, 4);
				prevMon5.setFactor(coefs.at(count++));
				//cout << "Mon5 is: " << prevMon5 << endl;
				pTemp.addMonom(prevMon5);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp5: " << pTemp << endl;
				}
				break;
			case 6: {// Add monomials for (0,1,1,0) dc.
				int prev6[4] = {0, 1, 2, 3};
				Monom2 prevMon6(prev6, 4);
				prevMon6.setFactor(coefs.at(count++));
				//cout << "Mon6 is: " << prevMon6 << endl;
				pTemp.addMonom(prevMon6);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp6: " << pTemp << endl;
				}
				break;
			case 7:	{// Add monomials for (1,1,1,0) dc.
				int prev7[4] = {s1Index, s2Index, s3Index, 3};
				Monom2 secMon(prev7, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon7 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceNOT(3, s4Index);
				//cout << "pTemp7: " << pTemp << endl;
				}
				break;
			case 8:	{// Add monomials for (0,0,0,1) dc.
				int prev8[4] = {0, 1, 2, 3};
				Monom2 secMon(prev8, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon8 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp8: " << pTemp << endl;
				}
				break;
			case 9:	{// Add monomials for (1,0,0,1) dc.
				int prev9[4] = {0, 1, 2, 3};
				Monom2 secMon(prev9, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon9 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp9: " << pTemp << endl;
				}
				break;
			case 10:	{// Add monomials for (0,1,0,1) dc.
				int prev10[4] = {0, 1, 2, 3};
				Monom2 secMon(prev10, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon10 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp10: " << pTemp << endl;
				}
				break;
			case 11:	{// Add monomials for (1,1,0,1) dc.
				int prev11[4] = {0, 1, 2, 3};
				Monom2 secMon(prev11, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon11 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceNOT(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp11: " << pTemp << endl;
				}
				break;
			case 12:	{// Add monomials for (0,0,1,1) dc.
				int prev12[4] = {0, 1, 2, 3};
				Monom2 secMon(prev12, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon12 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp12: " << pTemp << endl;
				}
				break;
			case 13:	{// Add monomials for (1,0,1,1) dc.
				int prev13[4] = {0, 1, 2, 3};
				Monom2 secMon(prev13, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon13 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceNOT(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp13: " << pTemp << endl;
				}
				break;
			case 14:	{// Add monomials for (0,1,1,1) dc.
				int prev15[4] = {0, 1, 2, 3};
				Monom2 secMon(prev15, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon15 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceNOT(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp15: " << pTemp << endl;
				}
				break;
			case 15:	{// Add monomials for (1,1,1,1) dc.
				int prev15[4] = {0, 1, 2, 3};
				Monom2 secMon(prev15, 4);
				secMon.setFactor(coefs.at(count++));
				//cout << "Mon15 is: " << secMon << endl;
				pTemp.addMonom(secMon);
				pTemp.replaceBUFFER(0, s1Index);
				pTemp.replaceBUFFER(1, s2Index);
				pTemp.replaceBUFFER(2, s3Index);
				pTemp.replaceBUFFER(3, s4Index);
				//cout << "pTemp15: " << pTemp << endl;
				}
				break;
			default:
				cout << "switch in addDCMonomials(): value was not found." << endl;
				break;
		}
	}
	cout << "polynomial before: " << this->poly << endl;
	cout << "added poynomial after ILP optimization: " << pTemp << endl;
	this->poly.addPolynom(pTemp);
	cout << "polynomial afterwards: " << this->poly << endl;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::addVariableMonomialsFromDC(gendc dontCare) {
	vector<varIndex> sigIndices;
	vector<varIndex>  res_dc;
	const int dcSize = dontCare.signals.size();

	

	for (auto& elem: dontCare.signals) {
		
		sigIndices.push_back(this->circuit.edges.at(elem).eIndex);
	}
		std::set<varIndex> sigIndicesSet(sigIndices.begin(), sigIndices.end());
	const int maxVarIndex = *(std::max_element(sigIndices.begin(), sigIndices.end()));
	varIndex* tempVars = new varIndex[dcSize];
	int currMaxTemp = 1;
	for (size_t i=0; i < dcSize; ++i) {  // Get enough temp variable indices which are not part of actual signal indices for replacement operations later on.
		if (std::count(sigIndices.begin(), sigIndices.end(), currMaxTemp) == 0) {
			tempVars[i] = currMaxTemp;
			currMaxTemp++;
		} else {
			currMaxTemp++;
			--i;
		}
	}
	
	bool* dcPhases = new bool[dcSize];
	for (size_t i=0; i < dontCare.possSize; ++i) {
		if (!dontCare.poss[i]) {  // If poss[i]==false add this DC to polynomial.
			int tempNum = i;
			int step = 0;
			while (tempNum > 0) {  // Check whether dc signals have to be 1 or 0.
				dcPhases[step] = (tempNum % 2 == 1) ? 1 : 0;
				tempNum = tempNum / 2;
				step++;
			}
			while (step < dcSize) {  // Fill leading 0 bits in dcPhases array.
				dcPhases[step] = 0;
				step++;
			}
			Monom2 tempMon(tempVars, dcSize);  //Create tempMon from tempVars.
			Polynom pTemp(maxVarIndex);
			pTemp.addMonom(tempMon);
	
			for (size_t j=0; j < dcSize; ++j) {

				if (!dcPhases[j]) pTemp.replaceNOT(tempVars[j], sigIndices.at(j));
				
				else pTemp.replaceBUFFER(tempVars[j], sigIndices.at(j));
			}
			/*cout << "polynomial before: " << this->poly << endl;
			cout << "added poynomial after ILP optimization: " << pTemp << endl;
			this->poly.addPolynom(pTemp);*/
			auto it1 = pTemp.getSet()->begin();// iterator to the first monomial of the poly.
			Monom2 tempMon_1, tempMon_split_cube, tempMon_single;
			std::vector<Monom2*> res_1,res_cubes;
			std::set<Monom2*> pure_dc_set;
			Monom2* res_pure;
			int count_og=0;
			tempMon_1 = *it1;
			cout<<" dc poly size "<<pTemp.size()<<endl;
			//check if dc polynomial contains more than one term
			auto sz = pTemp.size();
			std::string s1 = tempMon_1.to_string();
			cout<<"s1"<< s1<<endl;
			if(pTemp.size()>1){
				
				if(s1=="[1*]")
				{ cout<<"this is type1 dc assignment"<<endl;
					// code for dc with 1 as its term.  eg.  1-a-b+ab
					for(auto it_og = pTemp.getSet()->begin(); it_og != pTemp.getSet()->end(); ++it_og)
					{	Monom2 tempMon_og = *it_og;
						cout<<"find pure dc for "<<tempMon_og<<endl;
						res_pure= this->poly.findExact(tempMon_og);
						if(res_pure!=NULL)
						{cout<<"found pure dc "<<*res_pure<<endl;
						pure_dc_set.insert(res_pure);
						++count_og; 
						}
						else{cout<<" no pure dc"<<endl;}// no original dc found
					}
					
					if(count_og ==pTemp.size()){
						this->addSingleDCPolynomial(pTemp);// add the original dc poly to this->poly.
					}
					auto it_split_cube  =  pTemp.getSet()->begin();
					auto it_split_cube_copy = it_split_cube;
					// increment to avoid 1 as the first monomial to search in this->poly
					int increment_end = (pTemp.size() ==4)? 2: (pTemp.size() ==8)? 3:0;
					int k=0;
					
						std::set<Monom2> dc_cubes_split;
						std::map<Monom2, std::pair<int, int>> map_modified;
						Monom2 modifiedMonom;
					while (k < increment_end ){
						++it_split_cube_copy;
						++k;
						//cout<<"value of k"<<k<<endl;
						tempMon_split_cube = *it_split_cube_copy;
						res_cubes = this->poly.findContaining(tempMon_split_cube);
						if(res_cubes.empty()){cout<<" no dc cubes present "<<endl;break;}
						for(auto &elem1: res_cubes){
						if(pure_dc_set.find(elem1) == pure_dc_set.end())
						dc_cubes_split.insert(*elem1);}
					}
					for (auto it_set = dc_cubes_split.begin();it_set != dc_cubes_split.end();++it_set){
						cout<<"elements in the split cubes :"<<*it_set<<endl;
						}// to display the split candidates.Can be commented out.

					for (auto it_set = dc_cubes_split.begin(); it_set != dc_cubes_split.end(); ++it_set) {
 				   			 modifiedMonom = it_set->removeVars(sigIndicesSet);
							  int sign = (modifiedMonom.getFactor() < 0) ? -1 : 1;
							  map_modified[*modifiedMonom.getVars()].first++;
            				  map_modified[*modifiedMonom.getVars()].second += sign;}
					for(auto &w:map_modified){
						std::cout << "map_modified key " << w.first << std::endl;
            			std::cout << "map_modified count " << w.second.first << std::endl;
            			std::cout << "map_modified sign " << w.second.second << std::endl;
						if (w.second.first  == pTemp.size()-1){
							tempMon_single = w.first;
							cout<<"find the exact match for"<<tempMon_single<<endl;
							res_pure = this->poly.findExact(tempMon_single);
							if(res_pure == NULL){continue;}
							int sign = (res_pure->getFactor()< 0)? -1: 1;
							if(sign +w.second.second ==0)
							cout<<"dc cube successfully found "<<endl;
							/* write a code to multiply the orginal 
							dc poly with the vars found in w.first and this polynomial 
							addSingleDCPolynomial()*/ 
						}
					}
		
				}
				else{
					cout<<"type 2 dc"<<endl;
					//code to search original dc poly in this->poly
					for(auto it_og = pTemp.getSet()->begin(); it_og != pTemp.getSet()->end(); ++it_og)
					{	Monom2 tempMon_og = *it_og;
						cout<<"find pure dc for "<<tempMon_og<<endl;
						res_pure= this->poly.findExact(tempMon_og);
						if(res_pure!=NULL)
						{cout<<"found pure dc "<<*res_pure<<endl;
						pure_dc_set.insert(res_pure);
						++count_og; 
						}
						if(res_pure==NULL){ cout<<" no pure dc"<<endl;}// no original dc found
						// TBD if sign check is required
						
					}
					if(count_og ==pTemp.size()){
						this->addSingleDCPolynomial(pTemp);// add the original dc poly to this->poly.
					}
					// search for dc cubes.
					auto it_split_cube  =  pTemp.getSet()->begin();
					tempMon_split_cube = *it_split_cube;
					res_cubes = this->poly.findContaining(tempMon_split_cube);
					if(res_cubes.empty()){cout<<" no dc cubes present "<<endl; continue;}
					std::set<Monom2> dc_cubes_split;
					std::map<Monom2, std::pair<int, int>> map_modified;
					Monom2 modifiedMonom;
					
					//remove duplicates found in pure dcs.
					for(auto &elem1: res_cubes){
					if(pure_dc_set.find(elem1) == pure_dc_set.end())
						dc_cubes_split.insert(*elem1);}
					if(dc_cubes_split.empty()){cout<<"no dc splits available"<<endl;}
					
					for (auto it_set = dc_cubes_split.begin();it_set != dc_cubes_split.end();++it_set){
						cout<<"elements in the split cubes :"<<*it_set<<endl;
						}
					for (auto it_set = dc_cubes_split.begin(); it_set != dc_cubes_split.end(); ++it_set) {
 				   			 modifiedMonom = it_set->removeVars(sigIndicesSet);
							  int sign = (modifiedMonom.getFactor() < 0) ? -1 : 1;
							  map_modified[*modifiedMonom.getVars()].first++;
            				  map_modified[*modifiedMonom.getVars()].second += sign;}
					
					for(auto &w:map_modified){
						std::cout << "map_modified key " << w.first << std::endl;
            			std::cout << "map_modified count " << w.second.first << std::endl;
            			std::cout << "map_modified sign " << w.second.second << std::endl;
						if (w.second.first  == pTemp.size() && w.second.second==0){
							cout<<"dc cube successfully found "<<endl;
							/* write a code to multiply the orginal 
							dc poly with the vars found in w.first and this->polynomial 
							addSingleDCPolynomial()*/ 
						}
					}


				}

			}
			else // for dcs like ab or abc

			{ cout<<"type 3 dc"<<endl;
				res_1 = this->poly.findContaining(tempMon_1);//get cubes matching the  dc_poly
				if(res_1.empty()){cout<<"no dc cubes present"<<endl; continue;} // go to next dc candidate
				std::set<Monom2> dc_cubes; 
				for(auto elem_1 :res_1 ){cout<<"dc split candidates "<<*elem_1<<endl;
			        dc_cubes.insert(*elem_1);}
				for(auto it_cube = dc_cubes.begin();it_cube!= dc_cubes.end(); ++it_cube)
				{Monom2 copy = *it_cube;
				copy.setFactor(1); 
				if(dc_cubes.size()>10)
				{Polynom pTemp1(maxVarIndex+100);}
				Polynom pTemp1(maxVarIndex+100);
				pTemp1.addMonom(copy);
				this->addSingleDCPolynomial(pTemp1);}
			}
		
			
			//this->addSingleDCPolynomial(pTemp);

		cout << "polynomial afterwards: " << this->poly << endl;
		}
	}
	delete[] tempVars;
	delete[] dcPhases;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::addSingleDCPolynomial(Polynom& pDC) {
	int currDCVar = ++this->poly.maxDCNum;
	Monom2 tempMon;
	for (auto& elem: *pDC.getSet()) {
		tempMon = elem;
		cout << "tempMon is " << tempMon << endl;
		if (elem.getFactor() == 1) this->poly.addDCListEntry({currDCVar}, {1}, tempMon);
		else if (elem.getFactor() == -1) this->poly.addDCListEntry({currDCVar}, {-1}, tempMon);
	}
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::readPolynomialInformationsForDCs(vector<vector<int>*>& dcVarsVector, vector<vector<int>*>& dcCoefsVector, vector<mpz_class>& monCoefs, vector<size_t>& monLength, int maxActivatedDC) {
//	pair<vector<vector<int>*>, pair<vector<vector<int>*>, vector<mpz_class>>> ret;
//	vector<mpz_class> bigCoefs;
	for (auto& elem: *this->poly.getDCList()) {
		for (size_t i=0; i < elem.pToMon.size(); ++i) {
			if (elem.pToMon.at(i) == NULL) continue;
//			cout << "Monomen: " << *elem.pToMon.at(i) << endl;
			dcVarsVector.push_back(const_cast<std::vector<int>*>(&elem.dcVars));
			dcCoefsVector.push_back(const_cast<std::vector<int>*>(&elem.coefs));
			monCoefs.push_back(elem.pToMon.at(i)->getFactor());
			monLength.push_back(elem.pToMon.at(i)->getSize());
		}
	}
//	cout << "coefs intern" << endl;
//	for (auto& num: monCoefs) cout << num << "|";
//	cout << endl;
	/*
	for (auto& num: bigCoefs) cout << num << "|";
	cout << endl;
	mpz_class ggt = findGCD(bigCoefs);
	cout << "ggt is " << ggt << endl;
	vector<long> reducedCoefs(bigCoefs.size());
	mpz_class tempNum;
	if (ggt == 0) { cout << "ggt is 0!!!" << endl;}
	for (size_t j=0; j < bigCoefs.size(); j++) {
		tempNum = bigCoefs.at(j) / ggt;
		if (!tempNum.fits_slong_p()) cout << j << " < tempNum too big!" << endl;
		reducedCoefs.at(j) = tempNum.get_si();
	}
	*/
//	ret.second.second = bigCoefs;
//	cout << "adress is " << &bigCoefs << endl;
//	cout << "and " << &ret.second.second << endl;
//	cout << "adress 1 " << &ret.first << endl;
//	cout << "adress 2 " << &ret.second.first << endl;
//	return ret;
}

// ____________________________________________________________________________________________________________________________
pair<vector<int>, mpz_class> Verifizierer::readDCPolynomialAndSolveILP(int maxActivatedDC) {
	vector<int> solution;  // Store solution of ILP here and return it.
	//*
	vector<vector<int>*> dcVarsVector;
	vector<vector<int>*> dcCoefsVector;
	vector<mpz_class> monCoefs;
	vector<size_t> monLength;
//	pair<vector<vector<int>*>, pair<vector<vector<int>*>, vector<mpz_class>>> infos;
	this->readPolynomialInformationsForDCs(dcVarsVector, dcCoefsVector, monCoefs, monLength, maxActivatedDC);
//	cout << "first are" << endl;
//	for (auto& elem: infos.first) {
//		for (size_t i=0; i < elem->size(); ++i) {
//			cout << elem->at(i) << "|";
//		}
//		cout << endl;
//	}
//	cout << "coefs are " << &infos.second.second << endl;
//	cout << "adress 1 " << &infos.first << endl;
//	cout << "adress 2 " << &infos.second.first << endl;
//	for (auto& num: infos.second.first) cout << num << "|";
//	cout << endl;
//	for (auto& num: monCoefs) cout << num << "|";  // Find GCD of read Coefs and reduce coefs for ILP solving with smaller coefficients.
//	cout << endl;
	mpz_class ggt = findGCD(monCoefs);
	cout << "ggt is " << ggt << endl;
	vector<long> reducedCoefs(monCoefs.size(), 0);  // Use reduced coefs for ILP solving. Dont forget to multiplicate solution of ILP with ggt afterwards.
	mpz_class tempNum;
	if (ggt == 0) {
		cout << "ggt is 0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	} else {
		for (size_t j=0; j < monCoefs.size(); j++) {
			tempNum = monCoefs.at(j) / ggt;
			if (!tempNum.fits_slong_p()) cout << j << " < tempNum too big!!!!!!!!!!!!!!!!!!!!!!" << endl;
			reducedCoefs.at(j) = tempNum.get_si();
		}
	}
//	cout << "reduced coefs are" << endl;
//	for (auto& num: reducedCoefs) cout << num << "|";
//	cout << endl;
	// Create all names and sizes for the variables beforehand.
	int* viNum = new (nothrow) int;//int viNum = this->poly.maxDCNum + 1; // maxActivatedDC + 1; Number of dc Variables.
	if (viNum == nullptr) cout << "Error: viNum memory could not be allocated";
	*viNum = this->poly.maxDCNum + 1;
	cout << "viNum = " << *viNum << endl;
	int* diNum = new (nothrow) int;//int diNum = dcVarsVector.size(); // Number of d_i variables = number of constraints for ILP problem.
	if (diNum == nullptr) cout << "Error: diNum memory could not be allocated";
	*diNum = dcVarsVector.size();
	cout << "diNum = " << *diNum << endl;
	string* diNames = new (nothrow) string[*diNum]; //string diNames[diNum];
	if (diNames == nullptr) cout << "Error: diNames memory could not be allocated";
	char* diTypes = new (nothrow) char[*diNum]; //char diTypes[diNum];
	if (diTypes == nullptr) cout << "Error: diTypes memory could not be allocated";
	double* diMin = new (nothrow) double[*diNum]; //double diMin[diNum];
	if (diMin == nullptr) cout << "Error: diMin memory could not be allocated";
	double* diMax = new (nothrow) double[*diNum]; //double diMax[diNum];
	if (diMax == nullptr) cout << "Error: diMax memory could not be allocated";
	string* viNames = new (nothrow) string[*viNum]; //string viNames[viNum];
	if (viNames == nullptr) cout << "Error: viNames memory could not be allocated";
	char* viTypes = new (nothrow) char[*viNum]; //char viTypes[viNum];
	if (viTypes == nullptr) cout << "Error: viTypes memory could not be allocated";
	double* viMin = new (nothrow) double[*viNum]; //double viMin[viNum];
	if (viMin == nullptr) cout << "Error: viMin memory could not be allocated";
	double* viMax = new (nothrow) double[*viNum]; //double viMax[viNum];
	if (viMax == nullptr) cout << "Error: viMax memory could not be allocated";
	for (size_t i=0; i < *diNum; i++) {  // Define di parameters.
		diNames[i] = "d" + to_string(i+1);
		diTypes[i] = GRB_BINARY;
		diMin[i] = 0;
		diMax[i] = 1;
	}
	for (size_t i=0; i < *viNum; i++) {  // Define vi parameters
		viNames[i] = "v" + to_string(i);
		viTypes[i] = GRB_INTEGER;
		viMin[i] = -GRB_INFINITY;
		viMax[i] = GRB_INFINITY;
		//viMin[i] = -20000;
		//viMax[i] = 20000;
	}
	// Save decision variables in arrays.
	GRBVar* dis = 0;
	GRBVar* vis = 0;

	// Create gurobi environment for solving the ILP.
	try {
	    // Create an environment
	    GRBEnv env = GRBEnv(true);
	    env.set("LogFile", "ilp.log");
	    env.start();
	    // Create an empty model
	    GRBModel model = GRBModel(env);
//	    model.set(GRB_DoubleParam_IntFeasTol, 0.0000001);
	    model.set(GRB_IntParam_IntegralityFocus, 1);
	    // Create decision variables. First for d_i.
	    dis = model.addVars(diMin, diMax, NULL, diTypes, diNames, *diNum);
	    // Next for v_i.
	    vis = model.addVars(viMin, viMax, NULL, viTypes, viNames, *viNum);
	    // Set objective expression: minimize sum over all d_i.
	    GRBLinExpr obj = 0.0;
	    for (size_t i=0; i < *diNum; i++) {
	    	if (monLength.at(i) > 1) obj += 10 * dis[i];  // 2 * dis[i];
	    	else obj += 1 * dis[i];
	    }
//	    cout << "constraint: " << obj << endl;
	    model.setObjective(obj, GRB_MINIMIZE);

	    // Create expressions for constraints. First for right constant side.
	    GRBLinExpr* consPosRight = new (nothrow) GRBLinExpr[*diNum]; //GRBLinExpr consPosRight[diNum];
	    if (consPosRight == nullptr) cout << "Error: consPosRight memory could not be allocated";
	    GRBLinExpr* consNegRight = new (nothrow) GRBLinExpr[*diNum]; //GRBLinExpr consNegRight[diNum];
	    if (consNegRight == nullptr) cout << "Error: consNegRight memory could not be allocated";
	    for (size_t i=0; i < *diNum; i++) {
	    	 consPosRight[i] = 0.0;
	    	 consNegRight[i] = 0.0;
	    }
	    const long bigNumber = 200000;// 200000;  // Used for "activation variables" dis.
	    cout << "bignumber used is " << bigNumber << endl;
	    for (size_t i=0; i < *diNum; i++) {
	    	consPosRight[i] = -1 * reducedCoefs.at(i) + bigNumber * dis[i];
	    	consNegRight[i] = -1 * reducedCoefs.at(i) - bigNumber * dis[i];
	    	/*
	    	if (i < 3) {
	    		consPosRight[i] = -1 + -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 + -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	}
	    	if (i < 7) {
//	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
//	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = bigNumber * dis[i];
	    		consNegRight[i] = -1 * bigNumber * dis[i];
	    	}
	    	//*/
	    }
	    GRBLinExpr* consLeft = new (nothrow) GRBLinExpr[*diNum]; //GRBLinExpr consLeft[diNum];
	    if (consLeft == nullptr) cout << "Error: consLeft memory could not be allocated";
	    for (size_t i=0; i < *diNum; i++) {
	    	consLeft[i] = 0.0;
	    	for (size_t j=0; j < dcVarsVector.at(i)->size(); ++j) {
	    		int currVar = dcVarsVector.at(i)->at(j);
//	    		cout << "currVar " << currVar << endl;
//	    		cout << "currCoef " << dcCoefsVector.at(i)->at(j) << endl;
	    		int currCoef = dcCoefsVector.at(i)->at(j);
	    		consLeft[i] += currCoef * vis[currVar];
//	    		if (currVar > 0) consLeft[i] += vis[currVar];
//	    		else if (currVar < 0) consLeft[i] -= vis[-1 * currVar];
	    	}
	    }
	    // Constraints for all v_i which are not activated: v_i = 0.
	    for (size_t i= maxActivatedDC + 1; i < *viNum; ++i) {
	    	model.addConstr(vis[i] == 0);
	    	cout << "GUROBI: " << i << "was set to Zero." << endl;
	    }
		// Construct all constraints from linear expression parts.
		for (size_t i=0; i < *diNum; ++i) {
			model.addConstr(consLeft[i], GRB_LESS_EQUAL, consPosRight[i]);
			model.addConstr(consLeft[i], GRB_GREATER_EQUAL, consNegRight[i]);
		}
	    //GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
	    //GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
	    //GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");
	    // Set objective: maximize x + y + 2 z
//	    model.setObjective(x + y + 2* z, GRB_MAXIMIZE);
	    // Add constraint: x + 2 y + 3 z <= 4
	    //model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
	    // Add constraint: x + y >= 1
	    //model.addConstr(x + y >= 1, "c1");
	    // Optimize model
		cout << "GUROBI: start optimization." << endl;
	    model.optimize();
		cout << "GUROBI: Optimization ended." << endl;
		
//		if (model.get(GRB_IntAttr_Status) == GRB_INFEASIBLE) cout << "GUROBI OPTIMIZATION FAILED. MODEL IS INFEASIBLE." << endl;
		cout << "GUROBI OPTIMIZATION MODEL STATUS CODE IS " << model.get(GRB_IntAttr_Status) << endl;
		if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {  // Check if model could been solved successfully.
			for (size_t i=0; i < *viNum; i++) {
				solution.push_back(vis[i].get(GRB_DoubleAttr_X));
//				cout << vis[i].get(GRB_StringAttr_VarName) << " " << vis[i].get(GRB_DoubleAttr_X) << endl;
			}
			cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
		} else {
			for (size_t i=0; i < *viNum; i++) {
				solution.push_back(0);  //TODO: Change, such that we do not insert into solution array to later check if it is empty or not.
			}
		}
//		for (size_t i=0; i < diNum; i++) {
//			cout << dis[i].get(GRB_StringAttr_VarName) << " "
//			<< dis[i].get(GRB_DoubleAttr_X) << endl;
//		}
//		cout << "second best solution" << endl;
//		model.set(GRB_IntParam_SolutionNumber, 1);
//	    for (size_t i=0; i < viNum; i++) {
////			solution.push_back(vis[i].get(GRB_DoubleAttr_Xn));
//			cout << vis[i].get(GRB_StringAttr_VarName) << " " << vis[i].get(GRB_DoubleAttr_Xn) << endl;
//		}
//	    cout << "Obj: " << model.get(GRB_DoubleAttr_PoolObjVal) << endl;

		delete[] consPosRight;
		delete[] consNegRight;
		delete[] consLeft;
	} catch(GRBException& e) {
	    cout << "Error code = " << e.getErrorCode() << endl;
	    cout << e.getMessage() << endl;
	} catch(...) {
	    cout << "Exception during optimization" << endl;
	}
	cout << "end ILP" << endl;
	delete[] dis;
	delete[] vis;

	delete viNum;
	delete diNum;
	delete[] diNames;
	delete[] diTypes;
	delete[] diMin;
	delete[] diMax;
	delete[] viNames;
	delete[] viTypes;
	delete[] viMin;
	delete[] viMax;
//	cout << "dynamic memory deleted." << endl;

	//*/
	pair<vector<int>, mpz_class> retPair(solution, ggt);
	return retPair;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::applyDCSolutionToPolynomial(vector<mpz_class>& sol, int maxActivatedDC) {
	cout << "applyDCSolutionToPolynomial started." << endl;
	vector<dcMonEntry> newEntries;
//	for (auto& elem: *this->poly.getDCList()) {
//		for (size_t i=0; i < elem.pToMon.size(); ++i) {  // For every monomial pointer calculate dcCoef and add it to normal coefficient.
//			if (elem.pToMon.at(i) == NULL) continue;
//			cout << *elem.pToMon.at(i) << endl;
//			mpz_class addToCoef = 0;
//			for (size_t i=0; i < elem.dcVars.size(); ++i) {
//				addToCoef += sol.at(elem.dcVars.at(i)) * elem.coefs.at(i);
//			}
//			cout << "addToCoef is " << addToCoef << endl;
//			Monom2* saveP = elem.pToMon.at(i);
//			elem.pToMon.at(i) = NULL;
//			saveP->setFactor(saveP->getFactor() + addToCoef);
//			if (saveP->getFactor() == 0) this->poly.eraseMonom(*saveP);
//			else saveP->setDCPair(NULL, 0);
//		}
//	}
	list<dcMonEntry>::iterator saveIte;
	for (list<dcMonEntry>::iterator ite = this->poly.getDCList()->begin(); ite != this->poly.getDCList()->end(); ++ite) {
		bool biggerDCVarIncluded = false;  // Save information whether this DC entry has an variable bigger than currently activated.
		mpz_class addToCoef = 0;
		for (size_t i=0; i < ite->dcVars.size(); ++i) {
			addToCoef += sol.at(ite->dcVars.at(i)) * ite->coefs.at(i);
			if (!biggerDCVarIncluded && ite->dcVars.at(i) > maxActivatedDC) { biggerDCVarIncluded = true;}
		}
//		cout << "addToCoef is " << addToCoef << endl;
		bool emptyDC = true;  // Save information if this DC entry is no longer pointing to monomials. In this case delete entry from list.
		for (size_t i=0; i < ite->pToMon.size(); ++i) {  // For every monomial pointer add calculated dcCoef to normal coefficient.
			if (ite->pToMon.at(i) == NULL) continue;
			emptyDC = false;
//			cout << *(ite->pToMon.at(i)) << endl;
			Monom2* saveP = ite->pToMon.at(i);
			//ite->pToMon.at(i) = NULL;
			saveP->setFactor(saveP->getFactor() + addToCoef);
			if (!biggerDCVarIncluded) { // In this case create new dc Entry with only the dc variables > maxActivatedDC. Save entry in newEntries vec.
				if (saveP->getFactor() == 0) this->poly.eraseMonom(*saveP);
				else saveP->setDCPair(NULL, 0);
			}
		}
		if (!emptyDC && biggerDCVarIncluded) {
			newEntries.emplace_back(*ite, maxActivatedDC);
//			cout << "new Entry added " << endl;
//			for (size_t i=0; i < newEntries.back().size(); ++i) {
//				std::cout << "|" << newEntries.back().coefs.at(i) << " * " << newEntries.back().dcVars.at(i);
//			}
//			std::cout << std::endl;
//			for (size_t j=0; j < newEntries.back().pToMon.size(); ++j) {
//				if (newEntries.back().pToMon.at(j) != NULL) {
//					std::cout << *newEntries.back().pToMon.at(j) << std::endl;
//				}
//			}
		}
//		saveIte = ite;
//		++saveIte;
//		this->poly.getDCList()->erase(ite);
	}
	this->poly.clearDCList();  // Old entries are solved and no longer needed.

//	cout << "iteration completed!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//	cout << "newEntries size is : " << newEntries.size() << endl;
//	for (auto& elem: newEntries) {
//		cout << "this entry has size " << elem.dcVars.size() << endl;
//		for (size_t i=0; i < elem.dcVars.size(); ++i) {
//			cout << "|" << elem.coefs.at(i) << " * " << elem.dcVars.at(i);
//		}
//		cout << endl;
//		cout << "this entry has pToMon size " << elem.pToMon.size() << endl;
//		for (size_t j=0; j < elem.pToMon.size(); ++j) {
//			if (elem.pToMon.at(j) != NULL) {
//				cout << *elem.pToMon.at(j) << endl;
//			}
//		}
//	}
//	cout << "before inserting back" << endl;
	dcMonEntry* newEntryP;
	for (auto& elem: newEntries) {
		this->poly.getDCList()->push_back(elem);
		newEntryP = &(this->poly.getDCList()->back());
		for (size_t i=0; i < elem.pToMon.size(); ++i) {
			if (elem.pToMon.at(i) != NULL) elem.pToMon.at(i)->setDCPair(newEntryP, i);
		}
	}
//	for (auto& elem: *(this->poly.getDCList())) {
//			cout << endl;
//			for (size_t i=0; i < elem.dcVars.size(); ++i) {
//				cout << "|" << elem.coefs.at(i) << " * " << elem.dcVars.at(i);
//			}
//			cout << endl;
//			for (size_t j=0; j < elem.pToMon.size(); ++j) {
//				if (elem.pToMon.at(j) != NULL) {
//					cout << *elem.pToMon.at(j) << endl;
//				cout << elem.coefToMon.at(j) << endl;
//				}
//			}
//		}
//	cout << "before polynom is now: " << endl;
//	cout << "polynom is now: " << this->poly << endl;
//	cout << "before clearDCList" << endl;
//	this->poly.clearDCList();  // Clear DC list after applying all DCs.
//	this->poly.maxDCNum = 0;
//	if (this->poly.getDCList()->empty()) this->poly.maxDCNum = 0;
//	cout << "applyDCSolutionToPolynomial finished." << endl;
}

// ____________________________________________________________________________________________________________________________
void Verifizierer::applyDCSolutionToOtherPolynomial(Polynom& pol, vector<mpz_class>& sol, int maxActivatedDC) {
//	cout << "applyDCSolutionToOtherPolynomial started." << endl;
	vector<dcMonEntry> newEntries;
	list<dcMonEntry>::iterator saveIte;
	for (list<dcMonEntry>::iterator ite = pol.getDCList()->begin(); ite != pol.getDCList()->end(); ++ite) {
		bool biggerDCVarIncluded = false;  // Save information whether this DC entry has an variable bigger than currently activated.
		mpz_class addToCoef = 0;
		for (size_t i=0; i < ite->dcVars.size(); ++i) {
			addToCoef += sol.at(ite->dcVars.at(i)) * ite->coefs.at(i);
			if (!biggerDCVarIncluded && ite->dcVars.at(i) > maxActivatedDC) { biggerDCVarIncluded = true;}
		}
//		cout << "addToCoef is " << addToCoef << endl;
		bool emptyDC = true;  // Save information if this DC entry is no longer pointing to monomials. In this case delete entry from list.
		for (size_t i=0; i < ite->pToMon.size(); ++i) {  // For every monomial pointer add calculated dcCoef to normal coefficient.
			if (ite->pToMon.at(i) == NULL) continue;
			emptyDC = false;
//			cout << *(ite->pToMon.at(i)) << endl;
			Monom2* saveP = ite->pToMon.at(i);
			//ite->pToMon.at(i) = NULL;
			saveP->setFactor(saveP->getFactor() + addToCoef);
			if (!biggerDCVarIncluded) { // In this case create new dc Entry with only the dc variables > maxActivatedDC. Save entry in newEntries vec.
				if (saveP->getFactor() == 0) pol.eraseMonom(*saveP);
				else saveP->setDCPair(NULL, 0);
			}
		}
		if (!emptyDC && biggerDCVarIncluded) {
			newEntries.emplace_back(*ite, maxActivatedDC);
		}
	}
	pol.clearDCList();  // Old entries are solved and no longer needed.
	dcMonEntry* newEntryP;
	for (auto& elem: newEntries) {
		pol.getDCList()->push_back(elem);
		newEntryP = &(pol.getDCList()->back());
		for (size_t i=0; i < elem.pToMon.size(); ++i) {
			if (elem.pToMon.at(i) != NULL) elem.pToMon.at(i)->setDCPair(newEntryP, i);
		}
	}
	//pol.polySet.clear();
//	cout << "applyDCSolutionToOtherPolynomial finished." << endl;
}

// ____________________________________________________________________________________________________________________________
vector<int> Verifizierer::solveILPforFA(dc dontCare, vector<long> readCoefs) {
	vector<int> solution;  // Store solution of ILP here and return it.
	//*
	int sig1 = circuit.edges.at(dontCare.sig1).eIndex;
	int sig2 = circuit.edges.at(dontCare.sig2).eIndex;
	int sig3 = circuit.edges.at(dontCare.sig3).eIndex;

	// Save which dont cares on this fullAdder were found.
	vector<int> varsILP;
	for (size_t i=0; i < 8; i++) {
		//if (i != 0 && i != 7) continue;
		//if (i == 2) continue;
		if (!dontCare.poss[i]) varsILP.push_back(i);
	}
	cout << "varsILP size: " << varsILP.size() << endl;
	for (auto& elem: varsILP) cout << elem << endl;

	// Create all names and sizes for the variables beforehand.
	int viNum = varsILP.size();
	int diNum = 7 + viNum;
	string diNames[diNum];
	char diTypes[diNum];
	double diMin[diNum];
	double diMax[diNum];
	string viNames[viNum];
	char viTypes[viNum];
	double viMin[viNum];
	double viMax[viNum];
	for (size_t i=0; i < diNum; i++) {  // Define di parameters.
		diNames[i] = "d" + to_string(i+1);
		diTypes[i] = GRB_BINARY;
		diMin[i] = 0;
		diMax[i] = 1;
	}

	for (size_t i=0; i < viNum; i++) {  // Define vi parameters
		viNames[i] = "v" + to_string(i+1);
		viTypes[i] = GRB_INTEGER;
		viMin[i] = -GRB_INFINITY;
		viMax[i] = GRB_INFINITY;
		//viMin[i] = -10000000;
		//viMax[i] = 10000000;
	}
	// Save decision variables in arrays.
	GRBVar* dis = 0;
	GRBVar* vis = 0;

	// Create gurobi environment for solving the ILP.
	try {
	    // Create an environment
	    GRBEnv env = GRBEnv(true);
	    env.set("LogFile", "ilp.log");
	    env.start();
	    // Create an empty model
	    GRBModel model = GRBModel(env);
	    // Create decision variables. First for d_i.
	    dis = model.addVars(diMin, diMax, NULL, diTypes, diNames, diNum);
	    // Next for v_i.
	    vis = model.addVars(viMin, viMax, NULL, viTypes, viNames, viNum);
	    // Set objective expression: minimize sum over all d_i.
	    GRBLinExpr obj = 0.0;
	    for (size_t i=0; i < diNum; i++) {
	    	if (i < 3 || i > 6) obj += 1 * dis[i];
	    	else obj += 10 * dis[i];
	    }
	    model.setObjective(obj, GRB_MINIMIZE);
	    // Create expressions for constraints. First for right constant side.
	    GRBLinExpr consPosRight[diNum];
	    GRBLinExpr consNegRight[diNum];
	    for (size_t i=0; i < diNum; i++) {
	    	 consPosRight[diNum] = 0.0;
	    	 consNegRight[diNum] = 0.0;
	    }
	    const long bigNumber = 20000;  // Used for "activation variables" dis.
	    for (size_t i=0; i < diNum; i++) {
	    	/*
	    	if (i < 3) {
	    		consPosRight[i] = -1 + -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 + -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	}//*/
	    	if (i < 7) {
	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = bigNumber * dis[i];
	    		consNegRight[i] = -1 * bigNumber * dis[i];
	    	}
	    }
	    GRBLinExpr consLeft[diNum];
	    for (size_t i=0; i < diNum; i++) {
	    	if (i < 7) consLeft[i] = 0.0;
	    	else consLeft[i] = vis[i-7];  // constLeft[i >= 7] is only the v_i variable itself.
	    }
	    // Create left side of constraints with vis.
	    for (size_t i=0; i < viNum; i++) {
	    	if (varsILP.at(i)==0) {
	    		consLeft[0] -= vis[i];
	    		consLeft[1] -= vis[i];
	    		consLeft[2] -= vis[i];
	    		consLeft[3] += vis[i];
	    		consLeft[4] += vis[i];
	    		consLeft[5] += vis[i];
	    		consLeft[6] -= vis[i];
	    	} else if (varsILP.at(i)==1) {
	    		consLeft[0] += vis[i];
	    		consLeft[3] -= vis[i];
	    		consLeft[4] -= vis[i];
	    		consLeft[6] += vis[i];
	    	} else if (varsILP.at(i)==2) {
	    		consLeft[1] += vis[i];
	    		consLeft[3] -= vis[i];
	    		consLeft[5] -= vis[i];
	    		consLeft[6] += vis[i];
	    	} else if (varsILP.at(i)==3) {
	    		consLeft[3] += vis[i];
	    		consLeft[6] -= vis[i];
	    	} else if (varsILP.at(i)==4) {
	    		consLeft[2] += vis[i];
	    		consLeft[4] -= vis[i];
	    		consLeft[5] -= vis[i];
	    		consLeft[6] += vis[i];
	    	} else if (varsILP.at(i)==5) {
	    		consLeft[4] += vis[i];
	    		consLeft[6] -= vis[i];
	    	} else if (varsILP.at(i)==6) {
	    		consLeft[5] += vis[i];
	    		consLeft[6] -= vis[i];
	    	} else if (varsILP.at(i)==7) {
	    		consLeft[6] += vis[i];
	    	}
	    }

		// Construct all constraints from linear expression parts.
		for (size_t i=0; i < diNum; i++) {
			model.addConstr(consLeft[i], GRB_LESS_EQUAL, consPosRight[i]);
			model.addConstr(consLeft[i], GRB_GREATER_EQUAL, consNegRight[i]);
		}
	    //GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
	    //GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
	    //GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");
	    // Set objective: maximize x + y + 2 z
//	    model.setObjective(x + y + 2* z, GRB_MAXIMIZE);
	    // Add constraint: x + 2 y + 3 z <= 4
	    //model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
	    // Add constraint: x + y >= 1
	    //model.addConstr(x + y >= 1, "c1");
	    // Optimize model
	    model.optimize();

		for (size_t i=0; i < viNum; i++) {
			solution.push_back(vis[i].get(GRB_DoubleAttr_X));
		}
		//cout << vis[0].get(GRB_StringAttr_VarName) << " "
		//<< vis[0].get(GRB_DoubleAttr_X) << endl;
		//solution.push_back(vis[0].get(GRB_DoubleAttr_X));
		//cout << vis[1].get(GRB_StringAttr_VarName) << " "
		//<< vis[1].get(GRB_DoubleAttr_X) << endl;
		//solution.push_back(vis[1].get(GRB_DoubleAttr_X));
		//for (size_t i=0; i < diNum; i++) {
		//	cout << dis[i].get(GRB_StringAttr_VarName) << " "
		//	<< dis[i].get(GRB_DoubleAttr_X) << endl;
		//}
	    //cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
	} catch(GRBException& e) {
	    cout << "Error code = " << e.getErrorCode() << endl;
	    cout << e.getMessage() << endl;
	} catch(...) {
	    cout << "Exception during optimization" << endl;
	}
	//cout << "end ILP" << endl;
	delete[] dis;
	delete[] vis;
	//*/
	return solution;
}

// ____________________________________________________________________________________________________________________________
vector<int> Verifizierer::solveILPforFA4(input4dc dontCare, vector<long> readCoefs) {
	vector<int> solution;  // Store solution of ILP here and return it.
	//*
	int sig1 = circuit.edges.at(dontCare.sig1).eIndex;
	int sig2 = circuit.edges.at(dontCare.sig2).eIndex;
	int sig3 = circuit.edges.at(dontCare.sig3).eIndex;
	int sig4 = circuit.edges.at(dontCare.sig4).eIndex;

	// Save which dont cares on this fullAdder were found.
	vector<int> varsILP;
	for (size_t i=0; i < 16; i++) {
		//if (i != 0 && i != 7) continue;
		//if (i == 2) continue;
		if (!dontCare.poss[i]) varsILP.push_back(i);
	}
	cout << "varsILP size: " << varsILP.size() << endl;
	for (auto& elem: varsILP) cout << elem << endl;

	// Create all names and sizes for the variables beforehand.
	int viNum = varsILP.size();
	int diNum = 15 + viNum;
	string diNames[diNum];
	char diTypes[diNum];
	double diMin[diNum];
	double diMax[diNum];
	string viNames[viNum];
	char viTypes[viNum];
	double viMin[viNum];
	double viMax[viNum];
	for (size_t i=0; i < diNum; i++) {  // Define di parameters.
		diNames[i] = "d" + to_string(i+1);
		diTypes[i] = GRB_BINARY;
		diMin[i] = 0;
		diMax[i] = 1;
	}

	for (size_t i=0; i < viNum; i++) {  // Define vi parameters
		viNames[i] = "v" + to_string(i+1);
		viTypes[i] = GRB_INTEGER;
		viMin[i] = -GRB_INFINITY;
		viMax[i] = GRB_INFINITY;
		//viMin[i] = -10000000;
		//viMax[i] = 10000000;
	}
	// Save decision variables in arrays.
	GRBVar* dis = 0;
	GRBVar* vis = 0;

	// Create gurobi environment for solving the ILP.
	try {
	    // Create an environment
	    GRBEnv env = GRBEnv(true);
	    env.set("LogFile", "ilp.log");
	    env.start();
	    // Create an empty model
	    GRBModel model = GRBModel(env);
	    // Create decision variables. First for d_i.
	    dis = model.addVars(diMin, diMax, NULL, diTypes, diNames, diNum);
	    // Next for v_i.
	    vis = model.addVars(viMin, viMax, NULL, viTypes, viNames, viNum);
	    // Set objective expression: minimize sum over all d_i.
	    GRBLinExpr obj = 0.0;
	    // int objfac = 0;
	    //cout << "type in objfac" << endl;
	    //cin >> objfac;
	    for (size_t i=0; i < diNum; i++) {
	    	if (i < 4) obj += 0 * dis[i];  // single variable monomials weighted.
	    	else if (i > 14) obj += 5 * dis[i];  // weight of using a DC or deactivating it.
	    	else obj += 100 * dis[i];  // mixed terms weighted here. Should be more?
	    }
	    model.setObjective(obj, GRB_MINIMIZE);
	    // Create expressions for constraints. First for right constant side.
	    GRBLinExpr consPosRight[diNum];
	    GRBLinExpr consNegRight[diNum];
	    for (size_t i=0; i < diNum; i++) {
	    	 consPosRight[diNum] = 0.0;
	    	 consNegRight[diNum] = 0.0;
	    }
	    const long bigNumber = 20000;  // Used for "activation variables" dis.
	    for (size_t i=0; i < diNum; i++) {
	    	/*
	    	if (i < 3) {
	    		consPosRight[i] = -1 + -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 + -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	}//*/
	    	if (i < 15) {
	    		consPosRight[i] = -1 * readCoefs.at(i) + bigNumber * dis[i];
	    		consNegRight[i] = -1 * readCoefs.at(i) - bigNumber * dis[i];
	    	} else {
	    		consPosRight[i] = bigNumber * dis[i];
	    		consNegRight[i] = -1 * bigNumber * dis[i];
	    	}
	    }
	    GRBLinExpr consLeft[diNum];
	    for (size_t i=0; i < diNum; i++) {
	    	if (i < 15) consLeft[i] = 0.0;
	    	else consLeft[i] = vis[i-15];  // constLeft[i >= 15] is only the v_i variable itself in this constraint.
	    }
	    // Create left side of constraints with vis.
	    for (size_t i=0; i < viNum; i++) {
	    	if (varsILP.at(i)==0) {
	    		consLeft[0] -= vis[i];
	    		consLeft[1] -= vis[i];
	    		consLeft[2] -= vis[i];
	    		consLeft[3] -= vis[i];
	    		consLeft[4] += vis[i];
	    		consLeft[5] += vis[i];
	    		consLeft[6] += vis[i];
	    		consLeft[7] += vis[i];
	    		consLeft[8] += vis[i];
	    		consLeft[9] += vis[i];
	    		consLeft[10] -= vis[i];
	    		consLeft[11] -= vis[i];
	    		consLeft[12] -= vis[i];
	    		consLeft[13] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==1) {
	    		consLeft[0] += vis[i];
	    		consLeft[4] -= vis[i];
	    		consLeft[5] -= vis[i];
	    		consLeft[6] -= vis[i];
	    		consLeft[10] += vis[i];
	    		consLeft[11] += vis[i];
	    		consLeft[12] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==2) {
	    		consLeft[1] += vis[i];
	    		consLeft[4] -= vis[i];
	    		consLeft[7] -= vis[i];
	    		consLeft[8] -= vis[i];
	    		consLeft[10] += vis[i];
	    		consLeft[11] += vis[i];
	    		consLeft[13] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==3) {
	    		consLeft[4] += vis[i];
	    		consLeft[10] -= vis[i];
	    		consLeft[11] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==4) {
	    		consLeft[2] += vis[i];
	    		consLeft[5] -= vis[i];
	    		consLeft[7] -= vis[i];
	    		consLeft[9] -= vis[i];
	    		consLeft[10] += vis[i];
	    		consLeft[12] += vis[i];
	    		consLeft[13] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==5) {
	    		consLeft[5] += vis[i];
	    		consLeft[10] -= vis[i];
	    		consLeft[12] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==6) {
	    		consLeft[7] += vis[i];
	    		consLeft[10] -= vis[i];
	    		consLeft[13] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==7) {
	    		consLeft[10] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==8) {
	    		consLeft[3] += vis[i];
	    		consLeft[6] -= vis[i];
	    		consLeft[8] -= vis[i];
	    		consLeft[9] -= vis[i];
	    		consLeft[11] += vis[i];
	    		consLeft[12] += vis[i];
	    		consLeft[13] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==9) {
	    		consLeft[6] += vis[i];
	    		consLeft[11] -= vis[i];
	    		consLeft[12] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==10) {
	    		consLeft[8] += vis[i];
	    		consLeft[11] -= vis[i];
	    		consLeft[13] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==11) {
	    		consLeft[11] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==12) {
	    		consLeft[9] += vis[i];
	    		consLeft[12] -= vis[i];
	    		consLeft[13] -= vis[i];
	    		consLeft[14] += vis[i];
	    	} else if (varsILP.at(i)==13) {
	    		consLeft[12] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==14) {
	    		consLeft[13] += vis[i];
	    		consLeft[14] -= vis[i];
	    	} else if (varsILP.at(i)==15) {
	    		consLeft[14] += vis[i];
	    	}
	    }

		// Construct all constraints from linear expression parts.
		for (size_t i=0; i < diNum; i++) {
			model.addConstr(consLeft[i], GRB_LESS_EQUAL, consPosRight[i]);
			model.addConstr(consLeft[i], GRB_GREATER_EQUAL, consNegRight[i]) ;
		}
	    //GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
	    //GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
	    //GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");
	    // Set objective: maximize x + y + 2 z
//	    model.setObjective(x + y + 2* z, GRB_MAXIMIZE);
	    // Add constraint: x + 2 y + 3 z <= 4
	    //model.addConstr(x + 2 * y + 3 * z <= 4, "c0");
	    // Add constraint: x + y >= 1
	    //model.addConstr(x + y >= 1, "c1");
	    // Optimize model
	    model.optimize();

		for (size_t i=0; i < viNum; i++) {
			solution.push_back(vis[i].get(GRB_DoubleAttr_X));
		}
		//cout << vis[0].get(GRB_StringAttr_VarName) << " "
		//<< vis[0].get(GRB_DoubleAttr_X) << endl;
		//solution.push_back(vis[0].get(GRB_DoubleAttr_X));
		//cout << vis[1].get(GRB_StringAttr_VarName) << " "
		//<< vis[1].get(GRB_DoubleAttr_X) << endl;
		//solution.push_back(vis[1].get(GRB_DoubleAttr_X));
		//for (size_t i=0; i < diNum; i++) {
		//	cout << dis[i].get(GRB_StringAttr_VarName) << " "
		//	<< dis[i].get(GRB_DoubleAttr_X) << endl;
		//}
	    //cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
	} catch(GRBException& e) {
	    cout << "Error code = " << e.getErrorCode() << endl;
	    cout << e.getMessage() << endl;
	} catch(...) {
	    cout << "Exception during optimization" << endl;
	}
	//cout << "end ILP" << endl;
	delete[] dis;
	delete[] vis;
	//*/
	return solution;
}

// ____________________________________________________________________________________________________________________________
Minisat::Var Verifizierer::addInputConstraintToSolver(Minisat::Solver& solver, std::map<int, Minisat::Var>& varMap) {
	// Add constraint 0 <= R^0 < D*2^(n-1) to the solver.
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divider = this->pMap.at("D");
	assert(dividend.size() == 2 * divider.size());
	int lastR0Index = dividend.size() - divider.size();

	Minisat::Var neg, xnorSig, andFirst, andSecond, prevOut, orSig;  // Intermediate signals.
	Minisat::Var dVar, r0Var;  // Solver variables of used primary signals.
	for (size_t i = 0; i < divider.size(); i++) {
		dVar = (*(varMap.insert(std::pair<int, Minisat::Var>(this->circuit.node(divider.at(i)).outputs.at(0)->eIndex, solver.newVar()))).first).second;
		r0Var = (*(varMap.insert(std::pair<int, Minisat::Var>(this->circuit.node(dividend.at(lastR0Index + i)).outputs.at(0)->eIndex, solver.newVar()))).first).second;
		if (i==0) { // d_0 & !r_(lastR0Index) special case.
			neg = solver.newVar();
			addTseitinClauses(solver, r0Var, 1 , neg, vp::Node::NOT);
			andFirst = solver.newVar();
			addTseitinClauses(solver, dVar, neg, andFirst, vp::Node::AND);
			prevOut = andFirst;  // Used for next stage.
		} else {  // General cases.
			// XNOR:
			xnorSig = solver.newVar();
			addTseitinClauses(solver, dVar, r0Var , xnorSig, vp::Node::XNOR);
			// prevOut & XNOR:
			andFirst = solver.newVar();
			addTseitinClauses(solver, prevOut, xnorSig, andFirst, vp::Node::AND);
			// Negation of r bit:
			neg = solver.newVar();
			addTseitinClauses(solver, r0Var, 1 , neg, vp::Node::NOT);
			// !r_bit & d_bit:
			andSecond = solver.newVar();
			addTseitinClauses(solver, neg, dVar, andSecond, vp::Node::AND);
			// andFirst OR andSecond:
			orSig = solver.newVar();
			addTseitinClauses(solver, andFirst, andSecond, orSig, vp::Node::OR);
			prevOut = orSig;
		}
	}
	// Return sat variable of output of this whole input constraint circuit, which is prevOut at the end.
	return prevOut;
}


// _______________________________________________________________________________________________________________________________________
bool Verifizierer::testSAT(string e1, string e2, bool equivalence) {
	assert(e1 != e2);

	//if ((e1 != "Q[0]" && e2 != "Q[0]") && (e1 != "_1552_" && e2 != "_1552_")) return false;
	//if ((e1 != "Q[0]" && e2 != "Q[0]")) return false;
	
	// Check if equivalence holds only because both are the negation of the same signal.
	/*vp::Node* currNode1 = &circuit.node(circuit.edges.at(e1).source);
	vp::Node* currNode2 = &circuit.node(circuit.edges.at(e2).source);
	if (currNode1->type == vp::Node::NOT && currNode2->type == vp::Node::NOT) {
		if (currNode1->inputs.at(0)->eIndex == currNode2->inputs.at(0)->eIndex) return false;
	}
	if (currNode1->adder > -1 || currNode2->adder > -1) return false;
	*/
	/*
	bool isIntern = true;
	if (circuit.node(circuit.edges.at(e1).source).adder == circuit.node(circuit.edges.at(e1).getDestination(0)).adder) isIntern = true;
	else isIntern = false;
	if (this->dcFAs.count(circuit.node(circuit.edges.at(e1).source).adder) > 0 && isIntern) return false;
	//*/
	//cout << "IN SAT check for signals: " << e1 << " and " << e2 << " and equi: " << equivalence << endl;
	
	// Use eIndex instead of edge name for determining and saving equivalence/antivalence.
	int e1Index = circuit.edges.at(e1).eIndex;
	int e2Index = circuit.edges.at(e2).eIndex;
	
	//if (e1Index == 1894 || e2Index == 1894) return false;
	//if ((e1Index != 774 && e2Index != 774) && (e1Index != 1894 && e2Index != 1894) ) return false;
	
	//if (this->dcSignals.count(e1Index) > 0) return false;

	// Check if representative of e1 and e2 are the same. In this case no SAT check is needed.
	if (this->represent[e1Index].sig->eIndex == this->represent[e2Index].sig->eIndex) {
		//cout << "both signals already represented by same signal. Stop SAT check here." << endl;
		return false;
	}

	// Create solver and map containing SAT variables corresponding to circuits edge indices.
	Minisat::Solver solver;
	std::map<int, Minisat::Var> varMap;

	// Insert clauses for input constraint into the solver.
	addInputConstraintToSolver(solver, varMap); //TODO: Input Constraint richtig hinzufügen.

	// Create miter output gate of both edges and add it to the map and solver.
	int miterOutputIndex = 0;
	varMap.insert(std::pair<int, Minisat::Var>(miterOutputIndex, solver.newVar())); // Insert miter output variable.
	if (equivalence) solver.addClause(Minisat::mkLit(varMap.at(miterOutputIndex)));  // Miter output is one for equivalence check.
	else solver.addClause(~Minisat::mkLit(varMap.at(miterOutputIndex)));  // Miter output is zero for antivalence check.
	varMap.insert(std::pair<int, Minisat::Var>(e1Index, solver.newVar()));  // Insert variable for edge1.
	varMap.insert(std::pair<int, Minisat::Var>(e2Index, solver.newVar()));  // Insert variable for edge2.
	addTseitinClauses(solver, varMap.at(e1Index), varMap.at(e2Index), varMap.at(miterOutputIndex), vp::Node::XOR);  // Insert miter xor.

	// Add all found DCs to SAT instance to compute the equivalences/antivalences modulo dont cares.
	Minisat::Var f1, f2, f3;
	int f1Repr, f2Repr, f3Repr;
	for (auto& elem: this->generalDCs) {
		if (elem.size() != 3) continue;
		f1Repr = this->represent[circuit.edges.at(elem.signals.at(0)).eIndex].sig->eIndex;
		f2Repr = this->represent[circuit.edges.at(elem.signals.at(1)).eIndex].sig->eIndex;
		f3Repr = this->represent[circuit.edges.at(elem.signals.at(2)).eIndex].sig->eIndex;
		f1 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(0)).eIndex, solver.newVar()))).first).second;
		f2 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(1)).eIndex, solver.newVar()))).first).second;
		f3 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(2)).eIndex, solver.newVar()))).first).second;
		for (size_t i=0; i < 8; i++) {
			if (elem.poss[i] == true) continue;  // This one is not DC. Continue with next one. 
			if (i == 0) {
				solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 1) {
				solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 2) {
				solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 3) {
				solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 4) {
				solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
			if (i == 5) {
				solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
				}
			if (i == 6) {
				solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
			if (i == 7) {
				solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
		}		
	}
	/* Now for 4 input dcs.
	for (auto& elem: this->generalDCs) {
		if (elem.size() != 4) continue;
		f1Repr = this->represent[circuit.edges.at(elem.signals.at(0)).eIndex].sig->eIndex;
		f2Repr = this->represent[circuit.edges.at(elem.signals.at(1)).eIndex].sig->eIndex;
		f3Repr = this->represent[circuit.edges.at(elem.signals.at(2)).eIndex].sig->eIndex;
		f1 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(0)).eIndex, solver.newVar()))).first).second;
		f2 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(1)).eIndex, solver.newVar()))).first).second;
		f3 = (*(varMap.insert(std::pair<int, Minisat::Var>(circuit.edges.at(elem.signals.at(2)).eIndex, solver.newVar()))).first).second;
		for (size_t i=0; i < 8; i++) {
			if (elem.poss[i] == true) continue;  // This one is not DC. Continue with next one.
			if (i == 0) {
				solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 1) {
				solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 2) {
				solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 3) {
				solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), Minisat::mkLit(f3));
			}
			if (i == 4) {
				solver.addClause(Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
			if (i == 5) {
				solver.addClause(~Minisat::mkLit(f1), Minisat::mkLit(f2), ~Minisat::mkLit(f3));
				}
			if (i == 6) {
				solver.addClause(Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
			if (i == 7) {
				solver.addClause(~Minisat::mkLit(f1), ~Minisat::mkLit(f2), ~Minisat::mkLit(f3));
			}
		}
	}
	*/
	// Traverse the circuit backwards to primary inputs (or until end of window) and add tseitin clauses of every node.
	std::vector<vp::Node*> frontierNodes;
	frontierNodes.push_back(&circuit.node(circuit.edges.at(e1).getSource()));
	frontierNodes.push_back(&circuit.node(circuit.edges.at(e2).getSource()));
	vp::Node* currNode;
	vp::Node* nextNode1 = NULL;
	vp::Node* nextNode2 = NULL;
	int outputIndex, input1Index, input2Index;
	int in1, in2, outNum;
	bool in1Inverted = false, in2Inverted = false;
	std::set<int> insertedNodes;

	// Before building tseitin, traverse circuit from both starting nodes up to window width with breadth first search. Safe all nodes in the given window in frontierNodes, which will be all put in tseitin transformation in next step.
	int windowWidth = 2;  // 6 for AIGs?
	std::set<int> windowNodes;
	this->nodesInWindowRangeWithRepr(frontierNodes.at(0), windowWidth, windowNodes);
	this->nodesInWindowRangeWithRepr(frontierNodes.at(1), windowWidth, windowNodes);

	//Add tseitin clauses of all nodes in window range. Use representative if they are given for specific nodes.
	for (size_t i = 0; i < frontierNodes.size(); i++) {
		in1Inverted = false;
		in2Inverted = false;
		currNode = frontierNodes.at(i);
		if (windowNodes.count(currNode->nIndex) == 0) continue;  // Considered node is not in window range. Skip it.
		if (currNode->type == vp::Node::INPUT_PORT || currNode->type == vp::Node::DELETED) continue;
		outputIndex = currNode->outputs.at(0)->eIndex;
		outNum = varMap.at(outputIndex);
		// Easy way of bringing in previously found equi/anti pairs: Just add tseitin clause for the pair of signals.
//		if (i > 1) {  // Only use representatives for nodes which are not the two starting nodes.
//			if (this->represent[outputIndex].sig != 0) {
//				outputIndex = this->represent[outputIndex].sig->eIndex;
//				if (!(this->represent[outputIndex].state)) addTseitinClauses(solver, outNum, outNum, (*(varMap.insert(std::pair<int, Minisat::Var>(represent[outputIndex].sig->eIndex, solver.newVar()))).first).second, vp::Node::NOT);
//				else //addTseitinClauses(solver, outNum, outNum, (*(varMap.insert(std::pair<int, Minisat::Var>(represent[outputIndex].sig->eIndex, solver.newVar()))).first).second, vp::Node::NOT);
//
//		}
		if (insertedNodes.count(outNum) > 0) continue;	// Node output already in map -> Node already added to solver. Skip it.

		// New node. At inputs to map and add tseitin clauses to solver. Before: Check whether the inputs have representatives and use those instead.
		input1Index = currNode->inputs.at(0)->eIndex;
		in1 = (*(varMap.insert(std::pair<int, Minisat::Var>(input1Index, solver.newVar()))).first).second;
		if (this->represent[input1Index].sig != 0) {  // input1 has representative.
			if (!(this->represent[input1Index].state)) {
				addTseitinClauses(solver, in1, in1, (*(varMap.insert(std::pair<int, Minisat::Var>(represent[input1Index].sig->eIndex, solver.newVar()))).first).second, vp::Node::NOT);
				in1Inverted = true;
			} else {
				in1 = (*(varMap.insert(std::pair<int, Minisat::Var>(represent[input1Index].sig->eIndex, solver.newVar()))).first).second;
			}
			nextNode1 = &(this->circuit.nodes.at(represent[input1Index].sig->source));
		} else {  // input1 has no representative.
			nextNode1 = &currNode->inputNode(0);
		}

		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER) {
			input2Index = currNode->inputs.at(1)->eIndex;
			in2 = (*(varMap.insert(std::pair<int, Minisat::Var>(input2Index, solver.newVar()))).first).second;
			if (this->represent[input2Index].sig != 0) {  // input2 has representative.
				if (!(this->represent[input2Index].state)) {
					addTseitinClauses(solver, in2, in2, (*(varMap.insert(std::pair<int, Minisat::Var>(represent[input2Index].sig->eIndex, solver.newVar()))).first).second, vp::Node::NOT);
					in2Inverted = true;
				} else {
					in2 = (*(varMap.insert(std::pair<int, Minisat::Var>(represent[input2Index].sig->eIndex, solver.newVar()))).first).second;
				}
				nextNode2 = &(this->circuit.nodes.at(represent[input2Index].sig->source));
			} else {  // input2 has no representative.
				nextNode2 = &currNode->inputNode(1);
			}
		} else {
			in2 = in1;
		}

		addTseitinClauses(solver, in1, in2, outNum, currNode->type);  // Add tseitin clauses of the current node to the solver.

		//cout << "represent pointer: " << this->represent[input1Index].sig << endl;
		if (in1Inverted) in1 = varMap.at(this->represent[input1Index].sig->eIndex);
		if (in2Inverted) in2 = varMap.at(this->represent[input2Index].sig->eIndex);

		// Add inputNodes of current node to frontierNodes vector if the node with this output was not already inserted.
		if (insertedNodes.count(in1) == 0 && nextNode1->type != vp::Node::INPUT_PORT) frontierNodes.push_back(nextNode1);
		if (currNode->type != vp::Node::NOT && currNode->type != vp::Node::BUFFER && nextNode2->type != vp::Node::INPUT_PORT && insertedNodes.count(in2) == 0) frontierNodes.push_back(nextNode2);
		insertedNodes.insert(outNum);  // Add to set to remember that this node is already inserted.
	}

	// Solve and if unsatisfiable, save information of found equivalences/antivalences in represent field (or alternatively replace the wire in the circuit).

    bool sat = solver.solve();
    if (sat) {
    	//std::cout << "SAT Model found: " << e1 << " and " << e2 << " and equi: " << equivalence << endl;
    	/*
    		for (std::map<int, Minisat::Var>::iterator it = varMap.begin(); it != varMap.end(); it++) {
       		std::cout << "Edge index: " << it->first << " := " << (solver.modelValue(it->second) == l_True) << std::endl;
       	}//*/
       	return false;
    } else {
    	//cout << "Equivalent Signals: " << e1 << " | " << e2 << " | " << equivalence << endl;
    	// Add e2 as representative of e1 into the array.
    	this->addRepresentative(e1Index, e2Index, equivalence);
    	//this->addRepresentativeSimple(e1Index, e2Index, equivalence, &(this->circuit.edges.at(e2)));
        return true;
    }
    cout << "TESTSAT: THIS SHOULD NEVER HAPPEN!!" << endl;
    return false;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::addRepresentative(varIndex e1Index, varIndex e2Index, bool equi) {
	// Determine representatives of both signals first.
	repr* rep1 = &this->represent[this->represent[e1Index].sig->eIndex];
	repr* rep2 = &this->represent[this->represent[e2Index].sig->eIndex];
//	if (rep1 == rep2) return; // Both signals already represented by same signal. Skip this.
	bool repState;
	int levelRep1 = 0, levelRep2 = 0;
	if (this->circuit.node(rep1->sig->getSource()).type == vp::Node::INPUT_PORT) levelRep1 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
	else levelRep1 = this->circuit.node(rep1->sig->getSource()).revLevel;
	if (this->circuit.node(rep2->sig->getSource()).type == vp::Node::INPUT_PORT) levelRep2 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
	else levelRep2 = this->circuit.node(rep2->sig->getSource()).revLevel;
	if (levelRep1 < levelRep2) {
		// rep2 < rep1 wrt. top. order -> attach represented signals of rep1 to rep2.
		repState = this->represent[e2Index].state;
	} else {
		// rep1 < rep2 wrt. top. order -> swap rep1 and rep2, so rep2 can be used for operations.
		repr* tmp = rep1;
		rep1 = rep2;
		rep2 = tmp;
		repState = this->represent[e1Index].state;
	}
	// Determine leaf entry of representative where all other signals get attached.
	repr* leafEntry = rep2->leaf;
//	cout << "rep2 name is: " << rep2->sig->name << endl;
//	cout << "rep2 next is: " << rep2->next << endl;
//	cout << "rep2 leaf is: " << rep2->leaf->sig->name << endl;
	assert(leafEntry->next == NULL);
	repState = (equi == repState);  // Determine equi state between rep1 and rep2.
	// Iterate through all signals of rep1 and attach them to list of signals of rep2, use correct state.
	for (repr* currEntry = rep1; currEntry != NULL; currEntry = currEntry->next) {
		assert(currEntry != NULL);
		//cout << "next: " << leafEntry->next << " and entry: " << currEntry << endl;
		if (currEntry == rep1) {  // Special case. First entry of rep1 list.
			leafEntry->next = currEntry;
			currEntry->leaf = NULL;
		}
		currEntry->sig = rep2->sig;  // Change all entries to rep2.
		currEntry->state = (currEntry->state == repState);
		// Special case. Last element is set to new leaf element of rep2.
		if (currEntry->next == NULL) rep2->leaf = currEntry;
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::addRepresentativeSimple(varIndex e1Index, varIndex e2Index, bool equi, vp::Edge* reprEdge) {
	// Attach new edge to the end of the list of members of signals represented by reprEdge.
//	repr* leafEntry = this->represent[e2Index].leaf;
//	assert(leafEntry->next == NULL);
//	leafEntry->next = &this->represent[e1Index];
//	this->represent[e2Index].leaf = leafEntry->next;
//	// Change entries of representative for e1.
//	this->represent[e1Index].sig = reprEdge;
//	this->represent[e1Index].state = equi;
	if (this->represent[e1Index].sig->eIndex == e1Index) {  // Normal case: First representative found is applied.
		this->represent[e1Index].sig = reprEdge;
		this->represent[e1Index].state = equi;
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::initRepresentatives(){
	represent = new repr[this->circuit.maxEdgeIndex+1]();
	int currIndex;
	for (auto& elem: this->circuit.edges) {
		currIndex = elem.second.eIndex;
		represent[currIndex].initRepresentEntry(&elem.second);
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::addTseitinClauses(Minisat::Solver& solver, Minisat::Var in1, Minisat::Var in2, Minisat::Var out, vp::Node::NodeType gateType) {
	if (gateType == vp::Node::AND) {  // Tseitin of AND.
		solver.addClause( ~Minisat::mkLit(in1),  ~Minisat::mkLit(in2),  Minisat::mkLit(out));
    	solver.addClause(Minisat::mkLit(in1), ~Minisat::mkLit(out));
    	solver.addClause(Minisat::mkLit(in2), ~Minisat::mkLit(out));
	} else if (gateType == vp::Node::OR) {  // Tseitin of OR.
		solver.addClause(Minisat::mkLit(in1), Minisat::mkLit(in2), ~Minisat::mkLit(out));
    	solver.addClause(~Minisat::mkLit(in1), Minisat::mkLit(out));
    	solver.addClause(~Minisat::mkLit(in2), Minisat::mkLit(out));
	} else if (gateType == vp::Node::XOR) {  // Tseitin of XOR.
		solver.addClause( ~Minisat::mkLit(in1), ~Minisat::mkLit(in2), ~Minisat::mkLit(out));
    	solver.addClause( ~Minisat::mkLit(in1), Minisat::mkLit(in2), Minisat::mkLit(out));
    	solver.addClause( Minisat::mkLit(in1), ~Minisat::mkLit(in2), Minisat::mkLit(out));
    	solver.addClause( Minisat::mkLit(in1), Minisat::mkLit(in2), ~Minisat::mkLit(out));
	} else if (gateType == vp::Node::XNOR) {  // Tseitin of XNOR.
		solver.addClause( ~Minisat::mkLit(in1), ~Minisat::mkLit(in2), Minisat::mkLit(out));
	    solver.addClause( Minisat::mkLit(in1), Minisat::mkLit(in2), Minisat::mkLit(out));
	    solver.addClause( Minisat::mkLit(in1), ~Minisat::mkLit(in2), ~Minisat::mkLit(out));
	    solver.addClause( ~Minisat::mkLit(in1), Minisat::mkLit(in2), ~Minisat::mkLit(out));
	} else if (gateType == vp::Node::NOT) {  // Tseitin of NOT.
    	solver.addClause( ~Minisat::mkLit(in1), ~Minisat::mkLit(out));
    	solver.addClause( Minisat::mkLit(in1), Minisat::mkLit(out));
	} else if (gateType == vp::Node::BUFFER) {  // Tseitin of buffer aka equivalence.
		solver.addClause( ~Minisat::mkLit(in1), Minisat::mkLit(out));
    	solver.addClause( Minisat::mkLit(in1), ~Minisat::mkLit(out));
    }
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::addTseitinClausesActivated(Minisat::Solver& solver, Minisat::Var in1, Minisat::Var in2, Minisat::Var out, vp::Node::NodeType gateType, Minisat::Var activation) {
	Minisat::vec<Minisat::Lit> lits;
	if (gateType == vp::Node::AND) {  // Tseitin of AND.
		lits.push(~Minisat::mkLit(in1));  // First clause.
		lits.push(~Minisat::mkLit(in2));
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in1));  // Second clause.
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in2));  // Third clause.
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
	} else if (gateType == vp::Node::OR) {  // Tseitin of OR.
		lits.push(Minisat::mkLit(in1));  // First clause.
		lits.push(Minisat::mkLit(in2));
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(~Minisat::mkLit(in1));  // Second clause.
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(~Minisat::mkLit(in2));  // Third clause.
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
	} else if (gateType == vp::Node::XOR) {  // Tseitin of XOR.
		lits.push(~Minisat::mkLit(in1));  // First clause.
		lits.push(~Minisat::mkLit(in2));
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(~Minisat::mkLit(in1));  // Second clause.
		lits.push(Minisat::mkLit(in2));
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in1));
		lits.push(~Minisat::mkLit(in2));  // Third clause.
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in1));
		lits.push(Minisat::mkLit(in2));  // Fourth clause.
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
	} else if (gateType == vp::Node::NOT) {  // Tseitin of NOT.
		lits.push(~Minisat::mkLit(in1));  // First clause.
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in1));  // Second clause.
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
	} else if (gateType == vp::Node::BUFFER) {  // Tseitin of buffer aka equivalence.
		lits.push(~Minisat::mkLit(in1));  // First clause.
		lits.push(Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
		lits.clear();
		lits.push(Minisat::mkLit(in1));  // Second clause.
		lits.push(~Minisat::mkLit(out));
		lits.push(~Minisat::mkLit(activation));
		solver.addClause_(lits);
    }
}

//_____________________________________________________________________________________________________________________________________
void Verifizierer::nodesInWindowRangeWithRepr(vp::Node* startNode, int range, set<int>& allNodesInWindow) {
	int currNodeIndex = 0;
	vp::Node* nextNode = NULL;
	if (range < 0) return;
	if (range > 1) {
		for (size_t i = 0; i < startNode->getInputsCount(); i++) {
			nextNode = &(startNode->inputNode(i));
			if (this->represent[nextNode->outputs.at(0)->eIndex].sig != NULL) {  // Has nextNode a representative -> use it instead of nextNode for the window creation.
				nextNode = &this->circuit.nodes.at(this->represent[nextNode->outputs.at(0)->eIndex].sig->source);
			}
			nodesInWindowRangeWithRepr(nextNode, range - 1, allNodesInWindow);  // Recursively collect all nodes with window smaller by one.
			//allNodesInWindow.insert(currNodeIndex);
		}
	} else if (range == 1) {
		for (size_t i = 0; i < startNode->getInputsCount(); i++) {
			currNodeIndex = startNode->inputNode(i).nIndex;
			nextNode = &(startNode->inputNode(i));
			if (this->represent[nextNode->outputs.at(0)->eIndex].sig != NULL) {  // Has nextNode a representative -> use it instead of nextNode for the window creation.
				currNodeIndex = this->represent[nextNode->outputs.at(0)->eIndex].sig->eIndex;
			}
			allNodesInWindow.insert(currNodeIndex);
		}
	}
	allNodesInWindow.insert(startNode->nIndex);  // At the end always add the starting node.
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::checkAllEquiAnti() {
	cout << "Check equivalences/antivalences has started." << endl;
	double time1 = 0.0, tstart;
	tstart = clock(); // Zeitmessung beginnt.
	
	std::set<satPair> comPairs;
	vp::Edge* currEdge;
	vp::Edge* compareEdge;
	size_t classSize;
	int lev1, lev2, result;
	bool inserted;
	// Check all simClasses and insert equivalences.
	for (size_t i = 0; i < simTable.validEntries()->size(); i++ ) {
		//cout << "Sims Schritt: " << i << " mit Entry: " << simTable.validEntries()->at(i) << endl;
		currEdge = simTable._table[simTable.validEntries()->at(i)];  
		if (currEdge == 0) {
			cout << "SimClass entry 0 encountered." << endl;
			continue;	
		}
		classSize = simTable.simClassSize(currEdge);
//		cout << "Class names are: " << currEdge->name << " with size: " << classSize << endl;
		if (classSize == 1) {
			if (currEdge->_nEqualSimHash == currEdge) continue;
		}
		for (size_t j = 0; j < classSize; currEdge = currEdge->_nEqualSim) {
			//cout << "CurrEdge is: " << currEdge->name << endl;
			compareEdge = currEdge->_nEqualSim;
			j++;
			for (size_t k = 0; k < classSize - j; compareEdge = compareEdge->_nEqualSim) {
				//cout << "	Compare class is: " << compareEdge->name << endl;
				assert (currEdge->name != compareEdge->name);
				lev1 = circuit.node(currEdge->getSource()).revLevel;
				lev2 = circuit.node(compareEdge->getSource()).revLevel;
				if (circuit.node(currEdge->getSource()).type == vp::Node::INPUT_PORT) {
					lev1 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
				} else if (circuit.node(compareEdge->getSource()).type == vp::Node::INPUT_PORT) {
					lev2 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
				}
				//result = lev1 < lev2 ? lev2 : lev1;
//				cout << "equi e1: " << currEdge->name << ", e2: " << compareEdge->name << endl;
				if (lev1 < lev2) {
					comPairs.insert(satPair(lev1, lev2, currEdge->name, compareEdge->name, true));
				} else {
					comPairs.insert(satPair(lev2, lev1, compareEdge->name, currEdge->name, true));
				}
				k++;
			}
		}
		//cout << "Sim hashes. " << endl;
		// Check for equal sim hashes.
		if (currEdge->_nEqualSimHash == currEdge) continue;
		currEdge = currEdge->_nEqualSimHash;
		vp::Edge* firstEdge = currEdge;
		classSize = simTable.simClassSize(currEdge);
		//cout << "Sim hash class: " << currEdge->name << " has size: " << classSize << endl;
		for (size_t j = 0; j >= 0; currEdge = currEdge->_nEqualSimHash) {
		  	if (currEdge == firstEdge && j != 0) break;
		  	classSize = simTable.simClassSize(currEdge);
//			cout << "Sim hash2 class: " << currEdge->name << " has size: " << classSize << endl;
			j++;
			if (classSize == 1) continue;
			for (size_t h = 0; h < classSize; currEdge = currEdge->_nEqualSim) {
				//cout << "CurrEdge is: " << currEdge->name << endl;
				compareEdge = currEdge->_nEqualSim;
				h++;
				for (size_t k = 0; k < classSize - h; compareEdge = compareEdge->_nEqualSim) {
					//cout << "	Compare class is: " << compareEdge->name << endl;
					lev1 = circuit.node(currEdge->getSource()).revLevel;
					lev2 = circuit.node(compareEdge->getSource()).revLevel;
					if (circuit.node(currEdge->getSource()).type == vp::Node::INPUT_PORT) {
						lev1 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
					} else if (circuit.node(compareEdge->getSource()).type == vp::Node::INPUT_PORT) {
						lev2 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
					}
					//result = lev1 < lev2 ? lev2 : lev1;
//					cout << "equi e1: " << currEdge->name << ", e2: " << compareEdge->name << endl;
					if (lev1 < lev2) {
						comPairs.insert(satPair(lev1, lev2, currEdge->name, compareEdge->name, true));
					} else {
						comPairs.insert(satPair(lev2, lev1, compareEdge->name, currEdge->name, true));
					}
					k++;
				}
			}
		}
	}
	//cout << "Equivalences checked." << endl;
	///* Check for antivalences now.
	vp::Edge* invertedEdge;
	int invertedSize =  0;
	bool noInverted = false;
	bool isNot;
	// Check all inverse simClasses and add to list if they are not just an NOT gate between two signals.
	for (size_t i = 0; i < simTable.validEntries()->size(); i++ ) {
		noInverted = false;
		currEdge = simTable._table[simTable.validEntries()->at(i)];
		if (currEdge == 0) {
			cout << "In Inverted: SimClass entry 0 encountered." << endl;
			continue;
		}
//		cout << "currEdge: " << currEdge->name << endl;
		invertedEdge = simTable.getInvertedClass(currEdge);
		if (invertedEdge == 0) noInverted = true;  // No inverted simClass for this simClass.
		if (!noInverted) {
		classSize = simTable.simClassSize(currEdge);
		invertedSize =  simTable.simClassSize(invertedEdge);
		//cout << "Invert Class names are: " << invertedEdge->name << " with size: " << invertedSize << endl;
		for (size_t j = 0; j < invertedSize; invertedEdge = invertedEdge->_nEqualSim) {
			//cout << "Inverted Edge is: " << invertedEdge->name << endl;
			compareEdge = currEdge;
			j++;
			for (size_t k = 0; k < classSize; compareEdge = compareEdge->_nEqualSim) {
				//cout << "	Compare class is: " << compareEdge->name << endl;
				// Check if both signals connected via a NOT node. In this case do not consider antivalenz.
				/*
				isNot = false;
				for (auto& elem: invertedEdge->destinations) {
					if (compareEdge->getSource() == elem && circuit.nodes.at(elem).type == vp::Node::NOT) isNot = true;
				}
				for (auto& elem: compareEdge->destinations) {
					if (invertedEdge->getSource() == elem && circuit.nodes.at(elem).type == vp::Node::NOT) isNot = true;
				}
				//*/
				/*
				if (isNot) {
					//cout << "NOT detected. Skip." << endl;
					k++;
					continue;
				}//*/
				// Inverted Sim pair is not a NOT gate. Add it to the list.
				lev1 = circuit.node(invertedEdge->getSource()).revLevel;
				lev2 = circuit.node(compareEdge->getSource()).revLevel;
				if (circuit.node(invertedEdge->getSource()).type == vp::Node::INPUT_PORT) {
					lev1 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
				} else if (circuit.node(compareEdge->getSource()).type == vp::Node::INPUT_PORT) {
					lev2 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
				}
				//result = lev1 < lev2 ? lev2 : lev1;
//				cout << "anti e1: " << invertedEdge->name << ", e2: " << compareEdge->name << endl;
				if (lev1 < lev2) {
					comPairs.insert(satPair(lev1, lev2, invertedEdge->name, compareEdge->name, false));
				} else {
					comPairs.insert(satPair(lev2, lev1, compareEdge->name, invertedEdge->name, false));
				}
				k++;
				
			}
		}
		}
		
		// Check for equal sim hashes.
		if (currEdge->_nEqualSimHash == currEdge) continue;
		currEdge = currEdge->_nEqualSimHash;
		vp::Edge* firstEdge = currEdge;
		classSize = simTable.simClassSize(currEdge);
		//cout << "Sim hash class: " << currEdge->name << " has size: " << classSize << endl;
		for (size_t j = 0; j >= 0; currEdge = currEdge->_nEqualSimHash) {
		  	if (currEdge == firstEdge && j != 0) break;
		  	classSize = simTable.simClassSize(currEdge);
//			cout << "anti Sim hash2 class: " << currEdge->name << " has size: " << classSize << endl;
			j++;
			invertedEdge = simTable.getInvertedClass(currEdge);
			if (invertedEdge == 0) continue;  // No inverted simClass for this simClass.   
			invertedSize =  simTable.simClassSize(invertedEdge);
			//cout << "Invert Class names are: " << invertedEdge->name << " with size: " << invertedSize << endl;
			for (size_t h = 0; h < invertedSize; invertedEdge = invertedEdge->_nEqualSim) {
				//cout << "Inverted Edge is: " << invertedEdge->name << endl;
				compareEdge = currEdge;
				h++;
				for (size_t k = 0; k < classSize; compareEdge = compareEdge->_nEqualSim) {
					//cout << "	Compare class is: " << compareEdge->name << endl;
					// Check if both signals connected via a NOT node. In this case do not consider antivalenz.
					/*
					isNot = false;
					for (auto& elem: invertedEdge->destinations) {
						if (compareEdge->getSource() == elem && circuit.nodes.at(elem).type == vp::Node::NOT) isNot = true;
					}
					for (auto& elem: compareEdge->destinations) {
						if (invertedEdge->getSource() == elem && circuit.nodes.at(elem).type == vp::Node::NOT) isNot = true;
					}
					//*/
					/*
					if (isNot) {
						//cout << "NOT detected. Skip." << endl;
						k++;
						continue;
					}//*/
					// Inverted Sim pair is not a NOT gate. Add it to the list.
					lev1 = circuit.node(invertedEdge->getSource()).revLevel;
					lev2 = circuit.node(compareEdge->getSource()).revLevel;
					if (circuit.node(invertedEdge->getSource()).type == vp::Node::INPUT_PORT) {
						lev1 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
					} else if (circuit.node(compareEdge->getSource()).type == vp::Node::INPUT_PORT) {
						lev2 = this->circuit.node(this->circuit.sortedNodes.back()).revLevel + 1;
					}
					//result = lev1 < lev2 ? lev2 : lev1;
//					cout << "anti e1: " << invertedEdge->name << ", e2: " << compareEdge->name << endl;
					if (lev1 < lev2) {
						comPairs.insert(satPair(lev1, lev2, invertedEdge->name, compareEdge->name, false));
					} else {
						comPairs.insert(satPair(lev2, lev1, compareEdge->name, invertedEdge->name, false));
					}
					k++;
				}
			}
		}
	}//*/
	
	int countEquis = 0;
	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "Finding candidates for equivalences needed time in sec. : " << time1 << endl;
	cout << "Compairs size is : " << comPairs.size() << endl;
	
	time1 = 0.0;
	tstart = clock(); // Zeitmessung beginnt.
	for (std::set<satPair>::reverse_iterator it = comPairs.rbegin(); it != comPairs.rend(); ++it) {
		//cout << "Edges are: " << it->e1 << " and " << it->e2 << " with bool: " << it->equi << " and lev1: " << it->lev1 << " and lev2: " << it->lev2 << endl; // " and foundPairsSize: " << this->foundPairs.size() << endl;
		if (testSAT(it->e1, it->e2, it->equi)) {
			countEquis++;
			//cout << it->equi << " found for " << it->e1 << "|" << it->e2 << endl;
		}
	}
	
	time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "SAT checking (and applying) possible equivalences needed time in sec. : " << time1 << endl;
	cout << "Equivalences/Antivalences found and applied: " << countEquis << endl;
}

// _______________________________________________________________________________________________________________________________________
std::pair<std::pair<std::vector<varIndex>, std::vector<varIndex>>, std::vector<varIndex>> Verifizierer::readVarsForBDD() {
	// Count how big the vectors have to be.
	int countRemainder = 0;
	for (int i = 0; i < this->circuit.getOutputNodesCount(); i++) {  // Count remainder bits.
		 if (this->circuit.outputNode(i).type == vp::Node::OUTPUT_PORT) {
		 	if ((this->circuit.outputNode(i).name).find("R_n1") != string::npos) countRemainder++;
		 }
	}
	int countDivisor = 0;
	for (int i = 0; i < this->circuit.getInputNodesCount(); i++) {  // Count divisor bits.
		 if (this->circuit.inputNode(i).type == vp::Node::INPUT_PORT) {
		 	if ((this->circuit.inputNode(i).name).find("D") != string::npos) countDivisor++;
		 }
		 
	}
	int countDividend = 0;
	for (int i = 0; i < this->circuit.getInputNodesCount(); i++) {  // Count dividend bits.
		 if (this->circuit.inputNode(i).type == vp::Node::INPUT_PORT) {
		 	if ((this->circuit.inputNode(i).name).find("R_0") != string::npos) countDividend++;
		 }
		 
	}
	//cout << "Remainder: " << countRemainder << ", Divisor: " << countDivisor << endl;
	std::vector<varIndex> remainder (countRemainder);
	std::vector<varIndex> divisor (countDivisor);
	std::vector<varIndex> dividend (countDividend);
	
	// Make lists of remainder and divisor signal indices. Remainder has to be named "R_n1", divisor has to be "D"and dividend "R_0".
	for (int i = 0; i < this->circuit.getOutputNodesCount(); i++) {  // Add all remainder signals to the list.
		 if (this->circuit.outputNode(i).type == vp::Node::OUTPUT_PORT) {
		 	string name = this->circuit.outputNode(i).name;
		 	if (name.find("R_n1") != string::npos) {
		 		string bitNumber = name.substr(name.find("[") + 1, name.find("]") - name.find("[") - 1);
		 		remainder[std::stoi(bitNumber)] = this->circuit.outputNode(i).inputs.at(0)->eIndex;
		 	}
		 } else {
		 	//cout << "Something went wrong. This node is not an output node." << endl;
		 } 
	}
	for (int i = 0; i < this->circuit.getInputNodesCount(); i++) {  // Add all divisor signals in the list.
		 if (this->circuit.inputNode(i).type == vp::Node::INPUT_PORT) {
		 	string name = this->circuit.inputNode(i).name;
		 	if (name.find("D") != string::npos) {
		 		string bitNumber = name.substr(name.find("[") + 1, name.find("]") - name.find("[") - 1);
		 		divisor[std::stoi(bitNumber)] = this->circuit.inputNode(i).outputs.at(0)->eIndex;
		 	}
		 } else {
		 	//cout << "Something went wrong. This node is not an input node." << endl;
		 } 
	}
	for (int i = 0; i < this->circuit.getInputNodesCount(); i++) {  // Add all dividend signals in the list.
		 if (this->circuit.inputNode(i).type == vp::Node::INPUT_PORT) {
		 	string name = this->circuit.inputNode(i).name;
		 	if (name.find("R_0") != string::npos) {
		 		string bitNumber = name.substr(name.find("[") + 1, name.find("]") - name.find("[") - 1);
		 		dividend[std::stoi(bitNumber)] = this->circuit.inputNode(i).outputs.at(0)->eIndex;
		 	}
		 } else {
		 	//cout << "Something went wrong. This node is not an input node." << endl;
		 } 
	}
	/*
	cout << "Remainder:" << endl;
	for (auto& elem: remainder) {
		cout << elem << endl;
	}
	cout << "Divisor:" << endl;
	for (auto& elem: divisor) {
		cout << elem << endl;
	}
	cout << "Dividend:" << endl;
	for (auto& elem: dividend) {
		cout << elem << endl;
	}//*/
	
	std::pair<std::pair<std::vector<varIndex>, std::vector<varIndex>>, std::vector<varIndex>> result;
	result.first.first = remainder;
	result.first.second = divisor;
	result.second = dividend;
	return result;
	
}

// _______________________________________________________________________________________________________________________________________
bool Verifizierer::applyICOnPolynomial(Polynom& polynomial) {
	cout << "Applying Input constraint on polynomial started." << endl;
	if (polynomial.size() == 0) return true;
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divisor = this->pMap.at("D");
	int Dpos = divisor.size() - 1;
	int R0pos = dividend.size() - 1;
	varIndex DVar;
	varIndex R0Var;
	Polynom testPol;
	for (size_t i=0; i < divisor.size(); i++) {
		cout << "step: " << i << endl;
		DVar = this->circuit.node(divisor.at(Dpos-i)).outputs.at(0)->eIndex;
		R0Var = this->circuit.node(dividend.at(R0pos-i)).outputs.at(0)->eIndex;
		cout << "DVAR: " << DVar << " R0VAR: " << R0Var << endl;
		testPol = polynomial;
		testPol.replaceCON0(R0Var);
		testPol.replaceCON1(DVar);
		//if ( testPol.size() < 50) cout << "testPol: " << testPol << endl;
		//else cout << "testPol size: " << testPol.size() << endl;
		//cout<<"test pol"<<testPol<<endl;
		if (testPol.size() != 0)  return false;
		if (i == divisor.size() - 1) continue; // Last bit comparison was made.
		polynomial.replaceBUFFER(R0Var, DVar);
		if ( polynomial.size() < 50) cout << "polynomial: " << polynomial << endl;
		if (polynomial.size() == 0) return true;
		else cout << "polynomial size: " << polynomial.size() << endl;
	}
//	if (polynomial.size() != 0) return false;
//	else return true;
	return true;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::buildInputConstraintBDD(DdManager* &gbm, DdNode* &bdd_ic, vector<int>& dividend, vector<int>& divisor, DdNode** vars) {
	DdNode *tmp_neg_c, *tmp2_c, *tmp_c;
	bdd_ic = Cudd_ReadOne(gbm); /*Returns the logic one constant of the manager*/
	Cudd_Ref(bdd_ic); /*Increases the reference count of a node*/
		
	int n = this->pMap.at("Q").size();	
		
	// Create BDD "bdd_c" for contraint 0 <= R^(0) < D * 2^(n-1).     	
    for (size_t i = 0; i < divisor.size(); i++) {
    	if (i == 0) {  // First constraint L_0 has no XNOR and no conjunction with previous constraint. 
    		//cout << "1: " << this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex << endl;
    		tmp_neg_c = Cudd_Not(vars[this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex]);  // NOT of r0_n-1.
    		//tmp = vars[i+2];  // Identity of d_0.
    		tmp2_c = Cudd_bddAnd(gbm, tmp_neg_c, bdd_ic); // NOT r0_n-1.
    		Cudd_Ref(tmp2_c); 
    		//cout << "2: " << this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex << endl;
    		tmp_c = Cudd_bddAnd(gbm, vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex], tmp2_c);  // AND of !r0_n-1 and d_0.
    		Cudd_Ref(tmp_c);
    		Cudd_RecursiveDeref(gbm,bdd_ic);
    		Cudd_RecursiveDeref(gbm,tmp2_c);
    		//Cudd_RecursiveDeref(gbm,tmp_neg);
    		bdd_ic = tmp_c;	// Copy in: AND of !r0_n-1 and d_0.
    	} else {  // Next constraints L_i. 
    		//*
    		//cout << "3: " << this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex << endl;
    		tmp_neg_c = Cudd_Not(vars[this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex]);  // NOT r0_(n-1+i).
    		//tmp = vars[2*i+1];  // Identity of d_i.
    		//cout << "4: " << this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex << endl;
    		tmp_c = Cudd_bddAnd(gbm, tmp_neg_c, vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]); // !r0_(n-1+i) AND d_i.
    		Cudd_Ref(tmp_c);
    		//Cudd_RecursiveDeref(gbm,tmp_neg);
    		tmp2_c = Cudd_bddXnor(gbm, vars[this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex], vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]);  // Equality: r0_(n-1+i) = d_i.
    		Cudd_Ref(tmp2_c);
    		tmp_neg_c = Cudd_bddAnd(gbm, tmp2_c, bdd_ic);  // (r0_(n-1+i)=d_i) AND L_(i-1).
    		Cudd_Ref(tmp_neg_c);
    		Cudd_RecursiveDeref(gbm,bdd_ic);
    		Cudd_RecursiveDeref(gbm,tmp2_c);
    		bdd_ic = tmp_neg_c;
    		tmp_neg_c = Cudd_bddOr(gbm, tmp_c, bdd_ic);  // [!r0_(n-1+i) AND d_i] OR [(r0_(n-1+i)=d_i) AND L_(i-1)].
    		Cudd_Ref(tmp_neg_c);
    		Cudd_RecursiveDeref(gbm, bdd_ic);
    		Cudd_RecursiveDeref(gbm, tmp_c);
    		bdd_ic = tmp_neg_c;
    		//*/
    	}
    }
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::buildRemainderConstraintBDD(DdManager* &gbm, DdNode* &bdd_rc, vector<int>& remainder, vector<int>& divisor, DdNode** vars) {
	DdNode *tmp_neg_c, *tmp2_c, *tmp_c;
	bdd_rc = Cudd_ReadOne(gbm); /*Returns the logic one constant of the manager*/
	Cudd_Ref(bdd_rc); /*Increases the reference count of a node*/			
	
	// Create starting BDD for the contraint R^(n+1) < D.     	
    for (int i = 0; i < remainder.size(); i++) {
    	//cout << "i: " << i << endl;
    	if (i < divisor.size()) {  // Steps with r_i and d_i.
    		if (i == 0) {  // First constraint L_0 has no XNOR and no conjunction with previous constraint.
    			tmp_neg_c = Cudd_Not(vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex]);  // NOT of r_0.
    			tmp2_c = Cudd_bddAnd(gbm, tmp_neg_c, bdd_rc); // NOT r_0.
    			Cudd_Ref(tmp2_c); 
    			tmp_c = Cudd_bddAnd(gbm, tmp2_c, vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]);  // AND of !r_0 and d_0.
    			Cudd_Ref(tmp_c);
    			Cudd_RecursiveDeref(gbm,bdd_rc);
    			Cudd_RecursiveDeref(gbm,tmp2_c);
    			bdd_rc = tmp_c;
    		} else {  // Next constraints L_i. 
    			tmp_neg_c = Cudd_Not(vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex]);  // NOT of r_i.
    			tmp_c = Cudd_bddAnd(gbm, tmp_neg_c, vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]); // AND of !r_i and d_i.
    			Cudd_Ref(tmp_c);
    			tmp2_c = Cudd_bddXnor(gbm, vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex], vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]);  // Equality: r_i = d_i.
    			Cudd_Ref(tmp2_c);
    			tmp_neg_c = Cudd_bddAnd(gbm, tmp2_c, bdd_rc);  // AND of (r_i=d_i) and L_(i-1).
    			Cudd_Ref(tmp_neg_c);
    			Cudd_RecursiveDeref(gbm,bdd_rc);
    			Cudd_RecursiveDeref(gbm,tmp2_c);
    			bdd_rc = tmp_neg_c;
    			tmp_neg_c = Cudd_bddOr(gbm, tmp_c, bdd_rc);  // OR of (r_i=d_i) and L_(i-1) and tmp.
    			Cudd_Ref(tmp_neg_c);
    			Cudd_RecursiveDeref(gbm, bdd_rc);
    			Cudd_RecursiveDeref(gbm, tmp_c);
    			bdd_rc = tmp_neg_c;
    		}
		} else {  // Steps with only r_i and no d_i anymore.
			tmp_neg_c = Cudd_Not(vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex]);  // NOT of r_i.
			tmp_c = Cudd_bddAnd(gbm, tmp_neg_c, bdd_rc); // AND of !r_i and L_(i-1).
			Cudd_Ref(tmp_c);
    		Cudd_RecursiveDeref(gbm,bdd_rc);
    		bdd_rc = tmp_c;
		} 
    }
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::calcHelpersForImageCompGeneral(set<gendc>& candidates, vector<vector<string>>& borders, vector<vector<int>>& dcNums, vector<int>& dcPos, vector<vector<int>>& orderedNodes, vector<set<int>>& cutInputs, vector<set<int>>& cutOutputs, vector<set<int>>& signalsToQuantify) {
	int pos = 0;
	vector<int> dcNum;
	for (auto& elem: candidates) {
		dcNum.clear();
    	//if ((elem.poss[0]) || (elem.poss[7])) { pos++; continue; }
    	for (size_t i=0; i < elem.possSize; i++) {
    		if (!elem.poss[i]) dcNum.push_back(i);
    	}
    	if (dcNum.size() > 0) {
    		borders.push_back(elem.signals);
    		dcPos.push_back(pos);
    		dcNums.push_back(dcNum);
    	}
    	pos++;
    }
    vp::Node* currentNode;
    // Add last border which is last node in the circuit (used for computing R < D at the end.)
    for (std::vector<int>::iterator it = this->circuit.sortedNodes.begin(); it != this->circuit.sortedNodes.end(); ++it) {
    	currentNode = &this->circuit.node(*it);
    	if (currentNode->type == vp::Node::INPUT_PORT || currentNode->type == vp::Node::OUTPUT_PORT || currentNode->type == vp::Node::DELETED) continue;
    	else { borders.push_back({currentNode->outputs.at(0)->name}); break; }
    }
    
    vector<set<int>> partialNodes;
    set<int> prevOutputNodes;
    vector<int> minPartLevel(borders.size(), this->circuit.node(this->circuit.sortedNodes.back()).revLevel);
    int currLevel = 0;
     
    std::vector<int>::reverse_iterator saveRit = this->circuit.sortedNodes.rbegin();  // Iterator for saving prev. position in nodes.
    for (size_t i=0; i < borders.size(); i++) {
    	//cout << " ||||||||||||||||||||||||| image number " << i << endl;
    	for (auto& elem: borders.at(i)) {  // Calculate minPartLevel for every image border.
			if (this->circuit.node(this->circuit.edges.at(elem).source).type == vp::Node::INPUT_PORT) continue;
			currLevel = this->circuit.node(this->circuit.edges.at(elem).source).revLevel;
   			if (currLevel < minPartLevel.at(i)) minPartLevel.at(i) = currLevel;
   		}
   		partialNodes.push_back(set<int>{});
   		orderedNodes.push_back(vector<int>{}); 
   		// Collect all nodes of this cut in partialNodes set and orderedNodes sub vector.
    	for (std::vector<int>::reverse_iterator rit = saveRit; rit != circuit.sortedNodes.rend(); ++rit) {
			currentNode = &this->circuit.node(*rit);
			if (currentNode->type != vp::Node::DELETED && currentNode->type != vp::Node::INPUT_PORT && currentNode->type != vp::Node::OUTPUT_PORT) {
				if (currentNode->revLevel >= minPartLevel.at(i)) {
					partialNodes.at(i).insert(currentNode->nIndex);
					orderedNodes.at(i).push_back(currentNode->nIndex);
				} else {
					saveRit = rit;
					break;
				}
			} 
		}
		// Iterate through all nodes in partialNodes and save inputs and output signals of this cut.
		set<int> cutFront;
		set<int> tempCutInputs;
		set<int> tempPrevNodes;
		vp::Node* currNode;
		for (auto& elem: partialNodes.at(i)) {
			currNode = &this->circuit.node(elem);
			for (size_t j=0; j < currNode->getOutputsCount(); j++) {  // Collecting outputs of the transition function.
				if (partialNodes.at(i).count(currNode->outputNode(j).nIndex) == 0) {  // currNode is output node of the set.
					cutFront.insert(currNode->outputs.at(0)->eIndex);
					tempPrevNodes.insert(currNode->nIndex);
				}
			} 
			// Do same for collecting input nodes of the cut.
			for (size_t j=0; j < currNode->getInputsCount(); j++) {  // Collecting inputs of the transition function.
				if (partialNodes.at(i).count(currNode->inputNode(j).nIndex) == 0) {  // currNode is input node of the set.
					tempCutInputs.insert(currNode->inputs.at(j)->eIndex);
				}
			} 
		}
		// Additionally check for all output signals of previous cut whether they are also a output signal of this cut.
		for (auto& elem: prevOutputNodes) {
			currNode = &this->circuit.node(elem);
			for (size_t j=0; j < currNode->getOutputsCount(); j++) {  // Collecting outputs of the transition function.
				if (partialNodes.at(i).count(currNode->outputNode(j).nIndex) == 0 && this->circuit.node(currNode->outputNode(j).nIndex).revLevel < minPartLevel.at(i) && this->circuit.node(currNode->outputNode(j).nIndex).type != vp::Node::OUTPUT_PORT) {  
					// currNode is output node of the set.
					cutFront.insert(currNode->outputs.at(0)->eIndex);
					tempCutInputs.insert(currNode->outputs.at(0)->eIndex);
					tempPrevNodes.insert(currNode->nIndex);
				}
			} 
		}
		cutOutputs.push_back(cutFront);
		prevOutputNodes = tempPrevNodes;
		cutInputs.push_back(tempCutInputs);
    }    
    // Determine which signals in every stage should be quantified out. 
    // RULE 1: If input of this stage is also input of any of next stages, don't quantify it out.
    // RULE 2: If input of this stage is also output of this stage, don't quantify it out.
    // RULE 3: Do not quantify out divisor signals since in last step the constraint R < D is checked.
    vector<set<int>> quantifyTemp;
	set<int> tempSet;
	set<int> tempSetPrev;
	set<int> divisorSignals;
	for (vector<int>::iterator it = this->pMap.at("D").begin(); it != this->pMap.at("D").end(); ++it) {
		divisorSignals.insert(this->circuit.node(*it).outputs.at(0)->eIndex);
	}
	for (size_t i=0; i < cutInputs.size() - 1; i++) {  // Apply RULE 1.
		for (size_t j=i+1; j < cutInputs.size(); j++) {
			if (j==i+1) tempSet = cutInputs.at(i);
			std::set_difference(tempSet.begin(), tempSet.end(), cutInputs.at(j).begin(), cutInputs.at(j).end(), 							std::inserter(tempSetPrev, tempSetPrev.begin()));
			tempSet = tempSetPrev;
			tempSetPrev.clear();
		}
		quantifyTemp.push_back(tempSet);
	}
	quantifyTemp.push_back(cutInputs.at(cutInputs.size() - 1));
	for (size_t i=0; i < quantifyTemp.size(); i++) {  // Apply RULE 2 and 3.
		tempSet.clear();
		tempSetPrev.clear();
		std::set_difference(quantifyTemp.at(i).begin(), quantifyTemp.at(i).end(), cutOutputs.at(i).begin(), cutOutputs.at(i).end(), std::inserter(tempSetPrev, tempSetPrev.begin()));
		std::set_difference(tempSetPrev.begin(), tempSetPrev.end(), divisorSignals.begin(), divisorSignals.end(), std::inserter(tempSet, tempSet.begin()));
		signalsToQuantify.push_back(tempSet);
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::calcHelpersForImageComp(set<dc>& candidates, vector<vector<string>>& borders, vector<vector<int>>& dcNums, vector<int>& dcPos, vector<vector<int>>& orderedNodes, vector<set<int>>& cutInputs, vector<set<int>>& cutOutputs, vector<set<int>>& signalsToQuantify) {
	int pos = 0;
	vector<int> dcNum;
	for (auto& elem: candidates) {
		dcNum.clear();
    	//if ((elem.poss[0]) || (elem.poss[7])) { pos++; continue; }
    	for (size_t i=0; i < 8; i++) {
    		if (!elem.poss[i]) dcNum.push_back(i);
    	}
    	if (dcNum.size() > 0) {
    		borders.push_back({elem.sig1,elem.sig2,elem.sig3});
    		dcPos.push_back(pos);
    		dcNums.push_back(dcNum);
    	}
    	pos++;
    }
    vp::Node* currentNode;
    // Add last border which is last node in the circuit (used for computing R < D at the end.)
    for (std::vector<int>::iterator it = this->circuit.sortedNodes.begin(); it != this->circuit.sortedNodes.end(); ++it) {
    	currentNode = &this->circuit.node(*it);
    	if (currentNode->type == vp::Node::INPUT_PORT || currentNode->type == vp::Node::OUTPUT_PORT || currentNode->type == vp::Node::DELETED) continue;
    	else { borders.push_back({currentNode->outputs.at(0)->name}); break; }
    }

    vector<set<int>> partialNodes;
    set<int> prevOutputNodes;
    vector<int> minPartLevel(borders.size(), this->circuit.node(this->circuit.sortedNodes.back()).revLevel);
    int currLevel = 0;

    std::vector<int>::reverse_iterator saveRit = this->circuit.sortedNodes.rbegin();  // Iterator for saving prev. position in nodes.
    for (size_t i=0; i < borders.size(); i++) {
    	//cout << " ||||||||||||||||||||||||| image number " << i << endl;
    	for (auto& elem: borders.at(i)) {  // Calculate minPartLevel for every image border.
			if (this->circuit.node(this->circuit.edges.at(elem).source).type == vp::Node::INPUT_PORT) continue;
			currLevel = this->circuit.node(this->circuit.edges.at(elem).source).revLevel;
   			if (currLevel < minPartLevel.at(i)) minPartLevel.at(i) = currLevel;
   		}
   		partialNodes.push_back(set<int>{});
   		orderedNodes.push_back(vector<int>{});
   		// Collect all nodes of this cut in partialNodes set and orderedNodes sub vector.
    	for (std::vector<int>::reverse_iterator rit = saveRit; rit != circuit.sortedNodes.rend(); ++rit) {
			currentNode = &this->circuit.node(*rit);
			if (currentNode->type != vp::Node::DELETED && currentNode->type != vp::Node::INPUT_PORT && currentNode->type != vp::Node::OUTPUT_PORT) {
				if (currentNode->revLevel >= minPartLevel.at(i)) {
					partialNodes.at(i).insert(currentNode->nIndex);
					orderedNodes.at(i).push_back(currentNode->nIndex);
				} else {
					saveRit = rit;
					break;
				}
			}
		}
		// Iterate trough all nodes in partialNodes and save inputs and output signals of this cut.
		set<int> cutFront;
		set<int> tempCutInputs;
		set<int> tempPrevNodes;
		vp::Node* currNode;
		for (auto& elem: partialNodes.at(i)) {
			currNode = &this->circuit.node(elem);
			for (size_t j=0; j < currNode->getOutputsCount(); j++) {  // Collecting outputs of the transition function.
				if (partialNodes.at(i).count(currNode->outputNode(j).nIndex) == 0) {  // currNode is output node of the set.
					cutFront.insert(currNode->outputs.at(0)->eIndex);
					tempPrevNodes.insert(currNode->nIndex);
				}
			}
			// Do same for collecting input nodes of the cut.
			for (size_t j=0; j < currNode->getInputsCount(); j++) {  // Collecting inputs of the transition function.
				if (partialNodes.at(i).count(currNode->inputNode(j).nIndex) == 0) {  // currNode is input node of the set.
					tempCutInputs.insert(currNode->inputs.at(j)->eIndex);
				}
			}
		}
		// Additionally check for all output signals of previous cut whether they are also a output signal of this cut.
		for (auto& elem: prevOutputNodes) {
			currNode = &this->circuit.node(elem);
			for (size_t j=0; j < currNode->getOutputsCount(); j++) {  // Collecting outputs of the transition function.
				if (partialNodes.at(i).count(currNode->outputNode(j).nIndex) == 0 && this->circuit.node(currNode->outputNode(j).nIndex).revLevel < minPartLevel.at(i) && this->circuit.node(currNode->outputNode(j).nIndex).type != vp::Node::OUTPUT_PORT) {
					// currNode is output node of the set.
					cutFront.insert(currNode->outputs.at(0)->eIndex);
					tempCutInputs.insert(currNode->outputs.at(0)->eIndex);
					tempPrevNodes.insert(currNode->nIndex);
				}
			}
		}
		cutOutputs.push_back(cutFront);
		prevOutputNodes = tempPrevNodes;
		cutInputs.push_back(tempCutInputs);
    }
    // Determine which signals in every stage should be quantified out.
    // RULE 1: If input of this stage is also input of any of next stages, don't quantify it out.
    // RULE 2: If input of this stage is also output of this stage, don't quantify it out.
    // RULE 3: Do not quantify out divisor signals since in last step the constraint R < D is checked.
    vector<set<int>> quantifyTemp;
	set<int> tempSet;
	set<int> tempSetPrev;
	set<int> divisorSignals;
	for (vector<int>::iterator it = this->pMap.at("D").begin(); it != this->pMap.at("D").end(); ++it) {
		divisorSignals.insert(this->circuit.node(*it).outputs.at(0)->eIndex);
	}
	for (size_t i=0; i < cutInputs.size() - 1; i++) {  // Apply RULE 1.
		for (size_t j=i+1; j < cutInputs.size(); j++) {
			if (j==i+1) tempSet = cutInputs.at(i);
			std::set_difference(tempSet.begin(), tempSet.end(), cutInputs.at(j).begin(), cutInputs.at(j).end(), 							std::inserter(tempSetPrev, tempSetPrev.begin()));
			tempSet = tempSetPrev;
			tempSetPrev.clear();
		}
		quantifyTemp.push_back(tempSet);
	}
	quantifyTemp.push_back(cutInputs.at(cutInputs.size() - 1));
	for (size_t i=0; i < quantifyTemp.size(); i++) {  // Apply RULE 2 and 3.
		tempSet.clear();
		tempSetPrev.clear();
		std::set_difference(quantifyTemp.at(i).begin(), quantifyTemp.at(i).end(), cutOutputs.at(i).begin(), cutOutputs.at(i).end(), std::inserter(tempSetPrev, tempSetPrev.begin()));
		std::set_difference(tempSetPrev.begin(), tempSetPrev.end(), divisorSignals.begin(), divisorSignals.end(), std::inserter(tempSet, tempSet.begin()));
		signalsToQuantify.push_back(tempSet);
	}
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::cofactorDCInImageGeneral(DdManager*& gbm, vector<string>& dcSignals, int dcPos, int dcNum, set<gendc>& candidates, DdNode** vars, DdNode*& currImage) {
	DdNode* con0Node = Cudd_ReadLogicZero(gbm);
	int dcSize = dcSignals.size();  // Number of signals of current dc.
	int* dcSignalIndices = new int[dcSize];
	//int dcSignalIndices[3] = {0,0,0};
	//assert(dcSignals.size() == 3);
	for (size_t i=0; i < dcSize; i++) {
		//cout << "Edge is " << dcSignals.at(i) << endl;
		//cout << "with index " << this->circuit.edges.at(dcSignals.at(i)).eIndex << endl;
		dcSignalIndices[i] = this->circuit.edges.at(dcSignals.at(i)).eIndex;
	}
	DdNode** cofacVars = new DdNode*[dcSize];
	for (size_t i=0; i < dcSize; i++) {
		cofacVars[i] = vars[dcSignalIndices[i]];
	}
	int* cofacPhase0 = new int[dcSize];
	//int cofacPhase0[3] = {0,0,0};
	int tempNum = dcNum;
	int step = 0;
	while (tempNum > 0) {  // Check which cofacVars has to be 1 depending on dcNum.
		cofacPhase0[step] = (tempNum % 2 == 1) ? 1 : 0;
		tempNum = tempNum / 2;
		step++;
	}
	while (step < dcSize) {  // Fill leading 0 bits in cofacPhase0 array.
		cofacPhase0[step] = 0;
		step++;
	}
	/*
	cout << "dcNum is: " << dcNum << ", cofacPhase0: " << endl;
	for (size_t i=0; i < dcSize; i++) {
		cout << "|" << cofacPhase0[i];
	}
	cout << endl; //*/

	DdNode* cofacCube0 = Cudd_bddComputeCube(gbm, cofacVars, cofacPhase0, dcSize);
	Cudd_Ref(cofacCube0);
	DdNode* testImage0 = Cudd_Cofactor(gbm, currImage, cofacCube0);
	Cudd_Ref(testImage0);
	Cudd_RecursiveDeref(gbm, cofacCube0);
	if (con0Node == testImage0) {
		//cout << "at dcPos: " << dcPos << " with dcNum: " << dcNum << " is a DC." << endl;
	} else {
		//cout << "at dcPos: " << dcPos << " with dcNum: " << dcNum << " is NOT A DC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		set<gendc>::iterator dcIt = candidates.begin();
		std::advance(dcIt, dcPos);
		//cout << "dc is : " << dcIt->sig1 << " " << dcIt->sig2 << " " << dcIt->sig3 << endl;
		dcIt->poss[dcNum] = true;
		dcIt->count++;
	}
	Cudd_RecursiveDeref(gbm, testImage0);
	delete[] dcSignalIndices;
	delete[] cofacPhase0;
//	for (size_t i=0; i < dcSize; i++) {
//		delete[] cofacVars[i];
//	}
	delete[] cofacVars;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::cofactorDCInImage(DdManager*& gbm, vector<string>& dcSignals, int dcPos, int dcNum, set<dc>& candidates, DdNode** vars, DdNode*& currImage) {
	DdNode* con0Node = Cudd_ReadLogicZero(gbm);
	int dcSignalIndices[3] = {0,0,0};
	assert(dcSignals.size() == 3);
	for (size_t i=0; i < dcSignals.size(); i++) {
		//cout << "Edge is " << dcSignals.at(i) << endl;
		//cout << "with index " << this->circuit.edges.at(dcSignals.at(i)).eIndex << endl;
		dcSignalIndices[i] = this->circuit.edges.at(dcSignals.at(i)).eIndex;
	}
	DdNode* cofacVars[3];
	for (size_t i=0; i < 3; i++) {
		cofacVars[i] = vars[dcSignalIndices[i]];
	}
	// Check for (0,0,0) DC.
	int cofacPhase0[3] = {0,0,0};
	switch (dcNum) {
		case 0:
			cofacPhase0[0] = 0;
			cofacPhase0[1] = 0;
			cofacPhase0[2] = 0;
			break;
		case 1:
			cofacPhase0[0] = 1;
			cofacPhase0[1] = 0;
			cofacPhase0[2] = 0;
			break;
		case 2:
			cofacPhase0[0] = 0;
			cofacPhase0[1] = 1;
			cofacPhase0[2] = 0;
			break;
		case 3:
			cofacPhase0[0] = 1;
			cofacPhase0[1] = 1;
			cofacPhase0[2] = 0;
			break;
		case 4:
			cofacPhase0[0] = 0;
			cofacPhase0[1] = 0;
			cofacPhase0[2] = 1;
			break;
		case 5:
			cofacPhase0[0] = 1;
			cofacPhase0[1] = 0;
			cofacPhase0[2] = 1;
			break;
		case 6:
			cofacPhase0[0] = 0;
			cofacPhase0[1] = 1;
			cofacPhase0[2] = 1;
			break;
		case 7:
			cofacPhase0[0] = 1;
			cofacPhase0[1] = 1;
			cofacPhase0[2] = 1;
			break;
		default: 
			cout << "switch in cofactorDCInImage(): value of dcNum was not found." << endl;
			break;	
	}   
	DdNode* cofacCube0 = Cudd_bddComputeCube(gbm, cofacVars, cofacPhase0, 3);
	Cudd_Ref(cofacCube0);
	DdNode* testImage0 = Cudd_Cofactor(gbm, currImage, cofacCube0);
	Cudd_Ref(testImage0);
	Cudd_RecursiveDeref(gbm, cofacCube0);
	if (con0Node == testImage0) {
		//cout << "at dcPos: " << dcPos << " with dcNum: " << dcNum << " is a DC." << endl;
	} else {
		cout << "at dcPos: " << dcPos << " with dcNum: " << dcNum << " is NOT A DC !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl; 
		set<dc>::iterator dcIt = candidates.begin();
		std::advance(dcIt, dcPos);
		//cout << "dc is : " << dcIt->sig1 << " " << dcIt->sig2 << " " << dcIt->sig3 << endl;
		dcIt->poss[dcNum] = true;
		dcIt->count++;
	}
	Cudd_RecursiveDeref(gbm, testImage0);
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::imageComputationForDCsGeneral(set<gendc>& DCcandidates) {
	// Used for input constraint.
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divisor = this->pMap.at("D");
	vector<int> quotient = this->pMap.at("Q");
	vector<int> remainder = this->pMap.at("R");
	int n = quotient.size();

	// Initialize Dd manager.
	DdManager *gbm; // Global BDD manager.
	gbm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0); // Initialize a new BDD manager.
	Cudd_AutodynDisable(gbm);  // Disable automatic dynamic reordering.
	//Cudd_AutodynEnable(gbm, CUDD_REORDER_SYMM_SIFT);
	int numVars = this->circuit.maxEdgeIndex + 1;
	//DdNode* vars[numVars];  // Used variables. One for every signal in the circuit.
	// First create all needed bdd variables. (Instead of mapping indices just use vars[e.eIndex] to retrieve the bdd variable for edge e.
	//for (size_t i=0; i < numVars; i++) {
	//	vars[i] = Cudd_bddNewVar(gbm);
	//}

	// Create array for permutation.

    //cout << "BDD TEST0" << endl;
    int permuSize = numVars;
    int* permu = new int[permuSize];  // Array to save variable order.
    map<int,int> permuMap;
    for (size_t i = 0; i < permuSize; i++) {
    	permu[i] = i;
    	permuMap.insert({i, i});
    }

	// Create initial variable order for interleaving d_i and corresponding r0_(i+n-1) and r_i.
    int prevD = 0, prevR0 = 0, prevR = 0;
    int last = (3 * divisor.size());  // Defines last position of D and R0 variables in the constraint bdd.
    prevR = permuMap.at(this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex);
    std::swap(permu[prevR], permu[0]);
    permuMap.erase(this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex);
    permuMap.erase(permu[prevR]);
    permuMap.insert({this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex, 0});
    permuMap.insert({permu[prevR], prevR});
    for (size_t i=0; i < divisor.size(); i++) {
    	//prevD = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]));
    	//prevR = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex]));
    	//prevR0 = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex]));
    	prevR0 = permuMap.at(this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex);
    	std::swap(permu[prevR0], permu[last-3*i]);
    	permuMap.erase(this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex);
    	permuMap.erase(permu[prevR0]);
    	permuMap.insert({this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex, last-3*i});
    	permuMap.insert({permu[prevR0], prevR0});
    	prevR = permuMap.at(this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex);
    	std::swap(permu[prevR], permu[last-1-3*i]);
    	permuMap.erase(this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex);
    	permuMap.erase(permu[prevR]);
    	permuMap.insert({this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex, last-1-3*i});
    	permuMap.insert({permu[prevR], prevR});
    	prevD = permuMap.at(this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex);
    	std::swap(permu[prevD], permu[last-2-3*i]);
    	permuMap.erase(this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex);
    	permuMap.erase(permu[prevD]);
    	permuMap.insert({this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex, last-2-3*i});
    	permuMap.insert({permu[prevD], prevD});
    }
    permuMap.clear();
   	/*
    cout << "permu before backward traversal is: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
	// */
	DdNode* varsDummy[1];
	obtainBackwardsBDDPermutation(permu, varsDummy, permuSize, dividend, divisor, quotient);

	//int check = Cudd_ShuffleHeap(gbm, permu);
	//cout << "Check of shuffle heap was " << check << endl;

	/*
    cout << "permu after backward traversal is: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/

    cout << "Before vars array creation." << endl;
    // Instead of Cudd_ShuffleHeap just create variables at positions with already desired level.
    DdNode** vars = new DdNode*[numVars];  // Used variables. One for every signal in the circuit.
	// First create all needed bdd variables. (Instead of mapping indices just use vars[e.eIndex] to retrieve the bdd variable for edge e.
	// Use obtained permutation in permu to create variables already at desired level.
	cout << "after vars array creation." << endl;
	for (size_t i=0; i < numVars; i++) {
		vars[permu[i]] = Cudd_bddNewVarAtLevel(gbm, i);
	}
	cout << "after BDD vars creation." << endl;

	delete[] permu;

	cout << "after permu deletion." << endl;
	//return;
	// Construct BDD for the input constraint ( 0 <= R0 < D * 2^(n-1) ), later used in image computation.
	cout << "before input constraint build." << endl;
	DdNode* bdd_c;
	buildInputConstraintBDD(gbm, bdd_c, dividend, divisor, vars);

	cout << "after input constraint build." << endl;
	//DdNode* bdd_printC = Cudd_BddToAdd(gbm, bdd_c);
	//char filename[30]; // Write .dot filename to a string
    //sprintf(filename, "./bdd_ic.dot");
    //write_dd(gbm, bdd_printC, filename);  // Write the resulting cascade dd to a file.
	//print_dd (gbm, bdd_printC, 1, 3);

    // Create the "cuts" through the network beforehand.
    vector<vector<string>> imageBorders;  // Save indices of signals until the image in one step should be computed.
    vector<vector<int>> imageDCNums;
    vector<int> dcPos;
    vector<vector<int>> orderedNodes;  // Save nodes of a cut in topological order.
    vector<set<int>> cutOutputs;  // Save indices of cutted edges. Used later in determining end of forward construction of BDDs.
    vector<set<int>> cutInputs;
    vector<set<int>> signalsToQuantify;

    // Fill previous defined vectors
    calcHelpersForImageCompGeneral(DCcandidates, imageBorders, imageDCNums, dcPos, orderedNodes, cutInputs, cutOutputs, signalsToQuantify);
	//return;
	/*
	cout << "inputs" << endl;
	for (auto& elem: cutInputs) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}

	cout << "outputs" << endl;
	for (auto& elem: cutOutputs) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}
	cout << "signalsToQuantify" << endl;
	for (auto& elem: signalsToQuantify) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}	//*/

	//return;
	// Now build up the BDD for image computation for every stage and quantify out the predertimed signals to get the image.
	//cout << "Anzahl Knoten davor: " << Cudd_ReadNodeCount(gbm) << endl;

	DdNode* images[imageBorders.size()]; // BDDs representing the image of a "cutted stage".
	map<int, int> indexToBDD;  // Map edge index to position in images array.
	// Use con0Node for checking equality to logical constant zero function (which should result after cofactoring).
	//DdNode* con0Node = Cudd_ReadLogicZero(gbm);
	vp::Node* currNode;
	int currIndex;
	DdNode* temp1;
	DdNode* temp2;
	DdNode* tempResult;
	for (size_t i=0; i < imageBorders.size(); i++) {
		//cout << "i: " << i << endl;
		indexToBDD.clear();
		int count = 0;
		vector<DdNode*> internSignals(orderedNodes.at(i).size(), NULL);
		vector<int> usedSignalsCount(orderedNodes.at(i).size(), 0);
		vector<int> currOutCount(orderedNodes.at(i).size(), 0);
		for (auto& elem: orderedNodes.at(i)) {
			//cout << this->circuit.node(elem).name << "|" << endl;
			currNode = &this->circuit.node(elem);
			currIndex = currNode->outputs.at(0)->eIndex;
			currOutCount.at(count) = currNode->getOutputsCount();
			if (indexToBDD.count(currNode->inputs.at(0)->eIndex) == 0) temp1 = vars[currNode->inputs.at(0)->eIndex];
			else {
				temp1 = internSignals.at(indexToBDD.at(currNode->inputs.at(0)->eIndex));
				usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex))++;
			}
			if (currNode->getInputsCount() == 2) {
				if (indexToBDD.count(currNode->inputs.at(1)->eIndex) == 0) temp2 = vars[currNode->inputs.at(1)->eIndex];
				else {
					temp2 = internSignals.at(indexToBDD.at(currNode->inputs.at(1)->eIndex));
					usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex))++;
				}
			}
			// Case distinction of node type.
			switch (currNode->type) {
				case vp::Node::AND:
					internSignals.at(count) = Cudd_bddAnd(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::XOR:
					internSignals.at(count) = Cudd_bddXor(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::OR:
					internSignals.at(count) = Cudd_bddOr(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::NOT:
					internSignals.at(count) = Cudd_Not(temp1);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::BUFFER:
					  internSignals.at(count) = temp1;
                    	Cudd_Ref(internSignals.at(count));
					//cout << "BUFFER IN IMAGE COMPUTATION DETECTED! NOT ALLOWED YET." << endl;
					break;
				default:
					cout << "imageComputation() switch: currNode type not specified." << endl;
					break;
			}
			indexToBDD.insert({currIndex, count});
			if (indexToBDD.count(currNode->inputs.at(0)->eIndex) > 0) {
				if (usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex)) == currOutCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex))) {
					Cudd_RecursiveDeref(gbm, internSignals.at(indexToBDD.at(currNode->inputs.at(0)->eIndex)));
				}
			}
			if (currNode->getInputsCount() == 2 && indexToBDD.count(currNode->inputs.at(1)->eIndex) > 0) {
				if (usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex)) == currOutCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex))) {
					Cudd_RecursiveDeref(gbm, internSignals.at(indexToBDD.at(currNode->inputs.at(1)->eIndex)));
				}
			}
			count++;
		}
//		cout << "transition function passed." << endl;

		if (i==0) images[i] = bdd_c;  // Start of this image computation is image of prev. step, if i=0 input constraint is used.
		else images[i] = images[i-1];
		// Construct function for image computation.
		for (auto& elem: cutOutputs.at(i)) {
			if (indexToBDD.count(elem) == 0) continue;  // Case of an output which is also input of the cut.
			temp2 = internSignals.at(indexToBDD.at(elem));
			temp1 = Cudd_bddXnor(gbm, vars[elem], temp2);  // y_i == f_i(x).
			Cudd_Ref(temp1);
			Cudd_RecursiveDeref(gbm, temp2);
			tempResult = Cudd_bddAnd(gbm, images[i], temp1);  // AND over all prev. y_i == f_i(x) and current one.
			Cudd_Ref(tempResult);
			Cudd_RecursiveDeref(gbm, images[i]);
			Cudd_RecursiveDeref(gbm, temp1);
			images[i] = tempResult;
		}
//		cout << "image function passed. #Nodes: " << Cudd_DagSize(images[i]) << endl;
		// At last existential quantification of some of the inputs (input signals saved in signalsToQuantify).
		int quantifySize = signalsToQuantify.at(i).size();
		int* cubeArray = new int[quantifySize];
		int i_cube = 0;
		for (auto& elem: signalsToQuantify.at(i)) {
			cubeArray[i_cube] = Cudd_NodeReadIndex(vars[elem]);
			i_cube++;
		}
		DdNode* quantCubeArray = Cudd_IndicesToCube(gbm, cubeArray, quantifySize);  // Cube for quantification.
		Cudd_Ref(quantCubeArray);
		tempResult = Cudd_bddExistAbstract(gbm, images[i], quantCubeArray);  // Existential quantification.
		Cudd_Ref(tempResult);
		Cudd_RecursiveDeref(gbm, images[i]);
		Cudd_RecursiveDeref(gbm, quantCubeArray);
		images[i] = tempResult;
//	cout << "quantification passed. #Nodes: " << Cudd_DagSize(images[i]) << endl;

		// Check image BDD for DC candidates by using cofactoring.
		if (i < imageBorders.size() - 1) {
			for (auto& elem: imageDCNums.at(i)) {
				cofactorDCInImageGeneral(gbm, imageBorders.at(i), dcPos.at(i), elem, DCcandidates, vars, images[i]);
			}
		}
		//cout << "Anzahl Knoten am Ende: " << Cudd_ReadNodeCount(gbm) << endl;
    	delete[] cubeArray;
	}

    DdNode* bdd_rc;
    buildRemainderConstraintBDD(gbm, bdd_rc, remainder, divisor, vars);
    /*
    DdNode* bdd_printC = Cudd_BddToAdd(gbm, bdd_rc);
	print_dd (gbm, bdd_printC, 1, 3);
    char filename[30]; // Write .dot filename to a string
    sprintf(filename, "./bdd_rc.dot");
    write_dd(gbm, bdd_printC, filename);  // Write the resulting cascade dd to a file.
    DdNode* bdd_printA = Cudd_BddToAdd(gbm, images[imageBorders.size() - 1]);
	print_dd (gbm, bdd_printA, 1, 3);
    char filename2[30]; // Write .dot filename to a string
    sprintf(filename2, "./image_bdd.dot");
    write_dd(gbm, bdd_printA, filename2);  // Write the resulting cascade dd to a file.
    //*/
    //if (bdd_rc == images[imageBorders.size() - 1]) cout << "They are the same picture." << endl;
    //else cout << "Remainder constraint is different! " << endl;

	//??
    cout << "bdd_rc is leq to last image: " << Cudd_bddLeq(gbm, bdd_rc, images[imageBorders.size() - 1]) << endl;
    cout << "last image is leq to bdd_rc: " << Cudd_bddLeq(gbm, images[imageBorders.size() - 1], bdd_rc) << endl;

    // Deref all remaining bdds.
    Cudd_RecursiveDeref(gbm, images[imageBorders.size() - 1]);
    Cudd_RecursiveDeref(gbm, bdd_rc);
    // Quit Dd manager. Check for still live nodes to avoid memory leakage.
    cout << "still live nodes: " << Cudd_CheckZeroRef(gbm) << endl;
    cout << "max. live node count was: " << Cudd_ReadPeakLiveNodeCount(gbm) << endl;
    cout << "# reorderings: " << Cudd_ReadReorderings(gbm) << endl;
    cout << "time (in ms) for reorderings: " << Cudd_ReadReorderingTime(gbm) << endl;

    delete[] vars;
    Cudd_Quit(gbm);

    /*
    for (size_t i=0; i < this->pMap.at("D").size(); i++) {
    	//cout << "D[" << i << "] has index: " << this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex << endl;
    }
    for (size_t i=0; i < this->pMap.at("R0").size(); i++) {
    	//cout << "R0[" << i << "] has index: " << this->circuit.node(dividend.at(i)).outputs.at(0)->eIndex << endl;
    }
    for (size_t i=0; i < this->pMap.at("R").size(); i++) {
    	//cout << "R[" << i << "] has index: " << this->circuit.node(this->pMap.at("R").at(i)).inputs.at(0)->eIndex << endl;
    }
	*/
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::imageComputationForDCs(set<dc>& DCcandidates) {
	// Used for input constraint.	
	vector<int> dividend = this->pMap.at("R0");
	vector<int> divisor = this->pMap.at("D");
	vector<int> quotient = this->pMap.at("Q");
	vector<int> remainder = this->pMap.at("R");
	int n = quotient.size();
	
	// Initialize Dd manager.	
	DdManager *gbm; // Global BDD manager.
	gbm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0); // Initialize a new BDD manager.
	Cudd_AutodynDisable(gbm);  // Disable automatic dynamic reordering.
	//Cudd_AutodynEnable(gbm, CUDD_REORDER_SYMM_SIFT);
	int numVars = this->circuit.maxEdgeIndex + 1;
	//DdNode* vars[numVars];  // Used variables. One for every signal in the circuit.
	// First create all needed bdd variables. (Instead of mapping indices just use vars[e.eIndex] to retrieve the bdd variable for edge e.
	//for (size_t i=0; i < numVars; i++) {
	//	vars[i] = Cudd_bddNewVar(gbm);
	//}

	// Create array for permutation. 
    
    //cout << "BDD TEST0" << endl;
    int permuSize = numVars;
    int* permu = new int[permuSize];  // Array to save variable order.
    map<int,int> permuMap;
    for (size_t i = 0; i < permuSize; i++) {
    	permu[i] = i;
    	permuMap.insert({i, i});
    }

	// Create initial variable order for interleaving d_i and corresponding r0_(i+n-1) and r_i.
    int prevD = 0, prevR0 = 0, prevR = 0;
    int last = (3 * divisor.size());  // Defines last position of D and R0 variables in the constraint bdd. 
    prevR = permuMap.at(this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex);
    std::swap(permu[prevR], permu[0]);
    permuMap.erase(this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex);
    permuMap.erase(permu[prevR]);
    permuMap.insert({this->circuit.node(remainder.at(remainder.size()-1)).inputs.at(0)->eIndex, 0});
    permuMap.insert({permu[prevR], prevR});
    for (size_t i=0; i < divisor.size(); i++) {
    	//prevD = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex]));
    	//prevR = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex]));
    	//prevR0 = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex]));
    	prevR0 = permuMap.at(this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex);
    	std::swap(permu[prevR0], permu[last-3*i]);
    	permuMap.erase(this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex);
    	permuMap.erase(permu[prevR0]);
    	permuMap.insert({this->circuit.node(dividend.at(n-1+i)).outputs.at(0)->eIndex, last-3*i});
    	permuMap.insert({permu[prevR0], prevR0});
    	prevR = permuMap.at(this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex);
    	std::swap(permu[prevR], permu[last-1-3*i]);
    	permuMap.erase(this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex);
    	permuMap.erase(permu[prevR]);
    	permuMap.insert({this->circuit.node(remainder.at(i)).inputs.at(0)->eIndex, last-1-3*i});
    	permuMap.insert({permu[prevR], prevR});
    	prevD = permuMap.at(this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex);
    	std::swap(permu[prevD], permu[last-2-3*i]);
    	permuMap.erase(this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex);
    	permuMap.erase(permu[prevD]);
    	permuMap.insert({this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex, last-2-3*i});
    	permuMap.insert({permu[prevD], prevD});
    }
    permuMap.clear();
   	/*
    cout << "permu before backward traversal is: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
	// */
	DdNode* varsDummy[1];
	obtainBackwardsBDDPermutation(permu, varsDummy, permuSize, dividend, divisor, quotient);
	
	//int check = Cudd_ShuffleHeap(gbm, permu);
	//cout << "Check of shuffle heap was " << check << endl;
	
	/*
    cout << "permu after backward traversal is: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
    
    cout << "Before vars array creation." << endl;
    // Instead of Cudd_ShuffleHeap just create variables at positions with already desired level.
    DdNode** vars = new DdNode*[numVars];  // Used variables. One for every signal in the circuit.
	// First create all needed bdd variables. (Instead of mapping indices just use vars[e.eIndex] to retrieve the bdd variable for edge e.
	// Use obtained permutation in permu to create variables already at desired level.
	cout << "after vars array creation." << endl;
	for (size_t i=0; i < numVars; i++) {
		vars[permu[i]] = Cudd_bddNewVarAtLevel(gbm, i);
	}
	cout << "after BDD vars creation." << endl;
	
	delete[] permu;
	
	cout << "after permu deletion." << endl;
	//return;
	// Construct BDD for the input constraint ( 0 <= R0 < D * 2^(n-1) ), later used in image computation.
	cout << "before input constraint build." << endl;
	DdNode* bdd_c;  
	buildInputConstraintBDD(gbm, bdd_c, dividend, divisor, vars); 
	
	cout << "after input constraint build." << endl;
	//DdNode* bdd_printC = Cudd_BddToAdd(gbm, bdd_c);
	//char filename[30]; // Write .dot filename to a string 
    //sprintf(filename, "./bdd_ic.dot"); 
    //write_dd(gbm, bdd_printC, filename);  // Write the resulting cascade dd to a file.
	//print_dd (gbm, bdd_printC, 1, 3);
	
    // Create the "cuts" through the network beforehand.
    vector<vector<string>> imageBorders;  // Save indices of signals until the image in one step should be computed.
    vector<vector<int>> imageDCNums;
    vector<int> dcPos;
    vector<vector<int>> orderedNodes;  // Save nodes of a cut in topological order.
    vector<set<int>> cutOutputs;  // Save indices of cutted edges. Used later in determining end of forward construction of BDDs.
    vector<set<int>> cutInputs;
    vector<set<int>> signalsToQuantify;
    
    // Fill previous defined vectors 
    calcHelpersForImageComp(DCcandidates, imageBorders, imageDCNums, dcPos, orderedNodes, cutInputs, cutOutputs, signalsToQuantify);
	//return;
	/*
	cout << "inputs" << endl;
	for (auto& elem: cutInputs) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}
	
	cout << "outputs" << endl;
	for (auto& elem: cutOutputs) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}
	cout << "signalsToQuantify" << endl;
	for (auto& elem: signalsToQuantify) {
		for (auto& inner: elem) {
			cout << "|" << inner;
		}
		cout << endl;
	}	//*/
	
	//return;
	// Now build up the BDD for image computation for every stage and quantify out the predertimed signals to get the image.
	//cout << "Anzahl Knoten davor: " << Cudd_ReadNodeCount(gbm) << endl;

	DdNode* images[imageBorders.size()]; // BDDs representing the image of a "cutted stage".
	map<int, int> indexToBDD;  // Map edge index to position in images array.
	// Use con0Node for checking equality to logical constant zero function (which should result after cofactoring).
	//DdNode* con0Node = Cudd_ReadLogicZero(gbm);
	vp::Node* currNode;
	int currIndex;
	DdNode* temp1;
	DdNode* temp2;
	DdNode* tempResult;
	for (size_t i=0; i < imageBorders.size(); i++) {
		//cout << "i: " << i << endl;
		indexToBDD.clear();
		int count = 0;
		vector<DdNode*> internSignals(orderedNodes.at(i).size(), NULL);
		vector<int> usedSignalsCount(orderedNodes.at(i).size(), 0);
		vector<int> currOutCount(orderedNodes.at(i).size(), 0);
		for (auto& elem: orderedNodes.at(i)) {
			//cout << this->circuit.node(elem).name << "|" << endl;
			currNode = &this->circuit.node(elem);
			currIndex = currNode->outputs.at(0)->eIndex;
			currOutCount.at(count) = currNode->getOutputsCount();
			if (indexToBDD.count(currNode->inputs.at(0)->eIndex) == 0) temp1 = vars[currNode->inputs.at(0)->eIndex];
			else { 
				temp1 = internSignals.at(indexToBDD.at(currNode->inputs.at(0)->eIndex));
				usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex))++;
			}
			if (currNode->getInputsCount() == 2) {
				if (indexToBDD.count(currNode->inputs.at(1)->eIndex) == 0) temp2 = vars[currNode->inputs.at(1)->eIndex];
				else {
					temp2 = internSignals.at(indexToBDD.at(currNode->inputs.at(1)->eIndex));
					usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex))++;
				}
			}
			// Case distinction of node type.
			switch (currNode->type) {
				case vp::Node::AND:
					internSignals.at(count) = Cudd_bddAnd(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::XOR:
					internSignals.at(count) = Cudd_bddXor(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::OR:
					internSignals.at(count) = Cudd_bddOr(gbm, temp1, temp2);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::NOT:
					internSignals.at(count) = Cudd_Not(temp1);
					Cudd_Ref(internSignals.at(count));
					break;
				case vp::Node::BUFFER:
					internSignals.at(count) = Cudd_Not(temp1);
					//cout << "BUFFER IN IMAGE COMPUTATION DETECTED! NOT ALLOWED YET." << endl;
					break;
				default:
					cout << "imageComputation() switch: currNode type not specified." << endl;
					break;
			}
			indexToBDD.insert({currIndex, count});
			if (indexToBDD.count(currNode->inputs.at(0)->eIndex) > 0) {
				if (usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex)) == currOutCount.at(indexToBDD.at(currNode->inputs.at(0)->eIndex))) {
					Cudd_RecursiveDeref(gbm, internSignals.at(indexToBDD.at(currNode->inputs.at(0)->eIndex)));
				}
			}
			if (currNode->getInputsCount() == 2 && indexToBDD.count(currNode->inputs.at(1)->eIndex) > 0) {
				if (usedSignalsCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex)) == currOutCount.at(indexToBDD.at(currNode->inputs.at(1)->eIndex))) {
					Cudd_RecursiveDeref(gbm, internSignals.at(indexToBDD.at(currNode->inputs.at(1)->eIndex))); 
				}
			}
			count++;
		}
		cout << "transition function passed." << endl;
    	
		if (i==0) images[i] = bdd_c;  // Start of this image computation is image of prev. step, if i=0 input constraint is used.
		else images[i] = images[i-1];
		// Construct function for image computation.
		for (auto& elem: cutOutputs.at(i)) {  
			if (indexToBDD.count(elem) == 0) continue;  // Case of an output which is also input of the cut.
			temp2 = internSignals.at(indexToBDD.at(elem));
			temp1 = Cudd_bddXnor(gbm, vars[elem], temp2);  // y_i == f_i(x). 
			Cudd_Ref(temp1);
			Cudd_RecursiveDeref(gbm, temp2);
			tempResult = Cudd_bddAnd(gbm, images[i], temp1);  // AND over all prev. y_i == f_i(x) and current one.
			Cudd_Ref(tempResult);
			Cudd_RecursiveDeref(gbm, images[i]);
			Cudd_RecursiveDeref(gbm, temp1);
			images[i] = tempResult;
		}
		cout << "image function passed. #Nodes: " << Cudd_DagSize(images[i]) << endl;
		// At last existential quantification of some of the inputs (input signals saved in signalsToQuantify).
		int quantifySize = signalsToQuantify.at(i).size();
		int* cubeArray = new int[quantifySize];
		int i_cube = 0;
		for (auto& elem: signalsToQuantify.at(i)) {
			cubeArray[i_cube] = Cudd_NodeReadIndex(vars[elem]);
			i_cube++;
		}
		DdNode* quantCubeArray = Cudd_IndicesToCube(gbm, cubeArray, quantifySize);  // Cube for quantification.
		Cudd_Ref(quantCubeArray);
		tempResult = Cudd_bddExistAbstract(gbm, images[i], quantCubeArray);  // Existential quantification.
		Cudd_Ref(tempResult);
		Cudd_RecursiveDeref(gbm, images[i]);
		Cudd_RecursiveDeref(gbm, quantCubeArray);
		images[i] = tempResult;
		cout << "quantification passed. #Nodes: " << Cudd_DagSize(images[i]) << endl; 
		
		// Check image BDD for DC candidates by using cofactoring.
		if (i < imageBorders.size() - 1) {
			for (auto& elem: imageDCNums.at(i)) {
				cofactorDCInImage(gbm, imageBorders.at(i), dcPos.at(i), elem, DCcandidates, vars, images[i]);
			}
		}
		//cout << "Anzahl Knoten am Ende: " << Cudd_ReadNodeCount(gbm) << endl;
    	delete[] cubeArray;
	}

    DdNode* bdd_rc;
    buildRemainderConstraintBDD(gbm, bdd_rc, remainder, divisor, vars);
    /* 
    DdNode* bdd_printC = Cudd_BddToAdd(gbm, bdd_rc);
	print_dd (gbm, bdd_printC, 1, 3); 
    char filename[30]; // Write .dot filename to a string 
    sprintf(filename, "./bdd_rc.dot"); 
    write_dd(gbm, bdd_printC, filename);  // Write the resulting cascade dd to a file.
    DdNode* bdd_printA = Cudd_BddToAdd(gbm, images[imageBorders.size() - 1]);
	print_dd (gbm, bdd_printA, 1, 3); 
    char filename2[30]; // Write .dot filename to a string 
    sprintf(filename2, "./image_bdd.dot"); 
    write_dd(gbm, bdd_printA, filename2);  // Write the resulting cascade dd to a file.
    //*/
    //if (bdd_rc == images[imageBorders.size() - 1]) cout << "They are the same picture." << endl;
    //else cout << "Remainder constraint is different! " << endl;
    cout << "bdd_rc is leq to last image: " << Cudd_bddLeq(gbm, bdd_rc, images[imageBorders.size() - 1]) << endl;
    cout << "last image is leq to bdd_rc: " << Cudd_bddLeq(gbm, images[imageBorders.size() - 1], bdd_rc) << endl;
    
    // Deref all remaining bdds.
    Cudd_RecursiveDeref(gbm, images[imageBorders.size() - 1]);
    Cudd_RecursiveDeref(gbm, bdd_rc);
    // Quit Dd manager. Check for still live nodes to avoid memory leakage.
    cout << "still live nodes: " << Cudd_CheckZeroRef(gbm) << endl;
    cout << "max. live node count was: " << Cudd_ReadPeakLiveNodeCount(gbm) << endl;
    cout << "# reorderings: " << Cudd_ReadReorderings(gbm) << endl;
    cout << "time (in ms) for reorderings: " << Cudd_ReadReorderingTime(gbm) << endl;
    
    delete[] vars;
    Cudd_Quit(gbm); 
    
    /*
    for (size_t i=0; i < this->pMap.at("D").size(); i++) {
    	//cout << "D[" << i << "] has index: " << this->circuit.node(divisor.at(i)).outputs.at(0)->eIndex << endl;
    } 
    for (size_t i=0; i < this->pMap.at("R0").size(); i++) {
    	//cout << "R0[" << i << "] has index: " << this->circuit.node(dividend.at(i)).outputs.at(0)->eIndex << endl;
    } 
    for (size_t i=0; i < this->pMap.at("R").size(); i++) {
    	//cout << "R[" << i << "] has index: " << this->circuit.node(this->pMap.at("R").at(i)).inputs.at(0)->eIndex << endl;
    } 
	*/
}


//________________________________________________________________________________________________________________________________________
void Verifizierer::obtainBackwardsBDDPermutation(int* permu, DdNode** vars, int permuSize, vector<int>& dividend, vector<int>& divisor, vector<int>& quotient)  {
	cout << "From here obtaining permutation starts." << endl;
    	
    double timePermu, tstartPermu;
    timePermu = 0.0;
    tstartPermu = clock();

    /* cout << "permu at beginning: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    } 
    cout << endl; */
    //*
    // Make except set containing indices of D and R_0, because their relative order should not be changed. 
    std::set<int> notChange;
    int pos = 0;
    for (auto& elem: divisor) {  // For D.
    	pos = this->circuit.node(elem).outputs.at(0)->eIndex;
    	notChange.insert(pos);
    }
    for (auto& elem: dividend) {  // For R_0.
    	pos = this->circuit.node(elem).outputs.at(0)->eIndex;
    	notChange.insert(pos);
    }   
    for (auto& elem: quotient) {  // For Q bits.
    	//pos = this->circuit.node(elem).inputs.at(0)->eIndex;
    	//notChange.insert(pos);
    }
    for (auto& elem: this->pMap.at("R")) {  // For R bits.
    	//pos = this->circuit.node(elem).inputs.at(0)->eIndex;
    	//notChange.insert(pos);
    }
    //*/   
    
    int* posOfIndex = new int[permuSize];  // Save new positions of index i at array position i.
    for (size_t i = 0; i < permuSize; i++) {
    	//cout << "Permu[" << i << "] is " << permu[i] << "p" << endl;
    	posOfIndex[permu[i]] = i;
    } 
    /*
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "posOfIndex[" << i << "] is " << posOfIndex[i] << endl;
    }
    //*/	
    
    // Variables used later.
    int input1 = -1, input2 = -1, output = -1, posOut = -1, posIn1 = -1, posIn2 = -1, idxout = -1, idx1 = -1, idx2 = -1;
    int tmpInt;
    int counter = 0;	
    
    std::set<int> oneChange;  // Set of variables which if already fixed should pull their partner to themself in the order.
    vp::Edge* edgeName;
    /*
    //EXTRA: Fix position of gate which is successor of d_i signals. 
    for (auto& elem: this->circuit.nodes) {  //TODO: dont iterate through nodes but just inputNodes.
    	if (elem.name.find("D") != string::npos) {
    		//cout << "D node found: " << elem.name << endl;
    		//edgeName = &*elem.outputs.at(0);
    		//cout << "EdgeName: " << edgeName << endl;
    		vector<vp::Node*> nextNodes;
    		nextNodes.push_back(&elem);
    		vector<vp::Node*> tempNodes;
    		int stop = 0;
    		while (nextNodes.size() > 0 && stop < 0) {
    			stop++;
    			for (size_t i=0; i < nextNodes.size(); i++) {
    				vp::Node* fixed = nextNodes.at(i);
    				//cout << "fixed is: " << fixed->name << endl;
    				for (size_t i = 0; i < fixed->getOutputsCount(); i++) {
    					//TODO: Several iterations since in AIG it is no longer directly conneced through 1 XOR gate but several AND/Inverter.	
    					//cout << "successor is: " << fixed->outputNode(i).name << endl;
    					if (fixed->outputNode(i).type == vp::Node::OUTPUT_PORT) continue;
    					//cout << "output edge index is:" << this->circuit.edgeIndex.at(*fixed->outputNode(i).outputs.at(0)) << endl;
    					if (fixed->outputNode(i).adder > -1) continue;
    					//cout << "fixed output node adder: " << fixed->outputNode(i).adder << endl;
    					//cout << "fixed output node name: " << fixed->outputNode(i).name << endl;
    					tempNodes.push_back(&fixed->outputNode(i));
    					pos = fixed->outputNode(i).outputs.at(0)->eIndex;
    					//cout << "pos: " << fixed->outputNode(i).outputs.at(0)->eIndex << endl;
    					notChange.insert(pos);
    					oneChange.insert(pos);
    					// Place all CAS XOR output signals right above d_i in the order before iterating through replacements.
    					//output = edgeName->eIndex;
    					output = fixed->outputs.at(0)->eIndex;
    					input2 = pos;
    					idxout = output;
    					idx2 = input2;
   	 					posOut = posOfIndex[idxout];
    					posIn2 = posOfIndex[idx2];
    					Verifizierer::arrayShifting(permu, permuSize, posOut, -1, posIn2, idxout, -1, idx2, posOfIndex);
    				}
    			}
    			//cout << "temps: " << endl;
    			//for (auto& temps: tempNodes) cout << temps->name << endl;
    			//cout << "temps end " << endl;
    			nextNodes = tempNodes;
    			tempNodes.clear();
    		}
    	}
    }//*/
    
    //EXTRA: New approach. Make a breadth first search from already fixed signals and create the variable order in this way.
    vector<vp::Node*> nextNodes;
    vector<vp::Node*> tempNodes;
    for (auto& elem: this->circuit.nodes) {  //TODO: dont iterate through nodes but just inputNodes.
    	if (elem.name.find("D") != string::npos) {
    		//cout << "D node found: " << elem.name << endl;
    		edgeName = &*elem.outputs.at(0);
    		//cout << "EdgeName: " << edgeName->name << endl;
    		nextNodes.push_back(&elem);
    	}
    }
    //int stop = 0;  // Helper variable for limiting steps.
    while (nextNodes.size() > 0 /*&& stop < 100*/) {
    	//cout << "stop: " << stop << " with size: " << nextNodes.size() << endl;
    	//stop++;
    	for (size_t i=0; i < nextNodes.size(); i++) {
    		vp::Node* fixed = nextNodes.at(i);
    		//cout << "fixed is: " << fixed->name << endl;
    		for (size_t i = 0; i < fixed->getOutputsCount(); i++) {
    			//cout << "successor is: " << fixed->outputNode(i).name << endl;
    			if (fixed->outputNode(i).type == vp::Node::OUTPUT_PORT || fixed->outputNode(i).type == vp::Node::DELETED) continue;
    			//cout << "successor type is: " << fixed->outputNode(i).type << endl;
    			//cout << "output edge index is:" << this->circuit.edgeIndex.at(*fixed->outputNode(i).outputs.at(0)) << endl;
    			//if (fixed->outputNode(i).adder > -1) continue;  
    			//cout << "fixed output node adder: " << fixed->outputNode(i).adder << endl;
    			//cout << "fixed output node name: " << fixed->outputNode(i).name << endl;
    			pos = fixed->outputNode(i).outputs.at(0)->eIndex;
    			//cout << "pos: " << fixed->outputNode(i).outputs.at(0)->eIndex << endl;
    			if (notChange.count(pos) > 0) continue;
    			tempNodes.push_back(&fixed->outputNode(i));
    			notChange.insert(pos);
    			oneChange.insert(pos);
    			// Place all CAS XOR output signals right above d_i in the order before iterating through replacements.
    			//output = edgeName->eIndex;
    			output = fixed->outputs.at(0)->eIndex;
    			input2 = pos;
    			idxout = output;
    			idx2 = input2;
   				posOut = posOfIndex[idxout];
    			posIn2 = posOfIndex[idx2];
    			//cout << "shifting" << endl;
    			Verifizierer::arrayShifting(permu, permuSize, posOut, -1, posIn2, idxout, -1, idx2, posOfIndex);
    		}
    	}
    	//cout << "temps: " << endl;
    	//for (auto& temps: tempNodes) cout << temps->name << endl;
    	//cout << "temps end " << endl;
    	//cout << "tempNodes size: " << tempNodes.size() << endl;
    	nextNodes = tempNodes;
    	tempNodes.clear();
    }    
    /*
    cout << "permu after first step: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
    //*/
    //*
    // Shift q_i to beginning of order.
   /*
    int qposition = 0;
    for (auto& elem: this->circuit.edges) {  // For Q bits.
    	if (elem.first.find("Q") != string::npos) {
    		posOut = qposition;
    		idxout = 0;
    		pos = elem.second.eIndex;
    		idx2 = pos;
    		posIn2 = posOfIndex[idx2];
    		//Verifizierer::arrayShifting(permu, permuSize, posOut, -1, posIn2, idxout, -1, idx2, posOfIndex);
    	}
    }
    */
    /*
    cout << "permu after first step: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
    //*/
    //*
    // Start iterating through replacements and changing permu array to obtain good variable order BEFORE composing the BDD.
	
	vector<replacementOld> replacementList; // = this->makeReplaceList(this->createReplaceOrder());
	
	for (auto& elem: replacementList) {
		if (elem.type == 0) continue; 
		//counter++;
		//if (counter > 5) break;
		// Reset values.
		output = -1;
    	input1 = -1;
    	input2 = -1;
		idxout = -1;
    	idx1 = -1;
    	idx2 = -1;
		posOut = -1;
    	posIn1 = -1;
    	posIn2 = -1;

    	//cout << "elem type: " << elem.type << endl;
    	//cout << "elem output: " << elem.output << endl;
    	//cout << "elem in1: " << elem.inputOne << endl;
    	//cout << "elem in2: " << elem.inputTwo << endl;
    	
    	// Initialize all needed values.
		output = elem.output;
    	input1 = elem.inputOne;
    	if (elem.type != vp::Node::NOT && elem.type != vp::Node::BUFFER) input2 = elem.inputTwo;
    	idxout = output;
    	idx1 = input1;
    	if (input2 != -1) idx2 = input2;
    	posOut = posOfIndex[idxout];
    	posIn1 = posOfIndex[idx1];
    	if (idx2 != -1) posIn2 = posOfIndex[idx2];
    	
    	assert(posIn1 > -1);
    	assert(posOut > -1);
    	assert(posIn1 != posIn2);
    	assert(posOut < permuSize);
    	
    	//*  If one position is fixed, move other signal to its position in the order.
    	if (oneChange.count(idx1) > 0 && oneChange.count(idx2) == 0) {  // In1 is already at fixed position. Move In2 to In1.
    		posOut = posIn1;
    	} 
    	
    	if (oneChange.count(idx1) == 0 && oneChange.count(idx2) > 0) { // In2 is already at fixed position. Move In1 to In2.
    		posOut = posIn2;
    	} 
    	//*/
    	
    	//*
    	// Check whether one or both inputs already been placed in the order. Dont change their order in this case.
    	if (notChange.count(idx1) > 0) {
    		//cout << "Not change detected. Skip." << endl;
    		posIn1 = -1;
    		//continue;
    	} else {  // The variable will get a position in this iteration. Add it to notChange list.
    		if (idx1 != -1) notChange.insert(idx1);
    	}
    	if (notChange.count(idx2) > 0) {
    		//cout << "Not change detected. Skip." << endl;
    		posIn2 = -1;	
    		//continue;
    	} else {  // The variable will get a position in this iteration. Add it to notChange list.
    		if (idx2 != -1) notChange.insert(idx2);
    	}
    	
    	if (posIn1 == -1 && posIn2 == -1) continue;  // Both variables already positioned. Go to next step. 
    	
    	
    	// Assure that in1 has the lower position. Change both, position and index variables.
    	if (posIn1 > posIn2) {  
    			tmpInt = posIn1;
    			posIn1 = posIn2;
    			posIn2 = tmpInt;
    			tmpInt = idx1;
    			idx1 = idx2;
    			idx2 = tmpInt;
    	}
    	//cout << "posOut: " << posOut << " posIn1: " << posIn1 << " posIn2: " << posIn2 <<  endl;
    	//cout << "indexOut: " << idxout << " indexIn1: " << idx1 <<  " indexIn2: " << idx2 << endl;
    	
    //	Verifizierer::arrayShifting(permu, permuSize, posOut, posIn1, posIn2, idxout, idx1, idx2, posOfIndex);  // Give variables new position.
    	
    }
    /*
    cout << "permu after backward traversal is: " << endl;
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "|" << permu[i];
    }
    cout << endl;
    //*/
    //*/
    delete[] posOfIndex;
    timePermu += clock() - tstartPermu; // Zeitmessung endet.
	timePermu = timePermu/CLOCKS_PER_SEC;
	cout << "Obtaining permutation needed time in sec. : " << timePermu << endl;
}

// _______________________________________________________________________________________________________________________________________
std::vector<mpz_class> Verifizierer::getCoefficientsForDC(string sig1, string sig2, string sig3) {
	std::vector<mpz_class> result;
	int e1 = this->circuit.edges.at(sig1).eIndex;
	int e2 = this->circuit.edges.at(sig2).eIndex;
	int e3 = this->circuit.edges.at(sig3).eIndex;
	Monom2* curr = 0;
	Monom2 e1Mon(e1);
	Monom2 e2Mon(e2);
	Monom2 e3Mon(e3);
	//cout << "e1 monom is: " << e1Mon << endl;
	curr = this->poly.findExact(e1Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e2Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e3Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	Monom2 e12Mon(e1, e2);
	Monom2 e13Mon(e1, e3);
	Monom2 e23Mon(e2, e3);
	curr = this->poly.findExact(e12Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e13Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e23Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	int e123Vars[3] = {e1, e2, e3};
	Monom2 e123Mon(e123Vars, 3);
	curr = this->poly.findExact(e123Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	return result;
}

// _______________________________________________________________________________________________________________________________________
std::vector<mpz_class> Verifizierer::getCoefficientsForDC4(string sig1, string sig2, string sig3, string sig4) {
	std::vector<mpz_class> result;
	int e1 = this->circuit.edges.at(sig1).eIndex;
	int e2 = this->circuit.edges.at(sig2).eIndex;
	int e3 = this->circuit.edges.at(sig3).eIndex;
	int e4 = this->circuit.edges.at(sig4).eIndex;
	Monom2* curr = 0;
	Monom2 e1Mon(e1);
	Monom2 e2Mon(e2);
	Monom2 e3Mon(e3);
	Monom2 e4Mon(e4);
	//cout << "e1 monom is: " << e1Mon << endl;
	curr = this->poly.findExact(e1Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e2Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e3Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e4Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	Monom2 e12Mon(e1, e2);
	Monom2 e13Mon(e1, e3);
	Monom2 e14Mon(e1, e4);
	Monom2 e23Mon(e2, e3);
	Monom2 e24Mon(e2, e4);
	Monom2 e34Mon(e3, e4);
	curr = this->poly.findExact(e12Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e13Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e14Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e23Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e24Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e34Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	int e123Vars[3] = {e1, e2, e3};
	Monom2 e123Mon(e123Vars, 3);
	int e124Vars[3] = {e1, e2, e4};
	Monom2 e124Mon(e124Vars, 3);
	int e134Vars[3] = {e1, e3, e4};
	Monom2 e134Mon(e134Vars, 3);
	int e234Vars[3] = {e2, e3, e4};
	Monom2 e234Mon(e234Vars, 3);
	curr = this->poly.findExact(e123Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e124Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e134Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	curr = this->poly.findExact(e234Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	int e1234Vars[4] = {e1, e2, e3, e4};
	Monom2 e1234Mon(e1234Vars, 4);
	curr = this->poly.findExact(e1234Mon);
	if (curr != 0) result.push_back(curr->getFactor());
	else result.push_back(0);
	return result;
}

// _______________________________________________________________________________________________________________________________________
void Verifizierer::buildBDDConstraint(std::pair<std::pair<std::vector<varIndex>, std::vector<varIndex>>, std::vector<varIndex>> indices, std::vector<replacementOld> replacements) {
	int countR = indices.first.first.size();
	int countD = indices.first.second.size();
	int countS = indices.second.size();
	int sum = countR + countD + countS;
	cout << "R: " << countR << ", D: " << countD << ", S: " << countS << endl;
	
	// Initialize Dd manager.	
	DdManager *gbm; // Global BDD manager.
	gbm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0); // Initialize a new BDD manager.
	Cudd_AutodynDisable(gbm);  // Disable automatic dynamic reordering.
	//Cudd_AutodynEnable(gbm, CUDD_REORDER_SYMM_SIFT);
	DdNode *bdd, *tmp_neg, *tmp, *tmp2;  // Later used DD nodes.
	int numVars = this->circuit.maxEdgeIndex;
	DdNode* vars[numVars];  // Used variables. One for every signal in the circuit.
	bdd = Cudd_ReadOne(gbm); /*Returns the logic one constant of the manager*/
	Cudd_Ref(bdd); /*Increases the reference count of a node*/
	
	cout << "Initializing BDD completed." << endl;
	
	// Define variables for D and R^n+1 and additional for R_0 for BDD.
	for (int i = 0; i < countR; ++i) {
   		if (i >= countD) {  // R variable without corresponding D variable.
   			vars[countS + i] = Cudd_bddNewVarAtLevel(gbm, countR - 1 - i + countD);
   			bddIndex.insert(pair<int,int>(indices.first.first.at(i), countS + i));  // Map position of variable in vars vector to edgeIndex of the circuit.
   			if (i != countR - 1) {  // R_0 is one shorter than R_n1.
   				vars[countS + i + countD + 1] = Cudd_bddNewVarAtLevel(gbm,countR - 2 - i);  // r_i^(0).
   				bddIndex.insert(pair<int,int>(indices.second.at(countS - i - 1), countS + i + countD + 1));
   			}
   		} else {
   			vars[3*i] = Cudd_bddNewVarAtLevel(gbm,sum - 3*i - 3);  // r_i^(n+1).
   			bddIndex.insert(pair<int,int>(indices.first.first.at(i), 3*i));//  Map position of variable to edgeIndex of the circuit.
   			vars[3*i+1] = Cudd_bddNewVarAtLevel(gbm,sum - 3*i - 2);  // r_i^(0). Only r_n-1 to r_2n-3 needed for comparison with d.
   			bddIndex.insert(pair<int,int>(indices.second.at(countD + i), 3*i+1));
   			vars[3*i+2] = Cudd_bddNewVarAtLevel(gbm,sum - 3*i - 1);  // d_i.
   			bddIndex.insert(pair<int,int>(indices.first.second.at(i), 3*i+2));
   		}
    }
    //cout << "Hier müsste d0 index = 15 stehen:" << Cudd_NodeReadIndex(vars[2]) << endl;
    //cout << "und müsste level = " << sum - 1 << " haben, hat aber: " << Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[2])) << endl; 
    	
    cout << "Defining r_n1 and d variables completed." << endl;
    	
    // Create starting BDD for the contraint R^(n+1) < D.     	
    for (int i = 0; i < countR; i++) {
    	if (i < countD) {  // Steps with r_i and d_i.
    		if (i == 0) {  // First constraint L_0 has no XNOR and no conjunction with previous constraint.
    			tmp_neg = Cudd_Not(vars[i]);  // NOT of r_0.
    			//tmp = vars[i+2];  // Identity of d_0.
    			tmp2 = Cudd_bddAnd(gbm, tmp_neg, bdd); // NOT r_0.
    			Cudd_Ref(tmp2); 
    			tmp = Cudd_bddAnd(gbm, vars[i+2], tmp2);  // AND of !r_0 and d_0.
    			Cudd_Ref(tmp);
    			Cudd_RecursiveDeref(gbm,bdd);
    			Cudd_RecursiveDeref(gbm,tmp2);
    			//Cudd_RecursiveDeref(gbm,tmp_neg);
    			bdd = tmp;
    		} else {  // Next constraints L_i. 
    			//*
    			tmp_neg = Cudd_Not(vars[3*i]);  // NOT of r_i.
    			//tmp = vars[2*i+1];  // Identity of d_i.
    			tmp = Cudd_bddAnd(gbm, tmp_neg, vars[3*i+2]); // AND of !r_i and d_i.
    			Cudd_Ref(tmp);
    			//Cudd_RecursiveDeref(gbm,tmp_neg);
    			tmp2 = Cudd_bddXnor(gbm, vars[3*i], vars[3*i+2]);  // Equality: r_i = d_i.
    			Cudd_Ref(tmp2);
    			tmp_neg = Cudd_bddAnd(gbm, tmp2, bdd);  // AND of (r_i=d_i) and L_(i-1).
    			Cudd_Ref(tmp_neg);
    			Cudd_RecursiveDeref(gbm,bdd);
    			Cudd_RecursiveDeref(gbm,tmp2);
    			bdd = tmp_neg;
    			tmp_neg = Cudd_bddOr(gbm, tmp, bdd);  // OR of (r_i=d_i) and L_(i-1) and tmp.
    			Cudd_Ref(tmp_neg);
    			Cudd_RecursiveDeref(gbm, bdd);
    			Cudd_RecursiveDeref(gbm, tmp);
    			bdd = tmp_neg;
    			//*/
    		}
		} else {  // Steps with only r_i and no d_i anymore.
			tmp_neg = Cudd_Not(vars[countS + i]);  // NOT of r_i.
			tmp = Cudd_bddAnd(gbm, tmp_neg, bdd); // AND of !r_i and L_(i-1).
			Cudd_Ref(tmp);
    		Cudd_RecursiveDeref(gbm,bdd);
    		//Cudd_RecursiveDeref(gbm,tmp_neg);
    		bdd = tmp;
		} 
    }  	
    	
    // Fill other signals into the vars array.
    int step = 0;
	for (auto& elem: this->circuit.edges) {
		if (elem.first.find("R_n1") != string::npos || elem.first.find("D") != string::npos || elem.first.find("R_0") != string::npos) {
   			// Variable already created and inserted into map in previous step.
   		} else {
   			//cout << "Elem: " << elem.first.name << endl;
   			vars[sum + step] = Cudd_bddNewVarAtLevel(gbm, sum + step);  // Insert new variable for internal signal.
   			bddIndex.insert(pair<int,int>(elem.second.eIndex, sum + step));//  Map position of variable in vars array to edgeIndex of circuit.
   			step++;
   		}
	}    
    cout << "Building starting constraint BDD finished." << endl;
    
    /*
    for (auto& elem: bddIndex) {
    	cout << elem.first << " --> " << Cudd_NodeReadIndex(vars[elem.second]) << endl;
    }
    */
    
    cout << "From here obtaining permutation starts." << endl;
    	
    double timePermu, tstartPermu;
    timePermu = 0.0;
    tstartPermu = clock();	
    	
    cout << "BDD TEST -1" << endl;

    //*
    // Create array for permutation. 
    int maxOrder = 0;
    for (size_t i = 0; i < numVars; i++) {
    	if (Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[i])) > maxOrder) maxOrder = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[i]));
    }

    cout << "BDD TEST0" << endl;

    //cout << "maxOrder: " << maxOrder << endl;
    int permuSize = maxOrder + 1;
    int* permu = new int[permuSize];  // Array to save variable order.
    for (size_t i = 0; i < permuSize; i++) {
    	permu[i] = 0;
    }
    for (size_t i = 0; i < numVars; i++) {
    	permu[Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[i]))] = Cudd_NodeReadIndex(vars[i]);
    }

    cout << "BDD TEST1" << endl;

    /*	
    for (size_t i = 0; i < maxOrder+1; i++) {
    	cout << "Permu of " << i << " is: " << permu[i];
    	// R_0
    	if (permu[i] == 20) cout << " <- R_0[5]";
    	if (permu[i] == 17) cout << " <- R_0[4]";
    	if (permu[i] == 14) cout << " <- R_0[3]";
    	if (permu[i] == 23) cout << " <- R_0[2]";
    	if (permu[i] == 25) cout << " <- R_0[1]";
    	if (permu[i] == 27) cout << " <- R_0[0]";
    	// D
    	if (permu[i] == 21) cout << " <- D[2]";
    	if (permu[i] == 18) cout << " <- D[1]";
    	if (permu[i] == 15) cout << " <- D[0]";
    	// Q
    	if (permu[i] == 32) cout << " <- Q[3]";
    	if (permu[i] == 31) cout << " <- Q[2]";
    	if (permu[i] == 30) cout << " <- Q[1]";
    	if (permu[i] == 29) cout << " <- Q[0]";
    	// R_5
    	if (permu[i] == 28) cout << " <- R_n1[6]";
    	if (permu[i] == 26) cout << " <- R_n1[5]";
    	if (permu[i] == 24) cout << " <- R_n1[4]";
    	if (permu[i] == 22) cout << " <- R_n1[3]";
    	if (permu[i] == 19) cout << " <- R_n1[2]";
    	if (permu[i] == 16) cout << " <- R_n1[1]";
    	if (permu[i] == 13) cout << " <- R_n1[0]";
    	// R_1
    	if (permu[i] == 33) cout << " <- R_1[0]";
    	if (permu[i] == 144) cout << " <- R_1[1]";
    	if (permu[i] == 229) cout << " <- R_1[2]";
    	if (permu[i] == 240) cout << " <- R_1[3]";
    	if (permu[i] == 251) cout << " <- R_1[4]";
    	if (permu[i] == 262) cout << " <- R_1[5]";
    	if (permu[i] == 273) cout << " <- R_1[6]";
    	// R_2
    	if (permu[i] == 250) cout << " <- R_2[0]";
    	if (permu[i] == 252) cout << " <- R_2[1]";
    	if (permu[i] == 253) cout << " <- R_2[2]";
    	if (permu[i] == 254) cout << " <- R_2[3]";
    	if (permu[i] == 255) cout << " <- R_2[4]";
    	if (permu[i] == 256) cout << " <- R_2[5]";
    	if (permu[i] == 257) cout << " <- R_2[6]";
    	// R_3
    	if (permu[i] == 298) cout << " <- R_3[0]";
    	if (permu[i] == 299) cout << " <- R_3[1]";
    	if (permu[i] == 300) cout << " <- R_3[2]";
    	if (permu[i] == 301) cout << " <- R_3[3]";
    	if (permu[i] == 302) cout << " <- R_3[4]";
    	if (permu[i] == 303) cout << " <- R_3[5]";
    	if (permu[i] == 304) cout << " <- R_3[6]";
    	// R_4
    	if (permu[i] == 72) cout << " <- R_4[0]";
    	if (permu[i] == 73) cout << " <- R_4[1]";
    	if (permu[i] == 74) cout << " <- R_4[2]";
    	if (permu[i] == 75) cout << " <- R_4[3]";
    	if (permu[i] == 76) cout << " <- R_4[4]";
    	if (permu[i] == 78) cout << " <- R_4[5]";
    	if (permu[i] == 79) cout << " <- R_4[6]";
    	cout << endl;
    }
    //*/
    
    // Make except set containing indices of D and R_0, because their relative order should not be changed. 
    std::set<int> notChange;
    int pos;
    for (auto& elem: indices.first.second) {  // For D.
    	pos = bddIndex.at(elem);
    	notChange.insert(Cudd_NodeReadIndex(vars[pos]));
    }
    for (auto& elem: indices.second) {  // For R_0.
    	pos = bddIndex.at(elem);
    	notChange.insert(Cudd_NodeReadIndex(vars[pos]));
    }   
    for (auto& elem: this->circuit.edges) {  // For Q bits.
    	if (elem.first.find("Q") != string::npos) {
    		pos = bddIndex.at(elem.second.eIndex);
    		notChange.insert(Cudd_NodeReadIndex(vars[pos]));
    		//cout << "Q index is: " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    }
       
    cout << "BDD TEST2" << endl;
    //return;
    	
    /* Show index numbers of Divisor D and other primary signals.
    for (auto& elem: this->circuit.edgeIndex) {
    	if (elem.first.name.find("D") != string::npos) {
    		cout << "D index is: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	//*
    	if (elem.first.name.find("R_0") != string::npos) {
    		cout << "R_0 index of: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	if (elem.first.name.find("R_n1") != string::npos) {
    		cout << "R_n1 index is: " << elem.first.name << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	if (elem.first.name.find("Q") != string::npos) {
    		cout << "Q index is: " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	
    }//*/
    
    int* posOfIndex = new int[permuSize];  // Save new positions of index i at array position i.
    for (size_t i = 0; i < permuSize; i++) {
    	//cout << "Permu[" << i << "] is " << permu[i] << "p" << endl;
    	posOfIndex[permu[i]] = i;
    }
    
    /*
    for (size_t i = 0; i < permuSize; i++) {
    	cout << "posOfIndex[" << i << "] is " << posOfIndex[i] << endl;
    }
    */	
    cout << "BDD before shuffle has size " << Cudd_ReadNodeCount(gbm) << endl;
    
    // Variables used later.
    int input1 = -1;
    int input2 = -1;
    int output = -1;
    int posOut = -1;
    int posIn1 = -1;
    int posIn2 = -1;
    int idxout = -1;
    int idx1 = -1;
    int idx2 = -1;
    int tmpInt;
    int counter = 0;	
    
    std::set<int> oneChange;  // Set of variables which if already fixed should pull their partner to themself in the order.
    vp::Edge* edgeName;
    //*
    //EXTRA: Fix position of gate which is successor of d_i signals. 
    for (auto& elem: this->circuit.nodes) {
    	if (elem.name.find("D") != string::npos) {
    		//cout << "D node found: " << elem.name << endl;
    		edgeName = &*elem.outputs.at(0);
    		//cout << "EdgeName: " << edgeName << endl;
    		for (size_t i = 0; i < elem.getOutputsCount(); i++) {
    			//cout << "successor is: " << elem.outputNode(i).name << endl;
    			//cout << "output edge index is:" << this->circuit.edgeIndex.at(*elem.outputNode(i).outputs.at(0)) << endl;
    			pos = bddIndex.at(elem.outputNode(i).outputs.at(0)->eIndex);
    			notChange.insert(Cudd_NodeReadIndex(vars[pos]));
    			oneChange.insert(Cudd_NodeReadIndex(vars[pos]));
    			// Place all CAS XOR output signals right above d_i in the order before iterating through replacements.
    			//*
    			output = bddIndex.at(edgeName->eIndex);
    			//input1 = bddIndex.at(elem.inputOne);
    			input2 = pos;
    			idxout = Cudd_NodeReadIndex(vars[output]);
    			//idx1 = Cudd_NodeReadIndex(vars[input1]);
    			idx2 = Cudd_NodeReadIndex(vars[input2]);
   	 			posOut = posOfIndex[idxout];
    			//posIn1 = posOfIndex[idx1];
    			posIn2 = posOfIndex[idx2];
    			Verifizierer::arrayShifting(permu, permuSize, posOut, -1, posIn2, idxout, -1, idx2, posOfIndex);
    			//*/
    		}
    	}
    }
    //*/
    
    // Shift q_i to beginning of order.
    int qposition = 0;
    for (auto& elem: this->circuit.edges) {  // For Q bits.
    	if (elem.first.find("Q") != string::npos) {
    		posOut = qposition;
    		idxout = 0;
    		pos = bddIndex.at(elem.second.eIndex);
    		idx2 = Cudd_NodeReadIndex(vars[pos]);
    		posIn2 = posOfIndex[idx2];
    		Verifizierer::arrayShifting(permu, permuSize, posOut, -1, posIn2, idxout, -1, idx2, posOfIndex);
    	}
    }
    
    // Start iterating through replacements and changing permu array to obtain good variable order BEFORE composing the BDD.
	for (auto& elem: replacements) {
		//counter++;
		//if (counter > 5) break;
		// Reset values.
		output = -1;
    	input1 = -1;
    	input2 = -1;
		idxout = -1;
    	idx1 = -1;
    	idx2 = -1;
		posOut = -1;
    	posIn1 = -1;
    	posIn2 = -1;
    	
    	// Initialize all needed values.
//		output = bddIndex.at(elem.output);
//    	input1 = bddIndex.at(elem.inputOne);
//    	if (elem.type != vp::Node::NOT && elem.type != vp::Node::BUFFER) input2 = bddIndex.at(elem.inputTwo);
    	idxout = Cudd_NodeReadIndex(vars[output]);
    	idx1 = Cudd_NodeReadIndex(vars[input1]);
    	if (input2 != -1) idx2 = Cudd_NodeReadIndex(vars[input2]);
    	posOut = posOfIndex[idxout];
    	posIn1 = posOfIndex[idx1];
    	if (idx2 != -1) posIn2 = posOfIndex[idx2];
    	
    	assert(posIn1 > -1);
    	assert(posOut > -1);
    	assert(posIn1 != posIn2);
    	assert(posOut < permuSize);
    	
    	//*  If one position is fixed, move other signal to its position in the order.
    	if (oneChange.count(idx1) > 0 && oneChange.count(idx2) == 0) {  // In1 is already at fixed position. Move In2 to In1.
    		posOut = posIn1;
    	} 
    	
    	if (oneChange.count(idx1) == 0 && oneChange.count(idx2) > 0) { // In2 is already at fixed position. Move In1 to In2.
    		posOut = posIn2;
    	} 
    	//*/
    	
    	
    	// Check whether one or both inputs already been placed in the order. Dont change their order in this case.
    	if (notChange.count(idx1) > 0) {
    		//cout << "Not change detected. Skip." << endl;
    		posIn1 = -1;
    		//continue;
    	} else {  // The variable will get a position in this iteration. Add it to notChange list.
    		if (idx1 != -1) notChange.insert(idx1);
    	}
    	if (notChange.count(idx2) > 0) {
    		//cout << "Not change detected. Skip." << endl;
    		posIn2 = -1;	
    		//continue;
    	} else {  // The variable will get a position in this iteration. Add it to notChange list.
    		if (idx2 != -1) notChange.insert(idx2);
    	}
    	
    	if (posIn1 == -1 && posIn2 == -1) continue;  // Both variables already positioned. Go to next step. 
    	
    	
    	// Assure that in1 has the lower position. Change both, position and index variables.
    	if (posIn1 > posIn2) {  
    			tmpInt = posIn1;
    			posIn1 = posIn2;
    			posIn2 = tmpInt;
    			tmpInt = idx1;
    			idx1 = idx2;
    			idx2 = tmpInt;
    	}
    		
    	//cout << "posOut: " << posOut << " posIn1: " << posIn1 << " posIn2: " << posIn2 <<  endl;
    	//cout << "indexOut: " << idxout << " indexIn1: " << idx1 <<  " indexIn2: " << idx2 << endl;
    	
    	Verifizierer::arrayShifting(permu, permuSize, posOut, posIn1, posIn2, idxout, idx1, idx2, posOfIndex);  // Give variables new position.
    	/*
    	cout << "CHANGED >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
		for (size_t i = 0; i < maxOrder+1; i++) {
    		cout << " Changed Permu[" << i << "] is " << permu[i] << "pm";
    		// R_0
    	if (permu[i] == 20) cout << " <- R_0[5]";
    	if (permu[i] == 17) cout << " <- R_0[4]";
    	if (permu[i] == 14) cout << " <- R_0[3]";
    	if (permu[i] == 23) cout << " <- R_0[2]";
    	if (permu[i] == 25) cout << " <- R_0[1]";
    	if (permu[i] == 27) cout << " <- R_0[0]";
    	// D
    	if (permu[i] == 21) cout << " <- D[2]";
    	if (permu[i] == 18) cout << " <- D[1]";
    	if (permu[i] == 15) cout << " <- D[0]";
    	// Q
    	if (permu[i] == 32) cout << " <- Q[3]";
    	if (permu[i] == 31) cout << " <- Q[2]";
    	if (permu[i] == 30) cout << " <- Q[1]";
    	if (permu[i] == 29) cout << " <- Q[0]";
    	// R_5
    	if (permu[i] == 28) cout << " <- R_n1[6]";
    	if (permu[i] == 26) cout << " <- R_n1[5]";
    	if (permu[i] == 24) cout << " <- R_n1[4]";
    	if (permu[i] == 22) cout << " <- R_n1[3]";
    	if (permu[i] == 19) cout << " <- R_n1[2]";
    	if (permu[i] == 16) cout << " <- R_n1[1]";
    	if (permu[i] == 13) cout << " <- R_n1[0]";
    	// R_1
    	if (permu[i] == 33) cout << " <- R_1[0]";
    	if (permu[i] == 144) cout << " <- R_1[1]";
    	if (permu[i] == 229) cout << " <- R_1[2]";
    	if (permu[i] == 240) cout << " <- R_1[3]";
    	if (permu[i] == 251) cout << " <- R_1[4]";
    	if (permu[i] == 262) cout << " <- R_1[5]";
    	if (permu[i] == 273) cout << " <- R_1[6]";
    	// R_2
    	if (permu[i] == 250) cout << " <- R_2[0]";
    	if (permu[i] == 252) cout << " <- R_2[1]";
    	if (permu[i] == 253) cout << " <- R_2[2]";
    	if (permu[i] == 254) cout << " <- R_2[3]";
    	if (permu[i] == 255) cout << " <- R_2[4]";
    	if (permu[i] == 256) cout << " <- R_2[5]";
    	if (permu[i] == 257) cout << " <- R_2[6]";
    	// R_3
    	if (permu[i] == 298) cout << " <- R_3[0]";
    	if (permu[i] == 299) cout << " <- R_3[1]";
    	if (permu[i] == 300) cout << " <- R_3[2]";
    	if (permu[i] == 301) cout << " <- R_3[3]";
    	if (permu[i] == 302) cout << " <- R_3[4]";
    	if (permu[i] == 303) cout << " <- R_3[5]";
    	if (permu[i] == 304) cout << " <- R_3[6]";
    	// R_4
    	if (permu[i] == 72) cout << " <- R_4[0]";
    	if (permu[i] == 73) cout << " <- R_4[1]";
    	if (permu[i] == 74) cout << " <- R_4[2]";
    	if (permu[i] == 75) cout << " <- R_4[3]";
    	if (permu[i] == 76) cout << " <- R_4[4]";
    	if (permu[i] == 78) cout << " <- R_4[5]";
    	if (permu[i] == 79) cout << " <- R_4[6]";
    	cout << endl;
    	}
    	//*/
    }
    
    /*		
	cout << "CHANGED >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	for (size_t i = 0; i < maxOrder+1; i++) {
    	// R_0
    	if (permu[i] == 20) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[5]" << endl;
    	if (permu[i] == 17) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[4]" << endl;
    	if (permu[i] == 14) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[3]" << endl;
    	if (permu[i] == 23) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[2]" << endl;
    	if (permu[i] == 25) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[1]" << endl;
    	if (permu[i] == 27) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_0[0]" << endl;
    	// D
    	if (permu[i] == 21) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- D[2]" << endl;
    	if (permu[i] == 18) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- D[1]" << endl;
    	if (permu[i] == 15) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- D[0]" << endl;
    	// Q
    	if (permu[i] == 32) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- Q[3]" << endl;
    	if (permu[i] == 31) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- Q[2]" << endl;
    	if (permu[i] == 30) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- Q[1]" << endl;
    	if (permu[i] == 29) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- Q[0]" << endl;
    	// R_5
    	if (permu[i] == 28) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[6]" << endl;
    	if (permu[i] == 26) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[5]" << endl;
    	if (permu[i] == 24) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[4]" << endl;
    	if (permu[i] == 22) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[3]" << endl;
    	if (permu[i] == 19) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[2]" << endl;
    	if (permu[i] == 16) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[1]" << endl;
    	if (permu[i] == 13) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_n1[0]" << endl;
    	// R_1
    	if (permu[i] == 33) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[0]" << endl;
    	if (permu[i] == 144) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[1]" << endl;
    	if (permu[i] == 229) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[2]" << endl;
    	if (permu[i] == 240) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[3]" << endl;
    	if (permu[i] == 251) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[4]" << endl;
    	if (permu[i] == 262) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[5]" << endl;
    	if (permu[i] == 273) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_1[6]" << endl;
    	// R_2
    	if (permu[i] == 250) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[0]" << endl;
    	if (permu[i] == 252) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[1]" << endl;
    	if (permu[i] == 253) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[2]" << endl;
    	if (permu[i] == 254) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[3]" << endl;
    	if (permu[i] == 255) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[4]" << endl;
    	if (permu[i] == 256) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[5]" << endl;
    	if (permu[i] == 257) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_2[6]" << endl;
    	// R_3
    	if (permu[i] == 298) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[0]" << endl;
    	if (permu[i] == 299) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[1]" << endl;
    	if (permu[i] == 300) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[2]" << endl;
    	if (permu[i] == 301) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[3]" << endl;
    	if (permu[i] == 302) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[4]" << endl;
    	if (permu[i] == 303) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[5]" << endl;
    	if (permu[i] == 304) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_3[6]" << endl;
    	// R_4
    	if (permu[i] == 72) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[0]" << endl;
    	if (permu[i] == 73) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[1]" << endl;
    	if (permu[i] == 74) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[2]" << endl;
    	if (permu[i] == 75) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[3]" << endl;
    	if (permu[i] == 76) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[4]" << endl;
    	if (permu[i] == 78) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[5]" << endl;
    	if (permu[i] == 79) cout << "Changed Permu[" << i << "] is " << permu[i] << "pm" << " <- R_4[6]" << endl;
    }
    //*/
    
    timePermu += clock() - tstartPermu; // Zeitmessung endet.
	timePermu = timePermu/CLOCKS_PER_SEC;
	cout << "Obtaining permutation needed time in sec. : " << timePermu << endl;
    
    // Now change variable order according to created array.
    Cudd_ShuffleHeap(gbm, permu);
    cout << "BDD after shuffle has size " << Cudd_ReadNodeCount(gbm) << endl;
    
    // Free allocated memory.
    delete[] permu;
    delete[] posOfIndex;
    
    //return;
    
    int max = 0;
    step = 0;
    input1 = -1;
    input2 = -1;
    output = -1;
    posOut = -1;
    posIn1 = -1;
    posIn2 = -1;
    cout << "Begin with replacements. Size of BDD at start: " << Cudd_ReadNodeCount(gbm) << endl;
    
    double time1, tstart;
    time1 = 0.0;
    tstart = clock();
    for(auto& elem: replacements) {
    	//break;
    	posOut = -1;
    	posIn1 = -1;
    	posIn2 = -1;
    	output = bddIndex.at(elem.output);
    	input1 = bddIndex.at(elem.inputOne);
    	if (elem.type != vp::Node::NOT && elem.type != vp::Node::BUFFER) input2 = bddIndex.at(elem.inputTwo);

		// Save positions of the variabes in the order.
		posOut = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[output]));
		posIn1 = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[input1]));
		if (elem.type != vp::Node::NOT && elem.type != vp::Node::BUFFER) posIn2 = Cudd_ReadPerm(gbm, Cudd_NodeReadIndex(vars[input2]));

    	// Case destinction on type of replacement.
    	if (elem.type == vp::Node::AND) {
			tmp = Cudd_bddAnd(gbm, vars[input1], vars[input2]);
		} else if (elem.type == vp::Node::XOR)  {
			tmp = Cudd_bddXor(gbm, vars[input1], vars[input2]);
		} else if (elem.type == vp::Node::OR) {
			tmp = Cudd_bddOr(gbm, vars[input1], vars[input2]);
		} else if (elem.type == vp::Node::NOT) {
			tmp = Cudd_Not(vars[input1]);
		} else if (elem.type == vp::Node::BUFFER) {
			tmp = vars[input1];
		}
    	Cudd_Ref(tmp);
    	tmp_neg = Cudd_bddCompose(gbm, bdd, tmp, Cudd_NodeReadIndex(vars[output]));
    	Cudd_Ref(tmp_neg);
    	Cudd_RecursiveDeref(gbm,tmp);
    	Cudd_RecursiveDeref(gbm,bdd);
    	bdd = tmp_neg;
    	if (max < Cudd_ReadNodeCount(gbm)) max = Cudd_ReadNodeCount(gbm);
    	//cout << "BDD after step " << step << " has size: " << Cudd_ReadNodeCount(gbm) << " with o, in1, in2: " << elem.output << ", " << elem.inputOne << ", " << elem.inputTwo << " and type: " << elem.type << endl;
    	step++;
    	int diff = step;
    	if (Cudd_ReadNodeCount(gbm) > 500000) {
    		Cudd_AutodynEnable(gbm, CUDD_REORDER_SYMM_SIFT);
    		/*if (step - diff > 5000) {
    			Cudd_AutodynDisable(gbm);
    			Cudd_ShuffleHeap(gbm, permu);
    		}*/
    	}
    }
    time1 += clock() - tstart; // Zeitmessung endet.
	time1 = time1/CLOCKS_PER_SEC;
	cout << "BDD replacements needed time in sec. : " << time1 << endl;
    	
    /* Show index numbers of Divisor D and other primary signals.
    for (auto& elem: this->circuit.edgeIndex) {
    	if (elem.first.name.find("D") != string::npos) {
    		cout << "D index of: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	//*
    	if (elem.first.name.find("R_0") != string::npos) {
    		cout << "R_0 index of: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	if (elem.first.name.find("R_n1") != string::npos) {
    		cout << "R_n1 index of: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	if (elem.first.name.find("Q") != string::npos) {
    		cout << "Q index of: " << elem.first.name << " is " << Cudd_NodeReadIndex(vars[bddIndex.at(elem.second)]) << endl;
    	}
    	
    }//*/
    	
    // Build BDD C of R^(0) < D*2^(n-1) constraint, compute !C+B BDD where B is the final BDD after replacements. In correct case, resulting BDD should be implementing 1-function. 
		
	DdNode *bdd_c, *tmp_neg_c, *tmp_c, *tmp2_c;  // Later used DD nodes.
	bdd_c = Cudd_ReadOne(gbm); /*Returns the logic one constant of the manager*/
	Cudd_Ref(bdd_c); /*Increases the reference count of a node*/
		
	// Create BDD "bdd_c" for contraint 0 <= R^(0) < D * 2^(n-1).     	
    for (int i = 0; i < countD; i++) {
    	if (i == 0) {  // First constraint L_0 has no XNOR and no conjunction with previous constraint. 
    		tmp_neg_c = Cudd_Not(vars[i+1]);  // NOT of r_n-1.
    		//tmp = vars[i+2];  // Identity of d_0.
    		tmp2_c = Cudd_bddAnd(gbm, tmp_neg_c, bdd_c); // NOT r_n-1.
    		Cudd_Ref(tmp2_c); 
    		tmp_c = Cudd_bddAnd(gbm, vars[i+2], tmp2_c);  // AND of !r_n-1 and d_0.
    		Cudd_Ref(tmp_c);
    		Cudd_RecursiveDeref(gbm,bdd_c);
    		Cudd_RecursiveDeref(gbm,tmp2_c);
    		//Cudd_RecursiveDeref(gbm,tmp_neg);
    		bdd_c = tmp_c;
    	} else {  // Next constraints L_i. 
    		//*
    		tmp_neg_c = Cudd_Not(vars[3*i+1]);  // NOT of r_(n-1+i).
    		//tmp = vars[2*i+1];  // Identity of d_i.
    		tmp_c = Cudd_bddAnd(gbm, tmp_neg_c, vars[3*i+2]); // AND of !r_(n-1+i) and d_i.
    		Cudd_Ref(tmp_c);
    		//Cudd_RecursiveDeref(gbm,tmp_neg);
    		tmp2_c = Cudd_bddXnor(gbm, vars[3*i+1], vars[3*i+2]);  // Equality: r_(n-1+i) = d_i.
    		Cudd_Ref(tmp2_c);
    		tmp_neg_c = Cudd_bddAnd(gbm, tmp2_c, bdd_c);  // AND of (r_(n-1+i)=d_i) and L_(i-1).
    		Cudd_Ref(tmp_neg_c);
    		Cudd_RecursiveDeref(gbm,bdd_c);
    		Cudd_RecursiveDeref(gbm,tmp2_c);
    		bdd_c = tmp_neg_c;
    		tmp_neg_c = Cudd_bddOr(gbm, tmp_c, bdd_c);  // OR of (r_(n-1+i)=d_i) and L_(i-1) and tmp.
    		Cudd_Ref(tmp_neg_c);
    		Cudd_RecursiveDeref(gbm, bdd_c);
    		Cudd_RecursiveDeref(gbm, tmp_c);
    		bdd_c = tmp_neg_c;
    		//*/
    	}
    }  	
    	
    //Cudd_ReduceHeap(gbm, CUDD_REORDER_SAME, 10);  // Reduce BDD size after replacements.
    	
    tmp_neg_c = Cudd_Not(bdd_c);  // Negation of C constraint.
    Cudd_Ref(tmp_neg_c);
    tmp_c = Cudd_bddOr(gbm, tmp_neg_c, bdd);  // !C + B
    Cudd_Ref(tmp_c);
    Cudd_RecursiveDeref(gbm, bdd);
    Cudd_RecursiveDeref(gbm, tmp_neg_c);
    bdd_c = tmp_c;
    
    //* Show informations about the BDD and draw it.
    cout << "Max. Knoten waren: " << max << endl;
    cout << "Anzahl Knoten am Ende: " << Cudd_ReadNodeCount(gbm) << endl;
    //cout << "Max. Knoten waren: " << Cudd_ReadPeakNodeCount(gbm) << endl;
    print_dd (gbm, bdd_c, sum, 3); // Print the dd to standard output 
    // Display the bdd informations and the .dot graphical view.
    /*
    bdd_c = Cudd_BddToAdd(gbm, bdd_c); // Convert BDD to ADD for display purpose. 
    char filename[30]; // Write .dot filename to a string 
    sprintf(filename, "./BDD/testconstraint.dot"); 
    write_dd(gbm, bdd_c, filename);  // Write the resulting cascade dd to a file.
    //*/
    // Quit Dd manager.
    Cudd_Quit(gbm);
}
 
	
// ___________________________________________________________________________________________________________________________________
void Verifizierer::arrayShifting(int* permu, int size, int posOut, int posIn1, int posIn2, int idxout, int idx1, int idx2, int* posOfIndex) {
	//Make some assertions for invalid inputs.
	assert(posOut < size && posIn1 < size && posIn2 < size);
	assert(posIn1 < posIn2);
	assert(posOut > -1 && posIn2 > -1);
	assert(idxout != idx1 && idxout != idx2 && idx1 != idx2);
	assert(posOut != posIn1 && posOut != posIn2);
	
	// Create temporary array which is copy of array to be changed.
	int* tempArr = new int[size];
	std::copy(permu, permu + size, tempArr);
	int j;
	if (posIn1 > -1 && posIn2 > -1) {  // Case with two changes.
		if (posOut < posIn1 && posOut < posIn2) {  // Both bigger than posOut. Keep everything below posOut unchanged.
			permu[posOut + 1] = idx1;  // Move idx1 and idx2 to new positions. Save this new positions in posOfIndex.
			posOfIndex[idx1] = posOut + 1;
			permu[posOut + 2] = idx2;
			posOfIndex[idx2] = posOut + 2;
			j = posOut + 1;
			for (int i = posOut + 3; i < posIn2 + 1; i++) {  // Change position of all higher contents by copying from tempArr.
				if (j == posIn1) j++;
				permu[i] = tempArr[j];  // Write index (obtained from tempArr[j]) into permu at position i.
				posOfIndex[tempArr[j]] = i;
				j++;
			}
		} else if (posOut > posIn1 && posOut > posIn2) {  // Both smaller than posOut. Keep everything above posOut unchanged.
			permu[posOut - 1] = idx2;  // Move idx1 and idx2 to new positions. This time right below posOut.
			posOfIndex[idx2] = posOut - 1;
			permu[posOut - 2] = idx1;
			posOfIndex[idx1] = posOut -2;
			j = posOut - 1;
			for (int i = posOut - 3; i > posIn1 - 1; i--) {  // Change position of all higher contents by copying from tempArr.
				if (j == posIn2) j--;
				permu[i] = tempArr[j];
				posOfIndex[tempArr[j]] = i;
				j--;
			}
		} else if (posOut > posIn1 && posOut < posIn2) {  // posOut is in between the other two.
			permu[posOut + 1] = idx2;  // Move idx1 and idx2 to new positions. This time right below posOut.
			posOfIndex[idx2] = posOut + 1;
			permu[posOut - 1] = idx1;
			posOfIndex[idx1] = posOut - 1;
			j = posOut - 1;
			for (int i = posOut - 2; i > posIn1 - 1; i--) {  // Change first position of items below posOut.
				permu[i] = tempArr[j];
				posOfIndex[tempArr[j]] = i;
				j--;
			}
			j = posOut + 1;
			for (int i = posOut + 2; i < posIn2 + 1; i++) {  // Change next position of items above posOut.
				permu[i] = tempArr[j];
				posOfIndex[tempArr[j]] = i;
				j++;
			}
		} else cout << "This case is not possible. ABORT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	} else {  // Case with only one input to change. Assure posIn1 is -1.
		assert(posIn1 == -1);
		if (posOut < posIn2) {
			permu[posOut + 1] = idx2;  // Move idx2 to new positions.
			posOfIndex[idx2] = posOut + 1;
			j = posOut + 1;
			for (int i = posOut + 2; i < posIn2 + 1; i++) {  // Change position of items above posOut.
				permu[i] = tempArr[j];
				posOfIndex[tempArr[j]] = i;
				j++;
			}
		} else {
			//cout << "Here." << endl;
			permu[posOut - 1] = idx2;  // Move idx2 to new positions.
			posOfIndex[idx2] = posOut - 1;
			j = posOut - 1;
			//cout << j << endl;
			//cout << tempArr[j] << endl;
			for (int i = posOut - 2; i > posIn2 - 1; i--) {  // Change first position of items below posOut.
				//cout << i << ", ";
				//cout << j << ", ";
				//cout << tempArr[j] << endl;
				permu[i] = tempArr[j];
				posOfIndex[tempArr[j]] = i;
				j--;
			}
		}
	}
	
	delete[] tempArr;
	return;	
}

//________________________________________________________________________________________________________________________________
/**
 * Print a dd summary
 * pr = 0 : prints nothing
 * pr = 1 : prints counts of nodes and minterms
 * pr = 2 : prints counts + disjoint sum of product
 * pr = 3 : prints counts + list of nodes
 * pr > 3 : prints counts + disjoint sum of product + list of nodes
 * @param the dd node
 */
void Verifizierer::print_dd (DdManager *gbm, DdNode *dd, int n, int pr ) {
    printf("DdManager nodes: %ld | ", Cudd_ReadNodeCount(gbm)); /*Reports the number of live nodes in BDDs and ADDs*/
    printf("DdManager vars: %d | ", Cudd_ReadSize(gbm) ); /*Returns the number of BDD variables in existence*/
    printf("DdManager reorderings: %d | ", Cudd_ReadReorderings(gbm) ); /*Returns the number of times reordering has occurred*/
    printf("DdManager memory: %ld \n", Cudd_ReadMemoryInUse(gbm) ); /*Returns the memory in use by the manager measured in bytes*/
    Cudd_PrintDebug(gbm, dd, n, pr);  // Prints to the standard output a DD and its statistics: number of nodes, number of leaves, number of minterms.
}

/**
 * Writes a dot file representing the argument DDs
 * @param the node object
 */
void Verifizierer::write_dd (DdManager *gbm, DdNode *dd, char* filename) {
    FILE *outfile; // output file pointer for .dot file
    outfile = fopen(filename,"w");
    if (outfile == NULL) cout << "NUll pointer entdeckt" << endl; 
    DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initialize the function array
    ddnodearray[0] = dd;

    Cudd_DumpDot(gbm, 1, ddnodearray, NULL, NULL, outfile); // dump the function to .dot file
    free(ddnodearray);
    fclose (outfile); // close the file */
}

