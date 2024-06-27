/**************************************************************
*       
*       Polynom Package // Polynom.cpp
*
*       Author:
*         Alexander Konrad
*         University of Freiburg
*        konrada@informatik.uni-freiburg.de
*
***************************************************************/

#include "Polynom.h"

//***************************************************************************************
Polynom::Polynom(){
	this->refList = new MyList[1];
	this->varSize = 1; 
	this->maxDCNum = 0;
	this->phases = {true};
	this->historyActivated = false;
}

//***************************************************************************************
Polynom::Polynom(int varSize){
	this->refList = new MyList[varSize+1];
	this->varSize = varSize;
	this->maxDCNum = 0;
	this->phases = std::vector<bool>(varSize+1, true);
	this->historyActivated = false;
}

//***************************************************************************************
/*
Polynom::Polynom(Monom2 mon){
	this->polySet.insert(mon);
	this->varSize = 0;
	for (size_t i=0; i < mon.size; i++) {
		if (this->varSize < mon.vars[i]) this->varSize = mon.vars[i];
	}
	this->refList = new MyList[this->varSize];
}
*/

//***************************************************************************************
Polynom::Polynom(const Polynom& old){  // Copy constructor
	this->varSize = old.varSize;
	
	this->refList = new MyList[old.varSize];
	this->polySet.clear(); 
	for (std::set<Monom2>::iterator it = old.polySet.begin(); it != old.polySet.end(); ++it) {
		this->addMonom(*it);
	}
	this->maxDCNum = old.maxDCNum;
	this->phases = old.phases;
	this->historyActivated = old.historyActivated;
	this->history = old.history;

	// Use extra function to copy DC list informations since pointers have to be set new.
	this->copyDCEntries(old);
}

//***************************************************************************************
Polynom& Polynom::operator=(const Polynom& other) {  // Assignment operator.
	if (this != &other) {
		this->varSize = other.varSize;
		delete[] this->refList;
		this->refList = new MyList[other.varSize];
		this->polySet.clear();
		for (std::set<Monom2>::iterator it = other.polySet.begin(); it != other.polySet.end(); ++it) {
			this->addMonom(*it);
		}
		this->maxDCNum = other.maxDCNum;
		// Use extra function to copy DC list informations since pointers have to be set new.
		this->copyDCEntries(other);
		this->phases = other.phases;
		this->historyActivated = other.historyActivated;
		this->history = other.history;
	}
	return *this;
}

//***************************************************************************************
Polynom::~Polynom(){
	delete[] this->refList;
}

//***************************************************************************************
bool Polynom::addPolynom(const Polynom& other) {
	if (this->varSize < other.varSize) {
		std::cout << "Cant add big polynom to small polynom (considering variable range). " << std::endl;
		return false;
	} else {
		for (std::set<Monom2>::iterator it = other.polySet.begin(); it != other.polySet.end(); ++it) {
				this->addMonom(*it);
		}
		return true;
	}
}

//***************************************************************************************
Monom2* Polynom::addMonom(Monom2 mon){
	//std::cout << "adding monom: " << mon << std::endl;
	std::pair<std::set<Monom2>::iterator,bool> ret;
//	if (historyActivated) history.emplace_front(1, mon);
	ret = this->polySet.insert(mon);
	if (ret.second == false) {  //Monom alredy exists. Just add the factor. Check for 0 factor monoms.
		if ((ret.first->getFactor() + mon.getFactor()) == 0 && ret.first->getDCPair()->first == NULL) {
			this->eraseMonom(*ret.first);  // Only erase monom if it has no DC variables.
			return NULL;
		} else {
			ret.first->factor = ret.first->getFactor() + mon.getFactor();
//			if (historyActivated) history.emplace_front(1, mon);
		}
	} else { // New monom inserted.
//		if (historyActivated) history.emplace_front(1, mon);
		varIndex* vars = ret.first->getVars();
		int size = ret.first->getSize();
		for (int i = 0; i < size; i++) {  // Add reference to newly inserted monomials.
			this->addRefVar(const_cast<Monom2&>(*ret.first), vars[i], i);
			/*ref1.varIndex = vars[i];
			ref1.monPtr = &(*ret.first); 
			this->addRefVar(ref1, const_cast<Monom2&>(*ret.first), i); */
		}	
	}
	if (this->modReductionEnabled) {
		//mpz_class coefReduced = 0;
		mpz_mod(ret.first->factor.get_mpz_t(), ret.first->factor.get_mpz_t(), this->coefModReduction.get_mpz_t());
		if (ret.first->getFactor() == 0 && ret.first->getDCPair()->first == NULL) {
			this->eraseMonom(*ret.first); // Only erase monom if it has no DC variables.
			if (historyActivated) history.emplace_front(1, mon);
			return NULL;
		}
	}
	if (historyActivated) history.emplace_front(1, mon);
	//std::cout<<"return from add mon"<<std::endl;
	return &const_cast<Monom2&>(*ret.first);
	
}

//***************************************************************************************
Monom2* Polynom::addMonomWithDC(Monom2 mon, dcMonEntry* dcEntry){
	std::pair<std::set<Monom2>::iterator,bool> ret;
	ret = this->polySet.insert(mon);
	if (ret.second == false) {  //Monom alredy exists. Just add the factor. Check for 0 factor monoms.
		if ((ret.first->getFactor() + mon.getFactor()) == 0 && ret.first->getDCPair()->first == NULL && dcEntry == NULL) {
			this->eraseMonom(*ret.first);  // Only erase monom if it has no DC variables.
			return NULL;	// If monom was deleted, return NULL.
		} else {
			ret.first->factor = ret.first->getFactor() + mon.getFactor();
		}
	} else { // New monom inserted.
		varIndex* vars = ret.first->getVars();
		int size = ret.first->getSize();
		for (int i = 0; i < size; i++) {  // Add reference to newly inserted monomials.
			this->addRefVar(const_cast<Monom2&>(*ret.first), vars[i], i);
			/*ref1.varIndex = vars[i];
			ref1.monPtr = &(*ret.first);
			this->addRefVar(ref1, const_cast<Monom2&>(*ret.first), i); */
		}
	}
	return &const_cast<Monom2&>(*ret.first);
}

//***************************************************************************************
void Polynom::eraseMonom(Monom2 mon) {
	varIndex var = 0;
	for (int i = 0; i < mon.getSize(); i++) {
		//std::cout << *(mon.getPtrs()[i]) << '\n';
		//*(mon.getPtrs()[i]) = 0;
		var = (mon.getVars())[i];
		this->refList[var].deleteElement(mon.getPtrs()[i]);
		
	}
	size_t deletedElements = 0;	
	deletedElements = this->polySet.erase(mon);
	if (historyActivated) history.emplace_front(0, mon);
}

//***************************************************************************************
void Polynom::replaceANDDependingOnNegations(varIndex replace, varIndex in1, varIndex in2, bool phase1, bool phase2) {
	if (phase1) {
		if (phase2) {
			replaceAND(replace, in1, in2);
		} else {
			replaceANDOneNegation(replace, in2, in1);
		}
	} else if (phase2) {
			replaceANDOneNegation(replace, in1, in2);
		} else {
			replaceANDDoubleNegation(replace, in2, in1);
		}
}

//***************************************************************************************
void Polynom::replaceAND(varIndex replace, varIndex in1, varIndex in2) {
	varIndex tmp = -1;
	if (in1 == in2) {
		Monom2 oneMonom(in1);
		std::list<Monom2> mons;
		mons.push_back(oneMonom);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
		}	
//		varIndex* vars = new int[2]; 
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 andMon(in1, in2);
		std::list<Monom2> mons;
		mons.push_back(andMon);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceANDOneNegation(varIndex replace, varIndex in1, varIndex in2) {
	// It is important, that in1 is the negation signal.
	varIndex tmp = -1;
	bool switched = false;
	if (in1 == in2) {
		Monom2 zeroMonom;
		zeroMonom.setFactor(0);
		std::list<Monom2> mons;
		mons.push_back(zeroMonom);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) { // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
			switched = true;
		}
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 andMon(in1, in2);
		andMon.setFactor(-1);
		Monom2 andMon2;
		if (switched) andMon2 = Monom2(in1);  // If signals were swapped, in1 is now the not inverted signal.
		else andMon2 = Monom2(in2);
		std::list<Monom2> mons;
		mons.push_back(andMon);
		mons.push_back(andMon2);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceANDDoubleNegation(varIndex replace, varIndex in1, varIndex in2) {
	varIndex tmp = -1;
	if (in1 == in2) {
		replaceNOT(replace, in1);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
		}
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 andMon(in1, in2);
		Monom2 andMon2(in1);
		andMon2.setFactor(-1);
		Monom2 andMon3(in2);
		andMon3.setFactor(-1);
		Monom2 andMon4;
		andMon4.setFactor(1);
		std::list<Monom2> mons;
		mons.push_back(andMon);
		mons.push_back(andMon2);
		mons.push_back(andMon3);
		mons.push_back(andMon4);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceOR(varIndex replace, varIndex in1, varIndex in2) {
	varIndex tmp = -1;
	if (in1 == in2) {
		Monom2 oneMonom(in1);
		std::list<Monom2> mons;
		mons.push_back(oneMonom);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;		
		}	
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 orMon1(in1);
		Monom2 orMon2(in2);
		Monom2 orMon3(in1, in2);
		orMon3.setFactor(-1);
		std::list<Monom2> mons;
		mons.push_back(orMon1);
		mons.push_back(orMon2);
		mons.push_back(orMon3);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceOROneNegation(varIndex replace, varIndex in1, varIndex in2) {
	// It is important, that in1 is the negation signal.
	varIndex tmp = -1;
	bool switched = false;
	if (in1 == in2) {
		Monom2 oneMonom;
		oneMonom.setFactor(1);
		std::list<Monom2> mons;
		mons.push_back(oneMonom);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
			switched = true;
		}
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 orMon1;
		orMon1.setFactor(1);
		Monom2 orMon2;
		if (switched) orMon2 = Monom2(in2);  // If switched, in2 is now the negation signal.
		else orMon2 = Monom2(in1);
		orMon2.setFactor(-1);
		Monom2 orMon3(in1, in2);
		//orMon3.setFactor(1);
		std::list<Monom2> mons;
		mons.push_back(orMon1);
		mons.push_back(orMon2);
		mons.push_back(orMon3);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceORDoubleNegation(varIndex replace, varIndex in1, varIndex in2) { 
	varIndex tmp = -1;
	if (in1 == in2) {
		replaceNOT(replace, in1);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
		}
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 orMon1;
		orMon1.setFactor(1);
		Monom2 orMon2(in1, in2);
		orMon2.setFactor(-1);
		std::list<Monom2> mons;
		mons.push_back(orMon1);
		mons.push_back(orMon2);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceXOR(varIndex replace, varIndex in1, varIndex in2) {
	varIndex tmp = -1;
	if (in1 == in2) {
		Monom2 empty;
		empty.setFactor(0);
		std::list<Monom2> mons;
		mons.push_back(empty);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
		}	
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 xorMon1(in1);
		Monom2 xorMon2(in2);
		Monom2 xorMon3(in1, in2);
		xorMon3.setFactor(-2);
		std::list<Monom2> mons;
		mons.push_back(xorMon1);
		mons.push_back(xorMon2);
		mons.push_back(xorMon3);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceXOROneNegation(varIndex replace, varIndex in1, varIndex in2) {
	varIndex tmp = -1;
	if (in1 == in2) {
		Monom2 oneMonom;
		oneMonom.setFactor(1);
		std::list<Monom2> mons;
		mons.push_back(oneMonom);
		this->replaceVar(replace, mons);
	} else {
		if (in1 > in2) {  // Swap signals to assure in1 is the smaller one.
			tmp = in1;
			in1 = in2;
			in2 = tmp;
		}
//		varIndex* vars = new int[2];
//		vars[0] = in1;
//		vars[1] = in2;
		Monom2 xorMon1(in1);
		xorMon1.setFactor(-1);
		Monom2 xorMon2(in2);
		xorMon2.setFactor(-1);
		Monom2 xorMon3(in1, in2);
		xorMon3.setFactor(2);
		Monom2 xorMon4;
		xorMon4.setFactor(1);
		std::list<Monom2> mons;
		mons.push_back(xorMon1);
		mons.push_back(xorMon2);
		mons.push_back(xorMon3);
		mons.push_back(xorMon4);
		this->replaceVar(replace, mons);
//		delete[] vars;
	}
}

//***************************************************************************************
void Polynom::replaceNOT(varIndex replace, varIndex in1) {
	Monom2 notMon1(in1);
	notMon1.setFactor(-1);
	Monom2 notMon2;
	notMon2.setFactor(1);
	std::list<Monom2> mons;
	mons.push_back(notMon1);
	mons.push_back(notMon2);
	this->replaceVar(replace, mons);
}

//***************************************************************************************
void Polynom::replaceBUFFER(varIndex replace, varIndex in1) {
	if (replace == in1) {std::cout << "replaceBuffer with same variables" << std::endl; return; }
	//std::cout << "buffer: replace " << replace << " by " << in1 << std::endl;
	Monom2 notMon1(in1);
	notMon1.setFactor(1);
	std::list<Monom2> mons;
	mons.push_back(notMon1);
	this->replaceVar(replace, mons);
}

//***************************************************************************************
void Polynom::replaceCON0(varIndex replace) {
	Monom2 notMon1;
	notMon1.setFactor(0);
	std::list<Monom2> mons;
	mons.push_back(notMon1);
	this->replaceVar(replace, mons);
}

//***************************************************************************************
void Polynom::replaceCON1(varIndex replace) {
	Monom2 notMon1;
	notMon1.setFactor(1);
	std::list<Monom2> mons;
	mons.push_back(notMon1);
	this->replaceVar(replace, mons);
}

//***************************************************************************************
void Polynom::replaceVar(varIndex replace, std::list<Monom2>& mons) {
	Monom2 newMon;
	Monom2 oldMon;
	MyList::ListElement* nextElement;
	Monom2* newMonPointer = NULL;
	std::pair<std::set<Monom2>::iterator, bool> retPair;
	dcMonEntry* dcEntry;
	size_t dcPos;
	for (MyList::Iterator it=this->refList[replace].begin(); it != this->refList[replace].end(); it = nextElement) {
		if (refList[replace].isEmpty()) {
			std::cout << "Reflist Empty. Something went wrong." << std::endl;
			return;
		}
		oldMon = *(it.returnData());
//		std::cout << "relaceVar: oldMon: " << oldMon << std::endl;
		dcEntry = oldMon.dcPair.first;
		dcPos = oldMon.dcPair.second;
		nextElement = it.returnElement()->next;  // Save address of next element before deleting current element.
		this->eraseMonom(*(it.returnData()));  // ATTENTION: Iterator on the current element gets invalid.
		it = this->refList[replace].begin();
		for (std::list<Monom2>::iterator it2=mons.begin(); it2 != mons.end(); ++it2) {
			//std::cout<<"it2"<<*it2<<std::endl;
			newMon = oldMon.merge(replace, *it2);
		//std::cout << "replaceVar: newMon       : " << newMon << std::endl;
			if (newMon.getFactor() == 0 && dcEntry == NULL) continue; // Dont add monom with factor 0. Only caused by XOR with same inputs.
//			if (monContainsVanishing(newMon)) continue;
			if (dcEntry == NULL) {
				newMonPointer = this->addMonom(newMon);
//				if (newMonPointer != NULL) std::cout << "replaceVar: NULL newMonPointer: " << *newMonPointer << std::endl;
//				else std::cout << "replaceVar: NULL newMonPointer is NULL. " << std::endl;
			} else {
				newMonPointer = this->addMonomWithDC(newMon, dcEntry);
//				if (newMonPointer != NULL) std::cout << "replaceVar: WITH DC newMonPointer: " << *newMonPointer << std::endl;
//				else std::cout << "replaceVar: WITH DC newMonPointer is NULL. THIS SHOULD NEVER HAPPEN!!!!! " << std::endl;
			}
			// If old Mon had dcEntry, change DCEntries for new monomials appropriately.
			if (dcEntry != NULL) changeDCEntryAfterReplacement(dcEntry, dcPos, newMonPointer, (it2->getFactor()).get_si());
		}
	}
}

//***************************************************************************************
void Polynom::negateVar(varIndex replace) {
//	varIndex tmp = 0;
//	assert(!this->containsVar(tmp));
//	this->replaceBUFFER(replace, tmp);
//	this->replaceNOT(tmp, replace);
//	Monom2 repMon(replace);
//	std::vector<Monom2*> oldPointers = this->findContaining(repMon);
	std::vector<Monom2*> oldPointers = this->findContainingVar(replace);
	Monom2 tmpMon;
	Monom2 mergeMon;  // Create "1" monomial.
	mergeMon.setFactor(1);
	for (auto& elem: oldPointers) {
		tmpMon = *elem;  // Save old mon.
		this->eraseMonom(*elem);  // Erase old mon.
		tmpMon.setFactor(tmpMon.getFactor() * -1);  // Negate factor.
		this->addMonom(tmpMon);	// Insert mon with negated factor.
		tmpMon.setFactor(tmpMon.getFactor() * -1);  // Get previous factor back.
		this->addMonom(tmpMon.merge(replace, mergeMon));
	}
	this->phases[replace] = !this->phases[replace];
}

//***************************************************************************************
bool Polynom::testPhaseChangeSingleVariable(varIndex var) {
	size_t sizeBefore = this->size();
//	std::cout << "poly before phase change test for variable x" << var << ": " << *this << std::endl;
	bool historyWasActivated = this->historyActivated;
	bool phaseBefore = this->phases.at(var);
	if (containsVar(var)) {
		if (!historyWasActivated) this->startHistory();
		setHistoryCheckpoint();
		negateVar(var);
//		std::cout << "poly after phase change test for variable x" << var << ": " << *this << std::endl;
		if (sizeBefore <= this->size()) {
//			negateVar(var);  // If changing phase does not help, revert the change.
			restoreLastHistoryCheckpoint();
//			std::cout << "poly after restoring:"  << *this << std::endl;
			if (!historyWasActivated) this->stopHistory();
			this->phases.at(var) = phaseBefore;
			return false;
		} else {
			dropLastHistoryCheckpoint();
//			std::cout << "poly after dropping:"  << *this << std::endl;
			std::cout << var << " got negated." << std::endl;
			if (!historyWasActivated) this->stopHistory();
			return true;
		}
	}
	return false;
}

//***************************************************************************************
bool Polynom::testPhaseChangeSingleVariableMoreImproved(varIndex var) {
	std::cout << "test var for negation: " << var << std::endl;
//	this->showHistory();
	std::vector<Monom2*> oldPointers = this->findContainingVar(var);
//	bool historyWasActive = this->historyActivated;
//	this->pauseHistory();
//	size_t sizeBefore = this->size();
	int countSizeChange = 0;
	for (auto& elem: oldPointers) {
		countSizeChange += phaseChangeEffectOnMonom(*elem, var);
	}
	std::cout << "after counting " << std::endl;
	if (countSizeChange < 0) {
		Monom2 tmpMon;  // Create "1" monomial.
		tmpMon.setFactor(1);
//		std::vector<Monom2> addedMons;
//		addedMons.reserve(oldPointers.size());
		for (auto& elem: oldPointers) {
//			addedMons.push_back(elem->merge(var, tmpMon));
//			this->addMonom(addedMons.back());
			this->addMonom(elem->merge(var, tmpMon));
			mpz_neg(elem->factor.get_mpz_t(), elem->factor.get_mpz_t());
		}
		std::cout << "var x" << var << " negated." << std::endl;
		this->phases[var] = !this->phases[var];
		return true;
	} else {
		return false;
	}

////	std::cout << "poly before negation test: " << *this << std::endl;
//	for (auto& elem: oldPointers) {
//		addedMons.push_back(elem->merge(var, tmpMon));
//		this->addMonom(addedMons.back());
////		elem->setFactor(elem->getFactor() * -1);  //Durch GMP Methode zur Negation ersetzt. Ist aber nicht wirklich effizienter.
//		mpz_neg(elem->factor.get_mpz_t(), elem->factor.get_mpz_t());
//	}
////	std::cout << "poly after negation test: " << *this << std::endl;
//	if (sizeBefore <= this->size()) { // Revert the negation.
////		std::cout << "negation reverted." << std::endl;
//		for (auto& elem: addedMons) {
////			tmpMon = elem;
////			tmpMon.setFactor(tmpMon.getFactor() * -1);  //TODO: Durch GMP Methode zur Negation ersetzen. Sollte effizienter sein.
//			mpz_neg(elem.factor.get_mpz_t(), elem.factor.get_mpz_t());
//			this->addMonom(elem);  // Erase previosuly added monoms.
//		}
//		for (auto& elem: oldPointers) {
////			elem->setFactor(elem->getFactor() * -1);  // Reset factor back to normal.
//			mpz_neg(elem->factor.get_mpz_t(), elem->factor.get_mpz_t());
//		}
////		std::cout << "poly after restore: " << *this << std::endl;
//		if (historyWasActive) this->resumeHistory();
////		this->showHistory();
//		return false;
//	} else {  // Keep the negation. Add the added and changed monomials to the history.
//		std::cout << "var x" << var << " negated." << std::endl;
//		if (historyWasActive) {
//			for (auto& elem: addedMons) history.emplace_front(1, elem);  // Add the previously added monoms to the history.
//			for (auto& elem: oldPointers) {
//				tmpMon = *elem;  // Save the negation of the factor in history by adding it with double negated factor.
//				tmpMon.setFactor(tmpMon.getFactor() * 2);
//				history.emplace_front(1, tmpMon);
//				if (modReductionEnabled) {  // If modulo reduction is enabled, apply modulo to the negated factors.
//					mpz_mod(elem->factor.get_mpz_t(), elem->factor.get_mpz_t(), this->coefModReduction.get_mpz_t());
//				}
//			}
//		}
//		this->phases[var] = !this->phases[var];
////		std::cout << "poly after negation confirmed: " << *this << std::endl;
//		if (historyWasActive) this->resumeHistory();
////		this->showHistory();
//		return true;
//	}
}

//***************************************************************************************
bool Polynom::testPhaseChangeSingleVariableImproved(varIndex var) {
//	std::cout << "test var for negation: " << var << std::endl;
//	this->showHistory();
	size_t sizeBefore = this->size();
	std::vector<Monom2*> oldPointers = this->findContainingVar(var);
	Monom2 tmpMon;  // Create "1" monomial.
	tmpMon.setFactor(1);
	std::vector<Monom2> addedMons;
	addedMons.reserve(oldPointers.size());
	bool historyWasActive = this->historyActivated;
	this->pauseHistory();
//	std::cout << "poly before negation test: " << *this << std::endl;
	for (auto& elem: oldPointers) {
		addedMons.push_back(elem->merge(var, tmpMon));
		this->addMonom(addedMons.back());
//		elem->setFactor(elem->getFactor() * -1);  //TODO: Durch GMP Methode zur Negation ersetzen. Sollte effizienter sein.
		mpz_neg(elem->factor.get_mpz_t(), elem->factor.get_mpz_t());
	}
//	std::cout << "poly after negation test: " << *this << std::endl;
	if (sizeBefore <= this->size()) { // Revert the negation.
//		std::cout << "negation reverted." << std::endl;
		for (auto& elem: addedMons) {
//			tmpMon = elem;
//			tmpMon.setFactor(tmpMon.getFactor() * -1);  //TODO: Durch GMP Methode zur Negation ersetzen. Sollte effizienter sein.
			mpz_neg(elem.factor.get_mpz_t(), elem.factor.get_mpz_t());
			this->addMonom(elem);  // Erase previosuly added monoms.
		}
		for (auto& elem: oldPointers) {
//			elem->setFactor(elem->getFactor() * -1);  // Reset factor back to normal.
			mpz_neg(elem->factor.get_mpz_t(), elem->factor.get_mpz_t());
		}
//		std::cout << "poly after restore: " << *this << std::endl;
		if (historyWasActive) this->resumeHistory();
//		this->showHistory();
		return false;
	} else {  // Keep the negation. Add the added and changed monomials to the history.
//		std::cout << "var x" << var << " negated." << std::endl;
		if (historyWasActive) {
			for (auto& elem: addedMons) history.emplace_front(1, elem);  // Add the previously added monoms to the history.
			for (auto& elem: oldPointers) {
				tmpMon = *elem;  // Save the negation of the factor in history by adding it with double negated factor.
				tmpMon.setFactor(tmpMon.getFactor() * 2);
				history.emplace_front(1, tmpMon);
				if (modReductionEnabled) {  // If modulo reduction is enabled, apply modulo to the negated factors.
					mpz_mod(elem->factor.get_mpz_t(), elem->factor.get_mpz_t(), this->coefModReduction.get_mpz_t());
				}
			}
		}
		this->phases[var] = !this->phases[var];
//		std::cout << "poly after negation confirmed: " << *this << std::endl;
		if (historyWasActive) this->resumeHistory();
//		this->showHistory();
		return true;
	}
}

//***************************************************************************************
int Polynom::greedyPhaseChange() {
	int improvement = 0;
	size_t sizeStart = this->size();
	for (size_t i=0; i < getVarSize(); ++i) {
		testPhaseChangeSingleVariable(i);
	}
	improvement = (sizeStart - this->size());
	return improvement;
}

//***************************************************************************************
int Polynom::greedyPhaseChangeBackward() {
	int improvement = 0;
	size_t sizeStart = this->size();
	for (int i = getVarSize() - 1; i >= 0; --i) {
		testPhaseChangeSingleVariable(i);
	}
	improvement = (sizeStart - this->size());
	return improvement;
}

//***************************************************************************************
int Polynom::greedyPhaseChangeCustom(std::vector<varIndex>& signalsToChange) {
	int improvement = 0;
	size_t sizeStart = this->size();
	for (size_t i=0; i < signalsToChange.size(); ++i) {
		testPhaseChangeSingleVariable(signalsToChange.at(i));
	}
	improvement = (sizeStart - this->size());
	return improvement;
}

//***************************************************************************************
int Polynom::greedyPhaseChangeCustom(std::list<varIndex>& signalsToChange) {
//	std::cout << "inside greedyPhaseChangeCustom" << std::endl;
	int improvement = 0;
	size_t sizeStart = this->size();
	for (std::list<varIndex>::iterator it= signalsToChange.begin(); it != signalsToChange.end(); ++it) {
//		testPhaseChangeSingleVariable(*it);
		testPhaseChangeSingleVariableImproved(*it);
//		testPhaseChangeSingleVariableMoreImproved(*it);
	}
	improvement = (sizeStart - this->size());
//	std::cout << "out of greedyPhaseChangeCustom" << std::endl;
	return improvement;
}

//***************************************************************************************
int Polynom::greedyPhaseChangeCustom(std::list<varIndex>& signalsToChange, std::list<uint32_t>& changedPhases) {
//	std::cout << "inside greedyPhaseChangeCustom" << std::endl;
	int improvement = 0;
	size_t sizeStart = this->size();
	bool changed;
	for (std::list<varIndex>::iterator it= signalsToChange.begin(); it != signalsToChange.end(); ++it) {
		//testPhaseChangeSingleVariable(*it);
		changed = testPhaseChangeSingleVariableImproved(*it);
		if (changed) {
			changedPhases.push_back(*it);
//			std::cout << "phase changed: " << *it << std::endl;
		}
	}
	improvement = (sizeStart - this->size());
//	std::cout << "out of greedyPhaseChangeCustom" << std::endl;
	return improvement;
}

//***************************************************************************************
void Polynom::reportVarPhases() {
	std::cout << "Varialbes with face 0 are following: " << std::endl;
	for (size_t i=0; i < getVarSize(); ++i) {
		if (this->phases[i] == false) std::cout << "x" << i << " phase is " << this->phases[i] << std::endl;
	}
}

//****************************************************************************************************************************
std::vector<Monom2*> Polynom::findContainingVar(varIndex var) {
	std::vector<Monom2*> resultVec;
	int minListLength = INT_MAX;
	varIndex minListVar = -1;
	// Retrieve all monomials from refList which contain var.
	bool add;
	for (MyList::Iterator it=this->refList[var].begin(); it != this->refList[var].end(); it++) {
		resultVec.push_back(it.returnData());
	}
	return resultVec;
}

//****************************************************************************************************************************
std::vector<Monom2*> Polynom::findContaining(Monom2& mon) {
	std::vector<Monom2*> resultVec;
	int minListLength = INT_MAX;
	varIndex minListVar = -1;
	for (size_t i=0; i < mon.getSize(); i++) {  // Get the variable with shortest refList. 
		//std::cout << "|" << mon.getVars()[i] << "|"; 
		if (mon.getVars()[i] > this->getVarSize()) {
			std::cout << "Error in findContaining(): Monomial to find includes a variable out ouf range of this polynomial variable range." << std::endl;
			return resultVec;
		}
		if ((getRefList()[mon.getVars()[i]]).getSize() < minListLength) {
			minListVar = mon.getVars()[i]; 
			minListLength = (getRefList()[mon.getVars()[i]]).getSize();
			if (minListLength == 0) return resultVec;  // If one refList length is zero, this variable is not contained, so mon cannot be contained in polynomial.
		}
	}
	// Retrieve all monomials from shortest refList which contain mon.
	bool add;
	for (MyList::Iterator it=this->refList[minListVar].begin(); it != this->refList[minListVar].end(); it++) {
		add = true;
		for (size_t i=0; i < mon.getSize(); i++) {
			if (mon.getVars()[i] == minListVar) continue;  // Not needed to check for minListVar, since it is the list of this variable.
			if (it.returnData()->containsVar(mon.getVars()[i]) == false) {
				add = false;
				break;
			}
		}
		if (add) resultVec.push_back(it.returnData());
	}
	return resultVec;
}


std::vector<Monom2*> Polynom::findContaining_new(Monom2& mon, const std::vector<varIndex>& excludeVars) {
    std::vector<Monom2*> resultVec;
    int minListLength = INT_MAX;
    varIndex minListVar = -1;

    // Get the variable with the shortest refList.
    for (size_t i = 0; i < mon.getSize(); i++) {
        if (mon.getVars()[i] > this->getVarSize()) {
            std::cout << "Error in findContaining(): Monomial to find includes a variable out of range of this polynomial variable range." << std::endl;
            return resultVec;
        }
        if ((getRefList()[mon.getVars()[i]]).getSize() < minListLength) {
            minListVar = mon.getVars()[i];
            minListLength = (getRefList()[mon.getVars()[i]]).getSize();
            if (minListLength == 0) return resultVec; // If one refList length is zero, this variable is not contained, so mon cannot be contained in polynomial.
        }
    }

    // Retrieve all monomials from the shortest refList which contain mon and do not contain excludeVars.
    bool add;
    for (MyList::Iterator it = this->refList[minListVar].begin(); it != this->refList[minListVar].end(); it++) {
        add = true;
        Monom2* currentMonom = it.returnData();

        // Check if currentMonom contains all variables in mon.
        for (size_t i = 0; i < mon.getSize(); i++) {
            if (mon.getVars()[i] == minListVar) continue; // Not needed to check for minListVar, since it is the list of this variable.
            if (currentMonom->containsVar(mon.getVars()[i]) == false) {
                add = false;
                break;
            }
        }

        if (add) {
            for (const auto& var : excludeVars) {
                if (currentMonom->containsVar(var)) {
                    add = false;
                    break;
                }
            }
        }

        if (add) resultVec.push_back(currentMonom);
    }

    return resultVec;
}


//****************************************************************************************************************************
Monom2* Polynom::findExact(Monom2& mon) {
	// First check if special case: mon is the empty monomial (only a coefficient without variables).
	Monom2* temp;
	if (mon.getSize() == 0) {  // If size=0 it is the empty monomial which is always first in polySet
		temp = &const_cast<Monom2&>(*this->polySet.begin());
		if (temp->getSize() == 0) return &const_cast<Monom2&>(*this->polySet.begin());
		else return NULL;
	}
	//if (mon.getSize() == 0) return NULL;
	int minListLength = INT_MAX;
	varIndex minListVar = -1;
	for (size_t i=0; i < mon.getSize(); i++) {  // Get the variable with shortest refList. 
		//std::cout << "|" << mon.getVars()[i] << "|"; 
		if (mon.getVars()[i] > this->getVarSize()) {
			std::cout << "Error in findExact(): Monomial to find includes a variable out ouf range of this polynomial variable range." << std::endl;
			return NULL;
		}
		if ((getRefList()[mon.getVars()[i]]).getSize() < minListLength) {
			minListVar = mon.getVars()[i]; 
			minListLength = (getRefList()[mon.getVars()[i]]).getSize();
			if (minListLength == 0) return NULL;  // If one refList length is zero, this variable is not contained, so mon cannot be contained in polynomial.
		}
	}
	// Find exact monomial mon from shortest refList.
	for (MyList::Iterator it=this->refList[minListVar].begin(); it != this->refList[minListVar].end(); it++) {
		if (*(it.returnData()) == mon) {
			return it.returnData();
		} 
	}
	return NULL;  // This case should never happen.
}

//****************************************************************************************************************************
int Polynom::phaseChangeEffectOnMonom(Monom2& mon, varIndex var) {
	// First check if special case: mon is the empty monomial (only a coefficient without variables).
//	std::cout << "check for " << mon << " | x" << var << std::endl;
	Monom2* temp;
	if (mon.getSize() == 0) {  // If size=0 it is the empty monomial which is always first in polySet
		return 0;
	}
	//if (mon.getSize() == 0) return NULL;
	int minListLength = INT_MAX;
	varIndex minListVar = -1;
	for (size_t i=0; i < mon.getSize(); i++) {  // Get the variable with shortest refList.
		//std::cout << "|" << mon.getVars()[i] << "|";
		if (mon.getVars()[i] > this->getVarSize()) {
			std::cout << "Error in findExact(): Monomial to find includes a variable out ouf range of this polynomial variable range." << std::endl;
			return NULL;
		}
		if ((getRefList()[mon.getVars()[i]]).getSize() < minListLength) {
			minListVar = mon.getVars()[i];
			minListLength = (getRefList()[mon.getVars()[i]]).getSize();
			if (minListLength == 0) return NULL;  // If one refList length is zero, this variable is not contained, so mon cannot be contained in polynomial.
		}
	}
//	std::cout << "list found" << std::endl;
	// Find monomial which is the same except that var is missing. Use sum to faster find candidates.
	int64_t findSum = mon.getSum() - var;
	size_t findSize = mon.getSize() - 1;
	int polySizeChange = 1;  // If monomial not found we add 1 to the poly size. If found we either dont change size or reduce by 1.
	for (MyList::Iterator it=this->refList[minListVar].begin(); it != this->refList[minListVar].end(); it++) {
		if ((it.returnData())->getSum() != findSum) continue;
		if ((it.returnData())->getSize() != findSize) continue;
		varIndex currVar;
		varIndex monVar;
		int offset = 0;
		if (mon.getVars()[0] == var) ++offset;
		bool conOuter = false;
		for (size_t monPos=0; monPos < (it.returnData())->getSize(); ++monPos) {
			currVar = (it.returnData())->getVars()[monPos];
			if (offset == 0 & currVar > var) ++offset;
			monVar = mon.getVars()[monPos + offset];
			if (currVar != monVar) { conOuter = true; break; }  // This is not the searched monomial.
		}
		if (conOuter) continue;  // Check next monomial.
		// Searched monomial found. Poly size is not increased. If coefficient is negative of mon we even reduce poly size by 1.
		polySizeChange = 0;
		if ((it.returnData())->factor == (-1 * mon.factor)) polySizeChange = -1;
		//TODO: Noch Fall einfügen, bei dem der neue Faktor nach der modulo Reduktion 0 ist. Dann würde das Monom auch wegfallen.
		if ((it.returnData())->factor + mon.factor == this->coefModReduction) polySizeChange = -1;
		break;
	}
//	std::cout << "leave check" << std::endl;
	return polySizeChange;  // This case should never happen.
}


//****************************************************************************************************************************
bool Polynom::containsVar(varIndex var) {
	if (var > this->varSize) {
		std::cout << "Searched varIndex exceeds variable range of polynomial." << std::endl;
		return false;
	} else {
		if (this->refList[var].getSize() == 0) return false;
		else return true;
	}
}

//****************************************************************************************************************************
bool Polynom::monContainsVanishing(Monom2& mon) {
	bool oneNegated = false;
	bool twoNegated = false;
	for (auto& pair: vanishingPairs) {
		// Check if mon contains a vanishing Pair.
		if (!mon.containsVar(pair.var1) || !mon.containsVar(pair.var2))	continue;
		// If contained, check for correct phases of variables afterwards. If needed, change variable phase.
		if (pair.phase1 != this->phases[pair.var1]) {
			negateVar(pair.var1);
			oneNegated = true;
		}
		if (pair.phase2 != this->phases[pair.var2]) {
			negateVar(pair.var2);
			twoNegated = true;
		}
		return true;
	}
	return false;
}

//****************************************************************************************************************************
void Polynom::copyDCEntries(const Polynom& old) {
//	std::cout << "in copyDCEntries" << std::endl;
	this->dcList.clear();
	Monom2* refMon;
	Monom2* changeMon;
	dcMonEntry* oldEntry;
	dcMonEntry* currEntry;
	int monPos;
	for (std::set<Monom2>::iterator it = old.polySet.begin(); it != old.polySet.end(); ++it) {
		// Check if monomial has dc variables.
		oldEntry = it->getDCPair()->first;
		if (oldEntry == NULL) continue;
		else {
			this->dcList.push_back(*oldEntry);
			this->dcList.back().pToMon.clear();
			currEntry = &this->dcList.back();
			changeMon = this->findExact(const_cast<Monom2&>(*it));
			currEntry->pToMon.push_back(changeMon);
			monPos = currEntry->pToMon.size() - 1;
			changeMon->setDCPair(currEntry, monPos);
		}
	}
	/*
	for (auto& elem: old.dcList) {
		std::cout << "beginning for " << std::endl;
		this->dcList.push_back(elem);
		this->dcList.back().pToMon.clear();
		currEntry = &this->dcList.back();
		std::cout << "beginning inner for " << std::endl;
		for (size_t i=0; i < elem.pToMon.size(); ++i) {
			changeMon = NULL;
			std::cout << "before reading refMon " << std::endl;
			std::cout << "before reading refMon " << *elem.pToMon.at(i) << std::endl;
			refMon = elem.pToMon.at(i);
			std::cout << "refMon is " << *refMon << std::endl;
			changeMon = this->findExact(*refMon);
			std::cout << "after finding changeMon " << std::endl;
			if (changeMon != NULL) std::cout << "changeMon is " << *changeMon << std::endl;
			else std::cout << "changeMon is NULL!!! " << std::endl;
			currEntry->pToMon.push_back(changeMon);
			monPos = currEntry->pToMon.size() - 1;
			changeMon->setDCPair(currEntry, monPos);
		}
	}
	*/
//	std::cout << "out copyDCEntries" << std::endl;
}


//***************************************************************************************
void Polynom::addRefVar(Monom2& mon, varIndex index, int i) {
	(mon.ptrs)[i] = this->refList[index].add(&mon);
	return;
}

//***************************************************************************************
const std::set<Monom2>* Polynom::getSet() const {
	return &this->polySet;
}

//***************************************************************************************
std::list<dcMonEntry>* Polynom::getDCList() {
	return &this->dcList;
}

//***************************************************************************************
MyList* Polynom::getRefList() {
	return this->refList;
}

//***************************************************************************************
std::deque<std::pair<bool, Monom2>>* Polynom::getHistory() {
	return &this->history;
}

//***************************************************************************************
std::vector<bool>* Polynom::getPhases() {
	return &this->phases;
}

//***************************************************************************************
void Polynom::setPhases(std::vector<bool>& newPhases) {
	this->phases = newPhases;
}

//***************************************************************************************
size_t Polynom::getVarSize() {
	return this->varSize;
}

//***************************************************************************************
size_t Polynom::size() {
	return this->polySet.size();
}

//***************************************************************************************
size_t Polynom::sizeNoDCs() {
	size_t tempSizeNoDCs = 0;
	for (auto& elem: *this->getSet()) {
			if (elem.getFactor() != 0) ++tempSizeNoDCs;
	}
	return tempSizeNoDCs;
}

//***************************************************************************************
void Polynom::resize(size_t varSize) {
	delete[] this->refList;
	this->polySet.clear();
	this->refList = new MyList[varSize];
	this->phases= std::vector<bool>(varSize+1, true);
	this->varSize = varSize;
}

//***************************************************************************************
void Polynom::startHistory() {
	clearHistory();
	this->historyActivated = true;
	this->historyCheckpoints.emplace_front(0, 0);
}

//***************************************************************************************
bool Polynom::setHistoryCheckpoint() {
	if (!this->historyActivated) {
		std::cout << "Failed to set a checkpoint in history. History is not activated." << std::endl;
		return false;
	}
	if (this->history.empty()) {
//		std::cout << "Failed to set a checkpoint in history. History is empty." << std::endl;
		return false;
	}
	this->historyCheckpoints.emplace_front(this->history.size(), this->phaseHistory.size());
	return true;
}

//***************************************************************************************
void Polynom::pauseHistory() {
	this->historyActivated = false;
}

//***************************************************************************************
void Polynom::resumeHistory() {
	this->historyActivated = true;
}

//***************************************************************************************
void Polynom::clearHistory() {
	this->history.clear();
	this->historyCheckpoints.clear();
}

//***************************************************************************************
void Polynom::stopHistory() {
	this->clearHistory();
	this->historyActivated = false;
}

//***************************************************************************************
void Polynom::restoreCompleteHistory() {
	pauseHistory();
	Monom2 tmpMon;
	for (auto& elem: history) {
		tmpMon = elem.second;
		if (elem.first) {  // Revert monomial insertion by inserting it again with negated factor.
			tmpMon.setFactor(tmpMon.getFactor() * -1);
			addMonom(tmpMon);
		} else {	// Revert monomial deletion by inserting it again.
			addMonom(tmpMon);
		}
	}
//	clearHistory();
	startHistory();
}

//***************************************************************************************
void Polynom::restoreLastHistoryCheckpoint() {
	//assert(historyCheckpoints.size() > 0);
	pauseHistory();
	Monom2 tmpMon;
	size_t elementsToRestore = this->history.size() - this->historyCheckpoints.front().first;
	for (size_t i=0; i < elementsToRestore; ++i) {
		tmpMon = this->history.at(i).second;
		if (this->history.at(i).first) {  // Revert monomial insertion by inserting it again with negated factor.
			tmpMon.setFactor(tmpMon.getFactor() * -1);
			addMonom(tmpMon);
		} else {	// Revert monomial deletion by inserting it again.
			addMonom(tmpMon);
		}
	}
	for (size_t i=0; i < elementsToRestore; ++i) {
		this->history.pop_front();
	}

	// TODO integrate phases into history restoring.
	//	size_t phasesToRestore = this->phaseHistory.size() - this->historyCheckpoints.front().second;
//
//
//	for (size_t i=0; i < phasesToRestore; ++i) {
//		this->phaseHistory.pop_front();
//	}

	if (this->historyCheckpoints.size() > 1) this->historyCheckpoints.pop_front();
	resumeHistory();
}

//***************************************************************************************
void Polynom::dropLastHistoryCheckpoint() {
//	assert(historyCheckpoints.size() > 0);
//	pauseHistory();
//	size_t elementsToRestore = this->history.size() - this->historyCheckpoints.front();
//	for (size_t i=0; i < elementsToRestore; ++i) {
//		this->history.pop_front();
//	}
	if (this->historyCheckpoints.size() > 1) this->historyCheckpoints.pop_front();
//	resumeHistory();
}

//***************************************************************************************
void Polynom::showHistory() {
	std::cout << "History of polynomial is:" << std::endl;
	if (history.size() == 0) { std::cout << " empty." << std::endl; return;}
	size_t count = history.size() - 1;
	size_t checkpointPos = 0;
	if (historyCheckpoints.at(0).first >= history.size() && historyCheckpoints.size() > 1) checkpointPos++;
	for (auto& elem: history) {
		if (elem.first) std::cout << "added ";
		else std::cout << "deleted ";
		std::cout << elem.second;
		if (count == historyCheckpoints.at(checkpointPos).first) { std::cout << " <--- CP "; ++checkpointPos; }
		std::cout << std::endl;
		--count;
	}
}

//***************************************************************************************
Polynom Polynom::multiplyPoly(Polynom& p1, Polynom& p2) {
	int maxSize = 0;
	if (p1.getVarSize() < p2.getVarSize()) {
		maxSize = p2.getVarSize();
	} else {
		maxSize = p1.getVarSize();
	}
	Polynom mult(maxSize);
	
	Monom2 temp;
	for (std::set<Monom2>::iterator it= p1.polySet.begin(); it != p1.polySet.end(); ++it) {
		for (std::set<Monom2>::iterator it2= p2.polySet.begin(); it2 != p2.polySet.end(); ++it2) {
			temp = Monom2::multiply(*it, *it2);
			mult.addMonom(temp);
		}
	}
	
	return mult;
}

//***************************************************************************************
void Polynom::changeDCEntryAfterReplacement(dcMonEntry* dcEntry, size_t dcPos, Monom2* newMonP, signed long int replaceCoef) {
	if (newMonP == NULL) std::cout << "changeDCEntryAfterReplacement(): newMonP is NULL!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
//	std::cout << "changeDCEntry for " << *newMonP << " with replaceCoef " << replaceCoef << std::endl;
	dcMonEntry* newEntryP;
	dcEntry->changePointer(dcPos, NULL);  // First invalidate reference to old monomial which has been replaced.
	// Now check if new created monomial already has an dcEntry.
	if (newMonP->dcPair.first == NULL) {  // newMon has no dcEntry. Add dcEntry of old mon to it.
//		std::cout << "newMonP NULL" << std::endl;
		if (replaceCoef != 1) {
//			std::cout << "replaceCoef != 1" << std::endl;
			dcMonEntry changedEntry(*dcEntry);
			changedEntry.pToMon.clear();
			changedEntry.addPointer(newMonP);
			changedEntry.multCoefs(replaceCoef);
			this->dcList.emplace_back(changedEntry);  // Add merged entry for dcMonEntry.
			newEntryP = &(dcList.back());
			assert(newEntryP != NULL);
//			newMonP->getDCPair()->first->pToMon.at(newMonP->getDCPair()->second) = NULL;  // Set old DCMonEntry for this monomial to 0 since it was changed.
			newMonP->setDCPair(newEntryP, 0);
		} else {
//			std::cout << "replaceCoef ist 1" << std::endl;
			newMonP->setDCPair(dcEntry, dcEntry->pToMon.size());  // IMPORTANT: First setDCPair and afterwards addMonEntry. Otherwise the position information of newMonPs dcPair is wrong.
			dcEntry->addMonEntry(newMonP);
		}
	} else {  // newMon has already an dcEntry. Merge old and new Entry into a new one.
//		std::cout << "newMonP has dcEntry." << std::endl;
		std::vector<int> oldVars = newMonP->getDCPair()->first->dcVars;
		std::vector<int> oldCoefs = newMonP->getDCPair()->first->coefs;
		std::vector<int> prevVars = dcEntry->dcVars;
		std::vector<int> prevCoefs = dcEntry->coefs;
		if (replaceCoef != 1) {
			for (size_t i=0; i < prevCoefs.size(); ++i) {
				prevCoefs.at(i) = prevCoefs.at(i) * replaceCoef;
			}
		}
		std::pair<std::vector<int>, std::vector<int>> mergeResult = mergeTwoEntries(prevVars, prevCoefs, oldVars, oldCoefs);
		if (mergeResult.first.size() > 0) {
			this->dcList.emplace_back(mergeResult.first, newMonP, mergeResult.second);  // Add merged entry for dcMonEntry.
			newEntryP = &(dcList.back());
			assert(newEntryP != NULL);
			newMonP->getDCPair()->first->pToMon.at(newMonP->getDCPair()->second) = NULL;  // Set old DCMonEntry for this monomial to 0 since it was changed.
			newMonP->setDCPair(newEntryP, 0);
		} else {
			newMonP->getDCPair()->first->pToMon.at(newMonP->getDCPair()->second) = NULL;
			newMonP->setDCPair(NULL, 0);  // DC entries cancelled out each other, monomial has no longer dc Variables.
			if (newMonP->getFactor() == 0) this->eraseMonom(*newMonP);  // If monomial coefficient is 0, erase monomial since it has no dc Variable anymore.
		}
	}
}

//***************************************************************************************
void Polynom::addDCListEntry(std::vector<int> dcEntries, std::vector<int> dcCoefs, Monom2& mon) {
	assert(dcEntries.size() == dcCoefs.size() && dcEntries.size() > 0);
	dcMonEntry* entryP;
	Monom2* monP = this->findExact(mon);  // Find monomial in polynomial.
	if (monP == NULL) {   // if not found, monomial need to be created.
//		std::cout << "addDCListEntry: monomial not found!" << std::endl;
//		std::cout << mon << std::endl;
		mon.setFactor(0);
		this->addMonom(mon);
		monP = this->findExact(mon);
	}
	assert(monP != NULL);
	if (monP->getDCPair()->first != NULL) {  // This monomial already has DC variables.
		std::vector<int> tempVec = monP->getDCPair()->first->dcVars; // = monP->getDCPair()->first->dcVars;
		std::vector<int> tempCoef = monP->getDCPair()->first->coefs;
		std::pair<std::vector<int>, std::vector<int>> mergeResult = mergeTwoEntries(tempVec, tempCoef, dcEntries, dcCoefs);
		if (mergeResult.first.size() > 0) {
			this->dcList.emplace_back(mergeResult.first, monP, mergeResult.second);  // Add merged entry for dcMonEntry.
			entryP = &(dcList.back());
			assert(entryP != NULL);
			monP->getDCPair()->first->pToMon.at(monP->getDCPair()->second) = NULL;  // Set old DCMonEntry for this monomial to 0 since it was changed.
			monP->setDCPair(entryP, 0);
		} else {
			monP->getDCPair()->first->pToMon.at(monP->getDCPair()->second) = NULL;
			monP->setDCPair(NULL, 0);  // DC entries cancelled out each other. monomial has no longer dc Variables.
			if (monP->getFactor() == 0) this->eraseMonom(*monP);  // If monomial coefficient is 0, erase monomial since it has no dc Variable anymore.
		}
	} else {
		this->dcList.emplace_back(dcEntries, monP, dcCoefs);  // Add new entry for dcMonEntry.
		entryP = &(dcList.back());
		assert(entryP != NULL);
		monP->setDCPair(entryP, 0);
	}
}

//***************************************************************************************
std::pair<std::vector<int>, std::vector<int>> Polynom::mergeTwoEntries(std::vector<int>& firstVec, std::vector<int>& firstCoef, std::vector<int>& addVec, std::vector<int>& addCoef) {
	// Assumption: retVec and addVec are already sorted.
	std::vector<int> retVec;
	std::vector<int> retCoef;
	int firstPos = 0;
	int addPos = 0;
	bool firstStop = false;  // Remember that end of firstVec was reached.
	bool addStop = false;	// Remember that end of addVec was reached.
	if (firstVec.size() == 0) firstStop = true;
	if (addVec.size() == 0) addStop = true;
	int coefSum = 0;
	while (!(firstStop || addStop)) {  // Stop as soon as one vector has been traversed to the end.
		if (firstVec.at(firstPos) < addVec.at(addPos)) {  // Case 1: firstVec entry is smaller one.
			retVec.push_back(firstVec.at(firstPos));
			retCoef.push_back(firstCoef.at(firstPos));
			if (++firstPos == firstVec.size()) firstStop = true;
			continue;
		} else if (addVec.at(addPos) < firstVec.at(firstPos)) {  // Case 2: addVec entry is smaller one.
			retVec.push_back(addVec.at(addPos));
			retCoef.push_back(addCoef.at(addPos));
			if (++addPos == addVec.size()) addStop = true;
			continue;
		} else if (addVec.at(addPos) == firstVec.at(firstPos)) {  // Case 3: both entries are equal. Add up coefs and advance both positions.
			coefSum = firstCoef.at(firstPos) + addCoef.at(addPos);
			if (coefSum != 0) {
				retVec.push_back(firstVec.at(firstPos));
				retCoef.push_back(coefSum);
			}
			if (++firstPos == firstVec.size()) firstStop = true;
			if (++addPos == addVec.size()) addStop = true;
			continue;
		}
	}
	if (firstStop) {  // firstVec is finished. Add remaining of addVec to result vectors.
		while (!addStop) {
			retVec.push_back(addVec.at(addPos));
			retCoef.push_back(addCoef.at(addPos));
			//if (!addStop) {
			if (++addPos == addVec.size()) addStop = true;
		}
	}
	if (addStop) {  // addVec is finished. Add remaining of firstVec to result vectors.
		while (!firstStop) {
			retVec.push_back(firstVec.at(firstPos));
			retCoef.push_back(firstCoef.at(firstPos));
			//if (!addStop) {
			if (++firstPos == firstVec.size()) firstStop = true;
		}
	}
//	std::cout << "Vector: ";
//	for (auto& elem: retVec) {
//		std::cout << elem << "|";
//	}
//	std::cout << std::endl;
//	std::cout << "Coef: ";
//	for (auto& elem: retCoef) {
//		std::cout << elem << "|";
//	}
//	std::cout << std::endl;

	return std::pair<std::vector<int>, std::vector<int>>{retVec, retCoef};
}

//***************************************************************************************
void Polynom::clearDCList() {
	this->dcList.clear();
}

//***************************************************************************************
Polynom Polynom::adderSpecification(uint32_t sumSize, std::vector<uint32_t> outputs)
{
	mpz_class coef = 1;
	Polynom spec(outputs[sumSize - 1]+1);	// Add output signals.
	std::vector<Monom2> monomials;
	for (int i = 0; i < sumSize; i++)
	{
		Monom2 m(outputs[i]);
		m.setFactor(coef);
		monomials.push_back(m);
		coef *= 2;
	}
	mpz_class temp1;
	mpz_class temp2;
	mpz_class temp;
	mpz_class two = 2;
	for (int i = 0; i < (sumSize - 1); i++)  // Input A.
	{
		mpz_pow_ui (temp.get_mpz_t(), two.get_mpz_t(), i);
		temp*=-1;
		Monom2 m(i+1);
		m.setFactor(temp);
		monomials.push_back(m);

	}
	for (int i = 0; i < (sumSize - 1); i++)  // Input B.
	{
		mpz_pow_ui (temp.get_mpz_t(), two.get_mpz_t(), i);
		temp*=-1;
		Monom2 m(i + sumSize);
		m.setFactor(temp);
		monomials.push_back(m);

	}
//	Monom2 m(2*sumSize-1);  //  Add Cin signal.
//	m.setFactor(-1);
//	monomials.push_back(m);

	for (auto &i: monomials)
	{
		spec.addMonom(i);
	}
	return spec;
}

//***************************************************************************************
Polynom Polynom::multiplierSpecification(uint32_t multSize, std::vector<uint32_t> outputs)
{
	mpz_class coef = 1;
//	Polynom spec(outputs[multSize - 1]+1);
	Polynom spec(outputs[outputs.size() - 1] + 1);
	std::vector<Monom2> monomials;
//	for (int i = 0; i < multSize; i++)
	for (int i = 0; i < outputs.size(); i++)
	{
		Monom2 m(outputs[i]);
		m.setFactor(coef);
		monomials.push_back(m);
		coef *= 2;
	}
	mpz_class temp1;
	mpz_class temp2;
	mpz_class temp;
	mpz_class two = 2;
	for (int i = 0; i < multSize / 2; i++)
	{
		for (int j = 0; j < multSize / 2; j++)
		{
			mpz_pow_ui (temp1.get_mpz_t(), two.get_mpz_t(), i);
			mpz_pow_ui (temp2.get_mpz_t(), two.get_mpz_t(), j);
			temp = temp1*temp2;
			temp*=-1;

			Monom2 m (i+1, j+ (multSize / 2)+1);
			m.setFactor(temp);
			monomials.push_back(m);
		}
	}
	for (auto &i: monomials)
	{
		spec.addMonom(i);
	}
	return spec;
}

//***************************************************************************************
Polynom Polynom::dividerSpecification(int maxVar, std::vector<uint32_t> dividend, std::vector<uint32_t> divisor, std::vector<uint32_t> quotient, std::vector<uint32_t> remainder, std::vector<bool> quotientSigns, std::vector<bool> remainderSigns)
{
	mpz_class coef;
	Polynom spec(maxVar + 1);
	//std::vector<Monom2> monomials;

	Polynom dividendPol(maxVar + 1);
	coef = -1;
	for (int i = 0; i < dividend.size(); ++i)
	{
		Monom2 m(dividend[i]);
		m.setFactor(coef);
		dividendPol.addMonom(m);
		coef *= 2;
	}
	std::cout << "Dividend Pol is " << dividendPol << std::endl;

	Polynom divisorPol(maxVar + 1);
	coef = 1;
	for (int i = 0; i < divisor.size(); ++i)
	{
		Monom2 m(divisor[i]);
		m.setFactor(coef);
		divisorPol.addMonom(m);
		coef *= 2;
	}
	std::cout << "Divisor Pol is " << divisorPol << std::endl;

	assert(quotient.size() == quotientSigns.size());
	Polynom quotientPol(maxVar + 1);
	coef = 1;
	for (int i = 0; i < quotient.size(); ++i)
	{
		Monom2 m(quotient[i]);
		m.setFactor(coef);
		quotientPol.addMonom(m);
		if (!quotientSigns[i]) {
			quotientPol.replaceBUFFER(quotient[i], 0);
			quotientPol.replaceNOT(0, quotient[i]);
		}
		coef *= 2;
	}
	std::cout << "Quotient Pol is " << quotientPol << std::endl;

	assert(remainder.size() == remainderSigns.size());
	Polynom remainderPol(maxVar + 1);
	coef = 1;
	for (int i = 0; i < remainder.size(); ++i)
	{
		Monom2 m(remainder[i]);
		m.setFactor(coef);
		remainderPol.addMonom(m);
		if (!remainderSigns[i]) {
			remainderPol.replaceBUFFER(remainder[i], 0);
			remainderPol.replaceNOT(0, remainder[i]);
		}
		coef *= 2;
		//if (i == remainder.size() - 2) coef *= -1;
	}
	std::cout << "Remainder Pol is " << remainderPol << std::endl;

	spec.addPolynom(dividendPol);
	spec.addPolynom(remainderPol);
	spec.addPolynom(multiplyPoly(divisorPol, quotientPol));

	std::cout << "Specification polynomial is " << spec << std::endl;

	return spec;
}

//***************************************************************************************
std::ostream& operator<<(std::ostream& stdout, const Polynom& obj) {
    std::string start = "", end = "", delim = " + ";
    std::string s;
    int size = obj.getSet()->size();
    if (!obj.getSet()->empty()){
        s += start;
        int num = 0;
        for (std::set<Monom2>::iterator it=obj.getSet()->begin(); it != obj.getSet()->end(); ++it) {
            s += (*it).to_string();
            num += 1;
            //s += Integer.toString(obj.vars[i]);
            if (num != size) s += delim;
        }
        s += end;
        //Add factor at the end
        //std::string fact = "[" + std::to_string(obj.getFactor()) + "]";
        //s += fact;
    }
    else{
        //s += start;
        //s += end;
        s += "0 (empty polynomial)";
    }
    stdout << s;
    return stdout;
}

//***************************************************************************************
void Polynom::mod2n(mpz_class modNum) {
	std::vector<Monom2> toDelete;
	for (auto& elem: this->polySet) {
		//const_cast<Monom2&> (elem).setFactor();
		mpz_mod(elem.factor.get_mpz_t(), elem.getFactor().get_mpz_t(), modNum.get_mpz_t());
		if (elem.getFactor() == 0) toDelete.push_back(elem);
	}
	for (auto& elem: toDelete) {
		this->eraseMonom(elem);
	}
}

//***************************************************************************************
void Polynom::setModReduction(bool mode) {
	this->modReductionEnabled = mode;
}

//***************************************************************************************
void Polynom::setModReductionNumber(mpz_class modNum) {
	this->coefModReduction = modNum;
}
