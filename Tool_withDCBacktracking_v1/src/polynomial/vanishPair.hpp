/*
 * dcMonEntry.hpp
 *
 *  Created on: 02.07.2021
 *      Author: Alexander Konrad
 */

#pragma once

struct vanishPair{
	friend class Polynom;

	int var1;
	int var2;
	bool phase1;  // True: Variable has not to be negated, False: Variable has to be negated. Needed for AIGs.
	bool phase2;

	vanishPair() {
		this->var1 = 0;
		this->var2 = 0;
		this->phase1 = true;
		this->phase2 = true;
	}

	vanishPair(int var1, int var2, bool phase1, bool phase2) {
		this->var1 = var1;
		this->var2 = var2;
		this->phase1 = phase1;
		this->phase2 = phase2;
	}

	bool operator==(const vanishPair& elem) const{
		if (this->var1 != elem.var1) return false;
		if (this->var2 != elem.var2) return false;
		if (this->phase1 != elem.phase1) return false;
		if (this->phase2 != elem.phase2) return false;
		return true;
	}

	bool operator!=(const vanishPair& elem) const{
		return !(*this == elem);
	}

	bool operator<(const vanishPair& elem) const{
		if (this->var1 != elem.var1) return this->var1 < elem.var1;
		if (this->var2 != elem.var2) return this->var2 < elem.var2;
		if (this->phase1 != elem.phase1) return this->phase1 < elem.phase1;
		if (this->phase2 != elem.phase2) return this->phase2 < elem.phase2;
		return false;
	}

	bool operator>(const vanishPair& elem) const{
		return (elem < *this);
	}
};
