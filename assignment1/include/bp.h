#include <cstdint>

enum FsmState{
	SNT,WNT,WT,ST
};

enum ShareType{
	NO_SHARE,MID_SHARE,LSB_SHARE
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
	Branch();
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









