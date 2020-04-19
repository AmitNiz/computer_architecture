#include <cstdint>

enum FsmState{
	ST,WT,WNT,SNT
};

enum ShareType{
	NO_SHARE,MID_SHARE,LSB_SHARE
};

struct FsmTable{
	FsmState *fsm;
};

struct Branch{
	unsigned tag;
	uint32_t dest;
	unsigned *history;
	FsmTable *table;
	
};


class BTB{
private:
	unsigned size;
	unsigned history_size;
	unsigned fsm_table_size;
	unsigned tag_size;
	unsigned num_of_inputs;
	unsigned flush_num;
	FsmState fsm_state;
	bool is_global_history;
	bool is_global_table;
	ShareType share_type;
	Branch *inputs;


public:
	BTB() =delete;
	BTB(unsigned size,unsigned history_size,unsigned tag_size,
		FsmState fsm_state,bool is_global_history,bool is_global_table,ShareType share_type);
	BTB(const BTB& copy) =delete;
	BTB& operator=(const BTB& copy) =delete;
	~BTB();

	//methods
	bool predict(uint32_t pc,uint32_t *dest);
	void update(uint32_t pc,uint32_t target_pc,bool taken,uint32_t pred_dest);
	unsigned getNumOfFlushes() const;
	unsigned getSize() const;
	unsigned getNumOfBranches() const;
};









