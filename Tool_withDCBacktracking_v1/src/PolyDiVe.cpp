//============================================================================
// Name        : PolyDiVe.cpp
// Author      : Alexander Konrad
// Version     :
// Copyright   : Copyright 2020
// Description : Tool for formal verification of dividers with help of SCA.
//============================================================================

#include <list>
#include <iostream>
#include <iomanip>
#include <random>
#include "circuit/Circuit.h"
#include "circuit/ParseError.h"
#include "polynomial/Monom2.h"
#include "polynomial/Polynom.h"
#include "Verifizierer.h"
#include "math.h"
#include "time.h"
//#include "gurobi/gurobi_c++.h"
//#include <thread>
//#include <chrono>

using namespace std;
using namespace vp;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << " Wrong number of arguments. Usage: " << argv[0] << " [Verilog Circuit File] " << " [Signature File] " << endl;
		return 1;
	}

	cout << "************************************************************************" << endl;
	cout << "VERIFICATION OF DIVIDER CIRCUIT STARTED" << endl;
	cout << "************************************************************************" << endl;

	// Old verilog verifier.
	double timeOverall = 0.0;
	double tstartOverall = clock();
	
	//*
	double timeParsing = 0.0;
	double tstartParsing = clock();
	Verifizierer test1(argv[1]);
	//test1.pMap = test1.createDividerPrimarySignalsMap();
	timeParsing += clock() - tstartParsing; // Zeitmessung endet.
	timeParsing = timeParsing/CLOCKS_PER_SEC;
	cout << "Parsing part needed time: " << timeParsing << endl;
//	return 0;
	double timeDC = 0.0;
	double tstartDC = clock();
//	std::vector<Atomic> atomics2;
//	test1.provenDCs = test1.checkDCCandidates(atomics2);
	test1.generalDCs = test1.checkDCCandidatesGeneral();
	timeDC += clock() - tstartDC; // Zeitmessung endet.
	timeDC = timeDC/CLOCKS_PER_SEC;
	cout << "Complete DC computation needed time: " << timeDC << endl;
	double timeRewriting = 0.0;
	double tstartRewriting = clock();
	test1.startAdvancedVerification(argv[2]);
	timeRewriting += clock() - tstartRewriting; // Zeitmessung endet.
	timeRewriting = timeRewriting/CLOCKS_PER_SEC;
	cout << "Complete rewriting time: " << timeRewriting << endl;

	cout << "************************************************************************" << endl;
	cout << "SUMMARY TIME NEEDED " << endl;
	cout << "Parsing part needed time: " << timeParsing << endl;
	cout << "Complete DC computation needed time: " << timeDC << endl;
	cout << "Complete rewriting time: " << timeRewriting << endl;
	cout << "Overall time: " << timeRewriting + timeDC + timeParsing << endl;

	return 0;

}
