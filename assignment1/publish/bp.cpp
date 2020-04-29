#include "bp_api.h"
#include <cmath>
#include <iostream>
using namespace std;



// --------------------------------- bp.h ------------------------------------

#include <cstdint>

enum FsmState{
	SNT,WNT,WT,ST
};

enum ShareType{
	NO_SHARE,LSB_SHARE,MID_SHARE
};


class Branch{
	FsmState init_state;
	unsigned fsm_table_size;
	bool is_initialized;
	bool is_global_table;
	bool is_global_history;
	unsigned tag;
	uint32_t dest;
	unsigned *history;
	FsmState *table;
public:
	Branch() = default;
	Branch(FsmState fms_init,unsigned fsm_table_size,unsigned *history,
						FsmState *table,unsigned tag,unsigned dest);
	
	bool isInitialized() const;
	Branch &operator=(const Branch& copy);
	~Branch();	
	//setters
	void setTag(const unsigned tag);
	void setDest(const uint32_t dest);
	//getters
	unsigned getTag() const;
	uint32_t getDest() const;
	unsigned* getHistory();
	FsmState* getTable();
};


class BTB{
private:
	unsigned size;
	unsigned history_size;
	unsigned fsm_table_size;
	unsigned tag_size;
	unsigned flush_num;
	unsigned branch_num;
	unsigned *global_history;
	FsmState *global_fsm_table;
	FsmState fsm_state;
	bool is_global_history;
	bool is_global_table;
	ShareType share_type;
	Branch *inputs;
	unsigned get_table_position(uint32_t pc,unsigned position);

public:
	BTB() = delete;
	BTB(unsigned size,unsigned history_size,unsigned tag_size,
		FsmState fsm_state,bool is_global_history,bool is_global_table,ShareType share_type);
	BTB(const BTB& copy) = delete;
	BTB& operator=(const BTB& copy) = delete;
	~BTB();

	//methods
	bool predict(uint32_t pc,uint32_t *dest);
	void update(uint32_t pc,uint32_t target_pc,bool taken,uint32_t pred_dest);
	unsigned getNumOfFlushes() const;
	unsigned getSize() const;
	unsigned getNumOfBranches() const;
};

//------------------------Local Functions Signatures--------------------------
unsigned get_btb_tag(uint32_t pc,unsigned size, unsigned tag_size);
int get_btb_position(uint32_t pc ,unsigned size);


// -----------------------------------BTB-------------------------------------
BTB::BTB(unsigned size,unsigned history_size,unsigned tag_size,FsmState fsm_state,
	bool is_global_history, bool is_global_table,ShareType share_type):
		size(size),history_size(history_size),fsm_table_size((unsigned)pow(2,history_size))
		,tag_size(tag_size),flush_num(0),branch_num(0),fsm_state(fsm_state),
		is_global_history(is_global_history),is_global_table(is_global_table),
		share_type(share_type),inputs(new Branch[size]){

	this->global_history = (is_global_history) ? (new unsigned(0)) : (nullptr);
	this->global_fsm_table = (is_global_table) ? (new FsmState[fsm_table_size]) : (nullptr);
	if(is_global_table){
		//initialize the default state of the fsm.
		for(unsigned i=0 ; i< fsm_table_size;i++){
			this->global_fsm_table[i] = this->fsm_state;
		}
	}
}

bool BTB::predict(uint32_t pc,uint32_t *dest){
    int position = get_btb_position(pc, this->size);
    unsigned tag = get_btb_tag(pc,this->size, this->tag_size);
	
	//if the tag exists in the btb
    if(this->inputs[position].getTag() == tag && this->inputs[position].isInitialized() ) {
		// Get history with xor of the pc (the position of the FSM in the FSM table).
		unsigned table_pos = get_table_position(pc,position);
		// Get the current state of the FSM.
		FsmState state = this->inputs[position].getTable()[table_pos];
		//change the destination according to the fsm state.
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
	}else{ //if the tag doesn't exist.
		*dest = pc +4;
		return false;
	}
	return false ; //shouldn't get here.
}

void BTB::update(uint32_t pc, uint32_t target_pc,bool taken,uint32_t pred_dest){
	unsigned position = get_btb_position(pc, this->size);
	unsigned tag = get_btb_tag(pc,this->size,this->tag_size);
	//Check if the branch exists, if not create one.
	if(!this->inputs[position].isInitialized() || this->inputs[position].getTag() != tag){
		this->inputs[position] = Branch(this->fsm_state,this->fsm_table_size,this->global_history,this->global_fsm_table,tag,target_pc);
	}

    this->inputs[position].setDest(target_pc);
	//Update Fsm
    unsigned table_pos = get_table_position(pc,position);
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

	// update flash num.
	if((taken && pred_dest != target_pc) || (!taken && pred_dest != pc+4)){
		this->flush_num++;
	}
	// update branch num.
	this->branch_num++;
}

unsigned BTB::getNumOfFlushes() const{
	return this->flush_num;
}

unsigned BTB::getSize() const{
	if(!this->is_global_table  && !this->is_global_history)
		return this->size * (this->tag_size + 30 + this->history_size + 2*(int)pow(2,this->history_size));
	if(this->is_global_table  && !this->is_global_history)
		return this->size * (this->tag_size + 30 + this->history_size) + 2*(int)pow(2,this->history_size);
	if(!this->is_global_table  && this->is_global_history){
		return this->size * (this->tag_size + 30 + 2*(int)pow(2,this->history_size)) + this->history_size;
	}

	return this->size * (this->tag_size + 30) + this->history_size + 2*(int)pow(2,this->history_size);
}

unsigned BTB::getNumOfBranches() const{
	return this->branch_num;
}

BTB::~BTB(){
	if(global_history) delete global_history;
	if(global_fsm_table) delete global_fsm_table;
	delete[] inputs;
}

// -----------------------------------Branch---------------------------------------


bool Branch::isInitialized() const{
	return is_initialized;
}


Branch::Branch(FsmState fms_init,unsigned fsm_table_size,unsigned *history,FsmState *table,unsigned tag, uint32_t dest):
        init_state(fms_init), fsm_table_size(fsm_table_size),is_initialized(true),tag(tag),dest(dest){
    if(history){
        is_global_history = true;
        this->history = history;
    }else{
        is_global_history = false;
        this->history = new unsigned(0);
    }

    if(table){
        is_global_table = true;
        this->table = table;
    }else{
        is_global_table = false;
        this->table = new FsmState[fsm_table_size];
		for(unsigned i=0 ; i< fsm_table_size;i++){
			this->table[i] = this->init_state;
		}
    }
}

Branch::~Branch(){
	if(!is_global_table) delete[] table;
	if(!is_global_history) delete history;
}

Branch& Branch::operator=(const Branch& copy){
	this->is_initialized = copy.is_initialized;
	this->is_global_table = copy.is_global_table;
	this->is_global_history = copy.is_global_history;
	this->tag = copy.tag;
	this->dest = copy.dest;
	this->init_state = copy.init_state;
	this->fsm_table_size = copy.fsm_table_size;
	if(!is_global_history){
        delete this->history;
        this->history = new unsigned(0);
	}else{
        this->history = copy.history;
	}
	if(!is_global_table){
        delete[] this->table;
        this->table = new FsmState[copy.fsm_table_size];
		for(unsigned i=0 ; i< fsm_table_size;i++){
			this->table[i] = this->init_state;
		}
	}else{
        this->table = copy.table;
	}
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
			table_pos = *(this->inputs[position].getHistory()) ^ ((pc >> 16) & ((int) pow(2, history_size) - 1));
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
// -------------------------------- C Wrapper---------------------------------
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

