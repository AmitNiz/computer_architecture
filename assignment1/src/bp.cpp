#include "../include/bp.h"
#include <cmath>

using namespace std;


//Signatures
unsigned get_btb_tag(uint32_t pc,unsigned size, unsigned tag_size);
int get_btb_position(uint32_t pc ,unsigned size);



// -----------------------------------BTB------------------------------------------
BTB::BTB(unsigned size,unsigned history_size,unsigned tag_size,FsmState fsm_state,
	bool is_global_history, bool is_global_table,ShareType share_type):
		size(size),history_size(history_size),tag_size(tag_size),fsm_state(fsm_state),		  is_global_history(is_global_history),is_global_table(is_global_table),
		share_type(share_type),inputs(new Branch[size]),fsm_table_size(pow(2,history_size)){

	this->global_history = (is_global_history) ? (new unsigned(0)) : (nullptr);
	this->global_fsm_table = (is_global_table) ? (new FsmState[fsm_table_size]{fsm_state}) : (nullptr);

}

bool BTB::predict(uint32_t pc,uint32_t *dest){
    int position = get_btb_position(pc, this->size);
    unsigned tag = get_btb_tag(pc,this->size, this->tag_size);
    if(this->inputs[position].getTag() == tag) {
		// Get history with xor of the pc (the position of the FSM in the FSM table).
		unsigned table_pos = get_table_position(pc,position);
		// Get the current state of the FSM.
		FsmState state = this->inputs[position].getTable()[table_pos];
		switch (state) {
			case ST:
			case WT: {
				*dest = this->inputs[position].getDest();
				return true;
			}
			case WNT:
			case SNT: {
				*dest = pc + 4;
				return false;
			}
		}
	}
	*dest = pc+4;
	return false;
}

void BTB::update(uint32_t pc, uint32_t target_pc,bool taken,uint32_t pred_dest){
	unsigned position = get_btb_position(pc, this->size);
	unsigned tag = get_btb_tag(pc,this->size,this->tag_size);
	unsigned table_pos = get_table_position(pc,position);
	//Check if the branch exists, if not create one.
	if(!this->inputs[position].isInitialized() || this->inputs[position].getTag() != tag){
		this->inputs[position] = Branch(this->fsm_state,this->fsm_table_size,this->global_history,this->global_fsm_table,tag,target_pc);	
	}	
	//Update Fsm
	FsmState current_state = this->inputs[position].getTable()[table_pos];
	switch(current_state){
		case WT:
		{
			this->inputs[position].getTable()[table_pos] = (taken) ? (ST) : (WNT);
			break;
		}
		case WNT:
		{
			this->inputs[position].getTable()[table_pos] = (taken) ? (WT) : (SNT);
			break;
		}

		case ST:
		{
			this->inputs[position].getTable()[table_pos] = (taken) ? (ST) : (WT);
			break;
		}
		
		case SNT:
		{
			this->inputs[position].getTable()[table_pos] = (taken) ? (WNT) : (SNT);
			break;
		}
	}
	//Update history
	unsigned *history = this->inputs[position].getHistory();
	//Shift the history 1 bit to the left and add new bit to the lsb according to taken's state.
	*history = ((*history << 1) & ((int)pow(2,history_size)-1)) | unsigned(taken);
}




BTB::~BTB(){
	if(global_history) delete global_history;
	if(global_fsm_table) delete global_fsm_table;
	delete[] inputs;
}

// -----------------------------------Branch---------------------------------------

Branch::Branch():is_global_table(false),is_global_history(false),
	tag(0),dest(0),history(nullptr),table(nullptr),is_initialized(false){}

bool Branch::isInitialized() const{
	return is_initialized;
}



Branch::Branch(FsmState fms_init,unsigned fsm_table_size,unsigned *history,FsmState *table,unsigned tag, uint32_t dest):tag(tag),dest(dest),is_initialized(true){
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
			this->table = new FsmState[fsm_table_size]{fms_init};
		}
}

Branch::~Branch(){
	if(!is_global_table) delete[] table;
	if(!is_global_history) delete history;
}

Branch& Branch::operator=(const Branch& copy){
	this->is_global_table = copy.is_global_table;
	this->is_global_history = copy.is_global_history;
	this->tag = copy.tag;
	this->dest = copy.dest;
	if(!is_global_history) delete this->history;
	this->history = copy.history;
	if(!is_global_table) delete[] this->table;
	this->table = copy.table;
	return *this;
}


void Branch::setTag(const unsigned tag){
	this->tag =tag;
}

void Branch::setDest(const uint32_t dest){
	this->dest = dest;
}

unsigned Branch::getTag() const{
	return this->tag;
}

uint32_t Branch::getDest() const{
	return this->dest;
}

unsigned *Branch::getHistory(){
	return this->history;
}

FsmState* Branch::getTable(){
    return this->table;
}


// ----------------------------Local Functions-------------------------------------

unsigned BTB::get_table_position(uint32_t pc,unsigned position){
	unsigned table_pos;
	switch (this->share_type) {
		case NO_SHARE: {
			table_pos = *(this->inputs[position].getHistory());
			break;
		}
		case MID_SHARE: {
			table_pos = *(this->inputs[position].getHistory()) ^ ((pc >> 15) & ((int) pow(2, history_size) - 1));
			break;
		}
		case LSB_SHARE: {
			table_pos = *(this->inputs[position].getHistory()) ^ ((pc >> 2) & ((int) pow(2, history_size) - 1));
			break;
		}
	}
	return table_pos;
}

int get_btb_position(uint32_t pc ,unsigned size){
	return (pc >> 2) & (size-1);
}

unsigned get_btb_tag(uint32_t pc,unsigned size, unsigned tag_size){
	return (pc >> (2+(int)log2(size))) & ((int)pow(2,tag_size)-1);
}

