#include "libfaudes.h"
#include "safra.h"

// we make the faudes namespace available to our program
using namespace faudes;


int main() {
    FD_DF("Main started");
    vGenerator buechi;
    buechi.Read("data/safratest1.gen");
    // buechi.SetTransition("E", "beta_1", "F2");
    buechi.Name("SimpleBuechi");
    buechi.DWrite();

    RabinGenerator rabinGen;
    SafraDet(buechi, rabinGen);
    rabinGen.Write("tmp_rabin_from_buechi.gen");

    std::cout << "# Rabin Automaton from SafraDet:\n";
    rabinGen.DWrite();
    FD_DF("Main finished");
    return 0;


}