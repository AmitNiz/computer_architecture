/* 046267 Computer Architecture - Spring 2020 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>


class MultiThread{
public:
	MultiThread() =delete;
	MultiThread(bool is_blocked,int num_of_threads,int load_num_of_cycles,
								int store_num_of_cycles,int context_switch_num_of_cycles);
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
	int context_switch_num_of_cycles;
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

MultiThread::MultiThread(bool is_blocked,int num_of_threads,int load_num_of_cycles,
			int store_num_of_cycles,int context_switch_num_of_cycles):is_blocked(is_blocked),num_of_threads(num_of_threads),
			load_num_of_cycles(load_num_of_cycles),store_num_of_cycles(store_num_of_cycles),
			context_switch_num_of_cycles(context_switch_num_of_cycles),current_cycle(0),num_of_instructions(0)
			{}



MultiThread* multiThread;
int exit_count = 0;

void CORE_BlockedMT() {
	multiThread = new MultiThread(true,SIM_GetThreadsNum(),SIM_GetLoadLat(),SIM_GetStoreLat(),SIM_GetSwitchCycles());
}

void CORE_FinegrainedMT() {
}

double CORE_BlockedMT_CPI(){
    exit_count++;
    if(exit_count > 1) delete multiThread;
	return 0;
}

double CORE_FinegrainedMT_CPI(){
    exit_count++;
    if(exit_count > 1) delete multiThread;
	return 0;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
}
