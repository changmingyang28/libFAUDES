 #include "libfaudes.h"
 #include <cmath>
 #include <vector>
 #include <sstream>
 #include <map>
 
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

// 1. 获取 RabinPairs 属性
AttributeRabinPairs plantRabinPairs = GetRabinPairsAttribute(rPlant);
AttributeRabinPairs specRabinPairs = GetRabinPairsAttribute(rSpec);

std::cout << "Debug: Plant has " << plantRabinPairs.Size() << " RabinPairs" << std::endl;
std::cout << "Debug: Spec has " << specRabinPairs.Size() << " RabinPairs" << std::endl;

// 创建新的 RabinPairs 属性
AttributeRabinPairs productRabinPairs;

// 如果两者都没有 Rabin 对，则直接返回
if(plantRabinPairs.Size() == 0 && specRabinPairs.Size() == 0) {
std::cout << "Debug: No Rabin pairs in both generators, nothing to do." << std::endl;
return;
}

// 2. 为每一对 Rabin 对创建乘积
for(Idx i = 0; i < std::max(plantRabinPairs.Size(), (Idx)1); ++i) {
for(Idx j = 0; j < std::max(specRabinPairs.Size(), (Idx)1); ++j) {
StateSet newR, newI;

// 遍历组合映射
for(auto& entry : rCompositionMap) {
Idx plantState = entry.first.first;
Idx specState = entry.first.second;
Idx productState = entry.second;

// 按 Rabin 对定义检查是否属于 R 集合：R = R1 × X2 ∪ X1 × R2
if((i < plantRabinPairs.Size() && plantRabinPairs.RSet(i).Exists(plantState)) ||
(j < specRabinPairs.Size() && specRabinPairs.RSet(j).Exists(specState))) {
newR.Insert(productState);
}

// 按 Rabin 对定义检查是否属于 I 集合：I = I1 × X2 ∪ X1 × I2
if((i < plantRabinPairs.Size() && plantRabinPairs.ISet(i).Exists(plantState)) ||
(j < specRabinPairs.Size() && specRabinPairs.ISet(j).Exists(specState))) {
newI.Insert(productState);
}
}

// 如果 R 或 I 非空，添加新的 Rabin 对
if(!newR.Empty() || !newI.Empty()) {
std::cout << "Debug: Adding new Rabin pair with " 
<< newR.Size() << " states in R and " 
<< newI.Size() << " states in I" << std::endl;
productRabinPairs.AddRabinPair(newR, newI);
}
}
}

// 3. 设置产品 RabinPairs 属性
std::cout << "Debug: Created product with " << productRabinPairs.Size() 
<< " RabinPairs" << std::endl;
SetRabinPairsAttribute(GC, productRabinPairs);

// 验证设置是否成功
AttributeRabinPairs gcRabinPairs = GetRabinPairsAttribute(GC);
std::cout << "Debug: After setting, GC has " << gcRabinPairs.Size() 
<< " RabinPairs" << std::endl;
}

void RabinProjectNonDet(const Generator& rGen, const EventSet& rProjectAlphabet, Generator& rResGen) {
  std::cout << "Debug: Starting RabinProjectNonDet" << std::endl;
  
  // 如果 rResGen 不是 rGen，则先复制
  if(&rResGen != &rGen) {
    rResGen.Assign(rGen);
  }
  
  // 获取输入生成器的 RabinPairs 属性
  AttributeRabinPairs inputRabinPairs = GetRabinPairsAttribute(rGen);
  std::cout << "Debug: Input generator has " << inputRabinPairs.Size() << " RabinPairs" << std::endl;
  
  // HELPERS:
  std::stack<Idx> todod, todor; // states todo 
  StateSet doned, doner;        // states done (aka have been put to todo) 
  Idx currentstate;             // the currently processed state
  StateSet::Iterator sit;
  StateSet::Iterator sit_end;
  TransSet::Iterator tit;
  TransSet::Iterator tit_end;
  
  // 构建状态映射，用于追踪投影前后的状态对应关系
  std::map<Idx, StateSet> stateMapping;
  
  // 初始化 todo 栈，添加初始状态
  for(sit = rResGen.InitStatesBegin(); sit != rResGen.InitStatesEnd(); ++sit) {
    todod.push(*sit);
    doned.Insert(*sit);
    
    // 初始化状态映射
    StateSet initialMapping;
    initialMapping.Insert(*sit);
    stateMapping[*sit] = initialMapping;
  }
  
  // 处理主 todo 栈
  while(!todod.empty()) {
    // 进度回调
    FD_WPC(doned.Size() - todod.size(), rResGen.Size(), "RabinProjectNonDet(): done/size: " 
           << doned.Size() - todod.size() << " / " << rResGen.Size());
    
    // 获取栈顶元素
    currentstate = todod.top();
    todod.pop();
    std::cout << "Debug: Processing state: " << rResGen.SStr(currentstate) << std::endl;
    
    // 本地可达性分析
    todor.push(currentstate);
    doner.Clear();
    doner.Insert(currentstate);
    bool marked = rResGen.ExistsMarkedState(currentstate);
    while(!todor.empty()) {
      Idx reachstate = todor.top();
      todor.pop();
      std::cout << "Debug: Reach state: " << rResGen.SStr(reachstate) << std::endl;
      
      // 检查标记状态
      marked |= rResGen.ExistsMarkedState(reachstate);
      
      // 遍历后继状态
      tit = rResGen.TransRelBegin(reachstate);
      tit_end = rResGen.TransRelEnd();
      for(; tit != tit_end; ++tit) {
        if(tit->X1 != reachstate) break;
        
        // 处理可见事件：添加新转移，将目标状态添加到主 todo 栈
        if(rProjectAlphabet.Exists(tit->Ev)) {
          rResGen.SetTransition(currentstate, tit->Ev, tit->X2);
          
          // 更新状态映射：将 reachstate 可达的状态添加到 currentstate 的映射中
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
        // 处理不可见事件：添加到本地可达分析的 todo 栈
        else {
          // 更新映射：将 tit->X2 添加到 currentstate 的映射中
          stateMapping[currentstate].Insert(tit->X2);
          
          if(doner.Insert(tit->X2)) {
            todor.push(tit->X2);
          }
        }
      }
    } // 本地可达分析循环结束
    
    // 设置标记状态
    if(marked) rResGen.SetMarkedState(currentstate);
    
    // 删除所有从当前状态出发的不可见事件转移
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
  } // 主循环结束
  
  // 创建新的 Rabin 对
  AttributeRabinPairs outputRabinPairs;
  
  // 处理输入生成器的每个 Rabin 对
  for(Idx i = 0; i < inputRabinPairs.Size(); ++i) {
    StateSet newR, newI;
    const StateSet& oldR = inputRabinPairs.RSet(i);
    const StateSet& oldI = inputRabinPairs.ISet(i);
    
    // 针对保留下来的每个状态，检查它是否由原来的 R 和 I 集合中的状态可达
    for(std::map<Idx, StateSet>::iterator mapIt = stateMapping.begin(); mapIt != stateMapping.end(); ++mapIt) {
      Idx newState = mapIt->first;
      const StateSet& oldStates = mapIt->second;
      
      // 检查是否有任何一个原始状态在 R 集合中
      for(StateSet::Iterator oldIt = oldStates.Begin(); oldIt != oldStates.End(); ++oldIt) {
        if(oldR.Exists(*oldIt)) {
          newR.Insert(newState);
          break;
        }
      }
      
      // 检查是否有任何一个原始状态在 I 集合中
      for(StateSet::Iterator oldIt = oldStates.Begin(); oldIt != oldStates.End(); ++oldIt) {
        if(oldI.Exists(*oldIt)) {
          newI.Insert(newState);
          break;
        }
      }
    }
    
    // 添加新的 Rabin 对
    if(!newR.Empty() || !newI.Empty()) {
      std::cout << "Debug: Adding new Rabin pair with " 
                << newR.Size() << " states in R and " 
                << newI.Size() << " states in I" << std::endl;
      outputRabinPairs.AddRabinPair(newR, newI);
    }
  }
  
  // 设置结果生成器的 Rabin 对属性
  SetRabinPairsAttribute(rResGen, outputRabinPairs);
  
  // 限制字母表，保留属性
  rResGen.RestrictAlphabet(rProjectAlphabet);
  
  // 限制状态集，保留可达状态
  rResGen.RestrictStates(doned);
  
  // 设置名称
  rResGen.Name(CollapsString("RabinProjectNonDet(" + rGen.Name() + ")"));
  
  std::cout << "Debug: RabinProjectNonDet completed with " 
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
  std::cout << "# tutorial, specification \n";
  std::cout << "################################\n";

  System controlledPlant;
  ApplyControlPattern(cplant,contevents,controlledPlant);
  controlledPlant.Name("Controlled Plant");
  CustomDWrite(controlledPlant);

  System controlledSpec;
  ApplyControlPattern(specification,contevents,controlledSpec);
  controlledPlant.Name("Controlled Spec");
  CustomDWrite(controlledSpec);

  
  System GC;
  std::map<std::pair<Idx, Idx>, Idx> compositionMap;
  Product(controlledPlant, controlledSpec, compositionMap, GC);
  ProductRabinPair(controlledPlant, controlledSpec, compositionMap, GC);
  CustomDWrite(GC);

  System GO;
  EventSet obsevents;
  obsevents.Insert("alpha_1_G2");
  obsevents.Insert("alpha_1_G4");
  obsevents.Insert("alpha_2_G3");
  obsevents.Insert("alpha_2_G4");
  RabinProjectNonDet(GC,obsevents,GO);
  CustomDWrite(GO);
  
  return 0;
}