/*
 * gendc.hpp
 *
 *  Created on: 18.06.2021
 *      Author: Alexander Konrad
 */

#include <string>
#include <vector>

#pragma once

struct gendc{
	int lvl;
	mutable std::vector<std::string> signals;
	mutable bool* poss;
	int possSize;
	mutable int count;
	mutable bool activated;

	gendc()
		 : lvl(0), count(0), poss(NULL), possSize(0), activated(true)
	{}

	gendc(std::string s1, std::string s2, std::string s3, int level)  //Special case 3 input dc.
	  : lvl(level), count(0), possSize(8), activated(true)
	{	this->signals.push_back(s1); this->signals.push_back(s2); this->signals.push_back(s3);
		//cout << this->possSize << endl;
		poss = new bool[this->possSize];
		for (size_t i=0; i < this->possSize; i++) {
			this->poss[i] = false;
		}
	}

	gendc(std::vector<string>& newSignals, int givenLvl = 0) {  // General input length dc with level set.
		this->lvl = givenLvl;
		this->signals = newSignals;
		this->possSize = pow(2, newSignals.size());
		this->poss = new bool[this->possSize];
		for (size_t i=0; i< possSize; i++) {
			this->poss[i] = false;
		}
		this->count = 0;
		this->activated = true;
	}

	gendc(const gendc& old) {  // Copy constructor.
		this->lvl = old.lvl;
		this->signals = old.signals;
		this->possSize = old.possSize;
		this->poss = new bool[this->possSize];
		for (size_t i=0; i< old.possSize; i++) {
			this->poss[i] = old.poss[i];
		}
		this->count = old.count;
		this->activated = old.activated;
	}
	bool operator<(const gendc& elem) const{
		if (lvl > elem.lvl) return true;  // Careful: Compare with > since we consider reverse levels.
		else return false;
	}
	bool operator>(const gendc& elem) const{
		if (lvl < elem.lvl) return true;
		else return false;
	}
	bool operator==(const gendc& elem) const{
		if (lvl != elem.lvl) return false;
		if (signals.size() != elem.signals.size()) return false;
		for (size_t i=0; i < this->signals.size(); i++) {
			if (signals.at(i) != elem.signals.at(i)) return false;
		}
		return true;
	}
	bool operator!=(const gendc& elem) const{
		return !(*this == elem);
	}

	int size() const {
		return this->signals.size();
	}

	~gendc() {  // Destructor.
		delete[] this->poss;
	}

	friend std::ostream& operator<<(std::ostream& stdout, const gendc& obj) {
		std::string start = "", end = "   lvl: ", delim = " | ";
		std::string s;
	    	if (obj.size() > 0){
	        	s += start;
	        	for (int i = 0; i < obj.size(); i++){
	            	s += obj.signals.at(i);
	            	if (i != obj.size() - 1) s += delim;
	        	}
	        	s += end;
	        	s += to_string(obj.lvl);
	        	s += "\n";
	        	for (int i = 0; i < obj.possSize; i++){
	        		s += to_string(obj.poss[i]);
	        	   	if (i != obj.possSize - 1) s += delim;
	        	}
	        	s += " count: ";
	        	s += to_string(obj.count);
	    	}
	    	else{
	        	s += start;
	        	s += end;
	    	}
	   	stdout << s;
	   	return stdout;
	}

};
