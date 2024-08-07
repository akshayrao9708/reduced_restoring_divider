/*
 * Verifizierer.h
 *
 *  Created on: 08.01.2019
 *      Author: Alexander Konrad
 */

#include <string>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <gmpxx.h>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>
#include "polynomial/MyList.h"
#include "circuit/Circuit.h"
#include "polynomial/Polynom.h"
#include "circuit/ParseError.h"
#include "circuit/Edge.h"
#include "math.h"
#include "time.h"
#include "others/util.h"
#include "cudd/cudd.h"
#include "simVectors/SimulationTable.h"
#include "miniSat/Solver.h"
#include "gurobi/gurobi_c++.h"
#include "helperStructs/input4dc.hpp"
#include "helperStructs/gendc.hpp"


#ifndef VERIFIZIERER_H_
#define VERIFIZIERER_H_

typedef int varIndex;

struct dc_cand{
string dc_sign;
int count_dc_match =0;
int size_sz=0;
};

struct replacementOld {
	vp::Node::NodeType type;
	varIndex output;
	varIndex inputOne;
	varIndex inputTwo;
};

struct replacement {
	varIndex sig;
	Polynom pol;
};

struct repr{
	//int index;
	vp::Edge* sig;
	bool state;
	bool tracked;
	bool activated;
	repr* next;
	repr* leaf;
	repr() {  // Initialize all pointers with NULL.
		sig = NULL;
		state = true;
		next = NULL;
		leaf = NULL;
		tracked = false;
		activated = false;
	}
	void initRepresentEntry(vp::Edge* currEdge) {  // Use for filling repr entry with correct values.
		this->sig = currEdge;
		this->state = true;
		this->tracked = false;
		this->activated = false;
		this->next = NULL;
		this->leaf = this;
	}
};

struct satPair{
	size_t lev1;
	size_t lev2;
	string e1;
	string e2;
	bool equi;
	satPair(size_t lvl1 = 0, size_t lvl2 = 0, string e1 = "", string e2 = "", bool equi = true)
	  : lev1(lvl1), lev2(lvl2), e1(e1), e2(e2), equi(equi)
	{}
	bool operator==(const satPair& pair) const{
		if (lev1 == pair.lev1) {
			if (lev2 == pair.lev2) {
				if (e1 == pair.e1) {
					if (e2 == pair.e2) {
						return true;
					} else return false;
				} else return false;
			} else return false;
		} else return false;
	}
	bool operator!=(const satPair& pair) const{
		///*
		if (lev1 == pair.lev1) {
			if (lev2 == pair.lev2) {
				if (e1 == pair.e1) {
					if (e2 == pair.e2) {
						return false;
					} else return true;
				} else return true;
			} else return true;
		} else return true;
		//*/
	}
	bool operator<(const satPair& pair) const{
		if (lev1 == pair.lev1) {
			if (lev2 == pair.lev2) {
				if (e1 == pair.e1) {
					if (e2 == pair.e2) {
						return false;
					} else return (e2 < pair.e2);
				} else return (e1 < pair.e1);
			} else return (lev2 < pair.lev2);
		} else return (lev1 < pair.lev1);
	}
	bool operator>(const satPair& pair) const{
		if (lev1== pair.lev1) {
			if (lev2 == pair.lev2) {
				if (e1 == pair.e1) {
					if (e2 == pair.e2) {
						return false;
					} else return (e2 > pair.e2);
				} else return (e1 > pair.e1);
			} else return (lev2 > pair.lev2);
		} else return (lev1 > pair.lev1);
	}
};

struct dc{
	int lvl;
	int sumIndex;
	mutable string sig1;
	mutable string sig2;
	mutable string sig3;
	mutable bool poss[8] = {false};
	mutable int count;
	mutable bool activated;
	
	dc()
		 : sumIndex(0), sig1(""), sig2(""), sig3(""), lvl(0), count(0), activated(true)
	{}

	dc(int sIndex, string s1, string s2, string s3, int level)
	  : sumIndex(sIndex), sig1(s1), sig2(s2), sig3(s3), lvl(level), count(0), activated(true)
	{}
	
	dc(const dc& old) {  // Copy constructor.
		this->lvl = old.lvl;
		this->sumIndex = old.sumIndex;
		this->sig1 = old.sig1;
		this->sig2 = old.sig2;
		this->sig3 = old.sig3;
		for (size_t i=0; i<8; i++) {
			this->poss[i] = old.poss[i];
		}
		this->count = old.count;
		this->activated = old.activated;
	}
	bool operator<(const dc& elem) const{
		if (lvl > elem.lvl) return true;  // Careful: Compare with > since we consider reverse levels.
		else return false;
	}
	bool operator>(const dc& elem) const{
		if (lvl < elem.lvl) return true;
		else return false;
	}
	bool operator==(const dc& elem) const{
		if (lvl == elem.lvl && sig1 == elem.sig1 && sig2 == elem.sig2 && sig3 == elem.sig3) return true;
		else return false;
	}
	bool operator!=(const dc& elem) const{
		return !(*this == elem);
	}
};

class Verifizierer {

//friend class Polynom;
public:	
	Verifizierer(const string& fileName);  // Construct verifier instance from a parsed circuit file and signature file.
	Verifizierer();
	virtual ~Verifizierer();
	std::vector<vp::Node> createReplaceOrder();  // Create order of nodes in which they will be replaced.
	std::vector<replacementOld> makeReplaceList(std::vector<vp::Node> order);
	replacement convertNodeToReplacement(vp::Node& node);  // Convert a given node to a replacement struct.
	replacementOld convertNodeToReplacementOld(vp::Node& node);
	void executeReplacements();  // Here all the replacements take place.
	void replaceSingleNodeWithRepr(vp::Node&, bool backTrackedRepr);  // Replace one node with information of its representatives.
	void replaceSingleNodeSimple(vp::Node&);  // Replace one node NOT using representatives.
	void replaceSingleNodePhaseDependend(vp::Node&);  // Replace one node. Consider posssible phase changes.
	void addBitLevelDCs(int, varIndex, varIndex, varIndex, mpz_class coef);  // Replace one node NOT using representatives.
	void replaceVariablesInPoly();
	void startVerification(const string& sigFile);  // Start the verification process without additional steps.
	void startAdvancedVerification(const string& sigFile);  // Start the verification process with sat-based information forwarding.
	//std::ostream& showPolynom();  
	std::pair<std::map<std::string, std::vector<mpz_class>>, std::vector<string>> readSignatureFile(const string& fileName);  // Read in the factors for the given inputs and outputs to create the signature polynom from in next step.
	Polynom createSignaturePolynom(std::pair<std::map<std::string, std::vector<mpz_class>>, std::vector<string>> readValue);  // Create the signature polynom given in the signature file.
	std::map<varIndex, int> createBDDVarOrder();  // Give all edges a level according to the replacement order.
	static void write_dd (DdManager *gbm, DdNode *dd, char* filename);  // Help function for drawing the BDD.
	static void print_dd (DdManager *gbm, DdNode *dd, int n, int pr );  // Help function to get informations of the BDD. 
	std::pair<std::pair<std::vector<varIndex>, std::vector<varIndex>>, std::vector<varIndex>> readVarsForBDD(); // Get the varIndices of R^(n+1) and D.
	void buildBDDConstraint(std::pair<std::pair<std::vector<varIndex>, std::vector<varIndex>>, std::vector<varIndex>>, std::vector<replacementOld>);  // Create the starting BDD from R^(n+1) and D.
	void runFirstSimulation();  // Run simulation for given input vector.
	void runFirstSimulationBeforeConstProp();  // Run simulation for given input vector.
	std::map< std::string, std::vector<int> > createDividerPrimarySignalsMap();
	void checkSimInputConstraint();  // Check and asure that all simulation vectors fulfill the input constraint.
	void correctSimVectorEntry(size_t, int, int);  // Correct a simulation vector entry which does not fulfill input constraint.
	void calcSimulationValues();
	bool testSAT(string e1, string e2, bool equivalence);  // Test equivalence( or antivalence) of two Edges e1 and e2 with SAT solver.
	Minisat::Var addInputConstraintToSolver(Minisat::Solver&, std::map<int, Minisat::Var>&);
	void addTseitinClauses(Minisat::Solver& solver, Minisat::Var in1, Minisat::Var in2, Minisat::Var out, vp::Node::NodeType gateType);  // Add tseitin clauses to solver.
	void addTseitinClausesActivated(Minisat::Solver& solver, Minisat::Var in1, Minisat::Var in2, Minisat::Var out, vp::Node::NodeType gateType, Minisat::Var activation);  // Add tseitin clauses to solver.
	void checkAllEquiAnti();  // Check every equivalence or antivalence in simulationTable with SAT solver.
	static void arrayShifting(int* array, int size, int posOut, int posIn1, int posIn2, int idxout, int idx1, int idx2, int* posOfIndex);
	void nodesInWindowRangeWithRepr(vp::Node* startNode, int range, set<int>& allNodes);
	void addRepresentative(varIndex e1, varIndex e2, bool equi);
	void addRepresentativeSimple(varIndex e1, varIndex e2, bool equi, vp::Edge* reprEdge);
	void initRepresentatives();
//	std::set<dc> checkDCCandidates(vector<Atomic>& atomics);
	std::set<gendc> checkDCCandidatesGeneral();
	void checkDCsInSimulation(set<dc>& candidates);
	void checkDCsInSimulationGeneral(set<gendc>& candidates);
	bool proveDCwithSAT(string sig1, string sig2, string sig3, bool b1, bool b2, bool b3, int window);
	bool proveDCwithSAT4(string sig1, string sig2, string sig3, string sig4, bool b1, bool b2, bool b3, bool b4, int window);
	void proveSpecific4DC(string sig1, string sig2, string sig3, string sig4, bool check);
	void proveSpecific4DCSubset(string sig1, string sig2, string sig3, string sig4, bool check, int num);
	std::vector<int> solveILPforFA(dc dontCare, vector<long> reducedCoefs);
	std::vector<int> solveILPforFA4(input4dc dontCare, vector<long> reducedCoefs);
	void imageComputationForDCs(set<dc>& candidates);
	void imageComputationForDCsGeneral(set<gendc>& candidates);
	void cleanDCcandidates(set<gendc>& candidate);
	void buildInputConstraintBDD(DdManager* &gbm, DdNode* &bdd_ic, vector<int>& dividend, vector<int>& divisor, DdNode** vars);
	void buildRemainderConstraintBDD(DdManager* &gbm, DdNode* &bdd_rc, vector<int>& remainder, vector<int>& divisor, DdNode** vars);
	void calcHelpersForImageComp(set<dc>& candidates, vector<vector<string>>& borders, vector<vector<int>>& imageDCNums, vector<int>& dcPos, vector<vector<int>>& orderedNodes, vector<set<int>>& cutInputs, vector<set<int>>& cutOutputs, vector<set<int>>& signalsToQuantify);
	void calcHelpersForImageCompGeneral(set<gendc>& candidates, vector<vector<string>>& borders, vector<vector<int>>& imageDCNums, vector<int>& dcPos, vector<vector<int>>& orderedNodes, vector<set<int>>& cutInputs, vector<set<int>>& cutOutputs, vector<set<int>>& signalsToQuantify);
	void cofactorDCInImage(DdManager* &gbm, vector<string>& dcSignals, int dcPos, int dcNum, set<dc>& candidates, DdNode** vars, DdNode*& currImage);
	void cofactorDCInImageGeneral(DdManager* &gbm, vector<string>& dcSignals, int dcPos, int dcNum, set<gendc>& candidates, DdNode** vars, DdNode*& currImage);
	void obtainBackwardsBDDPermutation(int* permu, DdNode** vars, int permuSize, vector<int>& dividend, vector<int>& divisor, vector<int>& quotient);
	std::vector<mpz_class> getCoefficientsForDC(string sig1, string sig2, string sig3);
	std::vector<mpz_class> getCoefficientsForDC4(string sig1, string sig2, string sig3, string sig4);
	static mpz_class findGCD(vector<mpz_class>& nums);
	static mpz_class gcd(mpz_class a, mpz_class b);
	void addDCMonomials(dc dontCare, vector<mpz_class> coefs);
	void addDCMonomials4(input4dc dontCare, vector<mpz_class> coefs);
	void applyDCOnRewriting(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, vector<int>& marginDC, int& specialMargin, double& tDCOp);
	void applyDCOnRewriting4(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, vector<int>& marginDC, int& specialMargin, double& tDCOp);
	void changeDCsByRepresent();
	bool applyICOnPolynomial(Polynom& poly);

	void extendDCsFanoutFree(set<gendc>& dcCandidates, vector<vector<int>>& extendedAtomics);
	void addVariableMonomialsFromDC(gendc dontCare);
	void addSingleDCPolynomial(Polynom& pDC);
	pair<vector<int>, mpz_class> readDCPolynomialAndSolveILP(int maxActivatedDC);
	void readPolynomialInformationsForDCs(vector<vector<int>*>& dcVarsVector, vector<vector<int>*>& dcCoefsVector, vector<mpz_class>& monCoefs, vector<size_t>& monLength, int maxActivatedDC);
	void applyDCSolutionToPolynomial(vector<mpz_class>& sol, int maxActivatedDC);
	void applyDCSolutionToOtherPolynomial(Polynom& pol, vector<mpz_class>& sol, int maxActivatedDC);
	bool applyGeneralDCOnRewriting(bool& stopBacktrack, vector<int>::iterator& it, bool& backTrackedDC, vector<Polynom>& backTrackDCPoly, vector<vector<int>::iterator>& backTrackDCIterator, std::vector<std::string>& signal_type, vector<int>& marginDC, int& specialMargin, int& additiveMargin, double& tDCOp, bool& rewrite);
	void checkStepsWithDCInsertionBeforehand();
	bool lookForwardForDCInsertion(vector<int>::iterator& it);
	bool willDCBeInsertedThisStep(vector<int>::iterator& it);
	bool willSignalBeReplacedInNextAtomic(vector<int>::iterator& it, vector<varIndex>& signals);

	void backwardRewritingSimple();
	int rewritePermutation(vector<int> permu, bool withPhaseOpt);
	void createAllOrderPermutations(std::vector<std::vector<int>>& allOrderPermutations, std::vector<std::pair<int,int>>& polySizes);
	void createPermutationsFromNode(std::vector<std::vector<int>>& allOrderPermutations, vector<int> prevPermu, int chosenNode, unordered_set<int> readyNodes, std::vector<std::pair<int,int>>& polySizes, mpz_class& counter);
	void createRandomPermutationFromNode(std::vector<std::vector<int>>& allOrderPermutations, vector<int> prevPermu, int chosenNode, unordered_set<int> readyNodes, std::vector<std::pair<int,int>>& polySizes, mpz_class& counter);
	std::pair<int,int> rewritePermutationWithAndWithoutPhaseOpt(vector<int> permu);

public:
	vp::Circuit circuit;
	Polynom poly;
	std::map<int, int> bddIndex;  // Mapping edgeIndex to internal bddIndex.
	aigpp::SimulationTable simTable;  // Simulation table.
	repr* represent;  // Map of representants of all signals.
	std::map<string, vector<int>> pMap;  // Stored: node indices of primary inputs and outputs.
	std::vector<dc> foundDCs;  // Every previously found DC is stored here so it can be used in next DC checks.
	std::set<dc> provenDCs;
	std::set<gendc> generalDCs;
	std::map<int, dc> hasDC;  // Mapping found DCs on a FA to the corresponding sum output edgeIndex. Used for backward rewriting.
	std::set<int> dcFAs;
	std::set<int> dcSignals;
	std::set<input4dc> proven4DCs;
};
void printVector(const std::vector<varIndex>& vec);
#endif /* VERIFIZIERER_H_ */
