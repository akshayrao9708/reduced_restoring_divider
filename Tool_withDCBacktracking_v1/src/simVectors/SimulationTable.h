/**************************************************************
 *       
 *       AIGPP Package // SimulationTable.hh
 *
 *       Author:
 *         Florian Pigorsch
 *         University of Freiburg
 *         pigorsch@informatik.uni-freiburg.de
 *	
 *     	 Edited for use in PolyDiVe Package from: 
 *	   Alexander Konrad 
 * 	   University of Freiburg
 *
 ***************************************************************/

#ifndef SIMULATIONTABLE_HH
#define SIMULATIONTABLE_HH

#include "SimVector.h"
#include "../circuit/Edge.h"

	class Edge;
namespace aigpp
{
    /*!
     *  \brief The SimulationTable class is a hash table that maps SimVectors
     *         to lists containing pointers to all circuit edges with the 
     *         corresponding SimVector, e.g. the equivalence class wrt. to the
     *         SimVector.
     *
     *
     *   Collisions are resolved by chaining.
     *   The hash table grows each time it is filled at least 75% but it
     *   will never shrink.
     */
    class SimulationTable
    {
    public:
        SimulationTable();
        ~SimulationTable();

        vp::Edge* getClass( vp::Edge* n ) const;
        vp::Edge* getInvertedClass( vp::Edge* nonInvertedClass ) const;

        /*! 
         *   Returns the linked list containing all Edges whose SimVector is equal
         *   to \e sim, or 0 if no such Edges exist.
         *
         *   \param sim SimVector to find
         *
         *   \return list of Edges
         */
        vp::Edge* lookup( const SimVector& sim ) const;

        /*! 
         *   Returns the linked list containing all Edges whose SimVector is equal
         *   to the negation of \e sim, or 0 if no such Edges exist.
         *
         *   \param sim SimVector to find
         *
         *   \return list of Edges
         */
        vp::Edge* lookupInverted( const SimVector& sim, vp::Edge* nonInvertedClass = 0 ) const;

        /*! 
         *  Insert Edge \n into the appropriate list. 
         *
         *   \param n Edge to insert
         */
        void insert( vp::Edge* n );
     
        /*!
         *  Clears the SimulationTable.
         */
        void clear();
        void clearRememberClasses();
        void rebuild( vp::Edge* edges, bool useRememberedClasses = false );

        /*! 
         *  Grows the hash table to the size given by \e Primes[newPrimesIndex].
         *  If the new size is less or equal to the current size, nothing is done.
         *
         *   \param newPrimesIndex index for the desired size
         */
        void resize( int newPrimesIndex );


        /*!
         * Remove dead edges from the table.
         */
        void garbageCollect();
    
        /*! 
         *   Returns the size of the linked list of simulation-equivalent edges
         *   starting in \e simclass
         *
         *   \param simclass head of the list
         *
         *   \return size of the list
         */
        static std::size_t simClassSize( vp::Edge* simclass );  
    
        /*!
         * returns the maximum number of edges in a simclass of the current simulation table.
         */
        std::size_t maxSimClassSize() const;
    

        /*! 
         *   Checks the integrity of the SimulationTable and aborts if integrity
         *   is violated.
         */
        void checkIntegrity( vp::Edge* edgeslist ) const;
    

        std::size_t entries() const
        {
            return _entries;
        };
        
        std::vector<std::size_t>* validEntries() {
        	return &_validEntries;
        };

        
    
    private:
        /* copying is not allowed */
        SimulationTable( const SimulationTable& );
        SimulationTable& operator=( const SimulationTable& );
        
    public: 
        vp::Edge** _table;
        std::size_t _capacity;
        std::size_t _entries;
        unsigned int _primesIndex;
        std::vector<std::size_t> _validEntries;
    };
  
}

#endif /* SIMULATIONTABLE_HH */
