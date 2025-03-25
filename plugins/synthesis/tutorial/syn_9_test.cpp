#include "libfaudes.h"
using namespace faudes;

int main() {
    // 1. load Plant
    System plant;

    plant.Read("data/safratest_Plant2.gen");
    plant.DWrite();

    EventSet contevents;
    contevents.Insert("a");
    plant.SetControllable(contevents);


    // 2. construct G_C
    Generator controlledPlant;
    buildControlledSystem(plant, contevents, controlledPlant);
    controlledPlant.Write("tmp_controlled_plant.gen");
    controlledPlant.Name("Controlled Plant");
    controlledPlant.DWrite();

    // 3. load Specification
    Generator specGen;
    specGen.Read("data/safratest_Spec2.gen");
    InvProject(specGen, plant.Alphabet());
    specGen.Name("Specification");
    specGen.DWrite();   

    // RabinGenerator spec;
    // StateSet rSetSpec, iSetSpec;
    // rSetSpec.Insert(spec.StateIndex("F2"));
    // iSetSpec.Insert(spec.StateIndex("E"));
    // iSetSpec.Insert(spec.StateIndex("F1"));
    // spec.ClearRabinPairs();
    // spec.InsRabinPair(rSetSpec, iSetSpec);
    // stdcout << "# Specification:\n";
    // spec.DWrite();

    // 4.  product Gc and spec
    Generator AC;
    RabinProduct(controlledPlant, specGen, AC);
    AC.Write("tmp_aC.gen");
    AC.Name("A_c");
    AC.DWrite();

    EventSet obsevents;
    obsevents.Insert("a");
    
    //project into Non-Deterministic 
    Generator AO;
    RabinProjectND(AC,obsevents,AO);
    AO.Name("A_o");
    AO.DWrite();
    
    //Determine by safra algorithm
    RabinGenerator AD;
    SafraDet(AO,AD);
    AD.Name("A_d");
    AD.DWrite();


    return 0;
}
