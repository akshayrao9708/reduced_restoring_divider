/**************************************************************
*       
*       Polynom Package // Polynom.h
*
*       Author:
*         Alexander Konrad
*         University of Freiburg
*         konrada@informatik.uni-freiburg.de
*
***************************************************************/

#ifndef POLYNOM_H_
#define POLYNOM_H_

// std includes.
#include <stdlib.h>
#include <list>
#include <set>
#include <iostream>
#include <climits>
#include <vector>
#include <cassert>
#include <deque>

// Local includes.
#include "Monom2.h"
#include "vanishPair.hpp"
//#include "dcMonEntry.hpp"

class Polynom {
	
	friend class Circuit;
	friend class Verifizierer;
	public:
		// Constructors.
		Polynom();
		Polynom(int varSize);
		//Polynom(Monom2 mon);
		Polynom(const Polynom& old);  // Copy constructor. 
		Polynom& operator=(const Polynom&);  // Assignment operator.
		
		// Destructor.
		virtual ~Polynom();
		
		// Add another polynomial to this object.
		bool addPolynom(const Polynom& other);
		
		// Helping function for copying Don't Care Entries information from another polynomial.
		void copyDCEntries(const Polynom& old);

		// Functions for finding, adding and removing monomials.
//		std::pair<std::set<Monom2>::iterator, bool> addMonom(Monom2 mon);
		Monom2* addMonom(Monom2 mon);
		Monom2* addMonomWithDC(Monom2 mon, dcMonEntry* dcEntry);
		void eraseMonom(Monom2 mon);
		void addRefVar(Monom2& mon, varIndex index, int i);
		std::vector<Monom2*> findContaining(Monom2& mon);
		std::vector<Monom2*> findContaining_new(Monom2& mon, const std::vector<varIndex>& excludeVars);

		std::vector<Monom2*> findContainingVar(varIndex var);
		Monom2* findExact(Monom2& mon);
		int phaseChangeEffectOnMonom(Monom2& mon, varIndex var);
		bool containsVar(varIndex var);
		bool monContainsVanishing(Monom2& mon);
		
		void mod2n(mpz_class modNum);
		void setModReduction(bool mode);
		void setModReductionNumber(mpz_class modNum);

		// Change "phase" of a variable by replacing it with its own negation.
		void negateVar(varIndex replace);
		int greedyPhaseChange();
		int greedyPhaseChangeBackward();
		int greedyPhaseChangeCustom(std::vector<varIndex>& signalsToChange);
		int greedyPhaseChangeCustom(std::list<varIndex>& signalsToChange);
		int greedyPhaseChangeCustom(std::list<varIndex>& signalsToChange, std::list<uint32_t>& changedPhases);
		bool testPhaseChangeSingleVariable(varIndex var);
		bool testPhaseChangeSingleVariableImproved(varIndex var);
		bool testPhaseChangeSingleVariableMoreImproved(varIndex var);
		void reportVarPhases();

		void replaceANDDependingOnNegations(varIndex replace, varIndex in1, varIndex in2, bool phase1, bool phase2);

		// Functions for replacing one variable in the polynomial by gate functions (with or without inversions).
		void replaceAND(varIndex replace, varIndex in1, varIndex in2);
		void replaceOR(varIndex replace, varIndex in1, varIndex in2);
		void replaceXOR(varIndex replace, varIndex in1, varIndex in2);
		void replaceANDOneNegation(varIndex replace, varIndex in1, varIndex in2);
		void replaceOROneNegation(varIndex replace, varIndex in1, varIndex in2);
		void replaceXOROneNegation(varIndex replace, varIndex in1, varIndex in2);
		void replaceANDDoubleNegation(varIndex replace, varIndex in1, varIndex in2);
		void replaceORDoubleNegation(varIndex replace, varIndex in1, varIndex in2);
		void replaceNOT(varIndex replace, varIndex in1);
		void replaceBUFFER(varIndex replace, varIndex in1);
		void replaceCON0(varIndex replace);
		void replaceCON1(varIndex replace);
		

		// General function for replacing a variable in the polynomial by a list of monomials. Used by every function of above category. 
		void replaceVar(varIndex replace, std::list<Monom2>& mons);
		
		// Getters and Setters.
		const std::set<Monom2>* getSet() const;
		MyList* getRefList();
		std::vector<bool>* getPhases();
		void setPhases(std::vector<bool>& newPhases);
		size_t getVarSize();
		size_t size();
		size_t sizeNoDCs();
		std::list<dcMonEntry>* getDCList();
		std::deque<std::pair<bool, Monom2>>* getHistory();
		void setMaxDCNum(int maxNum) {this->maxDCNum = maxNum;};
		
		// Function for resizing the polynomial variable range.
		void resize(size_t varSize);
		
		// Multiply two polynomials. Return resulting polynomial.
		static Polynom multiplyPoly(Polynom& p1, Polynom& p2);
		
		// Outputstream for polynomial objects.
		friend std::ostream& operator<<(std::ostream& stdout, const Polynom& obj);
		
		//Specification Polynomial for a multiplier
		static Polynom multiplierSpecification(uint32_t multSize, std::vector<uint32_t> outputs);
		
		//Specification Polynomial for a adder
		static Polynom adderSpecification(uint32_t addSize, std::vector<uint32_t> outputs);

		//Specification Polynomial for a divider
		static Polynom dividerSpecification(int maxVar, std::vector<uint32_t> dividend, std::vector<uint32_t> divisor, std::vector<uint32_t> quotient, std::vector<uint32_t> remainder, std::vector<bool> quotientSigns, std::vector<bool> remainderSigns);
		
		// Add dcList entry.
		void addDCListEntry(std::vector<int> dcEntries, std::vector<int> coefs, Monom2& mon);
		void clearDCList();
		void changeDCEntryAfterReplacement(dcMonEntry*, size_t, Monom2*, signed long int);
		static std::pair<std::vector<int>, std::vector<int>> mergeTwoEntries(std::vector<int>& tempVec, std::vector<int>& tempCoef, std::vector<int>& addVec, std::vector<int>& addCoef);
		//dcMonEntry removeLowerVars(dcMonEn);

		// History management.
		void startHistory();
		void pauseHistory();
		void resumeHistory();
		void clearHistory();
		void stopHistory();
		bool setHistoryCheckpoint();
		void restoreCompleteHistory();
		void restoreLastHistoryCheckpoint();
		void dropLastHistoryCheckpoint();
		void showHistory();

	private:
		// Polynomial consists of two data structures: 1) Set of all monomials  2) List of all reference to monomials of a given variable.
		std::set<Monom2> polySet;
		MyList* refList;
		
		// Helping variable for remembering the variable range of polynomial.
		size_t varSize;

		// Structure to store dcMonEntries which can be used for DC optimization.
		std::list<dcMonEntry> dcList;

		// Helping variable for remembering maximum dc variable number.
		size_t maxDCNum;

		// Phase vector. Save which variables are currently in negated phase. False: Variable is negated. True: Variable is not negated.
		std::vector<bool> phases;

		// Vector of saved vanishing pairs.
		std::vector<vanishPair> vanishingPairs;

		// Keep history of last insertions/deletions in the polynomial to quickly revert replacements without copying complete polynomial.
		bool historyActivated;
		std::deque<std::pair<bool, Monom2>> history;
		std::deque<varIndex> phaseHistory;
		std::deque<std::pair<size_t, size_t>> historyCheckpoints;  // Remember position of history checkpoint to enable recovery of polynomial from a subset of history.

		bool modReductionEnabled = false;
		mpz_class coefModReduction = 0;
};

#endif /* POLYNOM_H_ */
