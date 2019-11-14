#ifndef CACHE_H
#define CACHE_H
#include <vector>
#include <queue>
#include "../common.h"
#include "../sim.h"
#include "FunctionalCache.h"
#include <unordered_set>
using namespace std;

class DRAMSimInterface;
class Simulator;
class Cache;
class Core;

struct TransPointerLT {
    bool operator()(const MemTransaction* a, const MemTransaction* b) const {
     
      if(a->id >= 0 && b->id >= 0) { //not eviction or prefetch
        return *(a->d) < *(b->d); //compare their dynamic nodes
      }
      else {
        return a < b; //just use their pointers, so they're unique in the set
      }        
    }
};

class MSHR_entry {
public:
  set<MemTransaction*, TransPointerLT> opset;
  bool hit=false;
  int non_prefetch_size=0;
  void insert(MemTransaction* t) {
    opset.insert(t);
    //inc num of real mem ops
   
    if(!t->isPrefetch) {
      non_prefetch_size++; 
    }
    
  }
};

class Cache {
public:
  Simulator *sim;
  Core *core;
  DRAMSimInterface *memInterface;
  Cache* parent_cache;
  Cache* child_cache;
  bool isLLC=false;
  bool isL1=false;

  int prefetch_set_size=128;
  queue<uint64_t> prefetch_queue; //order of loads
  unordered_set<uint64_t> prefetch_set;
  int pattern_threshold=4; //how many close addresses to check to determine spatially local accesses
  int min_stride=4; //bytes of strided access

  unordered_map<uint64_t,MSHR_entry> mshr; //map of cacheline to set to mshr_entry
  vector<MemTransaction*> to_execute;
  vector<MemTransaction*> next_to_execute;
  //unordered_map<uint64_t,set<MemTransaction*>> memop_map; //map of cacheline to set of dynamic nodes
  vector<MemTransaction*> to_send;
  vector<tuple<uint64_t, int>> to_evict;
  priority_queue<TransactionOp, vector<TransactionOp>, TransactionOpCompare> pq;
  
  int size;
  int latency;
  int size_of_cacheline;
  int load_ports;
  int store_ports;
  int free_load_ports;
  int free_store_ports;
  uint64_t cycles = 0;
  bool ideal;
  int prefetch_distance=0;
  int num_prefetched_lines=1;
  FunctionalCache *fc;

  Cache(Config cache_cfg):
    size(cache_cfg.cache_size), latency(cache_cfg.cache_latency), size_of_cacheline(cache_cfg.cache_linesize), load_ports(cache_cfg.cache_load_ports), store_ports(cache_cfg.cache_store_ports), ideal(cache_cfg.ideal_cache),  prefetch_distance(cache_cfg.prefetch_distance), num_prefetched_lines(cache_cfg.num_prefetched_lines) {
    fc = new FunctionalCache(cache_cfg.cache_size, cache_cfg.cache_assoc, cache_cfg.cache_linesize, cache_cfg.cache_by_signature, cache_cfg.partition_ratio, cache_cfg.perfect_llama, cache_cfg.eviction_policy, cache_cfg.cache_by_temperature, cache_cfg.node_degree_threshold);
    free_load_ports = load_ports;
    free_store_ports = store_ports;
  }

  Cache(int latency, int linesize, int load_ports, int store_ports, bool ideal, int prefetch_distance, int num_prefetched_lines, int size, int assoc, int eviction_policy, int cache_by_temperature, int node_degree_threshold):
    size(size), latency(latency), size_of_cacheline(linesize), load_ports(load_ports), store_ports(store_ports), ideal(ideal),  prefetch_distance(prefetch_distance), num_prefetched_lines(num_prefetched_lines) {
    fc = new FunctionalCache(size, assoc, linesize, 0, 2, 0, eviction_policy, cache_by_temperature, node_degree_threshold);
    free_load_ports = load_ports;
    free_store_ports = store_ports;
  }

  void evict(uint64_t addr);
  bool process();
  void execute(MemTransaction* t);
  void addTransaction(MemTransaction *t);
  void TransactionComplete(MemTransaction *t);
  bool willAcceptTransaction(MemTransaction *t);
};

#endif
