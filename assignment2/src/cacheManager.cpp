#include <vector>
#include <cmath>


using std::vector;
using std::pow;
using std::log2;

// -----------------------------------Structs---------------------------------
struct Entry{
	unsigned tag;
	bool dirty;
	bool valid;
	Entry(unsigned tag = 0,bool dirty = 0,bool valid = 0);
};


struct Level{
	unsigned num_of_blocks,num_of_cycles,assoc;
	double miss_rate;
	Entry *cache_table;
	vector<unsigned> *sets_lru_queue;
	Level(unsigned num_of_blocks =0 ,unsigned num_of_cycles =0 ,unsigned assoc = 1);
};

struct CacheManager{
	unsigned mem_cyc,wr_alloc,num_of_cycles,num_of_calls;	
	double avg_acc_time;
	Level cache_level[2];	
	void simulate(char operation,unsigned addr);
	CacheManager(unsigned mem_cyc,unsigned wr_alloc,unsigned block_size,
				 unsigned l1_size,unsigned l2_size, unsigned l1_assoc,
				 unsigned l2_assoc, unsigned l1_cyc, unsigned l2_cyc);

};

// ---------------------------Private Functions ------------------------------

unsigned _extractSet(unsigned addr, unsigned size);
void _updateQueue(vector<unsigned> *queue,unsigned tag);


// ------------------------- Implementations ---------------------------------

Entry::Entry(unsigned tag,bool dirty, bool valid):tag(tag),dirty(dirty),valid(valid){}

Level::Level(unsigned num_of_blocks,unsigned num_of_cycles,unsigned assoc):
			num_of_blocks(num_of_blocks),num_of_cycles(num_of_cycles),assoc(assoc),
			miss_rate(0){
		
	this->cache_table[num_of_blocks];
	this->sets_lru_queue[num_of_blocks/assoc];

}

CacheManager::CacheManager(unsigned mem_cyc,unsigned wr_alloc, unsigned block_size,
			unsigned l1_size, unsigned l2_size, unsigned l1_assoc,unsigned l2_assoc,
			unsigned l1_cyc, unsigned l2_cyc):mem_cyc(mem_cyc),wr_alloc(wr_alloc),
			num_of_cycles(0),num_of_calls(0),avg_acc_time(0){
				cache_level[0] = Level(pow(2,l1_size-block_size),l1_cyc,pow(2,l1_assoc));
				cache_level[1] = Level(pow(2,l2_size-block_size),l2_cyc,pow(2,l2_assoc));
}

/*
	simulate:
	---------
	Run simulation of the cache memory
	and update the results.
*/
void CacheManager::simulate(char operation,unsigned addr){ 
}	


//calculate and return the matching set of an address
unsigned _extractSet(unsigned addr, unsigned size){	
	addr = addr >> 2; //ignore the first 2 bits.
	return addr & (size -1); //mask the set portion and return it
}


//calculate and return the matching tag of an address
unsigned _extractTag(unsigned addr, unsigned size){	
	return addr >> (2 + unsigned(log2(size))); //return the tag portion of the address
}


//relocate a given tag in the queue
void _updateQueue(vector<unsigned> &queue,unsigned tag,unsigned max_size){	
	//check if the tag already exists
	for(auto item = queue.begin();item < queue.end();item++){
		if(*item == tag){
			queue.erase(item);
			queue.push_back(tag);
			return;
		}
	}
	//if the queue is full remove the first tag
	if(queue.size() >= max_size) queue.erase(queue.begin());
	queue.push_back(tag); //insert the new tag
}

