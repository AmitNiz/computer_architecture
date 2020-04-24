/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "../include/bp_api.h"
#include "../include/bp.h"

static BTB *predictor;

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	predictor = new BTB(btbSize,historySize,tagSize,FsmState(fsmState),isGlobalHist,isGlobalTable,ShareType(Shared));
	return (predictor) ? 0 : -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return predictor->predict(pc,dst);
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	predictor->update(pc,targetPc,taken,pred_dst);	
}

void BP_GetStats(SIM_stats *curStats){
	curStats->flush_num = predictor->getNumOfFlushes();
	curStats->br_num =predictor->getNumOfBranches();
	curStats->size =predictor->getSize();
	delete predictor;
}

