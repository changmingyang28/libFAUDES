#include "safra.h"

namespace faudes {

// RabinGenerator member function implementations
RabinGenerator::RabinGenerator() : vGenerator() {}
RabinGenerator::RabinGenerator(const std::string& rFileName) : vGenerator() { Read(rFileName); }
RabinGenerator::RabinGenerator(const RabinGenerator& rOther) : vGenerator(rOther), mRabinPairs(rOther.mRabinPairs) {}
RabinGenerator::~RabinGenerator() {}

void RabinGenerator::ClearRabinPairs() { mRabinPairs.clear(); }
void RabinGenerator::InsRabinPair(const StateSet& rRSet, const StateSet& rISet) {
    mRabinPairs.push_back(std::make_pair(rRSet, rISet));
}
const std::vector<RabinGenerator::RabinPair>& RabinGenerator::RabinPairs() const { return mRabinPairs; }

void RabinGenerator::Write(const std::string& rFileName) const { TokenWriter tw(rFileName); DoWrite(tw); }
void RabinGenerator::Read(const std::string& rFileName) { TokenReader tr(rFileName); DoRead(tr); }
void RabinGenerator::DWrite() const { TokenWriter tw(TokenWriter::Stdout); DoDWrite(tw, ""); }

void RabinGenerator::DoWrite(TokenWriter& rTw, const std::string& rLabel) const {
    vGenerator::DoWrite(rTw, rLabel);
    rTw.WriteBegin("RabinPairs");
    if (mRabinPairs.empty()) {
        rTw.WriteString("No Rabin pairs generated");
        std::cout << "Writing: No Rabin pairs generated" << std::endl;
    } else {
        std::ostringstream oss;
        for (size_t i = 0; i < mRabinPairs.size(); ++i) {
            const auto& pair = mRabinPairs[i];
            if (i > 0) oss << ", ";

            oss << "[F" << (i + 1) << "={";
            bool firstR = true;
            for (StateSet::Iterator it = pair.first.Begin(); it != pair.first.End(); ++it) {
                if (!firstR) oss << " ";
                oss << StateName(*it);
                firstR = false;
            }
            oss << "},E" << (i + 1) << "={";
            bool firstI = true;
            for (StateSet::Iterator it = pair.second.Begin(); it != pair.second.End(); ++it) {
                if (!firstI) oss << " ";
                oss << StateName(*it);
                firstI = false;
            }
            oss << "}]";
        }
        rTw.WriteString(oss.str());
    }
    rTw.WriteEnd("RabinPairs");
}

void RabinGenerator::DoRead(TokenReader& rTr, const std::string& rLabel) {
    vGenerator::DoRead(rTr, rLabel);
    mRabinPairs.clear();
    Token token;
    while (rTr.Peek(token)) {
        if (token.Type() == Token::Begin && token.StringValue() == "RabinPairs") {
            rTr.ReadBegin("RabinPairs");
            while (!rTr.Eos("RabinPairs")) {
                rTr.Get(token);
                if (token.StringValue() == "No Rabin pairs generated") break;

                std::string rabinStr = token.StringValue();
                std::vector<StateSet> rejectSets, acceptSets;

                size_t pos = 0;
                while (pos < rabinStr.length()) {
                    if (rabinStr[pos] == '[') {
                        pos++; // Skip '['
                        StateSet rSet, iSet;

                        pos += 3; // Skip "F#="
                        size_t rejectStart = pos;
                        while (rabinStr[pos] != '}') pos++;
                        std::string rejectStr = rabinStr.substr(rejectStart, pos - rejectStart);
                        std::istringstream issR(rejectStr);
                        std::string state;
                        while (issR >> state) rSet.Insert(StateIndex(state));
                        pos += 2; // Skip "},"

                        pos += 3; // Skip "E#="
                        size_t acceptStart = pos;
                        while (rabinStr[pos] != '}') pos++;
                        std::string acceptStr = rabinStr.substr(acceptStart, pos - acceptStart);
                        std::istringstream issI(acceptStr);
                        while (issI >> state) iSet.Insert(StateIndex(state));
                        pos += 2; // Skip "}]"

                        rejectSets.push_back(rSet);
                        acceptSets.push_back(iSet);
                    } else {
                        pos++;
                    }
                }

                for (size_t i = 0; i < rejectSets.size(); ++i) {
                    InsRabinPair(rejectSets[i], acceptSets[i]);
                }
            }
            rTr.ReadEnd("RabinPairs");
            break;
        } else {
            rTr.Get(token);
        }
    }
}

void RabinGenerator::DoDWrite(TokenWriter& rTw, const std::string& rLabel) const {
    DoWrite(rTw, rLabel); // Reuse DoWrite for debug output
}

// SafraNode member function implementations
SafraNode::SafraNode() : name(static_cast<Idx>(-1)), mark(false) {}
SafraNode::SafraNode(Idx n) : name(n), mark(false) {}

bool SafraNode::is_valid() const {
    if (label.Empty() && children.empty()) return true;
    if (label.Empty()) return false;
    StateSet child_union;
    for (const auto& child : children) {
        if (!child.is_valid()) return false;
        child_union.InsertSet(child.label);
        StateSet temp = label;
        temp.EraseSet(child.label);
        if (temp.Size() + child.label.Size() != label.Size()) return false;
    }
    StateSet temp_check = child_union;
    temp_check.EraseSet(label);
    return temp_check.Empty();
}

int SafraNode::find_name(Idx n) const {
    if (n == name && mark) return 1;
    if (n == name) return 0;
    for (const auto& child : children) {
        int status = child.find_name(n);
        if (status >= 0) return status;
    }
    return -1;
}

std::string SafraNode::to_string(const vGenerator& gen) const {
    std::ostringstream oss;
    if (mark) oss << "!";
    oss << name << "{";
    bool first = true;
    for (StateSet::Iterator it = label.Begin(); it != label.End(); ++it) {
        if (!first) oss << ",";
        oss << gen.StateName(*it);
        first = false;
    }
    oss << "}";
    for (const auto& child : children) {
        oss << " C:" << child.to_string(gen);
    }
    return oss.str();
}

bool SafraNode::operator==(const SafraNode& other) const {
    return (name == other.name && label == other.label && mark == other.mark && children == other.children);
}

// SafraTree member function implementations
SafraTree::SafraTree() : root(static_cast<Idx>(-1)) {}
SafraTree::SafraTree(size_t buechi_size) : names(buechi_size * 2, false), root(0) {}
SafraTree::SafraTree(const vGenerator& buechi) : names(buechi.Size() * 2, false) {
    root.name = 0;
    names[0] = true;
    root.mark = false;
    root.label = buechi.InitStates();
}

bool SafraTree::is_valid() const { return root.is_valid(); }

Idx SafraTree::next_name() {
    for (size_t i = 0; i < names.size(); ++i) {
        if (!names[i]) {
            names[i] = true;
            return static_cast<Idx>(i);
        }
    }
    throw Exception("SafraTree::next_name", "No available names", 100);
}

void SafraTree::remove_name(Idx n) {
    if (n < names.size()) names[n] = false;
}

void SafraTree::remove_names(SafraNode& node) {
    remove_name(node.name);
    for (auto& child : node.children) remove_names(child);
}

std::string SafraTree::to_string(const vGenerator& gen) const {
    std::ostringstream oss;
    oss << "Names: ";
    for (bool b : names) oss << (b ? "1" : "0") << " ";
    oss << "\nRoot: " << root.to_string(gen);
    return oss.str();
}

// SafraNodeHash implementation
std::size_t SafraNodeHash::operator()(const SafraNode& node) const {
    std::size_t h = std::hash<Idx>()(node.name);
    for (StateSet::Iterator it = node.label.Begin(); it != node.label.End(); ++it) {
        h = h ^ (std::hash<Idx>()(*it) + 0x9e3779b9 + (h << 6) + (h >> 2));
    }
    h = h ^ (std::hash<bool>()(node.mark) + 0x9e3779b9 + (h << 6) + (h >> 2));
    for (const auto& child : node.children) {
        h = h ^ (operator()(child) + 0x9e3779b9 + (h << 6) + (h >> 2));
    }
    return h;
}

// SafraTreeHash implementation
std::size_t SafraTreeHash::operator()(const SafraTree& tree) const {
    std::size_t h = 0;
    for (bool b : tree.names) h = h * 31 + std::hash<bool>()(b);
    h = h ^ (SafraNodeHash()(tree.root) + 0x9e3779b9 + (h << 6) + (h >> 2));
    return h;
}

// SafraTreeEqual implementation
bool SafraTreeEqual::operator()(const SafraTree& t1, const SafraTree& t2) const {
    if (t1.names != t2.names) return false;
    std::function<bool(const SafraNode&, const SafraNode&)> node_eq = 
        [&](const SafraNode& n1, const SafraNode& n2) {
            if (n1.name != n2.name || n1.mark != n2.mark || n1.label != n2.label) return false;
            if (n1.children.size() != n2.children.size()) return false;
            for (size_t i = 0; i < n1.children.size(); ++i) {
                if (!node_eq(n1.children[i], n2.children[i])) return false;
            }
            return true;
        };
    return node_eq(t1.root, t2.root);
}

// Helper function implementations
SafraNode CopyAndRemoveMarks(const SafraNode& node) {
    SafraNode new_node(node.name);
    new_node.label = node.label;
    new_node.mark = false;
    for (const auto& child : node.children) {
        new_node.children.push_back(CopyAndRemoveMarks(child));
    }
    return new_node;
}

void BranchAccept(const vGenerator& gen, SafraTree& tree, SafraNode& node) {
    for (auto& child : node.children) {
        BranchAccept(gen, tree, child);
    }
    StateSet common;
    for (StateSet::Iterator sit = node.label.Begin(); sit != node.label.End(); ++sit) {
        if (gen.ExistsMarkedState(*sit)) {
            common.Insert(*sit);
        }
    }
    if (!common.Empty()) {
        SafraNode new_child(tree.next_name());
        new_child.label = common;
        node.children.push_back(new_child);
    }
}

void PsetConst(const vGenerator& gen, SafraNode& node, Idx letter) {
    StateSet new_label;
    for (StateSet::Iterator sit = node.label.Begin(); sit != node.label.End(); ++sit) {
        for (TransSet::Iterator tit = gen.TransRelBegin(*sit); tit != gen.TransRelEnd(*sit); ++tit) {
            if (tit->Ev == letter) {
                new_label.Insert(tit->X2);
            }
        }
    }
    node.label = new_label;
    for (auto& child : node.children) {
        PsetConst(gen, child, letter);
    }
}

void HorizontalMerge(SafraNode& node, StateSet& brothers) {
    StateSet diff;
    for (StateSet::Iterator sit = node.label.Begin(); sit != node.label.End(); ++sit) {
        if (!brothers.Exists(*sit)) {
            diff.Insert(*sit);
        }
    }
    node.label = diff;
    for (auto& child : node.children) {
        HorizontalMerge(child, brothers);
    }
    brothers.InsertSet(node.label);
}

void RemoveEmpty(SafraTree& tree, SafraNode& node) {
    if (node.label.Empty()) {
        tree.remove_names(node);
        node.children.clear();
        node.name = static_cast<Idx>(-1);
        return;
    }
    std::vector<SafraNode> new_children;
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

void VerticalMerge(SafraTree& tree, SafraNode& node) {
    StateSet child_union;
    for (const auto& child : node.children) {
        child_union.InsertSet(child.label);
    }
    if (!child_union.Empty() && child_union.Size() == node.label.Size()) {
        for (auto& child : node.children) {
            tree.remove_names(child);
        }
        node.children.clear();
        node.mark = true;
    } else {
        for (auto& child : node.children) {
            VerticalMerge(tree, child);
        }
    }
}

SafraTree ComputeTransition(const vGenerator& gen, const SafraTree& tree, Idx letter) {
    if (tree.root.name == static_cast<Idx>(-1)) return tree;

    SafraTree next(gen.Size());
    next.names = tree.names;
    next.root = CopyAndRemoveMarks(tree.root);
    BranchAccept(gen, next, next.root);
    PsetConst(gen, next.root, letter);
    StateSet brothers;
    HorizontalMerge(next.root, brothers);
    RemoveEmpty(next, next.root);
    VerticalMerge(next, next.root);
    return next;
}

void SafraDet(const Generator& rBuechiGen, RabinGenerator& rRabinGen) {
    std::cout << "SafraDet(" << rBuechiGen.Name() << ") started" << std::endl;
    rRabinGen.Clear();
    rRabinGen.Name("SafraDet(" + rBuechiGen.Name() + ")");
    rRabinGen.InjectAlphabet(rBuechiGen.Alphabet());

    SafraTree initialTree(rBuechiGen);
    std::cout << "Initial tree: " << initialTree.to_string(rBuechiGen) << std::endl;
    if (!initialTree.is_valid()) {
        throw Exception("SafraDet", "Invalid initial tree", 101);
    }

    std::unordered_map<SafraTree, Idx, SafraTreeHash, SafraTreeEqual> stateMap;
    std::stack<SafraTree> todo;

    Idx initState = rRabinGen.InsInitState();
    stateMap[initialTree] = initState;
    rRabinGen.StateName(initState, "S0");
    todo.push(initialTree);

    size_t maxName = rBuechiGen.Size() * 2 - 1;
    std::vector<StateSet> acceptSets(maxName + 1);

    int stateCounter = 1;
    while (!todo.empty()) {
        SafraTree currentTree = todo.top();
        todo.pop();
        Idx currentState = stateMap[currentTree];
        std::cout << "Processing state " << rRabinGen.StateName(currentState) << ": " << currentTree.to_string(rBuechiGen) << std::endl;

        for (EventSet::Iterator eit = rBuechiGen.Alphabet().Begin(); eit != rBuechiGen.Alphabet().End(); ++eit) {
            Idx event = *eit;
            SafraTree nextTree = ComputeTransition(rBuechiGen, currentTree, event);

            std::cout << "After transition " << rBuechiGen.EventName(event) << ": " << nextTree.to_string(rBuechiGen) << std::endl;
            if (!nextTree.is_valid()) {
                std::cout << "Invalid next tree, skipping" << std::endl;
                continue;
            }

            Idx nextState;
            auto it = stateMap.find(nextTree);
            if (it == stateMap.end()) {
                nextState = rRabinGen.InsState();
                stateMap[nextTree] = nextState;
                rRabinGen.StateName(nextState, "S" + std::to_string(stateCounter++));
                todo.push(nextTree);
                std::cout << "New state " << rRabinGen.StateName(nextState) << " added" << std::endl;
            } else {
                nextState = it->second;
                std::cout << "Existing state " << rRabinGen.StateName(nextState) << std::endl;
            }
            rRabinGen.SetTransition(currentState, event, nextState);

            for (Idx n = 0; n <= maxName; ++n) {
                int currentStatus = currentTree.root.find_name(n);
                int nextStatus = nextTree.root.find_name(n);
                // std::cout << "Name " << n << ": currentStatus=" << currentStatus 
                //        << " (marked=" << (currentStatus == 1) << "), nextStatus=" << nextStatus << std::endl;
                if (currentStatus == 1 && (nextStatus == 0 || nextStatus == -1)) {
                    acceptSets[n].Insert(currentState);
                    std::cout << "State " << rRabinGen.StateName(currentState) << " added to acceptSet[" << n 
                              << "] (marked in current, unmarked or absent in next)" << std::endl;
                }
            }
        }
    }

    StateSet allStates = rRabinGen.States();
    std::cout << "All states: " << allStates.ToString() << std::endl;
    for (Idx n = 0; n <= maxName; ++n) {
        StateSet& acceptSet = acceptSets[n];
        StateSet rejectSet = allStates;
        rejectSet.EraseSet(acceptSet);
        // std::cout << "Name " << n << ": AcceptSet=" << acceptSet.ToString() 
        //           << ", RejectSet=" << rejectSet.ToString() << std::endl;
        if (!acceptSet.Empty()) {
            rRabinGen.InsRabinPair(rejectSet, acceptSet);
            std::cout << "Inserted Rabin pair for name " << n << std::endl;
        }
    }

    std::cout << "SafraDet done: " << rRabinGen.Size() << " states" << std::endl;
}

} // namespace faudes
