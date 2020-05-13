#include <vector>
#include <cmath>


using std::vector;
using std::pow;
using std::log2;

// -----------------------------------Structs---------------------------------
struct Entry{
    unsigned addr;
	unsigned tag;
	bool dirty;
	bool valid;
	explicit Entry(unsigned addr= 0, unsigned tag = 0,bool dirty = false,bool valid = false);
};

struct Queue{
    vector<unsigned> tags;
    void update(unsigned tag);
    unsigned popLRU();
};

struct Level{
	unsigned num_of_blocks,num_of_cycles,assoc,num_of_sets;
	double miss_rate;
	int num_of_requests,num_of_hit;
	Entry *cache_table;
	Queue *sets_lru_queue;
	explicit Level(unsigned num_of_blocks =0 ,unsigned num_of_cycles =0 ,unsigned assoc = 1);
	~Level();
	bool checkLvl(char operation, unsigned addr);
	Entry addToLvl(unsigned addr);
    Entry removeFromLvl(unsigned addr);
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
unsigned _extractTag(unsigned addr, unsigned size);
unsigned _reconstructAddr(unsigned tag, unsigned set);
void _updateQueue(vector<unsigned> &queue,unsigned tag,unsigned max_size);
unsigned _popFrontQueue(vector<unsigned> &queue);
// ------------------------- Implementations ---------------------------------

Entry::Entry(unsigned addr, unsigned tag,bool dirty, bool valid):addr(addr),tag(tag),dirty(dirty),valid(valid){}

Level::Level(unsigned num_of_blocks,unsigned num_of_cycles,unsigned assoc):
			num_of_blocks(num_of_blocks),num_of_cycles(num_of_cycles),assoc(assoc),
            num_of_sets(num_of_blocks/assoc),
			miss_rate(0),num_of_requests(0),num_of_hit(0),
			cache_table(new Entry[num_of_blocks]), sets_lru_queue(new Queue[num_of_sets]){

}


Level::~Level() {
    delete[] this->cache_table;
    delete[] this->sets_lru_queue;
}

CacheManager::CacheManager(unsigned mem_cyc,unsigned wr_alloc, unsigned block_size,
			unsigned l1_size, unsigned l2_size, unsigned l1_assoc,unsigned l2_assoc,
			unsigned l1_cyc, unsigned l2_cyc):mem_cyc(mem_cyc),wr_alloc(wr_alloc),
			num_of_cycles(0),num_of_calls(0),avg_acc_time(0){
				cache_level[0] = Level((unsigned)pow(2,l1_size-block_size),l1_cyc,(unsigned)pow(2,l1_assoc));
				cache_level[1] = Level((unsigned)pow(2,l2_size-block_size),l2_cyc,(unsigned)pow(2,l2_assoc));
}

/*
	simulate:
	---------
	Run simulation of the cache memory
	and update the results.
*/
void CacheManager::simulate(char operation,unsigned addr){
    this->num_of_calls++;
    // start with lvl 1
    this->num_of_cycles += this->cache_level[0].num_of_cycles;
    unsigned tag_0 = _extractTag(addr,this->cache_level[0].num_of_sets);
    unsigned set_0 = _extractSet(addr,this->cache_level[0].num_of_sets);
    bool hit_lvl_1 = this->cache_level[0].checkLvl(operation,addr);
	//if hit update lru queue and return
	if(hit_lvl_1){
        this->cache_level[0].sets_lru_queue[set_0].update(tag_0);
        return;
	}

    // miss in lvl 1 try lvl 2
    this->num_of_cycles += this->cache_level[1].num_of_cycles;
    unsigned tag_1 = _extractTag(addr,this->cache_level[1].num_of_sets);
    unsigned set_1 = _extractSet(addr,this->cache_level[1].num_of_sets);
    bool hit_lvl_2 = this->cache_level[0].checkLvl(operation,addr);

    if(hit_lvl_2){
        // update lru queue of lvl 2
        this->cache_level[1].sets_lru_queue[set_1].update(tag_1);
        // bring block to lvl 1
        Entry kicked_entry = this->cache_level[0].addToLvl(addr);
        unsigned kicked_tag_1 = _extractTag(kicked_entry.addr,this->cache_level[1].num_of_sets);
        if(kicked_entry.dirty){ // if adding caused kicking of another entry update if dirty (inclusion)
            this->cache_level[1].sets_lru_queue[set_1].update(kicked_tag_1);
        }
        return;
    }

    // not in lvl 2 then go to mem and bring to lvl 1 and lvl 2 (inclusion)
    this->num_of_cycles += this->mem_cyc;
    Entry kicked_entry_0 = this->cache_level[0].addToLvl(addr); // bring to lvl 1
    unsigned kicked_tag_1 = _extractTag(kicked_entry_0.addr,this->cache_level[1].num_of_sets);
    if(kicked_entry_0.dirty){ // if adding caused kicking of another entry.
        this->cache_level[1].sets_lru_queue[set_1].update(kicked_tag_1);
    }

    Entry kicked_entry_1 = this->cache_level[1].addToLvl(addr); // bring to lvl 2
    if(kicked_entry_1.valid){ // if adding caused kicking of another entry remove it from lvl 1 (inclusion)
        this->cache_level[0].removeFromLvl(kicked_entry_1.addr);
    }
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

unsigned _reconstructAddr(unsigned tag, unsigned set,int size){
    return ( tag << (2+(int)log2(size)) ) + set;
}

//relocate a given tag in the queue
void Queue::update(unsigned tag){
	//check if the tag already exists
	for(auto item = this->tags.begin();item < this->tags.end();item++){
		if(*item == tag){
            this->tags.erase(item);
            this->tags.push_back(tag);
			return;
		}
	}
	/*
	//if the queue is full remove the first tag
	if(queue.size() >= max_size) queue.erase(queue.begin());
	queue.push_back(tag); //insert the new tag
	 */
}

unsigned Queue::popLRU(){
    unsigned first = *(this->tags.begin());
    this->tags.erase(this->tags.begin());
    return first;
}

bool Level::checkLvl(char operation, unsigned addr){
    unsigned tag = _extractTag(addr,this->num_of_sets);
    unsigned set = _extractSet(addr,this->num_of_sets);

    // add request to lvl
    this->num_of_requests++;

    // check if block at addr is in lvl
    // assoc (num of ways) is column and set is row
    bool hit_lvl=false;
    for(int i=0;i< this->assoc;i++){
        Entry* entry = &(this->cache_table[i + set*this->assoc]);
        if(entry->valid && entry->tag == tag){
            hit_lvl = true;
            this->num_of_hit++;
            if(operation == 'w'){
                entry->dirty = true;
            }
        }
    }

    return  hit_lvl;
}

Entry Level::removeFromLvl(unsigned addr) {
    unsigned tag = _extractTag(addr,this->num_of_sets);
    unsigned set = _extractSet(addr,this->num_of_sets);

    for(int i=0;i< this->assoc;i++){
        Entry* entry = &(this->cache_table[i + set*this->assoc]);
        if(entry->valid && entry->tag == tag){
            Entry removed_entry = *entry;
            //set valid bit to false
            entry->valid = false;
            return removed_entry;
        }
    }
    // in case of fail
    return Entry();
}
Entry Level::addToLvl(unsigned addr) {
    unsigned tag = _extractTag(addr,this->num_of_sets);
    unsigned set = _extractSet(addr,this->num_of_sets);
    Entry removed_entry;

    if(this->sets_lru_queue[set].tags.size() == this->assoc){// ways are full kick lru
        unsigned lru_tag = this->sets_lru_queue[set].popLRU();
        unsigned lru_addr = _reconstructAddr(lru_tag,set,this->num_of_sets);
        removed_entry = this->removeFromLvl(lru_addr);
    }

    for(int i=0;i< this->assoc;i++) {
        Entry *entry = &(this->cache_table[i + set * this->assoc]);
        if (!entry->valid) {
            *entry = Entry(addr,tag,false,true);
        }
    }

    return removed_entry;
}

