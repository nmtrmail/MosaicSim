#ifndef CACHE_H
#define CACHE_H
#include <vector>
#include <queue>
#include "FunctionalCache.h"
#include "../core/DynamicNode.h"
using namespace std;

class DRAMSimInterface;
class Cache {
public:
  DRAMSimInterface *memInterface;
  vector<DynamicNode*> to_send;
  vector<uint64_t> to_evict;
  priority_queue<Operator, vector<Operator>, OpCompare> pq;
  int ports[4]; // ports[0] = cache loads; ports[1] = cache stores; //ports[2] = mem loads; ports[3] = mem stores;
  uint64_t cycles = 0;
  int latency;
  int size_of_cacheline;
  bool ideal;
  
  FunctionalCache *fc;
  Cache(int latency, int size, int assoc, int linesize, bool ideal): 
    latency(latency), size_of_cacheline(linesize), ideal(ideal), fc(new FunctionalCache(size, assoc)) {}
  bool process();
  void execute(DynamicNode* d);
  void addTransaction(DynamicNode *d);
};

#endif