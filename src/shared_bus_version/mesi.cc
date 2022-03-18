/************************************************************
                        Course          :       CSC456
                        Source          :       msi.cc
                        Instructor      :       Ed Gehringer
                        Email Id        :       efg@ncsu.edu
------------------------------------------------------------
        © Please do not replicate this code without consulting
                the owner & instructor! Thanks!
*************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
using namespace std;
#include "main.h"
#include "mesi.h"
#include "cache.h"

// You dont need to edit this file, as a part of Project 3. 
// However, you are welcome to edit this file, to enhance the understanding 
// Kindly avoid the Directory functions, as you will need to implement other files to test the same 

void MESI::PrRd(ulong addr, int processor_number) {
	cache_state state;
	current_cycle++;
	reads++;
	cache_line * line = find_line(addr);
	
	if(line == NULL) {
		read_misses++;
		cache_line *newline = allocate_line(addr);
		int count = sharers_exclude(addr, processor_number);
		if(count == 0) {
			newline->set_state(E);
			I2E++;
			memory_transactions++; 
		}
		else {
			I2S++;
			newline->set_state(S);
			cache2cache++;
		}
		bus_reads++;
		sendBusRd(addr, processor_number);
		
	}
	else if(line != NULL) {
		state = line->get_state();
		if(state == I) {
			read_misses++;
			cache_line * newline = allocate_line(addr);
			int count = sharers_exclude(addr, processor_number);
			if(count == 0) {
				newline->set_state(E);
				I2E++;
				memory_transactions++;
			}
			else {
				newline->set_state(S);
				I2S++;
				cache2cache++;
			}
			
			bus_reads++;
			sendBusRd(addr, processor_number);
		}
		if(state == M || state == S || state == E) {
			update_LRU(line);
		}
	}

}

void MESI::PrRdDir(ulong addr, int processor_number) {

}

void MESI::PrWrDir(ulong addr, int processor_number) {
}


void MESI::PrWr(ulong addr, int processor_number) {
	cache_state state;
	current_cycle++;
	writes++;
	cache_line * line = find_line(addr);
	if(line == NULL) {
		write_misses++;
		cache_line *newline = allocate_line(addr);
		int count = sharers_exclude(addr, processor_number);
		if(count == 0) {
			memory_transactions++;
		}
		else { 
			cache2cache++; 
		}
		bus_readxs++;
		sendBusRdX(addr, processor_number);
		newline->set_state(M);
		I2M++;
	}
	else {
		state = line->get_state();
		if(state == I) {
			write_misses++;
			I2M++;
			cache_line * newline = allocate_line(addr);
			int count = sharers_exclude(addr, processor_number);
			if(count == 0) {
				memory_transactions++;
			}
			else {
				cache2cache++;
			}
			bus_readxs++;
			sendBusRdX(addr, processor_number);
			newline->set_state(M);
		}
		if(state == E) {
			line->set_state(M);
			update_LRU(line);
			E2M++;
		}
		if(state == M) {
			update_LRU(line);
		}
		if(state == S) {
			line->set_state(M);
			S2M++;
			update_LRU(line);
			bus_upgrades++;
			sendBusUpgr(addr, processor_number);
		}
	}
}


void MESI::BusRd(ulong addr) {
	cache_state state;
	cache_line * line = find_line(addr);
	if (line!= NULL) {
		state = line->get_state();
		if(state == M) {
			flushes++;
			write_backs++;
			memory_transactions++;
			interventions++;
			M2S++;
			line->set_state(S);
		}
		
		if(state == E) {
			E2S++;
			interventions++;
			line->set_state(S);
		}
		
		if(state == S) { 
		}
	}
}

void MESI::BusRdX(ulong addr) {
	cache_state state;
	cache_line * line = find_line(addr);
	if(line!=NULL) {
		state = line->get_state();
		if(state == M) {
			flushes++;
			memory_transactions++;
			write_backs++;
			invalidations++;
			line->set_state(I);
			M2I++;
		}
		
		if(state == E) {
			invalidations++;
			line->set_state(I);
			E2I++;
		}
		
		if(state == S) {
			invalidations++;
			line->set_state(I);
			S2I++;
		}
	}
}


void MESI::BusUpgr(ulong addr) {
	cache_state state;
	cache_line * line = find_line(addr);	
	if(line!=NULL) {
		state = line->get_state();
		if(state == S) {
			invalidations++;
			line->set_state(I);
			S2I++;
		}
	}
}



bool MESI::writeback_needed(cache_state state) {
	if (state == M) return true;
	else return false;
}

void MESI::signalRd(ulong addr, int processor_number){

	
}


void MESI::signalRdX(ulong addr, int processor_number){
	
}

void MESI::signalUpgr(ulong addr, int processor_number){

}

void MESI::Int(ulong addr) {
	
}


void MESI::Inv(ulong addr) {
	
}


cache_line * MESI::allocate_line_dir(ulong addr) {
	return NULL;
}
