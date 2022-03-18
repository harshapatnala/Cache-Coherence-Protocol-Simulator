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
#include "moesi.h"


/*
------------------------------------------------------
Don't modify the fucntions included in this section
------------------------------------------------------
*/
cache_line * MOESI::allocate_line_dir(ulong addr) {
    return NULL;
}
void MOESI::signalRd(ulong addr, int processor_number){
}
void MOESI::signalRdX(ulong addr, int processor_number){
}
void MOESI::signalUpgr(ulong addr, int processor_number){
}
void MOESI::Inv(ulong addr) {
}
void MOESI::Int(ulong addr) {
}
void MOESI::PrRdDir(ulong addr, int processor_number) {
}
void MOESI::PrWrDir(ulong addr, int processor_number) {
}
//-------------------------------------------------------------
//Section ends here. edit the functions in the section below
//-------------------------------------------------------------

void MOESI::PrRd(ulong addr, int processor_number)
{
    cache_state state;
    current_cycle++;
    reads++;
    cache_line * line = find_line(addr);

    if(line == NULL)
    {
        read_misses++;
        cache_line * newline = allocate_line(addr);
        int count = sharers_exclude(addr, processor_number);
        if(count == 0)
        {   /* no copies - E */
            newline->set_state(E);
            I2E++;
            memory_transactions++;
        }
        else
        {
            if(c2c_supplier(addr, processor_number)) {
/* has copies */
                I2S++;
                newline->set_state(S);
                cache2cache++;
            }
            else {
                I2S++;
                newline->set_state(S);
                memory_transactions++;
            }
        }
        bus_reads++;
        sendBusRd(addr, processor_number);
    }
    else
    {
        state = line->get_state();
        if(state == I)
        {
            read_misses++;
            cache_line * newline = allocate_line(addr);
            int count = sharers_exclude(addr, processor_number);
            if(count == 0)
            {
                newline->set_state(E);
                I2E++;
                memory_transactions++;
            }
            else {
                if(c2c_supplier(addr, processor_number)) {
                    newline->set_state(S);
                    I2S++;
                    cache2cache++;
                }
                else {
                    newline->set_state(S);
                    I2S++;
                    memory_transactions++;
                }
            }
        }
        else
        {
            update_LRU(line);
        }
    }
}


void MOESI::PrWr(ulong addr, int processor_number)
{
    cache_state state;
    current_cycle++;
    writes++;
    cache_line * line = find_line(addr);
    if(line == NULL)
    {
        write_misses++;
        cache_line *newline = allocate_line(addr);
        int count = sharers_exclude(addr, processor_number);
        if(count == 0)
        {
            memory_transactions++;
        }
        else {
            if(c2c_supplier(addr, processor_number)) {
                cache2cache++;
            }
            else {memory_transactions++;}
        }
        bus_readxs++;
        sendBusRdX(addr, processor_number);
        newline->set_state(M);
        I2M++;
    }
    else
    {
        state = line->get_state();
        if(state == M)
        {
            update_LRU(line);
        }
        else if(state == O)
        {
            line->set_state(M);
            O2M++;
            update_LRU(line);
            bus_upgrades++;
            sendBusUpgr(addr, processor_number);
        }
        else if(state == E)
        {
            line->set_state(M);
            update_LRU(line);
            E2M++;
        }
        else if(state == S)
        {
            line->set_state(M);
            S2M++;
            update_LRU(line);
            bus_upgrades++;
            sendBusUpgr(addr, processor_number);
        }
        else if(state == I)
        {
            write_misses++;
            I2M++;
            cache_line * newline = allocate_line(addr);
            int count = sharers_exclude(addr, processor_number);
            if(count == 0)
            {
                memory_transactions++;
            }
            else {
                if(c2c_supplier(addr, processor_number)) {
                    cache2cache++;
                }
                else { memory_transactions++; }
            }
            newline->set_state(M);
            bus_readxs++;
            sendBusRdX(addr, processor_number);
        }
    }
}

void MOESI::BusRd(ulong addr)
{
    cache_state state;
    cache_line * line = find_line(addr);
    if(line != NULL)
    {
        state = line->get_state();
        if(state == M)
        {
            flushes++;
            //write_backs++;
            //memory_transactions++;
            interventions++;
            M2O++;
            line->set_state(O);
        }
        else if(state == O)
        {
            flushes++;
        }
        else if(state == E)
        {

            E2S++;
            //flushes++;
            line->set_state(S);
        }
    }
}

void MOESI::BusRdX(ulong addr)
{
    cache_state state;
    cache_line * line = find_line(addr);
    if(line != NULL)
    {
        state = line->get_state();
        if(state == M)
        {
            flushes++;
            invalidations++;
            memory_transactions++;
            write_backs++;
            line->set_state(I);
            M2I++;
        }
        else if(state == O)
        {
            flushes++;
            invalidations++;
            memory_transactions++;
            write_backs++;
            line->set_state(I);
            O2I++;
        }
        else if(state == E)
        {
//flushes++;
            invalidations++;
            line->set_state(I);
            E2I++;
        }
        else if(state == S)
        {
            invalidations++;
            line->set_state(I);
            S2I++;
        }
    }
}

void MOESI::BusUpgr(ulong addr)
{
    cache_state state;
    cache_line * line = find_line(addr);
    if(line != NULL)
    {
        state = line->get_state();
        if(state == O)
        {
            invalidations++;
            line->set_state(I);
            O2I++;
        }
        else if(state == S)
        {
            invalidations++;
            line->set_state(I);
            S2I++;
        }
    }
}

bool MOESI::writeback_needed(cache_state state)
{
    if(state == M || state == O)
    {
        return true;
    }
    else
        return false;
}