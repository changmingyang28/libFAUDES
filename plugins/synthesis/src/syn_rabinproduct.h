/** @file syn_rabinproduct.h Generator operations for Rabin-like behavior */

   #ifndef FAUDES_SYN_RABINPRODUCT_H
   #define FAUDES_SYN_RABINPRODUCT_H
   
   #include "corefaudes.h"
   #include <vector>
   #include <map>
   #include <stack>
   #include <cmath>
   
   namespace faudes {
   
   /**
    * Build control patterns for controllable events
    *
    * Generates all possible control patterns based on the alphabet and controllable events.
    *
    * @param alphabet
    *   Full event alphabet
    * @param controllableEvents
    *   Set of controllable events
    * @return
    *   Vector of control patterns (subsets of the alphabet)
    */
   extern FAUDES_API std::vector<EventSet> buildControlPatterns(
     const EventSet& alphabet,
     const EventSet& controllableEvents);
   
   /**
    * Build controlled system
    *
    * Constructs a controlled system from a plant generator by expanding controllable events
    * into multiple versions based on control patterns.
    *
    * @param rPlantGen
    *   Input plant generator
    * @param rControllableEvents
    *   Set of controllable events
    * @param rControlledGen
    *   Reference to resulting controlled generator
    *
    * @ingroup RabinPlugIn
    */
   extern FAUDES_API void buildControlledSystem(
     const Generator& rPlantGen,
     const EventSet& rControllableEvents,
     Generator& rControlledGen);
   
   /**
    * Computes the parallel composition of two generators, synchronizing on base events
    * derived from event names (e.g., stripping "_G#" suffixes).
    *
    * @param rGen1
    *   First generator
    * @param rGen2
    *   Second generator
    * @param rResGen
    *   Reference to resulting generator
    *
    * @ingroup RabinPlugIn
    */
   extern FAUDES_API void RabinProduct(
     const Generator& rGen1,
     const Generator& rGen2,
     Generator& rResGen);
   
   /**
    * Non-deterministic projection of a generator
    *
    * Projects a generator onto observable events, treating unobservable events as epsilon
    * transitions, and preserves marked states.
    *
    * @param rGen
    *   Input generator
    * @param rObsEvents
    *   Set of observable events
    * @param rResGen
    *   Reference to resulting generator
    *
    * @ingroup RabinPlugIn
    */
   extern FAUDES_API void RabinProjectND(
     const Generator& rGen,
     const EventSet& rObsEvents,
     Generator& rResGen);
   
   } // namespace faudes
   
   #endif