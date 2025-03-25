#include "syn_rabinproduct.h"

using namespace faudes;

namespace faudes {

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

void buildControlledSystem(const Generator& rPlantGen, const EventSet& rControllableEvents,
                           Generator& rControlledGen) {
    FD_DF("buildControlledSystem(" << rPlantGen.Name() << ")");
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
            if (controlPatterns[i].Exists(rPlantGen.EventName(ev))) {
                Idx newEv = eventMap[std::make_pair(ev, i)];
                rControlledGen.SetTransition(x1, newEv, x2);
            }
        }
    }

    rControlledGen.Name("Controlled_" + rPlantGen.Name());
}


void RabinProduct(const Generator& rGen1, const Generator& rGen2, Generator& rResGen) {
    FD_DF("RabinProduct(" << rGen1.Name() << "," << rGen2.Name() << ")");

    // prepare generator
    Generator* pResGen = &rResGen;
    if (&rResGen == &rGen1 || &rResGen == &rGen2) {
        pResGen = new Generator();
    }
    pResGen->Clear();
    pResGen->Name(CollapsString(rGen1.Name() + "||" + rGen2.Name()));

    // setting the alphabet of G_C
    EventSet gCAlphabet = rGen1.Alphabet();
    pResGen->InjectAlphabet(gCAlphabet);
    FD_DF("RabinProduct: alphabet: " << pResGen->Alphabet().ToString());

    // map the alphabet of G_C to to Spec
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

    // add all the todo-Statepair into stack
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

    // State searching
    while (!todo.empty()) {
        std::pair<Idx, Idx> current = todo.top();
        todo.pop();
        Idx xG = current.first;
        Idx xS = current.second;
        Idx currentIdx = stateMap[current];
        FD_DF("RabinProduct: processing (" << xG << "|" << xS << ") -> " << currentIdx);

        // traverse all the transition of rGen1
        for (TransSet::Iterator tit1 = rGen1.TransRelBegin(xG); tit1 != rGen1.TransRelEnd(xG); ++tit1) {
            std::string eventName = rGen1.EventName(tit1->Ev);
            std::string baseEvent = eventBaseMap[eventName];

            // search the match transition in Gen2
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

    // setting the marked state
    for (const auto& entry : stateMap) {
        Idx xG = entry.first.first;
        Idx xS = entry.first.second;
        Idx xAC = entry.second;
        if (rGen1.ExistsMarkedState(xG) && rGen2.ExistsMarkedState(xS)) {
            pResGen->SetMarkedState(xAC);
        }
    }
    FD_DF("RabinProduct: marked states: " << pResGen->MarkedStatesToString());

    if (pResGen != &rResGen) {
        rResGen = *pResGen;
        delete pResGen;
    }

    FD_DF("RabinProduct(...): done");
}

//based on the project(),but not using subset construction to determine
void RabinProjectND(const Generator& rGen, const EventSet& rObsEvents, Generator& rResGen) {
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

    FD_DF("RabinProjectND(...): done");
}

} // namespace faudes