/*
 * 4inputdc.hpp
 *
 *  Created on: 25.05.2021
 *      Author: Alexander Konrad
 */

#include <string>

struct input4dc{
	int lvl;
	int sumIndex;
	mutable std::string sig1;
	mutable std::string sig2;
	mutable std::string sig3;
	mutable std::string sig4;
	mutable bool poss[16] = {false};
	mutable int count;
	mutable bool activated;

	input4dc()
		 : sumIndex(0), sig1(""), sig2(""), sig3(""), sig4(""), lvl(0), count(0), activated(true)
	{}

	input4dc(int sIndex, std::string s1, std::string s2, std::string s3, std::string s4, int level)
	  : sumIndex(sIndex), sig1(s1), sig2(s2), sig3(s3), sig4(s4), lvl(level), count(0), activated(true)
	{}

	input4dc(const input4dc& old) {  // Copy constructor.
		this->lvl = old.lvl;
		this->sumIndex = old.sumIndex;
		this->sig1 = old.sig1;
		this->sig2 = old.sig2;
		this->sig3 = old.sig3;
		this->sig4 = old.sig4;
		for (size_t i=0; i<16; i++) {
			this->poss[i] = old.poss[i];
		}
		this->count = old.count;
		this->activated = old.activated;
	}
	bool operator<(const input4dc& elem) const{
		if (lvl > elem.lvl) return true;  // Careful: Compare with > since we consider reverse levels.
		else return false;
	}
	bool operator>(const input4dc& elem) const{
		if (lvl < elem.lvl) return true;
		else return false;
	}
	bool operator==(const input4dc& elem) const{
		if (lvl == elem.lvl && sig1 == elem.sig1 && sig2 == elem.sig2 && sig3 == elem.sig3 && sig4 == elem.sig4) return true;
		else return false;
	}
	bool operator!=(const input4dc& elem) const{
		return !(*this == elem);
	}
};
