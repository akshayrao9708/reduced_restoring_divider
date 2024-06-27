/**************************************************************
*       
*       Polynom Package // Monom2.cpp
*
*       Author:
*         Alexander Konrad
*         University of Freiburg
*         konrada@informatik.uni-freiburg.de
*
***************************************************************/

#include "Monom2.h"

//***************************************************************************************
Monom2::Monom2(){  // Empty constructor
	this->vars = 0;
	this->ptrs = 0;
	this->size = 0;
	this->sum = 0;
	this->factor = 0;
	this->dcPair = {NULL, 0};
}

//***************************************************************************************
Monom2::Monom2(const Monom2& old){  // Copy constructor
	this->vars = new int[old.size];
	this->ptrs = new MyList::ListElement*[old.size];
	std::memcpy(this->vars, old.vars, old.size * sizeof(varIndex));
	std::memcpy(this->ptrs, old.ptrs, old.size * sizeof(MyList::ListElement*));
	this->size = old.size;
	this->sum = old.sum;
	this->factor = old.factor;
	this->dcPair = old.dcPair;
}

//***************************************************************************************
Monom2& Monom2::operator= (const Monom2 &old)  // Assignment operator.
{ 
   	// Check for self assignment 
   	if(this != &old) {
   		delete[] this->vars;
		delete[] this->ptrs;
		this->vars = new int[old.size];
		this->ptrs = new MyList::ListElement*[old.size];
		std::memcpy(this->vars, old.vars, old.size * sizeof(varIndex));
		std::memcpy(this->ptrs, old.ptrs, old.size * sizeof(MyList::ListElement*));
		this->size = old.size;
		this->sum = old.sum;
		this->factor = old.factor;
		this->dcPair = old.dcPair;
  	}
   	return *this; 
} 

//***************************************************************************************
Monom2::Monom2(varIndex index) {
	this->vars = new int[1];
	this->ptrs = new MyList::ListElement*[1];
	this->vars[0] = index;
	this->size = 1;
	this->sum = index;
	this->factor = 1;
	this->dcPair = {NULL, 0};
}

//***************************************************************************************
Monom2::Monom2(varIndex index1, varIndex index2) {
	// Assure no duplicates of variables.
	if (index1 == index2) {
		this->vars = new int[1];
		this->ptrs = new MyList::ListElement*[1];
		this->vars[0] = index1;
		this->size = 1;
		this->sum = index1;
	} else {
		this->vars = new int[2];
		this->ptrs = new MyList::ListElement*[2]; 
		// Already sort variables in increasing way.
		if (index1 < index2) {
			this->vars[0] = index1;
			this->vars[1] = index2;
		} else {
			this->vars[0] = index2;
			this->vars[1] = index1;
		}
		this->size = 2;
		this->sum = index1 + index2;
	}
	this->factor = 1;
	this->dcPair = {NULL, 0};
}

//***************************************************************************************
Monom2::Monom2(varIndex myints[], int size) {
	// Before creating monomial, order given array myints and remove duplicates.
	std::sort(myints, myints + size);  // Sort array.
	int* ptr = 0;
	ptr = std::unique(myints, myints + size);  // Remove duplicates.
	int newSize = std::distance(myints, ptr);  // Calculate new size of array after removing duplicates.
	int calcSum = 0;
	
	this->vars = new int[newSize];
	this->ptrs = new MyList::ListElement*[newSize];
	for (int i=0; i < newSize; i++) {
		this->vars[i] = myints[i];
		calcSum += myints[i];  // Calculate sum simultaneously.
	}
	this->size = newSize;
	this->sum = calcSum;
	this->factor = 1;
	this->dcPair = {NULL, 0};
}

//***************************************************************************************
Monom2::Monom2(varIndex myints[], int size, int sum, mpz_class factor) {
	// Use this function only for already sorted and duplicate free myints.
	this->vars = new int[size];
	this->ptrs = new MyList::ListElement*[size];
	for (int i=0; i < size; i++) {
		this->vars[i] = myints[i];
	}
	this->size = size;
	this->sum =  sum;
	this->factor = factor;
	this->dcPair = {NULL, 0};
}

//***************************************************************************************
Monom2::~Monom2() {
	delete[] this->vars;
	delete[] this->ptrs;
}

//***************************************************************************************
varIndex* Monom2::getVars() const{
	return this->vars;
}

//***************************************************************************************
MyList::ListElement** Monom2::getPtrs() const{
	return this->ptrs;
}

//***************************************************************************************
void Monom2::setPtrs(MyList::ListElement** ptrs) {
	this->ptrs = ptrs;
}

//***************************************************************************************
int Monom2::getSize() const {
	return this->size;
}

//***************************************************************************************
void Monom2::setSize(int size) {
	this->size = size;
}

//***************************************************************************************
mpz_class Monom2::getFactor() const {
	return this->factor;
}

//***************************************************************************************
void Monom2::setFactor(mpz_class fact) {
	this->factor = fact;
}

//***************************************************************************************
int Monom2::getSum() const {
	return this->sum;
}

//***************************************************************************************
void Monom2::setSum(int newSum) {
	this->sum = newSum;
}

//***************************************************************************************
const std::pair<dcMonEntry*, size_t>* Monom2::getDCPair() const {
	return &this->dcPair;
}

//***************************************************************************************
void Monom2::setDCPair(dcMonEntry* entryP, size_t pos) {
	this->dcPair.first = entryP;
	this->dcPair.second = pos;
}

//***************************************************************************************
bool Monom2::containsVar(varIndex v) {
	return std::binary_search(this->vars, this->vars + this->size, v);
}

//***************************************************************************************
int Monom2::calculateSum() const{
	int result = 0;
	for (int i = 0; i < this->size; i++) {
		result += this->vars[i];
	}
	return result;
}

//***************************************************************************************
bool Monom2::operator>(const Monom2 &m1) const {
	return (m1 < *this);
	//return false;
}

//***************************************************************************************
bool Monom2::operator<(const Monom2 &m1) const {
	if (this->getSum() != m1.getSum()) return this->getSum() < m1.getSum();
	if (this->getSize() != m1.getSize()) return this->getSize() < m1.getSize(); 
	for (int i=0; i < this->getSize(); i++) {
		if (this->vars[i] != m1.vars[i]) return this->vars[i] < m1.vars[i];	
	}
	return false;
}

//***************************************************************************************
bool Monom2::operator==(const Monom2 &m1) const {
	if (this->getSize() != m1.getSize()) return false;
	if (this->getSum() != m1.getSum()) return false;	
	for (int i=0; i < this->getSize(); i++) {
		if (!(this->vars[i] == m1.vars[i])) return false;
	}
	return true;
}

//***************************************************************************************
bool Monom2::operator!=(const Monom2 &m1) const {
	return !(*this == m1);
}

//***************************************************************************************
std::ostream& operator<<(std::ostream& stdout, const Monom2& obj) {
	//std::string start = "[" + std::string(mpz_get_str(NULL, 0, obj.getFactor().get_mpz_t())) + "*" , end = "]", delim = "*";
	std::string s;
	int elem = 0;
	int elemCoef = 0;
	if (obj.dcPair.first != NULL) {
//		s += std::to_string(obj.dcPair.first->coefToMon.at(obj.dcPair.second));
//		s += "*";
		s += "(";
		for (size_t i=0; i < obj.dcPair.first->dcVars.size(); ++i) {
			elem = obj.dcPair.first->dcVars.at(i);
			elemCoef = obj.dcPair.first->coefs.at(i);
			if (elemCoef >= 0) s+= "+";
//			else s+= "+";
			s+= std::to_string(elemCoef);
			s+= "*";
			s+= "v" + std::to_string(elem);
		}
		s += ")";
	}
	std::string start = "[" + obj.getFactor().get_str() + "*" , end = "]", delim = "*";
    	if (obj.getSize() > 0){
        	s += start;
        	for (int i = 0; i < obj.getSize(); i++){
            	s += "x" + std::to_string(obj.vars[i]);
            	//s += Integer.toString(obj.vars[i]);
            	if (i != obj.getSize() - 1) s += delim;
        	}
        	s += end;
        	//Add factor at the end
        
    	}
    	else{
        	s += start;
        	s += end;
    	}
   	stdout << s;
   	return stdout;
}

//***************************************************************************************
const std::string Monom2::to_string() const {
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

//***************************************************************************************
Monom2 Monom2::merge(varIndex replace, Monom2 mon) {
	// Use this function only if you are sure that "this object" includes "replace" variable. 
	int newSize = this->size + mon.getSize() - 1; // Calculate new monom length. It will be one shorter since "replace" index will be replaced.
	if (newSize < 0) { // Merge empty monoms.
		Monom2 empty;
		empty.setFactor(this->factor * mon.getFactor());
		return empty;
	}
	varIndex* result = new int[newSize]; // Create new array for saving monom temporary.
	varIndex one = -1; // Used for variable indexes.
	varIndex two = -1;
	int pos1 = 0; // Used for current position in monom. 
	int pos2 = 0;
	int equal = 0; // Count how often an index is doubled.
	bool oneEnd = false; // Notice end of monoms.
	bool twoEnd = false;
	bool oneEmpty = false;  //Notice if one is the empty monom(e.g. just a factor).
	bool twoEmpty = false; 
	int sum = 0;  // While merging, calculate sum simultaneously.
	if (this->size == 0) { oneEmpty = true; }
	if (mon.getSize() == 0) { twoEmpty = true; }
	for(int i = 0; i < newSize; i++) {
		if (!oneEmpty) {
			one = this->vars[pos1];
			if (one == replace) { // Do not add replaced variable.
				pos1 += 1;
				if (pos1 == this->size) { 
			 		oneEnd = true;
			 		pos1--;
				} else {	 
					one = this->vars[pos1];
				}
			}
		} else {
			oneEnd = true;
		}
		if (!twoEmpty) {
			two = mon.vars[pos2];
			if (two == replace) { // Do not add replaced variable.
				pos2 += 1;
				if (pos2 == mon.getSize()) { 
					twoEnd = true;
				 	pos2--;
				} else {	
					two = mon.vars[pos2];
				}
			}
		} else {
			twoEnd = true;
		}

		if (!oneEnd && !twoEnd) {
			if (one == two) { // Both have current variable. Only need to copy one variable into new Monom.
				result[i - equal] = one;  // Copy variable index.
				sum += one;  // Calculate sum simultaneously.
				equal++;
			 	i++;
			 	pos1++;
			 	pos2++;
			 	if (pos1 == this->size) { 
			 		oneEnd = true;
			 		pos1--;
			 	}	 
			 	if (pos2 == mon.getSize()) { 
			 		twoEnd = true;
			 		pos2--;	
			 	}	
			} else { 
				if (one < two) {
		 			result[i - equal] = one;
		 			sum += one;  // Calculate sum simultaneously.
		 			pos1++;
		 			if (pos1 == this->size) { 
			 			oneEnd = true;
			 			pos1--;
			 		}	
				} else {
					result[i - equal] = two; 
					sum += two;  // Calculate sum simultaneously.
					pos2++;
					if (pos2 == mon.getSize()) { 
			 			twoEnd = true;
			 			pos2--;	
			 		}
				}
			}
		} else { // One of monoms already copied completely.
			if (oneEnd) {
				result[i - equal] = two;
				sum += two;  // Calculate sum simultaneously.
				if (one == two) {
					equal++;
				 	i++;
				 	pos2++;
				} else { 
					pos2++;
				}
			} else if (twoEnd) {
				result[i - equal] = one;
				sum += one;  // Calculate sum simultaneously.
				pos1++;
				if (one == two) {	
					equal++;
				 	i++;
				}
			}
		}			
	}
	Monom2 nMon(result, newSize - equal, sum, this->factor * mon.getFactor());
	// Free used memory.
	delete[] result;
	return nMon;
}

//***************************************************************************************
Monom2 Monom2::multiply(Monom2 mon1, Monom2 mon2) {
	int newSize = mon1.getSize() + mon2.getSize(); // Calculate new monom length.
	if (newSize < 0) { // Merge empty monoms.
		Monom2 empty;
		empty.setFactor(mon1.getFactor() * mon2.getFactor());
		return empty;
	}
	varIndex* result = new int[newSize]; // Create new array for saving monom temporary.
	varIndex one = -1; // Used for variable indexes.
	varIndex two = -1;
	int pos1 = 0; // Used for current position in monom. 
	int pos2 = 0;
	int equal = 0; // Count how often an index is doubled.
	bool oneEnd = false; // Notice end of monoms.
	bool twoEnd = false;
	bool oneEmpty = false;  //Notice if one is the empty monom (e.g. just a factor).
	bool twoEmpty = false; 
	int sum = 0;  // While merging, calculate sum simultaneously.
	if (mon1.getSize() == 0) {oneEmpty = true;}
	if (mon2.getSize() == 0) {twoEmpty = true;}
	for(int i = 0; i < newSize; i++) {
		if (!oneEmpty) {
			one = mon1.vars[pos1];
		} else {
			oneEnd = true;
		}
		
		if (!twoEmpty) {
			two = mon2.vars[pos2];
		} else {
			twoEnd = true;
		}

		if (!oneEnd && !twoEnd) {
			if (one == two) { // Both have current variable. Only need to copy one variable into new Monom.
				result[i - equal] = one;  // Copy variable index.
				sum += one;  // Calculate sum simultaneously.
				equal++;
			 	i++;
			 	pos1++;
			 	pos2++;
			 	if (pos1 == mon1.size) { 
			 		oneEnd = true;
			 		pos1--;
			 	}	 
			 	if (pos2 == mon2.getSize()) { 
			 		twoEnd = true;
			 		pos2--;	
			 	}	
			} else { 
				if (one < two) {
		 			result[i - equal] = one;
		 			sum += one;  // Calculate sum simultaneously.
		 			pos1++;
		 			if (pos1 == mon1.size) { 
			 			oneEnd = true;
			 			pos1--;
			 		}	
				} else {
					result[i - equal] = two; 
					sum += two;  // Calculate sum simultaneously.
					pos2++;
					if (pos2 == mon2.getSize()) { 
			 			twoEnd = true;
			 			pos2--;	
			 		}
				}
			}
		} else { // One of monoms already copied completely.
			if (oneEnd) {
				result[i - equal] = two;
				sum += two;  // Calculate sum simultaneously.
				if (one == two) {
					equal++;
				 	i++;
				 	pos2++;
				} else { 
					pos2++;
				}
			} else if (twoEnd) {
				result[i - equal] = one;
				sum += one;  // Calculate sum simultaneously.
				pos1++;
				if (one == two) {	
					equal++;
				 	i++;
				}
			}
		}		
	}
	Monom2 nMon(result, newSize - equal, sum, mon1.factor * mon2.getFactor());
	// Free used memory.
	delete[] result;
	return nMon;
}
Monom2 Monom2::removeVars(const std::set<varIndex>& varsToRemove) const {
    std::vector<varIndex> newVars;
    for (int i = 0; i < this->size; ++i) {
        if (varsToRemove.find(this->vars[i]) == varsToRemove.end()) {
            newVars.push_back(this->vars[i]);
        }
    }

    int newSize = newVars.size();
    int newSum = std::accumulate(newVars.begin(), newVars.end(), 0);
    varIndex* newVarsArray = new varIndex[newSize];
    std::copy(newVars.begin(), newVars.end(), newVarsArray);

    return Monom2(newVarsArray, newSize, newSum, this->factor);
}