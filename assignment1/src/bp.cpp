#include "../include/bp.h"
#include <cmath>

using namespace std;


// -----------------------------------BTB------------------------------------------
BTB::BTB(unsigned size,unsigned history_size,unsigned tag_size,FsmState fsm_state,
	bool is_global_history, bool is_global_table,ShareType share_type):
		size(size),history_size(history_size),tag_size(tag_size),fsm_state(fsm_state),		  is_global_history(is_global_history),is_global_table(is_global_table),
		share_type(share_type),inputs(new Branch[size]),fsm_table_size(pow(2,history_size)){

	this->global_history = (is_global_history) ? (new unsigned(0)) : (nullptr);
	this->global_fsm_table = (is_global_table) ? (new FsmState[fsm_table_size]{fsm_init}) : (nullptr);

	for(int i=0; i< size; i++){
		inputs[i] = Branch(global_history,global_fsm_table);
	}
}

~BTB(){
	if(global_history) delete global_history;
	if(global_fsm_table) delete global_fsm_table;
	delete[] inputs;
}

// -----------------------------------Branch---------------------------------------
Branch::Branch(FsmState fms_init,unsigned fsm_table_size,unsigned *history,FsmState *table):tag(0),dest(0){
		if(history){
			is_global_history = true;
			this->history = history;
		}else{
			is_global_history = false;
			this->history = new unsigned();
		}

		if(table){
			is_global_table = true;
			this->table = table;
		}else{
			is_global_table = false;
			this->table = new FsmState[fsm_table_size]{fsm_init};
		}
}

Branch::~Branch(){
	if(!is_global_table) delete[] table;
	if(!is_global_history) delete history;
}


void Branch::setTag(const unsigned tag){
	this->tag =tag;
}

void Branch::setDest(const uint32_t dest){
	this->dest = dest;
}

unsigned Branch::getTag(){
	return this->tag;
}

uint32_t Branch::getDest(){
	return this->dest;
}

unsigned *Branch::getHistory(){
	return this->history;
}

