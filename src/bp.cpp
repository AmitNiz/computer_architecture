#include "../include/bp.h"
#include <cmath>

using namespace std;

BTB::BTB(unsigned size,unsigned history_size,unsigned tag_size,FsmState fsm_state,
	bool is_global_history, bool is_global_table,ShareType share_type):
		size(size),history_size(history_size),tag_size(tag_size),fsm_state(fsm_state),		  is_global_history(is_global_history),is_global_table(is_global_table),
		ShareType(share_type),inputs(new Branch[size]),fsm_table_size(pow(2,history_size){}




