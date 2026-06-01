/* 046267 Computer Architecture - HW #1                                 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdio>
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
	private:
		unsigned btbSize;
		unsigned historySize;
		unsigned tagSize;
		unsigned fsmState;
		bool isGlobalHist;
		bool isGlobalTable;
		int Shared;
		vector<int> global_fsm_table;
		vector<BTBEntry> btb_table;
	public:


}
int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared){
	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}
