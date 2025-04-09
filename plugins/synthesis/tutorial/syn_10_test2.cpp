 #include "libfaudes.h"
 #include <cmath>
 #include <vector>
 #include <sstream>
 #include <map>
 #include <queue>
 
 using namespace faudes;
  /**
  * Class to represent Rabin pairs as a global attribute for a Generator.
  * A Rabin pair consists of an R-set and an I-set of states, where a run
  * is accepting if for at least one pair (R,I), the run visits states in R
  * only finitely often and states in I infinitely often.
  */
 class AttributeRabinPairs : public AttributeVoid {
 public:
   AttributeRabinPairs() : AttributeVoid() {
     FAUDES_OBJCOUNT_INC("AttributeRabinPairs");
   }
   
   AttributeRabinPairs(const AttributeRabinPairs& rOther) : AttributeVoid() {
     FAUDES_OBJCOUNT_INC("AttributeRabinPairs");
     DoAssign(rOther);
   }
   virtual ~AttributeRabinPairs() {
     FAUDES_OBJCOUNT_DEC("AttributeRabinPairs");
     Clear();
   }
   
   AttributeRabinPairs& operator=(const AttributeRabinPairs& rOther) {
     DoAssign(rOther);
     return *this;
   }
   
   bool operator==(const AttributeRabinPairs& rOther) const {
     if(Size() != rOther.Size()) return false;
     
     for(Idx i = 0; i < Size(); ++i) {
       if(!(RSet(i) == rOther.RSet(i))) return false;
       if(!(ISet(i) == rOther.ISet(i))) return false;
     }
     
     return true;
   }
   
   bool operator!=(const AttributeRabinPairs& rOther) const {
     return !(*this == rOther);
   }
   
   /**
    * Add a Rabin pair of R-set and I-set
    * R-set: states visited    infinitely often
    * I-set: states visited only finitely often
    */
   void AddRabinPair(const StateSet& rRSet, const StateSet& rISet) {
     mRSets.push_back(rRSet);
     mISets.push_back(rISet);
   }
   
   Idx Size() const {
     return mRSets.size();
   }

   const StateSet& RSet(Idx idx) const {
     if(idx >= Size()) {
       std::stringstream errstr;
       errstr << "Index " << idx << " out of range";
       throw Exception("AttributeRabinPairs::RSet", errstr.str(), 48);
     }
     return mRSets[idx];
   }

   const StateSet& ISet(Idx idx) const {
     if(idx >= Size()) {
       std::stringstream errstr;
       errstr << "Index " << idx << " out of range";
       throw Exception("AttributeRabinPairs::ISet", errstr.str(), 48);
     }
     return mISets[idx];
   }
   
   void Clear() {
     mRSets.clear();
     mISets.clear();
   }
   
   virtual bool IsDefault() const {
     return Size() == 0;
   }
   
   virtual void DoAssign(const AttributeVoid& rSrc) {
     const AttributeRabinPairs* pSrc = dynamic_cast<const AttributeRabinPairs*>(&rSrc);
     
     Clear();
     
     for(Idx i = 0; i < pSrc->Size(); ++i) {
       AddRabinPair(pSrc->RSet(i), pSrc->ISet(i));
     }
   }
   
   virtual AttributeRabinPairs* New(void) const {
     return new AttributeRabinPairs();
   }

   
 protected:
   std::vector<StateSet> mRSets;
   std::vector<StateSet> mISets;

 };


 namespace faudes {

   std::map<const Generator*, AttributeRabinPairs> gRabinPairsMap;

   void SetRabinPairsAttribute(Generator& rGen, const AttributeRabinPairs& rAttr) {
     gRabinPairsMap[&rGen] = rAttr;
   }

   AttributeRabinPairs GetRabinPairsAttribute(const Generator& rGen) {
     auto it = gRabinPairsMap.find(&rGen);
     if(it != gRabinPairsMap.end()) {
       return it->second;
     }

     return AttributeRabinPairs();
   }

   void CustomDWrite(const Generator& rGen) {
     rGen.DWrite();
     auto it = gRabinPairsMap.find(&rGen);
     if(it != gRabinPairsMap.end() && !it->second.IsDefault()) {
       std::cout << "<RabinPairs>" << std::endl;
       
       for(Idx i = 0; i < it->second.Size(); ++i) {
         std::cout << "{";
         bool first = true;
         for(StateSet::Iterator sit = it->second.RSet(i).Begin(); sit != it->second.RSet(i).End(); ++sit) {
           if(!first) std::cout << ",";
           if(rGen.ExistsState(*sit) && rGen.StateName(*sit) != "") {
             std::cout << rGen.StateName(*sit);
           } else {
             std::cout << *sit;
           }
           first = false;
         }
         std::cout << "},{";
         
         first = true;
         for(StateSet::Iterator sit = it->second.ISet(i).Begin(); sit != it->second.ISet(i).End(); ++sit) {
           if(!first) std::cout << ",";
           if(rGen.ExistsState(*sit) && rGen.StateName(*sit) != "") {
             std::cout << rGen.StateName(*sit);
           } else {
             std::cout << *sit;
           }
           first = false;
         }
         std::cout << "}" << std::endl;
       }
       
       std::cout << "</RabinPairs>" << std::endl ;
     }
   }


void ApplyControlPattern(const Generator& rPlantGen, const EventSet& rControllableEvents, 
  Generator& rControlledGen) {
FD_DF("ApplyControlPattern(" << rPlantGen.Name() << ")");

EventSet alphabet = rPlantGen.Alphabet();
EventSet uncontrollableEvents = alphabet - rControllableEvents;

std::vector<Idx> contEventsVec;
for (EventSet::Iterator eit = rControllableEvents.Begin(); eit != rControllableEvents.End(); ++eit) {
contEventsVec.push_back(*eit);
}

size_t nContEvents = contEventsVec.size();
size_t patternCount = static_cast<size_t>(std::pow(2, nContEvents));

std::vector<EventSet> controlPatterns;
for (size_t i = 0; i < patternCount; ++i) {
EventSet pattern = uncontrollableEvents;  
for (size_t j = 0; j < nContEvents; ++j) {
if (i & (1 << j)) {
pattern.Insert(contEventsVec[j]); 
}
}
controlPatterns.push_back(pattern);
}


rControlledGen.Clear();

std::map<std::pair<Idx, Idx>, Idx> eventMap;
for (EventSet::Iterator eit = alphabet.Begin(); eit != alphabet.End(); ++eit) {
Idx ev = *eit;
for (size_t i = 0; i < controlPatterns.size(); ++i) {
if (controlPatterns[i].Exists(ev)) {
std::string newEvName = rPlantGen.EventName(ev) + "_G" + std::to_string(i + 1);
Idx newEvIdx = rControlledGen.InsEvent(newEvName);
eventMap[std::make_pair(ev, i)] = newEvIdx;
}
}
}

for (StateSet::Iterator sit = rPlantGen.StatesBegin(); sit != rPlantGen.StatesEnd(); ++sit) {
rControlledGen.InsState(rPlantGen.StateName(*sit));
}
for (StateSet::Iterator iit = rPlantGen.InitStatesBegin(); iit != rPlantGen.InitStatesEnd(); ++iit) {
rControlledGen.SetInitState(*iit);
}
for (StateSet::Iterator mit = rPlantGen.MarkedStatesBegin(); mit != rPlantGen.MarkedStatesEnd(); ++mit) {
rControlledGen.SetMarkedState(*mit);
}

for (TransSet::Iterator tit = rPlantGen.TransRelBegin(); tit != rPlantGen.TransRelEnd(); ++tit) {
Idx x1 = tit->X1, ev = tit->Ev, x2 = tit->X2;
for (size_t i = 0; i < controlPatterns.size(); ++i) {
if (controlPatterns[i].Exists(ev)) {
Idx newEv = eventMap[std::make_pair(ev, i)];
rControlledGen.SetTransition(x1, newEv, x2);
}
}
}

rControlledGen.Name("Controlled_" + rPlantGen.Name());

    AttributeRabinPairs plantRabinPairs = GetRabinPairsAttribute(rPlantGen);
    SetRabinPairsAttribute(rControlledGen, plantRabinPairs);
}

void ProductRabinPair(const Generator& rPlant, const Generator& rSpec, 
  const std::map<std::pair<Idx, Idx>, Idx>& rCompositionMap, 
  Generator& GC) {
std::cout << "Debug: Starting ProductRabinPair with provided composition map" << std::endl;

AttributeRabinPairs plantRabinPairs = GetRabinPairsAttribute(rPlant);
AttributeRabinPairs specRabinPairs = GetRabinPairsAttribute(rSpec);

std::cout << "Debug: Plant has " << plantRabinPairs.Size() << " RabinPairs" << std::endl;
std::cout << "Debug: Spec has " << specRabinPairs.Size() << " RabinPairs" << std::endl;

AttributeRabinPairs productRabinPairs;

if(plantRabinPairs.Size() == 0 && specRabinPairs.Size() == 0) {
std::cout << "Debug: No Rabin pairs in both generators, nothing to do." << std::endl;
return;
}

for(Idx i = 0; i < std::max(plantRabinPairs.Size(), (Idx)1); ++i) {
for(Idx j = 0; j < std::max(specRabinPairs.Size(), (Idx)1); ++j) {
StateSet newR, newI;

for(auto& entry : rCompositionMap) {
Idx plantState = entry.first.first;
Idx specState = entry.first.second;
Idx productState = entry.second;

// R = R1 × X2 ∪ X1 × R2
if((i < plantRabinPairs.Size() && plantRabinPairs.RSet(i).Exists(plantState)) ||
(j < specRabinPairs.Size() && specRabinPairs.RSet(j).Exists(specState))) {
newR.Insert(productState);
}

// I = I1 × X2 ∪ X1 × I2
if((i < plantRabinPairs.Size() && plantRabinPairs.ISet(i).Exists(plantState)) ||
(j < specRabinPairs.Size() && specRabinPairs.ISet(j).Exists(specState))) {
newI.Insert(productState);
}
}

if(!newR.Empty() || !newI.Empty()) {
std::cout << "Debug: Adding new Rabin pair with " 
<< newR.Size() << " states in R and " 
<< newI.Size() << " states in I" << std::endl;
productRabinPairs.AddRabinPair(newR, newI);
}
}
}

std::cout << "Debug: Created product with " << productRabinPairs.Size() 
<< " RabinPairs" << std::endl;
SetRabinPairsAttribute(GC, productRabinPairs);

//DEBUG
AttributeRabinPairs gcRabinPairs = GetRabinPairsAttribute(GC);
std::cout << "Debug: After setting, GC has " << gcRabinPairs.Size() 
<< " RabinPairs" << std::endl;
}

void RabinProjectNonDet(const Generator& rGen, const EventSet& rProjectAlphabet, Generator& rResGen) {
  std::cout << "Debug: Starting RabinProjectNonDet" << std::endl;
  
  if(&rResGen != &rGen) {
    rResGen.Assign(rGen);
  }
  
  AttributeRabinPairs inputRabinPairs = GetRabinPairsAttribute(rGen);
  std::cout << "Debug: Input generator has " << inputRabinPairs.Size() << " RabinPairs" << std::endl;
  

  std::stack<Idx> todod, todor; // states todo 
  StateSet doned, doner;        // states done (aka have been put to todo) 
  Idx currentstate;             // the currently processed state
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;

  std::map<Idx, StateSet> stateMapping;
  
  // initialise todo stack
  for(sit = rResGen.InitStatesBegin(); sit != rResGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit);
    
  // initialise mapping
    StateSet initialMapping;
    initialMapping.Insert(*sit);
    stateMapping[*sit] = initialMapping;
  }
  
  while(!todod.empty()) {
    FD_WPC(doned.Size() - todod.size(), rResGen.Size(), "RabinProjectNonDet(): done/size: " 
           << doned.Size() - todod.size() << " / " << rResGen.Size());
    
    currentstate = todod.top();
    todod.pop();
    std::cout << "Debug: Processing state: " << rResGen.SStr(currentstate) << std::endl;
    
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate);
    bool marked = rResGen.ExistsMarkedState(currentstate);
    while(!todor.empty()) {
      Idx reachstate = todor.top();
      todor.pop();
      std::cout << "Debug: Reach state: " << rResGen.SStr(reachstate) << std::endl;
      
      marked |= rResGen.ExistsMarkedState(reachstate);
      
      tit = rResGen.TransRelBegin(reachstate);
      tit_end = rResGen.TransRelEnd();
      for(; tit != tit_end; ++tit) {
        if(tit->X1 != reachstate) break;
        
        if(rProjectAlphabet.Exists(tit->Ev)) {
          rResGen.SetTransition(currentstate, tit->Ev, tit->X2);
          
          if(stateMapping.find(tit->X2) == stateMapping.end()) {
            StateSet newMapping;
            newMapping.Insert(tit->X2);
            stateMapping[tit->X2] = newMapping;
          }
          
          if(doned.Insert(tit->X2)) {
            std::cout << "Debug: Adding to todo stack: " << rResGen.SStr(tit->X2) << std::endl;
            todod.push(tit->X2);
          }
        } 
        else {
          stateMapping[currentstate].Insert(tit->X2);
          
          if(doner.Insert(tit->X2)) {
            todor.push(tit->X2);
          }
        }
      }
    }

    if(marked) rResGen.SetMarkedState(currentstate);
    

    tit = rResGen.TransRelBegin(currentstate);
    tit_end = rResGen.TransRelEnd();
    while(tit != tit_end) {
      if(tit->X1 != currentstate) break;
      if(!rProjectAlphabet.Exists(tit->Ev)) {
        std::cout << "Debug: Deleting transition: " << rResGen.SStr(tit->X1)
                  << "-" << rResGen.EStr(tit->Ev) << "-" << rResGen.SStr(tit->X2) << std::endl;
        rResGen.ClrTransition(tit++);
      } else {
        ++tit;
      }
    }
  } 
  
  AttributeRabinPairs outputRabinPairs;

  for(Idx i = 0; i < inputRabinPairs.Size(); ++i) {
    StateSet newR, newI;
    const StateSet& oldR = inputRabinPairs.RSet(i);
    const StateSet& oldI = inputRabinPairs.ISet(i);
    
    for(std::map<Idx, StateSet>::iterator mapIt = stateMapping.begin(); mapIt != stateMapping.end(); ++mapIt) {
      Idx newState = mapIt->first;
      const StateSet& oldStates = mapIt->second;
      
      for(StateSet::Iterator oldIt = oldStates.Begin(); oldIt != oldStates.End(); ++oldIt) {
        if(oldR.Exists(*oldIt)) {
          newR.Insert(newState);
          break;
        }
      }
      
      for(StateSet::Iterator oldIt = oldStates.Begin(); oldIt != oldStates.End(); ++oldIt) {
        if(oldI.Exists(*oldIt)) {
          newI.Insert(newState);
          break;
        }
      }
    }
    
    if(!newR.Empty() || !newI.Empty()) {
      std::cout << "Debug: Adding new Rabin pair with " 
                << newR.Size() << " states in R and " 
                << newI.Size() << " states in I" << std::endl;
      outputRabinPairs.AddRabinPair(newR, newI);
    }
  }

  SetRabinPairsAttribute(rResGen, outputRabinPairs);
  
  rResGen.RestrictAlphabet(rProjectAlphabet);
  
  rResGen.RestrictStates(doned);
  
  rResGen.Name(CollapsString("RabinProjectNonDet(" + rGen.Name() + ")"));
  
  std::cout << "Debug: RabinProjectNonDet completed with " 
            << outputRabinPairs.Size() << " RabinPairs" << std::endl;
}

void PseudoDet(const Generator& rGen, Generator& rResGen) {
  FD_DF("PseudoDet(" << rGen.Name() << ")");
  
  // get the Rabin Pairs of Input GEN
  AttributeRabinPairs inputRabinPairs = GetRabinPairsAttribute(rGen);
  std::cout << "Debug: Input generator has " << inputRabinPairs.Size() << " RabinPairs" << std::endl;
  
  rResGen.Clear();
  rResGen.Name(CollapsString("PseudoDet(" + rGen.Name() + ")"));
  
  if(rGen.InitStatesEmpty()) {
    std::cout << "Debug: Input generator has no initial states, returning empty result." << std::endl;
    return;
  }
  
  // copy alphabet
  rResGen.InjectAlphabet(rGen.Alphabet());
  
  // for safety
  const int MAX_STATES = 1000;  // max states number
  const int MAX_ITERATIONS = 10000; // max iteration 
  const int MAX_TREE_NODES = 100;  // max nodes in a tree
  int stateCounter = 0;
  int iterationCounter = 0;
  
  typedef Idx NodeIdx;
  const Idx INVALID_NODE = 0;
  
  // Label Node class
  struct TreeNode {
    StateSet stateLabel;     // S: states label
    std::set<NodeIdx> aSet;  // A-set
    std::set<NodeIdx> rSet;  // R-set
    std::vector<NodeIdx> children; // child node
    enum Color { WHITE, RED, GREEN } color;  // judge the Rabin Acceptance Condition by coloring
    
    TreeNode() : color(WHITE) {}        //initialise with white color 
    
    // compare operator
    bool operator<(const TreeNode& other) const {
      if(stateLabel != other.stateLabel) return stateLabel < other.stateLabel;
      if(aSet != other.aSet) return aSet < other.aSet;
      if(rSet != other.rSet) return rSet < other.rSet;
      if(children != other.children) return children < other.children;
      return color < other.color;
    }
    
    bool operator==(const TreeNode& other) const {
      return stateLabel == other.stateLabel && 
             aSet == other.aSet && 
             rSet == other.rSet && 
             children == other.children && 
             color == other.color;
    }
    
    // debug
    std::string ToString() const {
      std::ostringstream oss;
      oss << "Node{states=[" << stateLabel.ToString() << "], ";
      oss << "color=" << (color == WHITE ? "WHITE" : (color == RED ? "RED" : "GREEN")) << ", ";
      oss << "children=" << children.size() << ", ";
      oss << "aSet=" << aSet.size() << ", ";
      oss << "rSet=" << rSet.size() << "}";
      return oss.str();
    }
  };
  
  
  class LabeledTree {
  public:
    std::map<NodeIdx, TreeNode> nodes;
    NodeIdx rootNode;
    NodeIdx nextNodeId;
    
    LabeledTree() : rootNode(INVALID_NODE), nextNodeId(1) {}
    
    NodeIdx createNode() {
      NodeIdx id = nextNodeId++;
      nodes[id] = TreeNode();
      return id;
    }
    
    void deleteNode(NodeIdx nodeId) {
      if(nodes.find(nodeId) == nodes.end()) return;
      
      // remove this node from all the A-set and R-set
      for(auto& pair : nodes) {
        pair.second.aSet.erase(nodeId);
        pair.second.rSet.erase(nodeId);
        
        // remove from children node
        auto& children = pair.second.children;
        children.erase(std::remove(children.begin(), children.end(), nodeId), children.end());
      }
      
      // remove node itself
      nodes.erase(nodeId);
    }
    
    // debug
    std::string ToString() const {
      std::ostringstream oss;
      oss << "Tree{root=" << rootNode << ", nodes=[";
      for(auto& pair : nodes) {
        oss << pair.first << ":" << pair.second.ToString() << ", ";
      }
      oss << "]}";
      return oss.str();
    }

    bool operator<(const LabeledTree& other) const {
      if(rootNode != other.rootNode) return rootNode < other.rootNode;
      
      // compare the number of node
      if(nodes.size() != other.nodes.size()) return nodes.size() < other.nodes.size();
      
      // compare every node 
      for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
          it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
        if(it1->first != it2->first) return it1->first < it2->first;
        if(!(it1->second == it2->second)) return it1->second < it2->second;
      }
      
      return false; 
    }
    
    bool operator==(const LabeledTree& other) const {
      if(rootNode != other.rootNode) return false;
      if(nodes.size() != other.nodes.size()) return false;
      
      for(auto it1 = nodes.begin(), it2 = other.nodes.begin();
          it1 != nodes.end() && it2 != other.nodes.end(); ++it1, ++it2) {
        if(it1->first != it2->first) return false;
        if(!(it1->second == it2->second)) return false;
      }
      
      return true;
    }
  };
  
  // store all the tree which is the same as the output GEN
  std::map<Idx, LabeledTree> stateToTree;
  
  // follow the signature tree
  std::map<std::string, Idx> treeSignatureToState;
  
  // compute the signiture of tree
  std::function<std::string(const LabeledTree&)> computeTreeSignature = [](const LabeledTree& tree) {
    std::ostringstream oss;
    
    // using the recursion to handle the signature
    std::function<void(NodeIdx)> dfs = [&](NodeIdx nodeId) {
      if(tree.nodes.find(nodeId) == tree.nodes.end()) return;
      
      const TreeNode& node = tree.nodes.at(nodeId);
      oss << nodeId << ":";
      oss << node.stateLabel.ToString() << ":";
      oss << static_cast<int>(node.color) << ":";
      
      // handle the child node
      for(NodeIdx childId : node.children) {
        dfs(childId);
      }
      
      oss << ";";
    };
    
    dfs(tree.rootNode);
    return oss.str();
  };
  
  // create the initial tree
  LabeledTree initialTree;
  NodeIdx root = initialTree.createNode();
  initialTree.rootNode = root;
  
  // set the root label including all the node states
  for(StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
    initialTree.nodes[root].stateLabel.Insert(*sit);
  }
  
  // create the initial state in the output GEN
  Idx initialState = rResGen.InsInitState();
  stateToTree[initialState] = initialTree;
  
  std::string initialSig = computeTreeSignature(initialTree);
  treeSignatureToState[initialSig] = initialState;
  
  std::queue<Idx> stateQueue;
  stateQueue.push(initialState);
  stateCounter++;
  
  // handle all the states
  while(!stateQueue.empty() && stateCounter < MAX_STATES && iterationCounter < MAX_ITERATIONS) {
    iterationCounter++;
    
    Idx currentState = stateQueue.front();
    stateQueue.pop();
    
    LabeledTree currentTree = stateToTree[currentState];
    std::cout << "Debug: Processing state " << currentState << " with tree: " 
              << currentTree.ToString() << std::endl;
    
    // handle every issues
    for(EventSet::Iterator evIt = rGen.AlphabetBegin(); evIt != rGen.AlphabetEnd(); ++evIt) {
      Idx event = *evIt;
      std::cout << "Debug: Processing event " << rGen.EventName(event) << std::endl;
      
      // clone this tree
      LabeledTree newTree = currentTree;
      
      // if the tree is too big, then jump
      if(newTree.nodes.size() > MAX_TREE_NODES) {
        std::cout << "Warning: Tree size exceeds limit, skipping event." << std::endl;
        continue;
      }
      
// 9 Steps
      
      // step 1：color all the node into WHITE
      for(auto& pair : newTree.nodes) {
        pair.second.color = TreeNode::WHITE;
      }
      
      // steu 2：update the states label based on the transition
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        // create new states based on the successor states
        StateSet newLabel;
        
        // check all the transitions from this node and this events
        for(StateSet::Iterator sit = node.stateLabel.Begin(); sit != node.stateLabel.End(); ++sit) {
          Idx state = *sit;
  
          for(TransSet::Iterator tit = rGen.TransRelBegin(state, event); 
              tit != rGen.TransRelEnd(state, event); ++tit) {
            newLabel.Insert(tit->X2);
          }
        }
        
        // update the state label of node
        node.stateLabel = newLabel;
      }
      
// Step 3: Create nodes for potential acceptance violations
if(inputRabinPairs.Size() > 0) {
  // Hard limit on total children that can be created in this step
  const int MAX_TOTAL_CHILDREN = 40;
  int totalChildrenCreated = 0;
  
  for(auto& pair : newTree.nodes) {
    NodeIdx nodeId = pair.first;
    TreeNode& node = pair.second;
    
    // Break out of the entire loop if we've created too many children
    if(totalChildrenCreated >= MAX_TOTAL_CHILDREN) {
      std::cout << "Warning: Reached maximum total children limit." << std::endl;
      break;
    }
    
    // Limit per node
    const int MAX_CHILDREN_PER_NODE = 3;
    int nodeChildrenCount = 0;
    
    // For each Rabin pair
    for(Idx i = 0; i < inputRabinPairs.Size() && nodeChildrenCount < MAX_CHILDREN_PER_NODE; ++i) {
      // Check for states outside I
      StateSet outsideI;
      const StateSet& I = inputRabinPairs.ISet(i);
      
      for(StateSet::Iterator sit = node.stateLabel.Begin(); 
          sit != node.stateLabel.End() && outsideI.Size() < 5; ++sit) {
        if(!I.Exists(*sit)) {
          outsideI.Insert(*sit);
        }
      }
      
      // Only create child if we have states outside I and haven't exceeded limits
      if(!outsideI.Empty() && nodeChildrenCount < MAX_CHILDREN_PER_NODE) {
        // Check if this label is already represented in existing children
        bool alreadyRepresented = false;
        for(NodeIdx childId : node.children) {
          if(newTree.nodes.find(childId) != newTree.nodes.end()) {
            // Consider labels equivalent if they have significant overlap
            StateSet intersection = newTree.nodes[childId].stateLabel * outsideI;
            if(!intersection.Empty()) {
              alreadyRepresented = true;
              break;
            }
          }
        }
        
        if(!alreadyRepresented) {
          NodeIdx newChild = newTree.createNode();
          newTree.nodes[newChild].stateLabel = outsideI;
          newTree.nodes[newChild].color = TreeNode::RED;
          node.children.push_back(newChild);
          
          nodeChildrenCount++;
          totalChildrenCreated++;
          
          std::cout << "Debug: Created new child " << newChild 
                    << " for violations with label " << outsideI.ToString() 
                    << " (total: " << totalChildrenCreated << ")" << std::endl;
        }
      }
    }
  }
}
      
      // step4： Maintain state disjointness between sibling nodes
      for(auto& pair : newTree.nodes) {
        NodeIdx parentId = pair.first;
        TreeNode& parent = pair.second;
        
        // Child nodes are already sorted by age (assuming node ID increases with creation time)
        // For each child node, remove the state that appears in the older sibling node
        for(size_t i = 1; i < parent.children.size(); ++i) {
          NodeIdx youngerId = parent.children[i];
          
          if(newTree.nodes.find(youngerId) == newTree.nodes.end()) continue;
          
          for(size_t j = 0; j < i; ++j) {
            NodeIdx olderId = parent.children[j];
            
            if(newTree.nodes.find(olderId) == newTree.nodes.end()) continue;
            
            // remove the every states in older brother node form younger node
            for(StateSet::Iterator sit = newTree.nodes[olderId].stateLabel.Begin(); 
                sit != newTree.nodes[olderId].stateLabel.End(); ++sit) {

              newTree.nodes[youngerId].stateLabel.Erase(*sit);
            }
          }
        }
      }
      
      // Step 5：remove all the nodes with empty state label
      std::vector<NodeIdx> nodesToRemove;
      for(auto& pair : newTree.nodes) {
        if(pair.second.stateLabel.Empty()) {
          nodesToRemove.push_back(pair.first);
        }
      }
      
      for(NodeIdx nodeId : nodesToRemove) {
        newTree.deleteNode(nodeId);
      }
      
      // Step 6：judge the red break point
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        // Calculate the union of child node status labels
        StateSet unionOfChildren;
        for(NodeIdx childId : node.children) {
          if(newTree.nodes.find(childId) == newTree.nodes.end()) continue;
          
          for(StateSet::Iterator sit = newTree.nodes[childId].stateLabel.Begin(); 
              sit != newTree.nodes[childId].stateLabel.End(); ++sit) {
            unionOfChildren.Insert(*sit);
          }
        }
        
        if(node.stateLabel == unionOfChildren && !unionOfChildren.Empty()) {
          std::cout << "Debug: Red breakpoint at node " << nodeId << std::endl;
          node.color = TreeNode::RED;
          
          std::vector<NodeIdx> descendants;
          std::queue<NodeIdx> bfs;
          for(NodeIdx child : node.children) {
            if(newTree.nodes.find(child) != newTree.nodes.end()) {
              bfs.push(child);
            }
          }
          
          while(!bfs.empty()) {
            NodeIdx current = bfs.front();
            bfs.pop();
            descendants.push_back(current);
            
            if(newTree.nodes.find(current) == newTree.nodes.end()) continue;
            
            for(NodeIdx child : newTree.nodes[current].children) {
              if(newTree.nodes.find(child) != newTree.nodes.end()) {
                bfs.push(child);
              }
            }
          }
          
          for(NodeIdx descendant : descendants) {
            newTree.deleteNode(descendant);
          }
          
          node.children.clear();
          node.aSet.clear();
          node.rSet.clear();
        }
      }
      
      // Step 7：update A-set and R-set
      // remove the nodes in Step 5 and Step 6
      for(auto& pair : newTree.nodes) {
        auto& aSet = pair.second.aSet;
        auto& rSet = pair.second.rSet;
        
        std::vector<NodeIdx> toRemoveFromA, toRemoveFromR;
        
        for(NodeIdx nodeInA : aSet) {
          if(newTree.nodes.find(nodeInA) == newTree.nodes.end()) {
            toRemoveFromA.push_back(nodeInA);
          }
        }
        
        for(NodeIdx nodeInR : rSet) {
          if(newTree.nodes.find(nodeInR) == newTree.nodes.end()) {
            toRemoveFromR.push_back(nodeInR);
          }
        }
        
        for(NodeIdx nodeId : toRemoveFromA) aSet.erase(nodeId);
        for(NodeIdx nodeId : toRemoveFromR) rSet.erase(nodeId);
      }
      
      // Step 8：handle the green color
      for(auto& pair : newTree.nodes) {
        NodeIdx nodeId = pair.first;
        TreeNode& node = pair.second;
        
        if(node.aSet.empty() && node.color != TreeNode::RED) {
          std::cout << "Debug: Green coloring for node " << nodeId << std::endl;
          node.color = TreeNode::GREEN;
          node.aSet = node.rSet;
          node.rSet.clear();
        }
      }
      
      // Step 9：follow the red node
      std::set<NodeIdx> redNodes;
      for(auto& pair : newTree.nodes) {
        if(pair.second.color == TreeNode::RED) {
          redNodes.insert(pair.first);
        }
      }
      
      for(auto& pair : newTree.nodes) {
        if(pair.second.color != TreeNode::RED) {
          for(NodeIdx redNode : redNodes) {
            if(newTree.nodes.find(redNode) != newTree.nodes.end()) {
              pair.second.rSet.insert(redNode);
            }
          }
        }
      }
      
      // check if this tree is already seen before
      std::string treeSig = computeTreeSignature(newTree);
      Idx targetState;
      
      auto stateIt = treeSignatureToState.find(treeSig);
      if(stateIt != treeSignatureToState.end()) {
        // if so ,using the existed states
        targetState = stateIt->second;
        std::cout << "Debug: Found existing state " << targetState << " for tree" << std::endl;
      } else {
        // create new state for this tree
        targetState = rResGen.InsState();
        stateToTree[targetState] = newTree;
        treeSignatureToState[treeSig] = targetState;
        
    
        stateQueue.push(targetState);
        stateCounter++;
        
        std::cout << "Debug: Created new state " << targetState << " for tree" << std::endl;
        
        // set mark according to Rabin AC
        bool shouldMark = false;
        
        //
        // In this implementation, we mark those states that contain green nodes but not red nodes
        bool hasGreen = false;
        bool hasRed = false;
        
        for(auto& nodePair : newTree.nodes) {
          if(nodePair.second.color == TreeNode::GREEN) hasGreen = true;
          if(nodePair.second.color == TreeNode::RED) hasRed = true;
        }
        
        if(hasGreen && !hasRed) {
          shouldMark = true;
        }
        
        if(shouldMark) {
          rResGen.SetMarkedState(targetState);
          std::cout << "Debug: Marking state " << targetState << std::endl;
        }
      }
      
      // add the transition from current state to target state
      rResGen.SetTransition(currentState, event, targetState);
    }
  }
  
  if(stateCounter >= MAX_STATES) {
    std::cout << "Warning: Reached maximum state limit of " << MAX_STATES << std::endl;
  }
  
  if(iterationCounter >= MAX_ITERATIONS) {
    std::cout << "Warning: Reached maximum iteration limit of " << MAX_ITERATIONS << std::endl;
  }
  
  // create rabin Pairs for output GEN
  AttributeRabinPairs outputRabinPairs;
  
  // create R and I set based on the color
  StateSet globalR, globalI;
  
  for(auto& statePair : stateToTree) {
    Idx state = statePair.first;
    const LabeledTree& tree = statePair.second;
    
    bool hasRedNode = false;
    bool hasGreenNode = false;
    
    for(auto& nodePair : tree.nodes) {
      if(nodePair.second.color == TreeNode::RED) hasRedNode = true;
      if(nodePair.second.color == TreeNode::GREEN) hasGreenNode = true;
    }
    
    if(hasRedNode) globalR.Insert(state);
    if(hasGreenNode) globalI.Insert(state);
  }
  
  // add RabinPairs when R AND I are both non-empty
  if(!globalR.Empty() && !globalI.Empty()) {
    outputRabinPairs.AddRabinPair(globalR, globalI);
  }
  
  SetRabinPairsAttribute(rResGen, outputRabinPairs);
  
  std::cout << "Debug: PseudoDet completed with " 
            << stateCounter << " states and "
            << outputRabinPairs.Size() << " RabinPairs" << std::endl;
}

}
 

int main() {

  // Compose plant dynamics from two very simple machines 
  Generator tempgen, machinea, machineb;
  System cplant; 
 
  tempgen.Read("data/verysimplemachine.gen");
  tempgen.Version("1",machinea);
  tempgen.Version("2",machineb);
  Parallel(machinea,machineb,cplant);

  // Declare controllable events
  EventSet contevents;
  contevents.Insert("alpha_1");
  contevents.Insert("alpha_2");
  cplant.SetControllable(contevents);

  // Write to file
  cplant.Write("tmp_cplant12.gen");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, plant model \n";
  cplant.DWrite();
  std::cout << "################################\n";

  // Read specification 
  System specification;
  specification.Read("data/buffer2.gen");
  InvProject(specification,cplant.Alphabet()); 
  specification.Name("simple machines specification");
  
  //Declare the RabinAcceptanceCondition of Spec
  AttributeRabinPairs rabinPairs;
  StateSet R1, I1;
  R1.Insert(2);  
  I1.Insert(1);     
  rabinPairs.AddRabinPair(R1, I1);
  SetRabinPairsAttribute(specification, rabinPairs);

  std::cout << "\nDebug Write (with state names):\n";
  CustomDWrite(specification);
  // Report to console
  std::cout << "################################\n";
  std::cout << "# specification \n";
  std::cout << "################################\n";
  
  //apply control pattern on both Plant and Spec 
  System controlledPlant;
  ApplyControlPattern(cplant,contevents,controlledPlant);
  controlledPlant.Name("Controlled Plant");
  CustomDWrite(controlledPlant);

  System controlledSpec;
  ApplyControlPattern(specification,contevents,controlledSpec);
  controlledPlant.Name("Controlled Spec");
  CustomDWrite(controlledSpec);

  //Compute the Product of E_C adn G_C
  System GC;
  std::map<std::pair<Idx, Idx>, Idx> compositionMap;
  Product(controlledPlant, controlledSpec, compositionMap, GC);
  ProductRabinPair(controlledPlant, controlledSpec, compositionMap, GC);
  CustomDWrite(GC);

  //Compute the projection of Rabin Automaton G_C without Determinization
  System GO;
  EventSet obsevents;
  obsevents.Insert("alpha_1_G2");
  obsevents.Insert("alpha_1_G4");
  obsevents.Insert("alpha_2_G3");
  obsevents.Insert("alpha_2_G4");
  RabinProjectNonDet(GC,obsevents,GO);
  CustomDWrite(GO);
  
  //Determinize with PseudoDeterminization 
  System AO;
  PseudoDet(GO, AO);
  CustomDWrite(AO);
  return 0;
}