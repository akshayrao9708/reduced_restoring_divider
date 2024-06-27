/**************************************************************
*       
*       Polynom Package // Monom2.h
*
*       Author:
*         Alexander Konrad
*         University of Freiburg
*         konrada@informatik.uni-freiburg.de
*
***************************************************************/

// std includes.
#include <string>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include<numeric>
#include<algorithm>
#include<set>
// Gnu multiprecision library.
#include <gmpxx.h>

// Local includes.
#include "MyList.h"
#include "dcMonEntry.hpp"

#ifndef MONOM2_H_
#define MONOM2_H_

// Integers used for variable indices. 
typedef int varIndex;

// Class to represent a monomial.
class Monom2 {

	friend class Polynom;
	public:	
		// Constructors.
		Monom2();
		Monom2(const Monom2& old);  // Copy constructor.
		Monom2& operator = (const Monom2& old); // Assignment operator.
		Monom2(varIndex index);
		Monom2(varIndex index1, varIndex index2);
		Monom2(varIndex myints[], int size);  // This constructor sorts myints array increasingly and removes all duplicates.
		//Monom2(varIndex myints[], int size, int sum, int64_t factor);
		Monom2(varIndex myints[], int size, int sum, mpz_class factor);  // Use this constructor only for already sorted and duplicate free myints.
		
		// Destructor.
		virtual ~Monom2();
		
		// Getters and Setters.
		varIndex* getVars() const;
		MyList::ListElement** getPtrs() const;
		void setPtrs(MyList::ListElement** ptrs);
		int getSize() const;
		void setSize(int size);
		//int64_t getFactor() const;
		mpz_class getFactor() const;
		//void setFactor(int64_t fact);
		void setFactor(mpz_class fact);
		int getSum() const;
		void setSum(int newSum);
		const std::pair<dcMonEntry*, size_t>*	getDCPair() const;
		void setDCPair(dcMonEntry* entryP, size_t pos);
		int dc_sign=0;
		int dc_size =0;
		int dc_cnt =0;
		int dc_diff=0;
		void set_values(int v0, int v1, int v2 =0, int v3=0){
			dc_sign = v0;
			dc_size =v1;
			dc_cnt =v2;
			dc_diff =v3;
		}
	
		// Calculate sum by adding all variable indices.
		int calculateSum() const;
		
		// Helping method for output stream of polynomials.
		const std::string to_string() const;
		
		// Look for a variable in monomial using binary search.
		bool containsVar(varIndex v);
		
		// Comparison operators.
		bool operator>(const Monom2 &m1) const;
		bool operator<(const Monom2 &m1) const;
		bool operator==(const Monom2 &m1) const;
		bool operator!=(const Monom2 &m1) const;
		
		// Replacing one variable in monomial by a whole new monomial mon. Return resulting monomial.
		Monom2 merge(varIndex replace, Monom2 mon);
		
		
		
		// Multiply two monomials and return resulting monomial.
		static Monom2 multiply(Monom2 mon1, Monom2 mon2);
		
		// Output stream operator.
		friend std::ostream& operator<<(std::ostream& stdout, const Monom2& obj);
		 Monom2 removeVars(const std::set<varIndex>& varsToRemove) const;
	
	
	private:
		// Array for all variables of monomial. 
		varIndex *vars;
		
		// Helper variables to track size (# of variables) and sum (sum of indices) of monomial.
		int size;
		int sum;
		
		// Coefficient of monomial. GMP library used because coefficients fastly exceed int64 range.
		mutable mpz_class factor;
		
		// Pointer back to ListElement entry, used for enabling constant deletion of elements from the list.
		MyList::ListElement** ptrs;

		// Pair of pointer and position for dcMonEntry of this monomial.
		std::pair<dcMonEntry*, size_t> dcPair;


};

#endif /* MONOM2_H_ */
