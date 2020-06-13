/* 046267 Computer Architecture - Spring 2020 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
#define NUM_OF_REGS 32
#define ENTRY -1

struct Dependencies{
    int src1_incruction_num;
    int src1_opcode;
    int src2_incruction_num;
    int src2_opcode;
};

struct MyProgCtx{
    int* depth_array;
    Dependencies* dependencies_array;
    int numOfInsts;
    int max_depth;
    explicit MyProgCtx(unsigned int numOfInsts):depth_array(new int[numOfInsts]),dependencies_array(new Dependencies[numOfInsts])
    ,numOfInsts(numOfInsts),max_depth(0){}
    ~MyProgCtx(){
        delete[] this->dependencies_array;
        delete[] this->depth_array;
    }
};

inline int max(int a, int b){
    return a>b ? a : b;
}


ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    if(numOfInsts < 0) {
        return PROG_CTX_NULL;
    }
    MyProgCtx* myProgCtx = new MyProgCtx(numOfInsts);

    for(int i=0;i<numOfInsts; i++){
        myProgCtx->depth_array[i] = 0;
    }

    int last_instuction_write[NUM_OF_REGS];

    for(int i=0; i<NUM_OF_REGS; i++){
        last_instuction_write[i] = ENTRY;
    }

    for(int i=0;i < numOfInsts; i++){
        myProgCtx->dependencies_array[i].src1_incruction_num = last_instuction_write[progTrace[i].src1Idx];
        myProgCtx->dependencies_array[i].src1_opcode = progTrace[last_instuction_write[progTrace[i].src1Idx]].opcode;
        myProgCtx->dependencies_array[i].src2_incruction_num = last_instuction_write[progTrace[i].src2Idx];
        myProgCtx->dependencies_array[i].src2_opcode = progTrace[last_instuction_write[progTrace[i].src2Idx]].opcode;

        int depth_src1,latency_src1,depth_src2,latency_src2;
        if(myProgCtx->dependencies_array[i].src1_incruction_num != ENTRY){
            depth_src1 =  myProgCtx->depth_array[myProgCtx->dependencies_array[i].src1_incruction_num];
            latency_src1 = opsLatency[myProgCtx->dependencies_array[i].src1_opcode];
            if(i == 5){
                std::cout <<"sr1_dep: " << myProgCtx->dependencies_array[i].src1_incruction_num <<" opcode: "<< myProgCtx->dependencies_array[i].src1_opcode << " latency: " << latency_src1 << std::endl;
            }

        }
        else{
            depth_src1 =  0;
            latency_src1 = 0;
        }

        if(myProgCtx->dependencies_array[i].src2_incruction_num != ENTRY){
            depth_src2 =  myProgCtx->depth_array[myProgCtx->dependencies_array[i].src2_incruction_num];
            latency_src2 = opsLatency[myProgCtx->dependencies_array[i].src2_opcode];
        }
        else{
            depth_src2 =  0;
            latency_src2 = 0;
        }
        myProgCtx->depth_array[i] = max( depth_src1 + latency_src1 , depth_src2 + latency_src2);

        last_instuction_write[progTrace[i].dstIdx] = i;
    }

    for(int i=0; i< myProgCtx->numOfInsts ; i++){
        if(myProgCtx->max_depth < myProgCtx->depth_array[i] + opsLatency[progTrace[i].opcode]){
            myProgCtx->max_depth = myProgCtx->depth_array[i] + opsLatency[progTrace[i].opcode];
        }
    }

    return myProgCtx;
}

void freeProgCtx(ProgCtx ctx) {
    delete  (MyProgCtx*)ctx;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    MyProgCtx* myProgCtx = (MyProgCtx*)ctx;
    if(theInst > myProgCtx->numOfInsts) return -1;

    return myProgCtx->depth_array[theInst];
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    MyProgCtx* myProgCtx = (MyProgCtx*)ctx;
    if(theInst > myProgCtx->numOfInsts) return -1;
    *src1DepInst = myProgCtx->dependencies_array[theInst].src1_incruction_num;
    *src2DepInst = myProgCtx->dependencies_array[theInst].src2_incruction_num;
    return 0;
}

int getProgDepth(ProgCtx ctx) {
    MyProgCtx* myProgCtx = (MyProgCtx*)ctx;

    return myProgCtx->max_depth;
}


