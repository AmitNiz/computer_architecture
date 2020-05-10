#include <vector>


using std::vector;

// -----------------------------------Structs---------------------------------
struct Entry{
	unsigned tag;
	bool dirty;
	bool valid;
	Entry(unsigned tag,bool dirty,bool valid);
};


struct Level{
	unsigned num_of_blocks,num_of_cycles,assoc;
	double miss_rate;
	Entry *cache_table;
	vector<unsigned> *sets_lru_queue;
	Level(unsigned num_of_blocks,unsigned num_of_cycles,unsigned assoc);
};

struct CacheManager{
	unsigned mem_cyc,wr_alloc,num_of_cycles,num_of_calls;	
	double avg_acc_time;
	Level cache_level[2];	
	void updateCache(char operation,unsigned addr);
	CacheManager(unsigned mem_cyc,unsigned wr_alloc,unsigned block_size,
				 unsigned l1_size,unsigned l2_size, unsigned l1_assoc,
				 unsigned l2_assoc, unsigned l1_cyc, unsigned l2_cyc);

};

// ---------------------------Private Functions ------------------------------

unsigned _extractSet(unsigned addr, unsigned size);
void _updateQueue(vector<unsigned> *queue,unsigned tag);



