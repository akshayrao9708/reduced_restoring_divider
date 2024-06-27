/*
 * dcMonEntry.hpp
 *
 *  Created on: 02.07.2021
 *      Author: Alexander Konrad
 */

#include <string>
#include <vector>

#pragma once

struct dcMonEntry{
	std::vector<int> dcVars;
	mutable std::vector<int> coefs;
	mutable std::vector<Monom2*> pToMon;
//	mutable std::vector<int> coefToMon;


	dcMonEntry(std::vector<int>& vars) {
		this->dcVars = vars;
	}

	dcMonEntry(std::vector<int>& vars, Monom2* mon, std::vector<int>& coefs) {
		this->dcVars = vars;
		this->pToMon.push_back(mon);
//		this->coefToMon.push_back(coef);
		this->coefs = coefs;

	}

	dcMonEntry(std::vector<int>& vars, std::vector<Monom2*>& mons, std::vector<int>& coefs) {
		this->dcVars = vars;
		this->pToMon = mons;
		this->coefs = coefs;
	}

	dcMonEntry(const dcMonEntry& old) {  // Copy constructor.
		this->dcVars = old.dcVars;
		this->pToMon = old.pToMon;
		this->coefs = old.coefs;
	}

	dcMonEntry(const dcMonEntry& old, int removeLessEqualVar) {  // Construct new Entry from old Entry by removing <= variables.
		for (size_t i=0; i < old.dcVars.size(); ++i) {
			if (old.dcVars.at(i) > removeLessEqualVar) {
				this->dcVars.push_back(old.dcVars.at(i));
				this->coefs.push_back(old.coefs.at(i));
			}
		}
		this->pToMon = old.pToMon;
//		std::cout << " pToMon size is: " << this->pToMon.size() << "|" << old.pToMon.size() << std::endl;
//		std::cout << " in dcMonEntry creation " << std::endl;
//		for (size_t i=0; i < this->dcVars.size(); ++i) {
//			std::cout << "|" << this->coefs.at(i) << " * " << this->dcVars.at(i);
//		}
//		std::cout << std::endl;
//		for (size_t j=0; j < this->pToMon.size(); ++j) {
//			if (true || this->pToMon.at(j) != NULL) {
//				std::cout << this->pToMon.at(j) << std::endl;
//			}
//		}
	}

	bool operator==(const dcMonEntry& elem) const{
		if (this->dcVars.size() != elem.dcVars.size()) return false;
		for (size_t i=0; i < this->dcVars.size(); i++) {
			if (this->dcVars.at(i) != elem.dcVars.at(i)) return false;
		}
		return true;
	}

	bool operator<(const dcMonEntry& elem) const{
		if (this->dcVars.size() < elem.dcVars.size()) return true;
		if (this->dcVars.size() > elem.dcVars.size()) return false;
		for (size_t i=0; i < this->dcVars.size(); i++) {
			if (this->dcVars.at(i) < elem.dcVars.at(i)) return true;
		}
		return false;
	}

	void addPointer(Monom2* newPointer) {
		pToMon.push_back(newPointer);
	}

//	void addCoef(int newCoef) {
//		coefToMon.push_back(newCoef);
//	}

	void addMonEntry(Monom2* newPointer) {
		pToMon.push_back(newPointer);
//		coefToMon.push_back(newCoef);
	}

	void changePointer(size_t pos, Monom2* newPointer) {
		pToMon.at(pos) = newPointer;
	}

//	void changeCoef(size_t pos, int newCoef) {
//		coefToMon.at(pos) = newCoef;
//	}

	size_t size() {
		return this->dcVars.size();
	}

	void multCoefs(int multiply) {
		for (size_t i=0; i < coefs.size(); ++i) {
			coefs.at(i) = coefs.at(i) * multiply;
		}
	}
};
