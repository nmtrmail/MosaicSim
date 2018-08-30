#include <map>
#include <set>
#include <utility>

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;
namespace apollo {
enum InstType {I_ADDSUB, I_MULT, I_DIV, I_REM, FP_ADDSUB, FP_MULT, FP_DIV, FP_REM, LOGICAL, CAST, GEP, LD, ST, TERMINATOR, PHI, INVALID};

enum EdgeType {
  Edge_Control,
  Edge_Data,
  Edge_Phi,
  Edge_Memory,
};
enum NodeType {
  Node_Instruction,
  Node_BasicBlock,
};

class Node {
public:
  const NodeType type;
  InstType itype;
  std::string name;
  Value *val;
  int id;
  int uid;
  int bb_id;
  Node(const NodeType nt, Value *v, int uid, int id, int bb_id) : type(nt), val(v), uid(uid), id(id), bb_id(bb_id) {
    if(type == Node_Instruction) {
      raw_string_ostream stream(name);
      v->print(stream);
      name = stream.str();
      Instruction *inst = cast<Instruction>(val);
      unsigned op = inst->getOpcode();
      if(op == Instruction::Add || op == Instruction::Sub)
        itype = I_ADDSUB;
      else if(op == Instruction::Mul)
        itype = I_MULT;
      else if(op == Instruction::SDiv || op == Instruction::UDiv)
        itype = I_DIV;
      else if(op == Instruction::SRem || op == Instruction::URem)
        itype = I_REM;
      else if(op == Instruction::FAdd || op == Instruction::FSub)
        itype = FP_ADDSUB;
      else if(op == Instruction::FMul)
        itype = FP_MULT;
      else if(op == Instruction::FDiv)
        itype = FP_DIV;
      else if(op == Instruction::FRem)
        itype = FP_REM;
      else if(op == Instruction::And || op == Instruction::Or || op == Instruction::Xor || op == Instruction::Shl || op == Instruction::LShr || op == Instruction::AShr 
        || op == Instruction::ICmp || op == Instruction::FCmp)
        itype = LOGICAL;
      else if(op == Instruction::Trunc || op == Instruction::ZExt || op == Instruction::SExt || op == Instruction::FPTrunc || op == Instruction::FPExt || op == Instruction::FPToUI
        || op == Instruction::FPToSI || op == Instruction::UIToFP || op == Instruction:: SIToFP || op == Instruction::IntToPtr || op == Instruction::PtrToInt || op == Instruction::BitCast|| op == Instruction::AddrSpaceCast)
        itype = CAST;
      else if(op == Instruction::GetElementPtr)
        itype = GEP;
      else if(op == Instruction::Load)
        itype = LD;
      else if(op == Instruction::Store)
        itype = ST;
      else if(isa<TerminatorInst>(val))
        itype = TERMINATOR;
      else if(op == Instruction::PHI)
        itype = PHI;
      else
        assert(false); //alloca, atomicCmpXchg, atomicRMW, Fence, Select, Call, VAArg, vector instructions
    }
    else if(type == Node_BasicBlock) {
      name = v->getName();
      itype = INVALID;
    }
  }
  ~Node() {
  }
};

class Graph {
public:
  typedef typename std::map<Node*, std::set<Node*>> EdgeMap;
  std::vector<Node*> bb_nodes;
  std::vector<Node*> i_nodes;
  std::vector<Node*> nodes;
  EdgeMap adjList;
  EdgeMap controlEdges;
  EdgeMap dataEdges;
  EdgeMap memoryEdges;
  EdgeMap phiEdges;
  int num_export_edges;
  Graph() {
    num_export_edges = 0;
  }

  void addEdge(Node *u, Node *v, EdgeType ek);
  void addNode(Node *u);
  void removeEdge(Node *u, Node *v, EdgeType ek);
};

void Graph::addNode(Node *u) {
  if(u->type == Node_Instruction)
    i_nodes.push_back(u);
  else if(u->type == Node_BasicBlock)
    bb_nodes.push_back(u);
  nodes.push_back(u);
  adjList.insert(make_pair(u, std::set<Node*>()));
}
void Graph::addEdge(Node *u, Node *v, EdgeType ek) {
  adjList[u].insert(v);
  switch(ek) {
    case Edge_Control:
      controlEdges[u].insert(v);
      break;
    case Edge_Data:
      dataEdges[u].insert(v);
      num_export_edges++;
      break;
    case Edge_Memory:
      memoryEdges[u].insert(v);
      break;
    case Edge_Phi:
      phiEdges[u].insert(v);
      num_export_edges++;
      break;
    default:
      assert(false);
      break;
  }
}


void Graph::removeEdge(Node *u, Node *v, EdgeType ek) {
  switch(ek) {
    case Edge_Control:
      controlEdges[u].erase(v);
      break;
    case Edge_Data:
      dataEdges[u].erase(v);
      num_export_edges--;
      break;
    case Edge_Memory:
      memoryEdges[u].erase(v);
      break;
    case Edge_Phi:
      phiEdges[u].erase(v);
      num_export_edges--;
      break;
    default:
      assert(false);
      break;
  }
}
}