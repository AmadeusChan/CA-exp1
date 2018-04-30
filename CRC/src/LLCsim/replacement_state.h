#ifndef REPL_STATE_H
#define REPL_STATE_H

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

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include "utils.h"
#include "crc_cache_defs.h"

#include <vector>
#include <deque>

#define LIR_STATUS 0
#define HIR_STATUS 1

using namespace std;

// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;

    // CONTESTANTS: Add extra state per cache line here

} LINE_REPLACEMENT_STATE;

// LIRS stack entry
typedef struct {
	UINT32 status;
	bool resident;
	Addr_t tag;
	UINT32 wayID;
} LIRS_STACK_ENTRY;

// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{

  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    
    LINE_REPLACEMENT_STATE   **repl;

    deque<LIRS_STACK_ENTRY> *LIRS_stack;
    deque<LIRS_STACK_ENTRY> *LIRS_Q;
    UINT32 Lhirs;
    INT32 *wayCnt;

    deque<int> *FIFO_stack;


    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here

  public:

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32  GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ostream&   PrintStats( ostream &out);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );

    // LIRS
    INT32 Get_LIRS_Victim(UINT32 setIndex, Addr_t paddr);
    void UpdateLIRS(UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, bool cahceHit);
    void LIRS_Stack_Pruning(UINT32 setIndex);

    // FIFO 
    INT32 Get_FIFO_Victim(UINT32 setIndex);
    void   UpdateFIFO( UINT32 setIndex, INT32 updateWayID , bool cacheHit);
};


#endif
