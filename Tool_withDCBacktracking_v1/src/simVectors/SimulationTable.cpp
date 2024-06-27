/**************************************************************
 *       
 *       AIGPP Package // SimulationTable.cc
 *
 *       Author:
 *         Florian Pigorsch
 *         University of Freiburg
 *         pigorsch@informatik.uni-freiburg.de
 *
 *       Last revision:
 *         $Revision: 717 $
 *         $Author: pigorsch $
 *
 ***************************************************************/

#include "SimulationTable.h"

#include "Primes.h"
#include "SimVector.h"

aigpp::SimulationTable::
SimulationTable()
    :_capacity( PowerPrimes[ 0 ] ),
     _entries( 0 ),
     _primesIndex( 0 )
{
    _table = new vp::Edge*[ _capacity ];
    std::fill( _table, _table + _capacity, (vp::Edge*)0 );
}

aigpp::SimulationTable::
~SimulationTable()
{
    assert( _entries == 0 );
    delete[] _table;
}

vp::Edge*
aigpp::SimulationTable::
getClass( vp::Edge* n ) const
{
    assert( n->_nEqualSim != 0 && n->_pEqualSim != 0 );

    vp::Edge* simclass = n;

    while( simclass->_nEqualSimHash == 0 )
    {
        simclass = simclass->_nEqualSim;
        if( simclass == n ) break;
    }

    assert( simclass->_pEqualSimHash != 0 );

    return simclass;
}

vp::Edge*
aigpp::SimulationTable::
getInvertedClass( vp::Edge* nonInvertedClass ) const
{
    return getClass( nonInvertedClass )->_invertedSimClass;
}

vp::Edge* 
aigpp::SimulationTable::
lookup( const aigpp::SimVector& sim ) const
{
    vp::Edge* simtableentry = _table[ sim.hash() % _capacity ];
    if( simtableentry == 0 )
    {
    	return 0;
    }

    vp::Edge* simclass = simtableentry;
    do {
        if( simclass->sim() == sim )
        {
            return simclass;
        }

        simclass = simclass->_nEqualSimHash;
    } while( simclass != simtableentry );

    return 0;
}

vp::Edge* 
aigpp::SimulationTable::
lookupInverted( const aigpp::SimVector& sim, vp::Edge* nonInvertedClass ) const
{
    if( nonInvertedClass != 0 )
    {
        /* assure "nonInvertedClass" is class head */
        assert( nonInvertedClass->_nEqualSimHash != 0 );
        return nonInvertedClass->_invertedSimClass;
    }

    vp::Edge* simtableentry = _table[ sim.hashInverted() % _capacity ];
    if( simtableentry == nullptr )
    {
        return nullptr;
    }

    vp::Edge* simclass = simtableentry;
    do {
        if( simclass->sim().equalInverted( sim ) )
        {
            return simclass;
        }

        simclass = simclass->_nEqualSimHash;
    } while( simclass != simtableentry );

    return nullptr;
}

void 
aigpp::SimulationTable::
insert( vp::Edge* n )
{
    n->_pEqualSimHash = 0;
    n->_nEqualSimHash = 0;
    n->_pEqualSim = 0;
    n->_nEqualSim = 0;
    n->_invertedSimClass = 0;

    // cast ok
    if( static_cast<double>( _entries ) > 0.75 * static_cast<double>( _capacity ) )
    {
        resize( _primesIndex + 1 );
    }

    vp::Edge** simtableentry = _table + n->sim().hash() % _capacity;

    /*
     * no entry with an appropriate hash value
     */
    if( *simtableentry == 0 )
    {
        ++_entries;
        _validEntries.push_back(n->sim().hash() % _capacity);

        *simtableentry = n;
        n->_pEqualSimHash = n;
        n->_nEqualSimHash = n;
        n->_pEqualSim = n;
        n->_nEqualSim = n;

        vp::Edge* inverted = lookupInverted( n->sim() );
        n->_invertedSimClass = inverted;
        if( inverted != 0 )
        {
            assert( inverted->_invertedSimClass == 0 );
            inverted->_invertedSimClass = n;
        }

        return;
    }

    /*
     * entry with matching hash value found
     */

    /*
     * search for simulation class
     */
    vp::Edge* simclass = *simtableentry;
    do {
        if( simclass->sim() == n->sim() )
        {
            /*
             * simulation class found. insert as second element of simclass.
             */
            simclass->_nEqualSim->_pEqualSim = n;
            n->_pEqualSim = simclass;
            n->_nEqualSim = simclass->_nEqualSim;
            simclass->_nEqualSim = n;

            return;
        }

        simclass = simclass->_nEqualSimHash;
    } while( simclass != *simtableentry );

    /*
     * no matching simulation class found.
     * create a new one and insert as second element of simtableentry.
     */
    ++_entries;

    n->_pEqualSim = n;
    n->_nEqualSim = n;

    ( *simtableentry )->_nEqualSimHash->_pEqualSimHash = n;
    n->_pEqualSimHash = ( *simtableentry );
    n->_nEqualSimHash = ( *simtableentry )->_nEqualSimHash;
    ( *simtableentry )->_nEqualSimHash = n;

    vp::Edge* inverted = lookupInverted( n->sim() );
    n->_invertedSimClass = inverted;
    if( inverted != nullptr )
    {
        assert( inverted->_invertedSimClass == 0 );
        inverted->_invertedSimClass = n;
    }
}

void
aigpp::SimulationTable::
clear()
{
    std::fill( _table, _table + _capacity, (vp::Edge*)0 );
    _entries = 0;
}

void
aigpp::SimulationTable::
clearRememberClasses()
{
    vp::Edge** simtableentry = _table;

    for( std::size_t i = 0; i < _capacity; ++i, ++simtableentry )
    {
        if( *simtableentry == nullptr )
        {
            continue;
        }

        vp::Edge* simclass = *simtableentry;

        do {
            vp::Edge* nextClass = simclass->_nEqualSimHash;

            vp::Edge* n = simclass;
            do {
                vp::Edge* next = n->_nEqualSim;

                n->_pEqualSim = 0;
                n->_nEqualSim = 0;
                n->_pEqualSimHash = 0;
                n->_nEqualSimHash = 0;
                n->_invertedSimClass = 0;

                n->_nEqualSim = simclass;

                n = next;
            } while( n != simclass );

            simclass = nextClass;
        } while( simclass != *simtableentry );

        *simtableentry = nullptr;
    }

    clear();
}

void
aigpp::SimulationTable::
rebuild( vp::Edge* edges, bool useRememberedClasses )
{
    if( useRememberedClasses )
    {
        /* insert remembered class heads first */
        for( vp::Edge* n = edges; n != nullptr; n++ )
        {
            assert( n->_nEqualSim != 0 );
            assert( n->_pEqualSim == 0 );

            if( n->_nEqualSim == n )
            {
                n->_nEqualSim = 0;
                insert( n );

                assert( n->_pEqualSim != 0 );
            }
        }

        /* insert remaining edges */
        for( vp::Edge* n = edges; n != nullptr; n++ )
        {
            /* Edge already inserted by previous loop */
            if( n->_pEqualSim != 0 )
            {
                continue;
            }

            /* remembered class still correct */
            vp::Edge* rememberedClass = n->_nEqualSim;
            assert( rememberedClass->_nEqualSim != 0 );
            assert( rememberedClass->_pEqualSim != 0 );

            n->_nEqualSim = 0;
            if( rememberedClass->_nEqualSim->sim() == n->sim() )
            {
                n->_pEqualSim = rememberedClass;
                n->_nEqualSim = rememberedClass->_nEqualSim;
                rememberedClass->_nEqualSim->_pEqualSim = n;
                rememberedClass->_nEqualSim = n;

                n->_invertedSimClass = 0;
            }
            else
            {
                insert( n );
            }
        }
    }
    else
    {
        for( vp::Edge* n = edges; n != nullptr; n++ )
        {
            n->_nEqualSim = 0;
            n->_pEqualSim = 0;
            n->_nEqualSimHash = 0;
            n->_pEqualSimHash = 0;
            n->_invertedSimClass = 0;

            insert( n );
        }
    }
}

void
aigpp::SimulationTable::
resize( int newPrimesIndex )
{
    if( newPrimesIndex < 0 || newPrimesIndex >= NumPowerPrimes )
    {
        return;
    }

    std::size_t oldcapacity = _capacity;
    
    _validEntries.clear();

    _primesIndex = newPrimesIndex;
    _capacity = PowerPrimes[ _primesIndex ];

    vp::Edge** newtable = new vp::Edge*[ _capacity ];
    std::fill( newtable, newtable + _capacity, (vp::Edge*)0 );

    vp::Edge** simtableentry = _table;

    for( std::size_t i = 0; i < oldcapacity; ++i, ++simtableentry )
    {
        if( *simtableentry == 0 )
        {
            continue;
        }

        std::vector<vp::Edge*> classes;

        vp::Edge* simclass = *simtableentry;
        do
        {
            classes.push_back( simclass );
            simclass = simclass->_nEqualSimHash;
        } while( simclass != *simtableentry );

        for( vp::Edge* n: classes )
        {
            vp::Edge* nn = n;

            vp::Edge** newsimtableentry = newtable + ( nn->sim().hash() % _capacity );
            if( *newsimtableentry == 0 )
            {
            	_validEntries.push_back(nn->sim().hash() % _capacity);
                nn->_nEqualSimHash = nn;
                nn->_pEqualSimHash = nn;
                *newsimtableentry = nn;
            }
            else
            {
                nn->_nEqualSimHash = ( *newsimtableentry )->_nEqualSimHash;
                ( *newsimtableentry )->_nEqualSimHash->_pEqualSimHash = nn;
                nn->_pEqualSimHash = ( *newsimtableentry );
                ( *newsimtableentry )->_nEqualSimHash = nn;
            }
        }
    }

    delete[] _table;
    _table = newtable;
}

void
aigpp::SimulationTable::
garbageCollect()
{
    /*
     * go through the whole table
     */
    std::vector<vp::Edge*> classes;
    std::vector<vp::Edge*> classedges;

    std::size_t newEntries = 0;
    std::size_t oldEdges = 0, newEdges = 0;

    for( std::size_t i = 0; i != _capacity; ++i )
    {
        vp::Edge* simtableentry = _table[i];
        if( simtableentry == 0 ) continue;

        /*
         * go through all simclasses of this table entry
         */


        /* collect simclasses */
        classes.clear();
        {
            vp::Edge* n = simtableentry;
            do {
                classes.push_back( n );
                n = n->_nEqualSimHash;
            } while( n != simtableentry );
        }

        _table[i] = 0;

        for( vp::Edge* simclass: classes )
        {
            /* collect classedges */
            classedges.clear();
            {
                vp::Edge* n = simclass;
                do {
                    classedges.push_back( n );
                    n = n->_nEqualSim;
                } while( n != simclass );
            }

            /* filter out dead edges and fill "newclass" */
            vp::Edge* inverted = 0;
            vp::Edge* newclass = 0;
            for( vp::Edge* nn: classedges )
            {
                ++oldEdges;

                vp::Edge* n = nn;

                if( n->_invertedSimClass != 0 )
                {
                    assert( n == simclass );
                    inverted = n->_invertedSimClass;
                    n->_invertedSimClass = 0;
                }

                /* dead edge */
                if( 0 )
                {
                    n->_nEqualSim = 0;
                    n->_pEqualSim = 0;
                    n->_nEqualSimHash = 0;
                    n->_pEqualSimHash = 0;
                }
                /* alive edge */
                else
                {
                    ++newEdges;

                    /* first edge of class -> create newclass */
                    if( newclass == 0 )
                    {
                        newclass = n;

                        n->_nEqualSimHash = 0;
                        n->_pEqualSimHash = 0;

                        n->_nEqualSim = n;
                        n->_pEqualSim = n;
                    }
                    /* other edges already in class -> insert after head */
                    else
                    {
                        n->_nEqualSimHash = 0;
                        n->_pEqualSimHash = 0;

                        n->_pEqualSim = newclass;
                        n->_nEqualSim = newclass->_nEqualSim;
                        newclass->_nEqualSim->_pEqualSim = n;
                        newclass->_nEqualSim = n;
                    }
                }
            }

            if( newclass != 0 )
            {
                ++newEntries;

                /* first class in hash table entry -> set as head */
                if( _table[i] == 0 )
                {
                    newclass->_nEqualSimHash = newclass;
                    newclass->_pEqualSimHash = newclass;

                    _table[i] = newclass;
                }
                /* other classes already in hash table entry -> insert after head */
                else
                {
                    newclass->_pEqualSimHash = _table[i];
                    newclass->_nEqualSimHash = _table[i]->_nEqualSimHash;
                    _table[i]->_nEqualSimHash->_pEqualSimHash = newclass;
                    _table[i]->_nEqualSimHash = newclass;
                }

                if( inverted != 0 )
                {
                    inverted->_invertedSimClass = newclass;
                    newclass->_invertedSimClass = inverted;
                }
                else
                {
                    newclass->_invertedSimClass = 0;
                }
            }
            else
            {
                if( inverted != 0 )
                {
                    inverted->_invertedSimClass = 0;
                }
            }
        }
    }

    _entries = newEntries;
}

std::size_t
aigpp::SimulationTable::
simClassSize( vp::Edge* simclass )
{
    std::size_t size = 0;

    vp::Edge* n = simclass;
    do
    {
        ++size;
        n = n->_nEqualSim;
    }
    while( n != simclass );

    return size;
}

std::size_t
aigpp::SimulationTable::
maxSimClassSize() const
{
    std::size_t maxsize = 0;

    for( std::size_t i = 0; i != _capacity; ++i )
    {
        vp::Edge* simtableentry = _table[ i ];

        if( simtableentry == 0 )
        {
            continue;
        }

        vp::Edge* simclass = simtableentry;
        do
        {
            std::size_t size = simClassSize( simclass );
            if( size > maxsize ) maxsize = size;

            simclass = simclass->_nEqualSimHash;
        }
        while( simclass != simtableentry );
    }

    return maxsize;
}

void
aigpp::SimulationTable::
checkIntegrity( vp::Edge* edgeslist ) const
{
    std::size_t simClassesFound = 0;
    std::size_t edgesFound = 0;

    vp::Edge** simtableentry = _table;

    for( std::size_t i = 0; i < _capacity; ++i, ++simtableentry )
    {
        if( *simtableentry == 0 )
        {
            continue;
        }

        vp::Edge* simclass = *simtableentry;
        assert( ( simclass->sim().hash() % _capacity ) == i );

        do {
            ++simClassesFound;

            if( simclass->_invertedSimClass != 0 )
            {
                assert( simclass->_invertedSimClass->_invertedSimClass == simclass );
            }

            //std::cout << "class # " << simClassesFound << std::endl;
            vp::Edge* n = simclass;
            do {
                //std::cout << n << std::endl;
                ++edgesFound;

                if( n != simclass )
                {
                    assert( n->_nEqualSimHash == 0 );
                    assert( n->_pEqualSimHash == 0 );
                    assert( n->_invertedSimClass == 0 );
                }

                assert( n->sim() == simclass->sim() );

                n = n->_nEqualSim;
            } while( n != simclass );

            simclass = simclass->_nEqualSimHash;
        } while( simclass != *simtableentry );
    }

    assert( _entries == simClassesFound );

    for( vp::Edge* n = edgeslist; n != 0; n++ )
    {
        vp::Edge* lookupResult = lookup( n->sim() );
        vp::Edge* getclassResult = getClass( n );

        assert( lookupResult != 0 );
        assert( lookupResult == getclassResult );
    }
}
