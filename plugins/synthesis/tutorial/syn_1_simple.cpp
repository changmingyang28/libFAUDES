/** @file syn_1_simple.cpp

Tutorial, std monolitic synthesis. 


This tutorial uses a very simple example to illustrate the 
std monolithic controller synthesis
as originally proposed by Ramadge and Wonham.


@ingroup Tutorials 

@include syn_1_simple.cpp

*/

#include "libfaudes.h"


// we make the faudes namespace available to our program
using namespace faudes;




/////////////////
// main program
/////////////////

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
  Generator specification;
  specification.Read("data/buffer2.gen");
  InvProject(specification,cplant.Alphabet()); 
  specification.Name("simple machines specification");

  // Write to file
  specification.Write("tmp_specification12.gen");

  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, specification \n";
  specification.DWrite();
  std::cout << "################################\n";

  // Run synthesis algorithm
  System supervisor;
  SupConNB(cplant,specification,supervisor);
  supervisor.Name("simple machines supervisor");
  supervisor.Write("tmp_supervisor12.gen");


  System G_o;
  EventSet obsevents;
  obsevents.Insert("alpha_1");
  obsevents.Insert("alpha_2");
  Project(supervisor,obsevents,G_o);
  // Report to console
  std::cout << "################################\n";
  std::cout << "# tutorial, supervisor\n";
  supervisor.DWrite();
  G_o.DWrite();
  std::cout << "################################\n";

  return 0;
}

