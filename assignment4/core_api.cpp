/* 046267 Computer Architecture - Spring 2020 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>


class MultiThread{
public:
	MultiThread() =delete;
	MultiThread(bool is_blocked,int num_of_threads,int load_num_of_cycles,
								int store_num_of_cycles,int current_cycle);
	MultiThread(const MultiThread& copy) = delete;
	MultiThread& operator=(const MultiThread& copy) = delete;
	~MultiThread();
	
	void simulate();
	void loadContex(tcontext* context,int thread_id)const;
	int getCPI()const;

private:
	bool is_blocked;
	int num_of_threads;
	int load_num_of_cycles;
	int store_num_of_cycles;
	int current_cycle;
	int num_of_instructions;
	struct Thread{
		int last_cycle_ran;
		int current_instruction;
		cmd_opcode current_cmd;
		int registers[REGS_COUNT];
	};
	Thread *threads; 
	void simulate_blocked();
	void simulate_grained();
};

void CORE_BlockedMT() {

}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
	return 0;
}

double CORE_FinegrainedMT_CPI(){
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
