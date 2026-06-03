/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <vector>

using std::vector;
//tahel is bad
class BTBEntry{
	public:
	bool valid = false;
    uint32_t tag = 0;
    uint32_t target = 0;
    uint8_t local_history = 0;
    std::vector<int> local_fsm_table;
	};
class BranchPredictor{
	public:
		uint8_t global_history;
		unsigned br_num =0;
		unsigned flush_num =0;
		unsigned btbSize;
		unsigned historySize;
		unsigned tagSize;
		unsigned fsmState;
		bool isGlobalHist;
		bool isGlobalTable;
		int Shared;
		vector<int> global_fsm_table;
		vector<BTBEntry> btb_table;
		BranchPredictor(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared): btbSize(btbSize), historySize(historySize), tagSize(tagSize), fsmState(fsmState),
	isGlobalHist(isGlobalHist), isGlobalTable(isGlobalTable), Shared(Shared){
			global_history = 0;
			global_fsm_table.resize(1 << historySize, fsmState);
			btb_table.resize(btbSize);
		}
		uint32_t getFSMIndex(uint32_t pc, uint8_t history) {
   			uint32_t mask = (1 << historySize) - 1;
    		uint32_t clean_history = history & mask; 
    	if (Shared == 0) {
        	return clean_history;
   		 }else if (Shared == 1) {
        	uint32_t pc_lsb = (pc >> 2) & mask;
        	return clean_history ^ pc_lsb;
    	}  else if (Shared == 2) {
        uint32_t pc_mid = (pc >> 16) & mask;
        return clean_history ^ pc_mid;
   		}
    
   		 return clean_history; 
	}
	uint32_t getIndex(uint32_t pc) {
    	return (pc / 4) % btbSize;
	};

	uint32_t getTag(uint32_t pc) {
		uint32_t shifted_pc = pc / (4 * btbSize);
		uint32_t mask = (1 << tagSize) - 1;
		return shifted_pc & mask;
	};
};

BranchPredictor* myPredictor = nullptr;

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared){
		try {
		if (btbSize == 0 || historySize == 0 || tagSize == 0 || fsmState > 3) {
			throw std::invalid_argument("Invalid input parameters");
		}
	} catch (const std::invalid_argument& e) {
		fprintf(stderr, "Error: %s\n", e.what());
		return -1;
	}
	try{
	myPredictor = new BranchPredictor(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
	return 0;
	} catch (const std::exception& e) {
		fprintf(stderr, "Error: Failed to initialize BranchPredictor - %s\n", e.what());
		return -1;
	}
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	uint32_t index = myPredictor->getIndex(pc);
	uint32_t tag = myPredictor->getTag(pc);
	BTBEntry& entry = myPredictor->btb_table[index];
	if (entry.valid && entry.tag == tag) {
		uint8_t history = myPredictor->isGlobalHist ? myPredictor->global_history : entry.local_history;
		uint32_t fsm_index = myPredictor->getFSMIndex(pc, history);
		int fsm_state = myPredictor->isGlobalTable ? myPredictor->global_fsm_table[fsm_index] : entry.local_fsm_table[fsm_index];
		
		if (fsm_state >= 2) { 
			*dst = entry.target;
			return true; 
		}
	}
	
	*dst = pc + 4;
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	myPredictor->br_num++;
	uint32_t actualdist = taken ? targetPc : pc + 4;
	if (pred_dst != actualdist) {
		myPredictor->flush_num++;
	}
	uint32_t index = myPredictor->getIndex(pc);
	uint32_t tag = myPredictor->getTag(pc);
	BTBEntry& entry = myPredictor->btb_table[index];
		if (!entry.valid) {
			entry.valid = true;
			entry.tag = tag;
			entry.target = targetPc;
			entry.local_history = 0;
			if(!myPredictor->isGlobalTable) {
			entry.local_fsm_table.resize(1 << myPredictor->historySize, myPredictor->fsmState);
			}
		} else if (entry.tag != tag) {
			entry.tag = tag;
			entry.target = targetPc;
			entry.local_history = 0;
			if(!myPredictor->isGlobalTable){
			std::fill(entry.local_fsm_table.begin(), entry.local_fsm_table.end(), myPredictor->fsmState);
		}
		} else {
			entry.target = targetPc;
	}
	uint8_t history = myPredictor->isGlobalHist ? myPredictor->global_history : entry.local_history;
	uint32_t fsm_index = myPredictor->getFSMIndex(pc, history);
	int& fsm_state = myPredictor->isGlobalTable ? myPredictor->global_fsm_table[fsm_index] : entry.local_fsm_table[fsm_index];
	if (taken) {
		if (fsm_state < 3) fsm_state++;
	} else {
		if (fsm_state > 0) fsm_state--;
	}
	if (myPredictor->isGlobalHist) {
		myPredictor->global_history = ((myPredictor->global_history << 1) | taken) & ((1 << myPredictor->historySize) - 1);
	} else {
		entry.local_history = ((entry.local_history << 1) | taken) & ((1 << myPredictor->historySize) - 1);
	}
	return;
}

void BP_GetStats(SIM_stats *curStats){
	curStats->br_num = myPredictor->br_num;
	curStats->flush_num = myPredictor->flush_num;

	unsigned btb_lines = myPredictor->btbSize;
	unsigned history_size = myPredictor->historySize;
	unsigned tag_size = myPredictor->tagSize;
	unsigned size = btb_lines * (1+ tag_size + 30);

	if (myPredictor->isGlobalHist) {
		size += history_size;
	} else {
		size += btb_lines * history_size;
	}
	unsigned fsm_table_size = (1 << history_size) * 2;
	if (myPredictor->isGlobalTable) {
		size += fsm_table_size;
	} else {
		size += btb_lines * fsm_table_size;
	}
	curStats->size = size;
	delete myPredictor;
	return;
}
