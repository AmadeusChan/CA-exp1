#include "replacement_state.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state
    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    LIRS_stack = new deque<LIRS_STACK_ENTRY> [numsets];
    LIRS_Q = new deque<LIRS_STACK_ENTRY> [numsets];
    Lhirs = assoc / 3;
    wayCnt = new INT32[numsets];
    for (UINT32 i = 0; i< numsets; ++ i) {
	    wayCnt[i] = 0;
    }

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the thread id, set index, pointers to ways in current set    //
// and the associativity.  We are also providing the PC, physical address,    //
// and accesstype should you wish to use them at victim selection time.       //
// The return value is the physical way index for the line being replaced.    //
// Return -1 if you wish to bypass LLC.                                       //
//                                                                            //
// vicSet is the current set. You can access the contents of the set by       //
// indexing using the wayID which ranges from 0 to assoc-1 e.g. vicSet[0]     //
// is the first way and vicSet[4] is the 4th physical way of the cache.       //
// Elements of LINE_STATE are defined in crc_cache_defs.h                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
	return Get_LIRS_Victim(setIndex, paddr);
    }

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
	UpdateLIRS(setIndex, updateWayID, currLine, cacheHit);
    }
    
    
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here
    
    /*
    for (int i = 0; i < 1; ++ i) {
	    out << "setIndex = " << i << endl;
	    out << "stack: "<<endl;
	    deque<LIRS_STACK_ENTRY> *stack = &LIRS_stack[i];
	    deque<LIRS_STACK_ENTRY> *Q = &LIRS_Q[i];
	    for (int j = 0 ; j < int(stack->size()); ++ j) {
		    cout << j << ": resident=" << stack->at(j).resident << " status=" << stack->at(j).status << " tag=" << stack->at(j).tag << " wayID=" << stack->at(j).wayID << endl;
	    }
	    cout << endl;
	    for (int j = 0 ; j < int(Q->size()); ++ j) {
		    cout << j << ": resident=" << Q->at(j).resident << " status=" << Q->at(j).status << " tag=" << Q->at(j).tag << " wayID=" << Q->at(j).wayID << endl;
	    }
	    cout << endl;
    }
    */

    return out;
    
}

INT32 CACHE_REPLACEMENT_STATE::Get_LIRS_Victim(UINT32 setIndex, Addr_t paddr) {
	INT32 way;
	deque<LIRS_STACK_ENTRY> *stack = &LIRS_stack[setIndex];
	deque<LIRS_STACK_ENTRY> *Q = &LIRS_Q[setIndex];     
	way=Q->at(Q->size() - 1).wayID;
	for (deque<LIRS_STACK_ENTRY>::iterator i = stack->begin(); i != stack->end(); ++ i) {
		if (i->tag == Q->at(Q->size() - 1).tag) {
			i->resident = false;
			break;
		}
	}
	Q->pop_back();
	return way;
}

void CACHE_REPLACEMENT_STATE::UpdateLIRS(UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, bool cahceHit) {
	deque<LIRS_STACK_ENTRY> *stack = &LIRS_stack[setIndex];
	deque<LIRS_STACK_ENTRY> *Q = &LIRS_Q[setIndex];
	if (cahceHit) {
		INT32 pos = -1;
		for (INT32 i = 0, j = stack->size(); i < j; ++ i) {
			if (stack->at(i).tag == currLine->tag) {
				pos = i;
				break;
			}
		}
		if (pos != -1) {
			LIRS_STACK_ENTRY entry = stack->at(pos);
			if (entry.status == LIR_STATUS) {
				stack->erase(stack->begin() + pos);
				stack->push_front(entry);
				LIRS_Stack_Pruning(setIndex);
			} else {
				entry.status = LIR_STATUS;
				for (deque<LIRS_STACK_ENTRY>::iterator i = Q->begin(); i != Q->end(); ++ i) {
					if (i->tag == currLine->tag) {
						Q->erase(i);
						break;
					}
				}
				stack->erase(stack->begin() + pos);
				stack->push_front(entry);
				stack->at(stack->size() - 1).status = HIR_STATUS;
				Q->push_front(stack->at(stack->size() - 1));
				LIRS_Stack_Pruning(setIndex);
			}
		} else {
			for (deque<LIRS_STACK_ENTRY>::iterator i = Q->begin(); i != Q->end(); ++ i) {
				if (i->tag == currLine->tag) {
					Q->erase(i);
					break;
				}
			}

			LIRS_STACK_ENTRY entry;
			entry.resident = true;
			entry.status = HIR_STATUS;
			entry.wayID = updateWayID;
			entry.tag = currLine->tag;
			stack->push_front(entry);
			Q->push_front(entry);
		}
	} else {
		if (wayCnt[setIndex] < INT32(assoc)) {
			assert(updateWayID == wayCnt[setIndex]);
			wayCnt[setIndex] += 1;
			LIRS_STACK_ENTRY *entry = new LIRS_STACK_ENTRY();
			if (updateWayID < (INT32)(assoc) - INT32(Lhirs)) {
				entry->status = LIR_STATUS;
			} else entry->status = HIR_STATUS;
			entry->resident = true;
			entry->tag = currLine->tag;
			entry->wayID = updateWayID;
			LIRS_stack[setIndex].push_front(*entry);
			if (entry->status == HIR_STATUS) {
				LIRS_Q[setIndex].push_front(*entry);
			}
			delete entry;
		} else {
			INT32 pos = -1;
			for (INT32 i = 0, j = stack->size(); i < j; ++ i) {
				if (stack->at(i).tag == currLine->tag) {
					pos = i;
					break;
				}
			}
			//printf("tag: %llu\n", currLine->tag);
			//assert(pos == -1);
			if (pos != -1) {
				LIRS_STACK_ENTRY entry = stack->at(pos);
				stack->erase(stack->begin() + pos);
				entry.resident = true;
				entry.status = LIR_STATUS;
				entry.wayID = updateWayID;
				stack->push_front(entry);

				stack->at(stack->size() - 1).status = HIR_STATUS;
				Q->push_front(stack->at(stack->size() - 1));
				LIRS_Stack_Pruning(setIndex);
			} else {
				LIRS_STACK_ENTRY entry;
				entry.status = HIR_STATUS;
				entry.resident = true;
				entry.tag = currLine->tag;
				entry.wayID = updateWayID;
				stack->push_front(entry);
				Q->push_front(entry);
			}
		}
	}
}

void CACHE_REPLACEMENT_STATE::LIRS_Stack_Pruning(UINT32 setIndex) {
	deque<LIRS_STACK_ENTRY> *stack = &LIRS_stack[setIndex];
	//deque<LIRS_STACK_ENTRY> *Q = &LIRS_Q[setIndex];
	while (1) {
		UINT32 size = stack->size();
		if (size == 0) break;
		if (stack->at(size - 1).status == LIR_STATUS) {
			break;
		}
		stack->pop_back();
	}
}
