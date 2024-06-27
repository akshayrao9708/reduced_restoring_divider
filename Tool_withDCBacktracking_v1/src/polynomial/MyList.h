/**************************************************************
*       
*       Polynom Package // MyList.h
*
*       Author:
*         Alexander Konrad
*         University of Freiburg
*         konrada@informatik.uni-freiburg.de
*
***************************************************************/

#include <string>

#ifndef MYLIST_H_
#define MYLIST_H_

class Monom2;

class MyList {

	friend class Monom2;
	friend class Polynom;

	private: 
	
		// Class used for ListElements.
		class ListElement {
    		public:
        	// Constructor.
        	ListElement(Monom2* data)
        	{
        	    this->data = data;
        	    next = NULL;
        	    prev = NULL;
        	}
 
        	// Pointers to next and previous element in the list.
        	ListElement* next;
        	ListElement* prev; 
 
        	// The actual data: Pointers to monomials.
        	Monom2* data;
    	};
    	
    // Members of List class.
	ListElement* head;
	ListElement* tail; 
	int size;
	
	public:	
		// Iterator class for the list.
		class Iterator
		{
			protected:
			    // Pointer to current list element.
			    ListElement* iter;
 	
			public:
			    friend MyList;
 		
			    // Constructor.
			    Iterator(void) : iter(NULL) {}
			    Iterator(ListElement* le) : iter(le) {}
			     
			    // Assignment operator.
			    void operator=(ListElement* le) { 
			    	iter = le; 
			    }
 		
			    // Comparison (not equal) operator.
			    bool operator!=(ListElement* le) {
			        if(NULL != iter && iter!=le) return true;
			        else return false;
			    }
		    
			    // Comparison (equal) operator.
			    bool operator==(ListElement* le) {
					if(NULL != iter && iter==le) return true;
					else return false;
    		    }
 	
			    // Incremental operator.
			    void operator++ (int) {
			        if (iter != NULL) iter = iter->next;
			    }
			    
			    void operator-- (int) {
					if(iter != NULL)
					iter = iter->prev;
    		    }
 		
			    // Access to data.
			    Monom2* returnData() {   
			        return iter->data;  
			    }
			    
			    ListElement* returnElement() {
			    	return iter;
			    }
		};
	
	public:	
		MyList();  // Constructor.
		virtual ~MyList();  // Destructor.
    	ListElement* add(Monom2* data);  // Add element to the list. Return pointer of this new element.
    	bool isEmpty() { return (head == NULL) ? true : false; }  
		void deleteElement(ListElement* element);  // Delete one element.
		void deleteList();  // Delete complete list (used in destructor).
		void show();  // Show list.
		ListElement* begin();  // Return head element.
		ListElement* end();  // Return end.
		ListElement* returnTail();	// Return tail element.
		int getSize();  // Return size.

};

#endif /* MYLIST_H_ */
