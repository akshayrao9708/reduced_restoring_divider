/*
 * gendc.hpp
 *
 *  Created on: 18.06.2021
 *      Author: Alexander Konrad
 */

#include <string>
#include <vector>

#pragma once

struct fullAdder {
	int or1;
	int and1;
	int and2;
	int xor1;
	int xor2;
	int maxLevel;
	int prevIn1 = -1;
	int prevIn2 = -1;
	int prevCin = -1;
	std::vector<int> nextOr;
	std::vector<int> nextXor;
	bool prevIn1IsFA = false;
	bool prevIn2IsFA = false;
	bool prevCinIsFA = false;
	std::vector<bool> nextOrIsFA;
	std::vector<bool> nextXorIsFA;

	fullAdder()
	  : or1(-1), and1(-1), and2(-1), xor1(-1), xor2(-1), maxLevel(-1)
	{}

	fullAdder(int or1, int and1, int and2, int xor1, int xor2)
	  : or1(or1), and1(and1), and2(and2), xor1(xor1), xor2(xor2), maxLevel(-1)
	{}
};
