/* 046267 Computer Architecture - Spring 2020 - HW #4 */

#include "core_api.h"
#include "sim_api.h"
//#include <iostream>
#include <stdio.h>

#define BLOCKED 1
#define FINEGRAINED 0

//using std::cout;
//using std::endl;

class MultiThread{
public:
	//MultiThread() =delete;
	MultiThread(bool is_blocked,int num_of_threads,int load_num_of_cycles,
								int store_num_of_cycles,int context_switch_num_of_cycles);
	//MultiThread(const MultiThread& copy) = delete;
	//MultiThread& operator=(const MultiThread& copy) = delete;
	~MultiThread();
	
	void simulate();
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
        Thread();
        //~Thread() = default;
    };
    Thread *threads;
private:
    int find_ready_thread(int thread_curr);
	void simulate_blocked();
	void simulate_grained();
};

MultiThread::Thread::Thread():last_cycle_ran(0),current_instruction(0),current_cmd(CMD_NOP) {
    for(int i=0;i<REGS_COUNT;i++){
        this->registers[i] = 0;
    }
}

MultiThread::MultiThread(bool is_blocked,int num_of_threads,int load_num_of_cycles,
			int store_num_of_cycles,int context_switch_num_of_cycles):is_blocked(is_blocked),num_of_threads(num_of_threads),
			load_num_of_cycles(load_num_of_cycles),store_num_of_cycles(store_num_of_cycles),
			context_switch_num_of_cycles(context_switch_num_of_cycles),current_cycle(0),num_of_instructions(0),
			threads(new Thread[num_of_threads])
			{}
MultiThread::~MultiThread() {
    delete[] this->threads;
}
void MultiThread::simulate() {
    if(this->is_blocked){
        this->simulate_blocked();
    }else{
        this->simulate_grained();
    }
}

void MultiThread::simulate_blocked() {
    int num_of_halt_threads = 0;
    int thread_curr=0;
    Instruction instruction_current;
    bool is_contex_swiched = false;
    // iterate over threads (RR).
    while (num_of_halt_threads < this->num_of_threads){
        // iterate over instructions.
        SIM_MemInstRead(this->threads[thread_curr].current_instruction,&instruction_current,thread_curr);
        this->threads[thread_curr].current_cmd = instruction_current.opcode;
        this->threads[thread_curr].current_instruction++;
        this->num_of_instructions++;
        this->threads[thread_curr].last_cycle_ran = this->current_cycle;
        switch (instruction_current.opcode){
            case CMD_ADD: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        + this->threads[thread_curr].registers[instruction_current.src2_index_imm];
                //cout << "Thread: " << thread_curr << " ADD" <<endl;
                break;
            }
            case CMD_ADDI: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        + instruction_current.src2_index_imm;
                //cout << "Thread: " << thread_curr << " ADDI" <<endl;
                break;
            }
            case CMD_SUB: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        - this->threads[thread_curr].registers[instruction_current.src2_index_imm];
                //cout << "Thread: " << thread_curr << " SUB" <<endl;
                break;
            }
            case CMD_SUBI: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        - instruction_current.src2_index_imm;
                //cout << "Thread: " << thread_curr << " SUBI" <<endl;
                break;
            }
            case CMD_STORE: {
                if (instruction_current.isSrc2Imm) {
                    SIM_MemDataWrite(this->threads[thread_curr].registers[instruction_current.dst_index] +
                                     instruction_current.src2_index_imm,
                                     this->threads[thread_curr].registers[instruction_current.src1_index]);
                } else {
                    SIM_MemDataWrite(this->threads[thread_curr].registers[instruction_current.dst_index] +
                                     this->threads[thread_curr].registers[instruction_current.src2_index_imm],
                                     this->threads[thread_curr].registers[instruction_current.src1_index]);
                }
                is_contex_swiched = true;
                //cout << "Thread: " << thread_curr << " STORE" <<endl;
                break;
            }
            case CMD_LOAD: {
                if (instruction_current.isSrc2Imm) {
                    SIM_MemDataRead(this->threads[thread_curr].registers[instruction_current.src1_index] +
                                    instruction_current.src2_index_imm,
                                    &(this->threads[thread_curr].registers[instruction_current.dst_index]));
                } else {
                    SIM_MemDataRead(this->threads[thread_curr].registers[instruction_current.src1_index] +
                                    this->threads[thread_curr].registers[instruction_current.src2_index_imm],
                                    &(this->threads[thread_curr].registers[instruction_current.dst_index]));
                }
                is_contex_swiched = true;
                //cout << "Thread: " << thread_curr << " LOAD" <<endl;
                break;
            }
            case CMD_HALT: {
                num_of_halt_threads++;
                is_contex_swiched = true;
                //cout << "Thread: " << thread_curr << " HALT" <<endl;
                break;
            }
            case CMD_NOP: {
                //cout << "Thread: " << thread_curr << " NOP" <<endl;
                break;
            }
        }
        if(is_contex_swiched) {
            // find the next thread that is ready
            int thread_ready = thread_curr;
            do{
                // if no thread ready then idle
                this->current_cycle++;
                //if(thread_ready<0) cout <<"idle" <<endl; // DEBUG
                thread_ready = this->find_ready_thread(thread_curr);
            }while(thread_ready < 0 && num_of_halt_threads < this->num_of_threads);
            is_contex_swiched = false;
            if(thread_curr != thread_ready && thread_ready != -1){
               // cout << "Switch overhead " << thread_curr << " -> "<<thread_ready<<endl;
                this->current_cycle+= this->context_switch_num_of_cycles;
                thread_curr = thread_ready;
            }
        }else{
            this->current_cycle++;
        }
    }
}

void MultiThread::simulate_grained() {
    int num_of_halt_threads = 0;
    int thread_curr=0;
    Instruction instruction_current;
    // iterate over threads (RR).
    while (num_of_halt_threads < this->num_of_threads){
        // iterate over instructions.
        SIM_MemInstRead(this->threads[thread_curr].current_instruction,&instruction_current,thread_curr);
        this->threads[thread_curr].current_cmd = instruction_current.opcode;
        this->threads[thread_curr].current_instruction++;
        this->num_of_instructions++;
        this->threads[thread_curr].last_cycle_ran = this->current_cycle;
        switch (instruction_current.opcode){
            case CMD_ADD: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        + this->threads[thread_curr].registers[instruction_current.src2_index_imm];
                //cout << "Thread: " << thread_curr << " ADD" <<endl;
                break;
            }
            case CMD_ADDI: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        + instruction_current.src2_index_imm;
                //cout << "Thread: " << thread_curr << " ADDI" <<endl;
                break;
            }
            case CMD_SUB: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        - this->threads[thread_curr].registers[instruction_current.src2_index_imm];
                //cout << "Thread: " << thread_curr << " SUB" <<endl;
                break;
            }
            case CMD_SUBI: {
                this->threads[thread_curr].registers[instruction_current.dst_index] =
                        this->threads[thread_curr].registers[instruction_current.src1_index]
                        - instruction_current.src2_index_imm;
                //cout << "Thread: " << thread_curr << " SUBI" <<endl;
                break;
            }
            case CMD_STORE: {
                if (instruction_current.isSrc2Imm) {
                    SIM_MemDataWrite(this->threads[thread_curr].registers[instruction_current.dst_index] +
                                     instruction_current.src2_index_imm,
                                     this->threads[thread_curr].registers[instruction_current.src1_index]);
                } else {
                    SIM_MemDataWrite(this->threads[thread_curr].registers[instruction_current.dst_index] +
                                     this->threads[thread_curr].registers[instruction_current.src2_index_imm],
                                     this->threads[thread_curr].registers[instruction_current.src1_index]);
                }
                //cout << "Thread: " << thread_curr << " STORE" <<endl;
                break;
            }
            case CMD_LOAD: {
                if (instruction_current.isSrc2Imm) {
                    SIM_MemDataRead(this->threads[thread_curr].registers[instruction_current.src1_index] +
                                    instruction_current.src2_index_imm,
                                    &(this->threads[thread_curr].registers[instruction_current.dst_index]));
                } else {
                    SIM_MemDataRead(this->threads[thread_curr].registers[instruction_current.src1_index] +
                                    this->threads[thread_curr].registers[instruction_current.src2_index_imm],
                                    &(this->threads[thread_curr].registers[instruction_current.dst_index]));
                }
                //cout << "Thread: " << thread_curr << " LOAD" <<endl;
                break;
            }
            case CMD_HALT: {
                num_of_halt_threads++;
                //cout << "Thread: " << thread_curr << " HALT" <<endl;
                break;
            }
            case CMD_NOP: {
                //cout << "Thread: " << thread_curr << " NOP" <<endl;
                break;
            }
        }
        int thread_ready = thread_curr;
        // switching thread every cycle
        thread_curr = (thread_curr+1)%(this->num_of_threads);
        do{
            this->current_cycle++;
            thread_ready = this->find_ready_thread(thread_curr);
            //if(thread_ready<0 && num_of_halt_threads < this->num_of_threads) cout <<"idle" <<endl; // DEBUG
        }while(thread_ready < 0 && num_of_halt_threads < this->num_of_threads);
        thread_curr = thread_ready;
    }
}

int MultiThread::find_ready_thread(int thread_curr) {
    int i = thread_curr;
    bool is_thread_ready = false;
        int k = 0;
    while (k < this->num_of_threads ){
        switch (this->threads[i].current_cmd){
            case CMD_HALT: {
                break;
            }
            case CMD_STORE: {
                if (this->current_cycle - this->threads[i].last_cycle_ran > this->store_num_of_cycles) {
                    thread_curr = i;
                    is_thread_ready = true;
                }
                break;
            }
            case CMD_LOAD: {
                if (this->current_cycle - this->threads[i].last_cycle_ran > this->load_num_of_cycles) {
                    thread_curr = i;
                    is_thread_ready = true;
                }
                break;
            }
            default: {
                thread_curr = i;
                is_thread_ready = true;
                break;
            }
        }
        if(is_thread_ready){
            break;
        }
        k++;
        i = (i+1)%(this->num_of_threads);
    }
    if(is_thread_ready){
        return thread_curr;
    }
    return -1;
}



MultiThread* multiThread_blocked = NULL;
MultiThread* multiThread_grained = NULL;
int exit_count = 0;

void CORE_BlockedMT() {
    multiThread_blocked = new MultiThread(BLOCKED,SIM_GetThreadsNum(),SIM_GetLoadLat(),SIM_GetStoreLat(),SIM_GetSwitchCycles());
    multiThread_blocked->simulate();
}

void CORE_FinegrainedMT() {
    multiThread_grained = new MultiThread(FINEGRAINED,SIM_GetThreadsNum(),SIM_GetLoadLat(),SIM_GetStoreLat(),SIM_GetSwitchCycles());
    multiThread_grained->simulate();
}

double CORE_BlockedMT_CPI(){
    double CPI = (double)multiThread_blocked->current_cycle / multiThread_blocked->num_of_instructions;
    delete multiThread_blocked;
    return CPI;
}

double CORE_FinegrainedMT_CPI(){
    exit_count++;
    double CPI = (double)multiThread_grained->current_cycle / multiThread_grained->num_of_instructions;
    delete multiThread_grained;
	return CPI;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
    for(int i=0; i<REGS_COUNT;i++){
        context[threadid].reg[i] = multiThread_blocked->threads[threadid].registers[i];
    }
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
    for(int i=0; i<REGS_COUNT;i++){
        context[threadid].reg[i] = multiThread_grained->threads[threadid].registers[i];
    }
}
