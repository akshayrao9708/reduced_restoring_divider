/*
 * gendc.hpp
 *
 *  Created on: 18.06.2021
 *      Author: Alexander Konrad
 */

#include <string>
#include <vector>
#include <unordered_set>
#include "./fullAdder.hpp"

#pragma once

struct extBlock{
	std::unordered_set<int> memNodes;
	std::unordered_set<int> fanIns;
	std::unordered_set<int> fanOuts;
	fullAdder containedAdder;
	bool levelIsSet = false;

	extBlock(int singleNode) {
		this->memNodes.insert(singleNode);
	}

	extBlock(fullAdder& adder): containedAdder(adder) {
		this->memNodes.insert(adder.or1);  // Add adder nodes.
		this->memNodes.insert(adder.and1);
		this->memNodes.insert(adder.and2);
		this->memNodes.insert(adder.xor1);
		this->memNodes.insert(adder.xor2);
		this->fanIns.insert(adder.prevIn1);  // Add adder fanIns.
		this->fanIns.insert(adder.prevIn2);
		this->fanIns.insert(adder.prevCin);
		this->fanOuts.insert(adder.nextOr.begin(), adder.nextOr.end());
		this->fanOuts.insert(adder.nextXor.begin(), adder.nextXor.end());
	}

	void addNode(int newNode) {
		this->memNodes.insert(newNode);
	}

};
