#include <vector>
#include <math.h>
#include <iostream>
#ifndef CACHE_MANAGER
#define CACHE_MANAGER
/*
using std::vector;
using std::pow;
using std::log2;
*/
using namespace std;
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
    void add(unsigned tag);
    unsigned popLRU();
};

struct Level{
	unsigned num_of_blocks,num_of_cycles,assoc,block_size,num_of_sets;
	int num_of_requests,num_of_hit;
	Entry *cache_table;
	Queue *sets_lru_queue;
	explicit Level(unsigned num_of_blocks,unsigned num_of_cycles ,unsigned assoc , unsigned block_size);
	~Level();
	bool checkLvl(char operation, unsigned addr);
	Entry addToLvl(unsigned addr, char op);
    Entry removeFromLvl(unsigned addr);
};

struct CacheManager{
	unsigned mem_cyc,wr_alloc,block_size,num_of_cycles,num_of_calls;
	double avg_acc_time;
	Level* cache_level[2];
	void simulate(char operation,unsigned addr);
	CacheManager(unsigned mem_cyc,unsigned wr_alloc,unsigned block_size,
				 unsigned l1_size,unsigned l2_size, unsigned l1_assoc,
				 unsigned l2_assoc, unsigned l1_cyc, unsigned l2_cyc);
	~CacheManager();

};

// ---------------------------Private Functions ------------------------------

unsigned _extractSet(unsigned addr, unsigned size, unsigned block_size);
unsigned _extractTag(unsigned addr, unsigned size, unsigned block_size);
unsigned _reconstructAddr(unsigned tag, unsigned set,int size, unsigned block_size);
// ------------------------- Implementations ---------------------------------

Entry::Entry(unsigned addr, unsigned tag,bool dirty,bool valid):addr(addr),tag(tag),dirty(dirty),valid(valid){}

Level::Level(unsigned num_of_blocks,unsigned num_of_cycles,unsigned assoc, unsigned block_size):
			num_of_blocks(num_of_blocks),num_of_cycles(num_of_cycles),assoc(assoc), block_size(block_size),
            num_of_sets(num_of_blocks/assoc),
			num_of_requests(0),num_of_hit(0),
			cache_table(new Entry[num_of_blocks]), sets_lru_queue(new Queue[num_of_sets]){}


Level::~Level() {
    delete[] this->cache_table;
    delete[] this->sets_lru_queue;
}

CacheManager::CacheManager(unsigned mem_cyc,unsigned wr_alloc, unsigned block_size,
			unsigned l1_size, unsigned l2_size, unsigned l1_assoc,unsigned l2_assoc,
			unsigned l1_cyc, unsigned l2_cyc):mem_cyc(mem_cyc),wr_alloc(wr_alloc), block_size(block_size),
			num_of_cycles(0),num_of_calls(0),avg_acc_time(0){
				cache_level[0] = new Level((unsigned)pow(2,l1_size-block_size),l1_cyc,(unsigned)pow(2,l1_assoc),block_size);
				cache_level[1] = new Level((unsigned)pow(2,l2_size-block_size),l2_cyc,(unsigned)pow(2,l2_assoc),block_size);
}
CacheManager::~CacheManager() {
    delete this->cache_level[0];
    delete this->cache_level[1];
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
    this->num_of_cycles += this->cache_level[0]->num_of_cycles;
    unsigned tag_0 = _extractTag(addr,this->cache_level[0]->num_of_sets,this->block_size);
    unsigned set_0 = _extractSet(addr,this->cache_level[0]->num_of_sets,this->block_size);
    //std::cout << "LVL 1" << std::endl;
    //std::cout << "tag: "<<std::hex<< tag_0 <<std::dec<< std::endl;
    bool hit_lvl_1 = this->cache_level[0]->checkLvl(operation,addr);
	//if hit update lru queue and return
	if(hit_lvl_1){
	    //std::cout << "---------------------------->HIT LVL 1" <<std::endl;
        this->cache_level[0]->sets_lru_queue[set_0].update(tag_0);
        return;
	}

    // miss in lvl 1 try lvl 2
    this->num_of_cycles += this->cache_level[1]->num_of_cycles;
    unsigned tag_1 = _extractTag(addr,this->cache_level[1]->num_of_sets,this->block_size);
    unsigned set_1 = _extractSet(addr,this->cache_level[1]->num_of_sets,this->block_size);
    //std::cout << "LVL 2" << std::endl;
    //std::cout << "tag: "<<std::hex<< tag_1 << std::dec<<std::endl;
    bool hit_lvl_2 = this->cache_level[1]->checkLvl(operation,addr);
    if(hit_lvl_2){
        //std::cout << "------------------------------->HIT LVL 2" <<std::endl;
        // update lru queue of lvl 2
        this->cache_level[1]->sets_lru_queue[set_1].update(tag_1);
        // bring block to lvl 1 if read or write and write_alloc
        if(operation=='r' || this->wr_alloc){
            Entry kicked_entry = this->cache_level[0]->addToLvl(addr,operation);
            unsigned kicked_tag_1 = _extractTag(kicked_entry.addr,this->cache_level[1]->num_of_sets,this->block_size);
            if(kicked_entry.dirty){ // if adding caused kicking of another entry update if dirty (inclusion)
                this->cache_level[1]->sets_lru_queue[set_1].update(kicked_tag_1);
            }
        }
        return;
    }

    // not in lvl 2 then go to mem and bring to lvl 1 and lvl 2 (inclusion)
    this->num_of_cycles += this->mem_cyc;
    if(operation=='r' || this->wr_alloc){
        Entry kicked_entry_0 = this->cache_level[0]->addToLvl(addr,operation); // bring to lvl 1
        Entry kicked_entry_1 = this->cache_level[1]->addToLvl(addr,operation); // bring to lvl 2 (inclusion)

        unsigned kicked_tag_1 = _extractTag(kicked_entry_0.addr,this->cache_level[1]->num_of_sets,this->block_size);
        if(kicked_entry_0.dirty){ // if adding caused kicking of another entry.
            this->cache_level[1]->sets_lru_queue[set_1].update(kicked_tag_1);
        }
        if(kicked_entry_1.valid){ // if adding caused kicking of another entry remove it from lvl 1 (inclusion)
            this->cache_level[0]->removeFromLvl(kicked_entry_1.addr);
        }
    }
}	


//calculate and return the matching set of an address
unsigned _extractSet(unsigned addr, unsigned size, unsigned block_size){
	addr = addr >> block_size; //ignore the first 2 bits.
	return addr & (size -1); //mask the set portion and return it
}


//calculate and return the matching tag of an address
unsigned _extractTag(unsigned addr, unsigned size, unsigned block_size){
	return addr >> ( block_size + unsigned(log2(size)) ); //return the tag portion of the address
}

unsigned _reconstructAddr(unsigned tag, unsigned set,int size, unsigned block_size){
    return ( ( tag << (int)log2(size) ) + set ) << block_size;
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
}

void Queue::add(unsigned tag){
    this->tags.push_back(tag);
}

unsigned Queue::popLRU(){
    unsigned first = *(this->tags.begin());
    this->tags.erase(this->tags.begin());
    return first;
}

bool Level::checkLvl(char operation, unsigned addr){
    unsigned tag = _extractTag(addr,this->num_of_sets, this->block_size);
    unsigned set = _extractSet(addr,this->num_of_sets, this->block_size);

    // add request to lvl
    this->num_of_requests++;
    //std::cout <<"| set: " << set <<" |";
    // check if block at addr is in lvl
    // assoc (num of ways) is column and set is row
    bool hit_lvl=false;
    for(int i=0;i< this->assoc;i++){
        Entry* entry = &(this->cache_table[i + set*this->assoc]);
        // debug
        if(entry->valid) //std::cout << std::hex << entry->tag << "  " <<std::dec;
        // else //std::cout << "X  ";
        //end debug
        if(entry->valid && entry->tag == tag){
            hit_lvl = true;
            this->num_of_hit++;
            if(operation == 'w'){
                entry->dirty = true;
            }
        }
    }
    //std::cout << std::endl;

    //debug
    //std::cout << "Qeueu: ";
    for(auto i = this->sets_lru_queue[set].tags.begin();i<this->sets_lru_queue[set].tags.end();i++){
        //std::cout << std::hex << *i << " "<< std::dec;
    }
    //std::cout << std::endl;


    return  hit_lvl;
}

Entry Level::removeFromLvl(unsigned addr) {
    unsigned tag = _extractTag(addr,this->num_of_sets, this->block_size);
    unsigned set = _extractSet(addr,this->num_of_sets, this->block_size);

    for(int i=0;i< this->assoc;i++){
        Entry* entry = &(this->cache_table[i + set*this->assoc]);
        if(entry->valid && entry->tag == tag){
            Entry removed_entry = *entry;
            //std::cout << "removed tag: " <<std::hex<< entry->tag<<std::dec << std::endl;
            //set valid bit to false
            entry->valid = false;
            return removed_entry;
        }
    }
    // in case of fail
    return Entry();
}
Entry Level::addToLvl(unsigned addr, char op) {
    unsigned tag = _extractTag(addr,this->num_of_sets, this->block_size);
    unsigned set = _extractSet(addr,this->num_of_sets, this->block_size);
    Entry removed_entry;
    //std::cout <<"set size: " << sets_lru_queue[set].tags.size() << " lvl assoc: " << this->assoc << std::endl;
    if(this->sets_lru_queue[set].tags.size() == this->assoc){// ways are full kick lru
        unsigned lru_tag = this->sets_lru_queue[set].popLRU();
        unsigned lru_addr = _reconstructAddr(lru_tag,set,this->num_of_sets, this->block_size);
        removed_entry = this->removeFromLvl(lru_addr);
    }

    for(int i=0;i< this->assoc;i++) {
        Entry *entry = &(this->cache_table[i + set * this->assoc]);
        if (!entry->valid) {
            *entry = Entry(addr,tag,(op == 'w'),true);
            this->sets_lru_queue[set].add(tag);
            break;
        }
    }

    return removed_entry;
}

#endif