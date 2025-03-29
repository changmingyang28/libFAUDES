/** @file syn_8_ecpo.cpp
 * Tutorial: Adding Rabin pair attribute with state names in display.
 */

 #include "libfaudes.h"
 #include <cmath>
 
 using namespace faudes;
 
 // create AttributeRabinPairs class
 class AttributeRabinPairs : public AttributeVoid {
  public:
    typedef std::pair<faudes::StateSet, faudes::StateSet> RabinPair; // first=R(拒绝), second=I(接受)
    FAUDES_TYPE_DECLARATION(RabinPair, AttributeRabinPairs, AttributeVoid)

    AttributeRabinPairs(void) {}
    AttributeRabinPairs(const AttributeRabinPairs& rOther) { DoAssign(rOther); }
    virtual ~AttributeRabinPairs(void) {}

    void Clear(void) { mRabinPairs.clear(); }
    void AddRabinPair(const StateSet& rSet, const StateSet& iSet) { // R=拒绝, I=接受
      mRabinPairs.push_back(RabinPair(rSet, iSet));
    }
    const std::vector<RabinPair>& RabinPairs(void) const { return mRabinPairs; }
    size_t Size(void) const { return mRabinPairs.size(); }
    const RabinPair& At(size_t index) const {
      if (index >= mRabinPairs.size()) {
        std::stringstream err;
        err << "Index out of range: " << index;
        throw Exception("AttributeRabinPairs::At()", err.str(), 47);
      }
      return mRabinPairs[index];
    }

    void Write(TokenWriter& rTw, const std::string& rLabel = "") const {
      DoWrite(rTw, rLabel);
    }

  protected:
    void DoAssign(const AttributeRabinPairs& rSrcAttr) { mRabinPairs = rSrcAttr.mRabinPairs; }
    bool DoEqual(const AttributeRabinPairs& rOther) const { return mRabinPairs == rOther.mRabinPairs; }
    void DoWrite(TokenWriter& rTw, const std::string& rLabel = "", const Type* pContext = 0) const {
      std::string label = rLabel.empty() ? "RabinPairs" : rLabel;
      rTw.WriteBegin(label);
      for (size_t i = 0; i < mRabinPairs.size(); ++i) {
        Token pairToken;
        pairToken.SetBegin("Pair");
        pairToken.InsAttributeInteger("index", i);
        rTw.WriteBegin("RSet"); // 拒绝集在前
        mRabinPairs[i].first.Write(rTw);
        rTw.WriteEnd("RSet");
        rTw.WriteBegin("ISet"); // 接受集在后
        mRabinPairs[i].second.Write(rTw);
        rTw.WriteEnd("ISet");
        rTw.WriteEnd("Pair");
      }
      rTw.WriteEnd(label);
    }
    void DoRead(TokenReader& rTr, const std::string& rLabel = "", const Type* pContext = 0) {
      std::string label = rLabel.empty() ? "RabinPairs" : rLabel;
      Clear();
      rTr.ReadBegin(label);
      while (!rTr.Eos(label)) {
        Token pairToken;
        rTr.ReadBegin("Pair", pairToken);
        StateSet rSet;
        rTr.ReadBegin("RSet");
        rSet.Read(rTr);
        rTr.ReadEnd("RSet");
        StateSet iSet;
        rTr.ReadBegin("ISet");
        iSet.Read(rTr);
        rTr.ReadEnd("ISet");
        AddRabinPair(rSet, iSet);
        rTr.ReadEnd("Pair");
      }
      rTr.ReadEnd(label);
    }

  private:
    std::vector<RabinPair> mRabinPairs;
};

FAUDES_TYPE_IMPLEMENTATION(RabinPair, AttributeRabinPairs, AttributeVoid)
 
 // 定义 MyRabinGenerator 类
 class MyRabinGenerator : public TaGenerator<AttributeRabinPairs, AttributeVoid, AttributeVoid, AttributeVoid> {
  public:
    MyRabinGenerator(const Generator& rGen) : TaGenerator<AttributeRabinPairs, AttributeVoid, AttributeVoid, AttributeVoid>(rGen) {}
    MyRabinGenerator() : TaGenerator<AttributeRabinPairs, AttributeVoid, AttributeVoid, AttributeVoid>() {}
    MyRabinGenerator(const MyRabinGenerator& other) : TaGenerator<AttributeRabinPairs, AttributeVoid, AttributeVoid, AttributeVoid>(other) {
      mRabinAttr = other.mRabinAttr;
    }

    void SetRabinPairs(const AttributeRabinPairs& attr) {
      mRabinAttr = attr;
    }

    const AttributeRabinPairs& RabinPairs() const {
      return mRabinAttr;
    }

    void PrintRabinPair() const {
      for (size_t i = 0; i < mRabinAttr.Size(); ++i) {
        const AttributeRabinPairs::RabinPair& pair = mRabinAttr.At(i);
        std::string rStr = "{"; // 拒绝集
        std::string iStr = "{"; // 接受集

        StateSet::Iterator it = pair.first.Begin(); // R 集
        StateSet::Iterator itEnd = pair.first.End();
        while (it != itEnd) {
          rStr += StateName(*it);
          ++it;
          if (it != itEnd) rStr += " ";
        }
        rStr += "}";

        it = pair.second.Begin(); // I 集
        itEnd = pair.second.End();
        while (it != itEnd) {
          iStr += StateName(*it);
          ++it;
          if (it != itEnd) iStr += " ";
        }
        iStr += "}";

        std::cout << "Pair " << (i + 1) << ": " << rStr << "," << iStr << "\n"; // R 在前, I 在后
      }
    }

  private:
    AttributeRabinPairs mRabinAttr;
};
 
 std::vector<EventSet> buildControlPatterns(const EventSet& alphabet, const EventSet& controllableEvents) {
   FD_DF("buildControlPatterns()");
   EventSet uncontrollableEvents = alphabet - controllableEvents;
   std::vector<Idx> contEventsVec;
   for (EventSet::Iterator eit = controllableEvents.Begin(); eit != controllableEvents.End(); ++eit) {
     contEventsVec.push_back(*eit);
   }
   size_t nContEvents = contEventsVec.size();
   size_t patternCount = static_cast<size_t>(std::pow(2, nContEvents));
   std::vector<EventSet> patterns;
 
   for (size_t i = 0; i < patternCount; ++i) {
     EventSet pattern = uncontrollableEvents;
     for (size_t j = 0; j < nContEvents; ++j) {
       if (i & (1 << j)) {
         pattern.Insert(contEventsVec[j]);
       }
     }
     patterns.push_back(pattern);
   }
   return patterns;
 }
 
 void buildControlledSystem(MyRabinGenerator rPlantGen, const EventSet& rControllableEvents,
                            MyRabinGenerator& rControlledGen) {
   FD_DF("buildControlledSystem(" << rPlantGen.Name() << ")");
   rPlantGen.PrintRabinPair();
 
   EventSet alphabet = rPlantGen.Alphabet();
   std::vector<EventSet> controlPatterns = buildControlPatterns(alphabet, rControllableEvents);
 
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
   const AttributeRabinPairs& plantRabinPairs = rPlantGen.RabinPairs();
   rControlledGen.SetRabinPairs(plantRabinPairs);
 }
 
 void RabinProduct(const MyRabinGenerator& rGen1, const MyRabinGenerator& rGen2, MyRabinGenerator& rResGen) {
  FD_DF("RabinProduct(" << rGen1.Name() << "," << rGen2.Name() << ")");
  MyRabinGenerator* pResGen = &rResGen;
  if (&rResGen == &rGen1 || &rResGen == &rGen2) {
    pResGen = new MyRabinGenerator();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString(rGen1.Name() + "||" + rGen2.Name()));
  EventSet gCAlphabet = rGen1.Alphabet();
  pResGen->InjectAlphabet(gCAlphabet);
  FD_DF("RabinProduct: alphabet: " << pResGen->Alphabet().ToString());

  std::map<std::string, std::string> eventBaseMap;
  EventSet specAlphabet = rGen2.Alphabet();
  for (EventSet::Iterator eit = gCAlphabet.Begin(); eit != gCAlphabet.End(); ++eit) {
    std::string eventName = rGen1.EventName(*eit);
    size_t pos = eventName.find("_G");
    if (pos != std::string::npos) {
      eventBaseMap[eventName] = eventName.substr(0, pos);
    } else {
      eventBaseMap[eventName] = eventName;
    }
  }

  std::map<std::pair<Idx, Idx>, Idx> stateMap;
  std::stack<std::pair<Idx, Idx>> todo;

  for (StateSet::Iterator git = rGen1.InitStatesBegin(); git != rGen1.InitStatesEnd(); ++git) {
    for (StateSet::Iterator sit = rGen2.InitStatesBegin(); sit != rGen2.InitStatesEnd(); ++sit) {
      std::pair<Idx, Idx> statePair(*git, *sit);
      Idx newState = pResGen->InsInitState();
      stateMap[statePair] = newState;
      todo.push(statePair);
      pResGen->StateName(newState, rGen1.StateName(*git) + "|" + rGen2.StateName(*sit));
      FD_DF("RabinProduct: initial state (" << *git << "|" << *sit << ") -> " << newState);
    }
  }

  while (!todo.empty()) {
    std::pair<Idx, Idx> current = todo.top();
    todo.pop();
    Idx xG = current.first;
    Idx xS = current.second;
    Idx currentIdx = stateMap[current];
    FD_DF("RabinProduct: processing (" << xG << "|" << xS << ") -> " << currentIdx);

    for (TransSet::Iterator tit1 = rGen1.TransRelBegin(xG); tit1 != rGen1.TransRelEnd(xG); ++tit1) {
      std::string eventName = rGen1.EventName(tit1->Ev);
      std::string baseEvent = eventBaseMap[eventName];

      for (TransSet::Iterator tit2 = rGen2.TransRelBegin(xS); tit2 != rGen2.TransRelEnd(xS); ++tit2) {
        if (rGen2.EventName(tit2->Ev) == baseEvent) {
          Idx xGNext = tit1->X2;
          Idx xSNext = tit2->X2;
          std::pair<Idx, Idx> nextPair(xGNext, xSNext);
          Idx nextIdx;

          if (stateMap.count(nextPair) == 0) {
            nextIdx = pResGen->InsState();
            stateMap[nextPair] = nextIdx;
            todo.push(nextPair);
            pResGen->StateName(nextIdx, rGen1.StateName(xGNext) + "|" + rGen2.StateName(xSNext));
            FD_DF("RabinProduct: new state (" << xGNext << "|" << xSNext << ") -> " << nextIdx);
          } else {
            nextIdx = stateMap[nextPair];
          }
          pResGen->SetTransition(currentIdx, tit1->Ev, nextIdx);
          FD_DF("RabinProduct: add transition " << currentIdx << " -" << eventName << "-> " << nextIdx);
        }
      }
    }
  }

  for (const auto& entry : stateMap) {
    Idx xG = entry.first.first;
    Idx xS = entry.first.second;
    Idx xAC = entry.second;
    if (rGen1.ExistsMarkedState(xG) && rGen2.ExistsMarkedState(xS)) {
      pResGen->SetMarkedState(xAC);
    }
  }
  FD_DF("RabinProduct: marked states: " << pResGen->MarkedStatesToString());

  const AttributeRabinPairs& rabin1 = rGen1.RabinPairs();
  const AttributeRabinPairs& rabin2 = rGen2.RabinPairs();
  StateSet X1 = rGen1.States();
  StateSet X2 = rGen2.States();

  StateSet R1 = (rabin1.Size() > 0) ? rabin1.At(0).first : StateSet();  // R1=拒绝集
  StateSet I1 = (rabin1.Size() > 0) ? rabin1.At(0).second : StateSet(); // I1=接受集
  StateSet R2 = (rabin2.Size() > 0) ? rabin2.At(0).first : StateSet();  // R2=拒绝集
  StateSet I2 = (rabin2.Size() > 0) ? rabin2.At(0).second : StateSet(); // I2=接受集

  // 计算 R = R1 × X2 ∪ X1 × R2 (拒绝集)
  StateSet R;
  for (StateSet::Iterator r1it = R1.Begin(); r1it != R1.End(); ++r1it) {
    for (StateSet::Iterator x2it = X2.Begin(); x2it != X2.End(); ++x2it) {
      std::pair<Idx, Idx> pair(*r1it, *x2it);
      if (stateMap.count(pair)) R.Insert(stateMap[pair]);
    }
  }
  for (StateSet::Iterator x1it = X1.Begin(); x1it != X1.End(); ++x1it) {
    for (StateSet::Iterator r2it = R2.Begin(); r2it != R2.End(); ++r2it) {
      std::pair<Idx, Idx> pair(*x1it, *r2it);
      if (stateMap.count(pair)) R.Insert(stateMap[pair]);
    }
  }

  // 计算 I = I1 × X2 ∪ X1 × I2 (接受集)
  StateSet I;
  for (StateSet::Iterator i1it = I1.Begin(); i1it != I1.End(); ++i1it) {
    for (StateSet::Iterator x2it = X2.Begin(); x2it != X2.End(); ++x2it) {
      std::pair<Idx, Idx> pair(*i1it, *x2it);
      if (stateMap.count(pair)) I.Insert(stateMap[pair]);
    }
  }
  for (StateSet::Iterator x1it = X1.Begin(); x1it != X1.End(); ++x1it) {
    for (StateSet::Iterator i2it = I2.Begin(); i2it != I2.End(); ++i2it) {
      std::pair<Idx, Idx> pair(*x1it, *i2it);
      if (stateMap.count(pair)) I.Insert(stateMap[pair]);
    }
  }

  AttributeRabinPairs newRabinPairs;
  newRabinPairs.AddRabinPair(R, I); // R=拒绝, I=接受
  pResGen->SetRabinPairs(newRabinPairs);

  if (pResGen != &rResGen) {
    rResGen = *pResGen;
    delete pResGen;
  }
  FD_DF("RabinProduct(...): done");
}

void RabinProjectND(const MyRabinGenerator& rGen, const EventSet& rObsEvents, MyRabinGenerator& rResGen) {
  FD_DF("RabinProjectND(" << rGen.Name() << ", obsEvents, " << rResGen.Name() << ")");
  Generator* pResGen = &rResGen;
  if (&rResGen == &rGen) {
    pResGen = new Generator();
  }
  pResGen->Clear();
  pResGen->Name(CollapsString("ProjND(" + rGen.Name() + ")"));

  std::map<std::string, std::string> eventBaseMap;
  EventSet newAlphabet;
  EventSet epsilonEvents;
  for (EventSet::Iterator eit = rGen.Alphabet().Begin(); eit != rGen.Alphabet().End(); ++eit) {
    std::string eventName = rGen.EventName(*eit);
    size_t pos = eventName.find("_G");
    if (pos != std::string::npos) {
      std::string baseEvent = eventName.substr(0, pos);
      eventBaseMap[eventName] = baseEvent;
      if (rObsEvents.Exists(baseEvent)) {
        newAlphabet.Insert(*eit);
      } else {
        epsilonEvents.Insert(*eit);
      }
    }
  }
  pResGen->InjectAlphabet(newAlphabet);

  std::map<Idx, Idx> stateMap;
  for (StateSet::Iterator sit = rGen.StatesBegin(); sit != rGen.StatesEnd(); ++sit) {
    Idx newState = pResGen->InsState();
    stateMap[*sit] = newState;
    pResGen->StateName(newState, rGen.StateName(*sit));
  }

  auto computeEpsilonClosure = [&rGen, &epsilonEvents](Idx startState, StateSet& closure) {
    std::stack<Idx> epsilonStack;
    StateSet visited;
    epsilonStack.push(startState);
    visited.Insert(startState);
    while (!epsilonStack.empty()) {
      Idx current = epsilonStack.top();
      epsilonStack.pop();
      closure.Insert(current);
      for (TransSet::Iterator tit = rGen.TransRelBegin(current); tit != rGen.TransRelEnd(current); ++tit) {
        if (epsilonEvents.Exists(tit->Ev) && !visited.Exists(tit->X2)) {
          epsilonStack.push(tit->X2);
          visited.Insert(tit->X2);
        }
      }
    }
  };

  StateSet initClosure;
  for (StateSet::Iterator sit = rGen.InitStatesBegin(); sit != rGen.InitStatesEnd(); ++sit) {
    computeEpsilonClosure(*sit, initClosure);
  }
  for (StateSet::Iterator sit = initClosure.Begin(); sit != initClosure.End(); ++sit) {
    pResGen->SetInitState(stateMap[*sit]);
  }

  for (StateSet::Iterator mit = rGen.MarkedStatesBegin(); mit != rGen.MarkedStatesEnd(); ++mit) {
    pResGen->SetMarkedState(stateMap[*mit]);
  }

  for (StateSet::Iterator sit = rGen.StatesBegin(); sit != rGen.StatesEnd(); ++sit) {
    Idx origState = *sit;
    Idx newState = stateMap[origState];
    for (EventSet::Iterator eit = newAlphabet.Begin(); eit != newAlphabet.End(); ++eit) {
      Idx eventIdx = *eit;
      StateSet nextClosure;
      for (TransSet::Iterator tit = rGen.TransRelBegin(origState); tit != rGen.TransRelEnd(origState); ++tit) {
        if (tit->Ev == eventIdx) {
          computeEpsilonClosure(tit->X2, nextClosure);
        }
      }
      for (StateSet::Iterator nit = nextClosure.Begin(); nit != nextClosure.End(); ++nit) {
        pResGen->SetTransition(newState, eventIdx, stateMap[*nit]);
      }
    }
  }

  if (pResGen != &rResGen) {
    rResGen = *pResGen;
    delete pResGen;
  }

  const AttributeRabinPairs& plantRabinPairs = rGen.RabinPairs();
  rResGen.SetRabinPairs(plantRabinPairs); // 直接传递，保持 R=拒绝, I=接受
  FD_DF("RabinProjectND(...): done");
}

// LabeledNode 定义
struct LabeledNode {
  Idx name;              // 节点名称
  StateSet label;        // 状态标签（子集）
  StateSet a_set;        // 接受集 (A-set)
  StateSet r_set;        // 拒绝集 (R-set)
  enum Color { RED, GREEN } color; // 节点颜色（红/绿）
  std::vector<LabeledNode> children; // 子节点

  LabeledNode() : name(static_cast<Idx>(-1)), color(RED) {}
  LabeledNode(Idx n) : name(n), color(RED) {}

  bool is_valid() const {
      if (label.Empty() && children.empty()) return true;
      if (label.Empty()) return false;
      StateSet child_union;
      for (const auto& child : children) {
          if (!child.is_valid()) return false;
          child_union.InsertSet(child.label);
          StateSet temp = label;
          temp.EraseSet(child.label);
          if (temp.Size() + child.label.Size() != label.Size()) return false; // 确保子集分割正确
      }
      StateSet temp_check = child_union;
      temp_check.EraseSet(label);
      return temp_check.Empty(); // 子节点并集应等于 label
  }

  std::string to_string(const MyRabinGenerator& gen) const {
      std::ostringstream oss;
      oss << (color == GREEN ? "G" : "R") << name << "{";
      bool first = true;
      for (StateSet::Iterator it = label.Begin(); it != label.End(); ++it) {
          if (!first) oss << ",";
          oss << gen.StateName(*it);
          first = false;
      }
      oss << "}[A={";
      first = true;
      for (auto it = a_set.Begin(); it != a_set.End(); ++it) {
          if (!first) oss << ",";
          oss << gen.StateName(*it);
          first = false;
      }
      oss << "},R={";
      first = true;
      for (auto it = r_set.Begin(); it != r_set.End(); ++it) {
          if (!first) oss << ",";
          oss << gen.StateName(*it);
          first = false;
      }
      oss << "}]";
      for (const auto& child : children) {
          oss << " C:" << child.to_string(gen);
      }
      return oss.str();
  }

  bool operator==(const LabeledNode& other) const {
      return (name == other.name && label == other.label && 
              a_set == other.a_set && r_set == other.r_set && 
              color == other.color && children == other.children);
  }
};

// LabeledTree 定义
struct LabeledTree {
  std::vector<bool> names; // 名称分配表
  LabeledNode root;

  LabeledTree() : root(static_cast<Idx>(-1)) {}
  LabeledTree(size_t size) : names(size * 2, false), root(0) {}
  LabeledTree(const MyRabinGenerator& rabin) : names(rabin.Size() * 2, false) {
      root.name = 0;
      names[0] = true;
      root.label = rabin.InitStates();
      UpdateRabinSets(rabin, root); // 初始化 A-set 和 R-set
      UpdateColor(root); // 初始化颜色
  }

  bool is_valid() const { return root.is_valid(); }

  Idx next_name() {
      for (size_t i = 0; i < names.size(); ++i) {
          if (!names[i]) {
              names[i] = true;
              return static_cast<Idx>(i);
          }
      }
      throw Exception("LabeledTree::next_name", "No available names", 100);
  }

  void remove_name(Idx n) { if (n < names.size()) names[n] = false; }
  void remove_names(LabeledNode& node) {
      remove_name(node.name);
      for (auto& child : node.children) remove_names(child);
  }

  std::string to_string(const MyRabinGenerator& gen) const {
      std::ostringstream oss;
      oss << "Names: ";
      for (bool b : names) oss << (b ? "1" : "0") << " ";
      oss << "\nRoot: " << root.to_string(gen);
      return oss.str();
  }

  void UpdateRabinSets(const MyRabinGenerator& gen, LabeledNode& node) {
    node.a_set.Clear();
    node.r_set.Clear();
    const auto& rabinAttr = gen.RabinPairs();
    if (rabinAttr.Size() == 0) return;

    for (size_t i = 0; i < rabinAttr.Size(); ++i) {
      const auto& pair = rabinAttr.At(i);
      StateSet intersect_i = node.label; // I 集 (接受集)
      intersect_i.RestrictSet(pair.second);
      if (!intersect_i.Empty()) node.a_set.InsertSet(intersect_i);
      StateSet intersect_r = node.label; // R 集 (拒绝集)
      intersect_r.RestrictSet(pair.first);
      if (!intersect_r.Empty()) node.r_set.InsertSet(intersect_r);
    }
  }

  void UpdateColor(LabeledNode& node) {
    node.color = node.a_set.Empty() ? LabeledNode::RED : LabeledNode::GREEN; // 基于 I 集判断颜色
    for (auto& child : node.children) UpdateColor(child);
  }
};

// LabeledNodeHash 和 LabeledTreeHash
struct LabeledNodeHash {
  std::size_t operator()(const LabeledNode& node) const {
      std::size_t h = std::hash<Idx>()(node.name);
      for (auto it = node.label.Begin(); it != node.label.End(); ++it)
          h = h ^ (std::hash<Idx>()(*it) + 0x9e3779b9 + (h << 6) + (h >> 2));
      for (auto it = node.a_set.Begin(); it != node.a_set.End(); ++it)
          h = h ^ (std::hash<Idx>()(*it) + 0x9e3779b9 + (h << 6) + (h >> 2));
      for (auto it = node.r_set.Begin(); it != node.r_set.End(); ++it)
          h = h ^ (std::hash<Idx>()(*it) + 0x9e3779b9 + (h << 6) + (h >> 2));
      h = h ^ (std::hash<int>()(node.color) + 0x9e3779b9 + (h << 6) + (h >> 2));
      for (const auto& child : node.children)
          h = h ^ (operator()(child) + 0x9e3779b9 + (h << 6) + (h >> 2));
      return h;
  }
};

struct LabeledTreeHash {
  std::size_t operator()(const LabeledTree& tree) const {
      std::size_t h = 0;
      for (bool b : tree.names) h = h * 31 + std::hash<bool>()(b);
      h = h ^ (LabeledNodeHash()(tree.root) + 0x9e3779b9 + (h << 6) + (h >> 2));
      return h;
  }
};

// LabeledTreeEqual
struct LabeledTreeEqual {
  bool operator()(const LabeledTree& t1, const LabeledTree& t2) const {
      if (t1.names != t2.names) return false;
      std::function<bool(const LabeledNode&, const LabeledNode&)> node_eq = 
          [&](const LabeledNode& n1, const LabeledNode& n2) {
              if (n1.name != n2.name || n1.color != n2.color || n1.label != n2.label || 
                  n1.a_set != n2.a_set || n1.r_set != n2.r_set) return false;
              if (n1.children.size() != n2.children.size()) return false;
              for (size_t i = 0; i < n1.children.size(); ++i) {
                  if (!node_eq(n1.children[i], n2.children[i])) return false;
              }
              return true;
          };
      return node_eq(t1.root, t2.root);
  }
};

// 复制节点并重置颜色为红色
LabeledNode CopyAndResetColor(const LabeledNode& node) {
  LabeledNode new_node(node.name);
  new_node.label = node.label;
  new_node.a_set = node.a_set;
  new_node.r_set = node.r_set;
  new_node.color = LabeledNode::RED; // 重置为红色
  for (const auto& child : node.children)
      new_node.children.push_back(CopyAndResetColor(child));
  return new_node;
}

// 分支接受：为 A-set 非空的节点创建新子节点
void BranchAccept(const MyRabinGenerator& gen, LabeledTree& tree, LabeledNode& node) {
  for (auto& child : node.children) BranchAccept(gen, tree, child);
  if (!node.a_set.Empty()) {
      LabeledNode new_child(tree.next_name());
      new_child.label = node.a_set;
      tree.UpdateRabinSets(gen, new_child);
      new_child.color = LabeledNode::GREEN; // 新子节点初始化为绿色
      node.children.push_back(new_child);
  }
}

// 子集构造：计算后继状态
void PsetConst(const MyRabinGenerator& gen, LabeledNode& node, Idx letter, LabeledTree& tree) {
  StateSet new_label;
  for (auto sit = node.label.Begin(); sit != node.label.End(); ++sit) {
      for (auto tit = gen.TransRelBegin(*sit); tit != gen.TransRelEnd(*sit); ++tit) {
          if (tit->Ev == letter) new_label.Insert(tit->X2);
      }
  }
  node.label = new_label;
  tree.UpdateRabinSets(gen, node); // 更新 A-set 和 R-set
  tree.UpdateColor(node); // 更新颜色
  for (auto& child : node.children) PsetConst(gen, child, letter, tree);
}

// 水平合并：移除兄弟节点中的重复状态
void HorizontalMerge(LabeledNode& node, StateSet& brothers) {
  StateSet diff;
  for (auto sit = node.label.Begin(); sit != node.label.End(); ++sit) {
      if (!brothers.Exists(*sit)) diff.Insert(*sit);
  }
  node.label = diff;
  node.a_set.RestrictSet(diff);
  node.r_set.RestrictSet(diff);
  for (auto& child : node.children) HorizontalMerge(child, brothers);
  brothers.InsertSet(node.label);
}

// 移除空节点
void RemoveEmpty(LabeledTree& tree, LabeledNode& node) {
  if (node.label.Empty()) {
      tree.remove_names(node);
      node.children.clear();
      node.name = static_cast<Idx>(-1);
      return;
  }
  std::vector<LabeledNode> new_children;
  for (auto& child : node.children) {
      if (child.label.Empty()) {
          tree.remove_names(child);
      } else {
          RemoveEmpty(tree, child);
          new_children.push_back(child);
      }
  }
  node.children = new_children;
}

// 垂直合并：如果子节点并集等于父节点，则合并
void VerticalMerge(LabeledTree& tree, LabeledNode& node) {
  StateSet child_union;
  for (const auto& child : node.children) child_union.InsertSet(child.label);
  if (!child_union.Empty() && child_union.Size() == node.label.Size()) {
      for (auto& child : node.children) tree.remove_names(child);
      node.children.clear();
      node.color = LabeledNode::GREEN; // 合并后标记为绿色
  } else {
      for (auto& child : node.children) VerticalMerge(tree, child);
  }
}

// 计算转移
LabeledTree ComputeTransition(const MyRabinGenerator& gen, const LabeledTree& tree, Idx letter) {
  if (tree.root.name == static_cast<Idx>(-1)) return tree;

  LabeledTree next(gen.Size());
  next.names = tree.names;
  next.root = CopyAndResetColor(tree.root);
  BranchAccept(gen, next, next.root);
  PsetConst(gen, next.root, letter, next);
  StateSet brothers;
  HorizontalMerge(next.root, brothers);
  RemoveEmpty(next, next.root);
  VerticalMerge(next, next.root);
  return next;
}




void PseudoDet(const MyRabinGenerator& rRabinInput, MyRabinGenerator& rRabinOutput) {
  std::cout << "PseudoDet(" << rRabinInput.Name() << ") started" << std::endl;
  rRabinOutput.Clear();
  rRabinOutput.Name("PseudoDet(" + rRabinInput.Name() + ")");
  rRabinOutput.InjectAlphabet(rRabinInput.Alphabet());

  LabeledTree initialTree(rRabinInput);
  std::cout << "Initial tree: " << initialTree.to_string(rRabinInput) << std::endl;
  if (!initialTree.is_valid()) {
    throw Exception("PseudoDet", "Invalid initial tree", 101);
  }

  std::unordered_map<LabeledTree, Idx, LabeledTreeHash, LabeledTreeEqual> stateMap;
  std::stack<LabeledTree> todo;

  Idx initState = rRabinOutput.InsInitState();
  stateMap[initialTree] = initState;
  rRabinOutput.StateName(initState, "S0");
  todo.push(initialTree);

  std::unordered_map<Idx, StateSet> stateToLabelMap;
  stateToLabelMap[initState] = initialTree.root.label;

  std::vector<StateSet> rejectSets(std::max<size_t>(1, rRabinInput.RabinPairs().Size())); // R 集 (拒绝集)
  std::vector<StateSet> acceptSets(std::max<size_t>(1, rRabinInput.RabinPairs().Size())); // I 集 (接受集)

  int stateCounter = 1;
  while (!todo.empty()) {
    LabeledTree currentTree = todo.top();
    todo.pop();
    Idx currentState = stateMap[currentTree];
    std::cout << "Processing state " << rRabinOutput.StateName(currentState) 
              << ": " << currentTree.to_string(rRabinInput) << std::endl;

    for (auto eit = rRabinOutput.Alphabet().Begin(); eit != rRabinOutput.Alphabet().End(); ++eit) {
      Idx event = *eit;
      LabeledTree nextTree = ComputeTransition(rRabinInput, currentTree, event);

      std::cout << "After transition " << rRabinInput.EventName(event) 
                << ": " << nextTree.to_string(rRabinInput) << std::endl;
      if (!nextTree.is_valid()) {
        std::cout << "Invalid next tree, skipping" << std::endl;
        continue;
      }

      Idx nextState;
      auto it = stateMap.find(nextTree);
      if (it == stateMap.end()) {
        nextState = rRabinOutput.InsState();
        stateMap[nextTree] = nextState;
        rRabinOutput.StateName(nextState, "S" + std::to_string(stateCounter++));
        stateToLabelMap[nextState] = nextTree.root.label;
        todo.push(nextTree);
        std::cout << "New state " << rRabinOutput.StateName(nextState) << " added" << std::endl;
      } else {
        nextState = it->second;
        std::cout << "Existing state " << rRabinOutput.StateName(nextState) << std::endl;
      }
      rRabinOutput.SetTransition(currentState, event, nextState);

      const auto& rabinAttr = rRabinInput.RabinPairs();
      for (size_t i = 0; i < rabinAttr.Size(); ++i) {
        // 拒绝条件：从绿变红（A-set 从非空变为空），加入 R 集 (rejectSets)
        if (currentTree.root.color == LabeledNode::GREEN && 
            nextTree.root.color == LabeledNode::RED) {
          rejectSets[i].Insert(currentState);
          std::cout << "State " << rRabinOutput.StateName(currentState) 
                    << " added to rejectSet[" << i << "] (Green -> Red)" << std::endl;
        }
        // 接受条件：从红变绿（A-set 从空变为非空），加入 I 集 (acceptSets)
        if (currentTree.root.color == LabeledNode::RED && 
            nextTree.root.color == LabeledNode::GREEN) {
          acceptSets[i].Insert(currentState);
          std::cout << "State " << rRabinOutput.StateName(currentState) 
                    << " added to acceptSet[" << i << "] (Red -> Green)" << std::endl;
        }
      }
    }
  }

  AttributeRabinPairs rabinAttr;
  const auto& inputRabinAttr = rRabinInput.RabinPairs();
  if (inputRabinAttr.Size() == 0) {
    std::cout << "No Rabin pairs in input, output will have no Rabin pairs." << std::endl;
  }
  for (size_t i = 0; i < inputRabinAttr.Size(); ++i) {
    const auto& pair = inputRabinAttr.At(i);
    StateSet inputRejectSet = pair.first;  // R 集 (拒绝集)
    StateSet inputAcceptSet = pair.second; // I 集 (接受集)

    StateSet rejectSet, acceptSet;
    for (const auto& entry : stateToLabelMap) {
      Idx state = entry.first;
      const StateSet& label = entry.second;

      // 如果 label 与输入的拒绝集 (R) 有交集，则加入 rejectSet
      StateSet intersectReject = label;
      intersectReject.RestrictSet(inputRejectSet);
      if (!intersectReject.Empty()) {
        rejectSet.Insert(state);
      }

      // 如果 label 与输入的接受集 (I) 有交集，则加入 acceptSet
      StateSet intersectAccept = label;
      intersectAccept.RestrictSet(inputAcceptSet);
      if (!intersectAccept.Empty()) {
        acceptSet.Insert(state);
      }
    }

    rejectSet.InsertSet(rejectSets[i]); // R 集
    acceptSet.InsertSet(acceptSets[i]); // I 集

    if (!acceptSet.Empty() || !rejectSet.Empty()) {
      rabinAttr.AddRabinPair(rejectSet, acceptSet); // R=拒绝, I=接受
      std::cout << "Inserted Rabin pair " << i << ": R={";
      for (auto it = rejectSet.Begin(); it != rejectSet.End(); ++it)
        std::cout << rRabinOutput.StateName(*it) << " ";
      std::cout << "}, I={";
      for (auto it = acceptSet.Begin(); it != acceptSet.End(); ++it)
        std::cout << rRabinOutput.StateName(*it) << " ";
      std::cout << "}" << std::endl;
    }
  }
  rRabinOutput.SetRabinPairs(rabinAttr);

  std::cout << "PseudoDet done: " << rRabinOutput.Size() << " states" << std::endl;
}

 
 int main() {
   System Plant;
   Plant.Read("data/safratest_Plant2.gen");
 
   EventSet contevents;
   contevents.Insert("a");
   Plant.SetControllable(contevents);
 
   MyRabinGenerator controlledPlant;
   buildControlledSystem(Plant, contevents, controlledPlant);
   controlledPlant.Write("tmp_controlled_plant.gen");
   controlledPlant.Name("Controlled Plant");
   std::cout << "################################\n";
   std::cout << "# Controlled Plant\n";
   controlledPlant.DWrite();
   controlledPlant.PrintRabinPair();
   std::cout << "################################\n";
 
   System Spec;
   Spec.Read("data/safratest_Spec2.gen");
 
   AttributeRabinPairs specRP;
   StateSet speclSet1, specuSet1;
   speclSet1.Insert(2);
   specuSet1.Insert(1);
   specRP.AddRabinPair(speclSet1, specuSet1);
 
   MyRabinGenerator cSpec(Spec);
   cSpec.SetRabinPairs(specRP);
   std::cout << "################################\n";
   std::cout << "# Specification\n";
   cSpec.DWrite();
   cSpec.PrintRabinPair();
   std::cout << "################################\n";
 
   MyRabinGenerator AC;
   RabinProduct(controlledPlant, cSpec, AC);
   std::cout << "################################\n";
   std::cout << "# Rabin Product Result\n";
   AC.DWrite();
   AC.PrintRabinPair();
   std::cout << "################################\n";

   EventSet obsevents;
   obsevents.Insert("a");
    
   //project into Non-Deterministic 
   MyRabinGenerator AO;
   RabinProjectND(AC,obsevents,AO);
   AO.Name("A_o");
   AO.DWrite();
   AO.PrintRabinPair();

   MyRabinGenerator AD;
   PseudoDet(AO,AD);
   AD.Name("A_d");
   AD.DWrite();
   AD.PrintRabinPair();
 
   return 0;
 }