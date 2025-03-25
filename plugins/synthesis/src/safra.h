/** @file safra.h Safra determinization algorithm */

   #ifndef FAUDES_SAFRA_H
   #define FAUDES_SAFRA_H
   
   #include "corefaudes.h"
   #include <vector>
   #include <set>
   #include <stack>
   #include <unordered_map>
   #include <algorithm>
   #include <iterator>
   #include <sstream>
   #include <iostream>

   namespace faudes {
   
   /**
    * Rabin Generator class
    *
    * Extends vGenerator to include Rabin acceptance conditions, represented as pairs of reject and accept state sets.
    */
   class FAUDES_API RabinGenerator : public vGenerator {
   public:
       typedef std::pair<StateSet, StateSet> RabinPair;
       std::vector<RabinPair> mRabinPairs;
   
       RabinGenerator();
       explicit RabinGenerator(const std::string& rFileName);
       RabinGenerator(const RabinGenerator& rOther);
       virtual ~RabinGenerator();
   
       void ClearRabinPairs();
       void InsRabinPair(const StateSet& rRSet, const StateSet& rISet);
       const std::vector<RabinPair>& RabinPairs() const;
   
       void Write(const std::string& rFileName) const;
       void Read(const std::string& rFileName);
       void DWrite() const;
   
   protected:
       virtual void DoWrite(TokenWriter& rTw, const std::string& rLabel = "") const;
       virtual void DoRead(TokenReader& rTr, const std::string& rLabel = "");
       virtual void DoDWrite(TokenWriter& rTw, const std::string& rLabel) const;
   };
   
   /**
    * Safra Node class
    *
    * Represents a node in a Safra tree, used in the Safra determinization algorithm.
    */
   class FAUDES_API SafraNode {
   public:
       Idx name;                     // Node identifier
       StateSet label;               // Set of states from the Büchi automaton
       bool mark;                    // Mark indicating acceptance condition
       std::vector<SafraNode> children; // Child nodes
   
       SafraNode();
       explicit SafraNode(Idx n);
   
       bool is_valid() const;
       int find_name(Idx n) const;
       std::string to_string(const vGenerator& gen) const;
       bool operator==(const SafraNode& other) const;
   };
   
   /**
    * Safra Tree class
    *
    * Represents a Safra tree, used to track the state of the Büchi automaton during determinization.
    */
   class FAUDES_API SafraTree {
   public:
       std::vector<bool> names;      // Availability of names
       SafraNode root;               // Root node of the tree
   
       SafraTree();
       explicit SafraTree(size_t buechi_size);
       SafraTree(const vGenerator& buechi);
   
       bool is_valid() const;
       Idx next_name();
       void remove_name(Idx n);
       void remove_names(SafraNode& node);
       std::string to_string(const vGenerator& gen) const;
   };
   
   /**
    * Hash function for SafraNode
    */
   struct FAUDES_API SafraNodeHash {
       std::size_t operator()(const SafraNode& node) const;
   };
   
   /**
    * Hash function for SafraTree
    */
   struct FAUDES_API SafraTreeHash {
       std::size_t operator()(const SafraTree& tree) const;
   };
   
   /**
    * Equality function for SafraTree
    */
   struct FAUDES_API SafraTreeEqual {
       bool operator()(const SafraTree& t1, const SafraTree& t2) const;
   };
   
   /**
    * Safra Determinization
    *
    * Given a Büchi automaton G (argument rBuechiGen), this procedure computes a deterministic
    * Rabin automaton R such that L(R) = L(G). The result is a deterministic Rabin automaton
    * that recognizes the same ω-language as the input Büchi automaton.
    *
    * Parameter restrictions: the input generator must be a valid Büchi automaton (i.e., have initial and marked states).
    *
    * @param rBuechiGen
    *   Input Büchi automaton G
    * @param rResGen
    *   Reference to resulting Rabin automaton R
    *
    * @exception Exception
    *   - Input generator has no marked states (id 300)
    *   - Input generator has no initial states (id 301)
    *   - Invalid initial Safra tree (id 101)
    *
    * @ingroup SafraPlugIn
    */
   extern FAUDES_API void SafraDet(
     const Generator& rBuechiGen,
     RabinGenerator& rResGen);
   
   /**
    * Safra Determinization (internal function)
    *
    * This version of SafraDet performs no consistency checks on the input parameters.
    * It directly applies the Safra algorithm to determinize the Büchi automaton into a Rabin automaton.
    *
    * @param rBuechiGen
    *   Input Büchi automaton G
    * @param rResGen
    *   Reference to resulting Rabin automaton R
    *
    * @ingroup SafraPlugIn
    */
   extern FAUDES_API void SafraDetUnchecked(
     const Generator& rBuechiGen,
     RabinGenerator& rResGen);
   
   /**
    * Consistency check for Safra determinization input data.
    *
    * Tests whether the input generator is a valid Büchi automaton (has initial and marked states).
    *
    * @param rBuechiGen
    *   Input Büchi automaton G
    *
    * @exception Exception
    *   - Input generator has no marked states (id 300)
    *   - Input generator has no initial states (id 301)
    */
   extern FAUDES_API void SafraConsistencyCheck(
     const Generator& rBuechiGen);
   
   /**
    * Helper function to copy a Safra node and remove marks
    *
    * @param node
    *   Source Safra node
    * @return
    *   New Safra node with marks removed
    */
   extern FAUDES_API SafraNode CopyAndRemoveMarks(const SafraNode& node);
   
   /**
    * Helper function to branch accepting states in a Safra tree
    *
    * @param gen
    *   Büchi automaton
    * @param tree
    *   Safra tree
    * @param node
    *   Current node to process
    */
   extern FAUDES_API void BranchAccept(const vGenerator& gen, SafraTree& tree, SafraNode& node);
   
   /**
    * Helper function to compute the successor states for a given event
    *
    * @param gen
    *   Büchi automaton
    * @param node
    *   Current node to process
    * @param letter
    *   Event to compute successors for
    */
   extern FAUDES_API void PsetConst(const vGenerator& gen, SafraNode& node, Idx letter);
   
   /**
    * Helper function to perform horizontal merge in a Safra tree
    *
    * @param node
    *   Current node to process
    * @param brothers
    *   Set of states from sibling nodes
    */
   extern FAUDES_API void HorizontalMerge(SafraNode& node, StateSet& brothers);
   
   /**
    * Helper function to remove empty nodes from a Safra tree
    *
    * @param tree
    *   Safra tree
    * @param node
    *   Current node to process
    */
   extern FAUDES_API void RemoveEmpty(SafraTree& tree, SafraNode& node);
   
   /**
    * Helper function to perform vertical merge in a Safra tree
    *
    * @param tree
    *   Safra tree
    * @param node
    *   Current node to process
    */
   extern FAUDES_API void VerticalMerge(SafraTree& tree, SafraNode& node);
   
   /**
    * Helper function to compute the next Safra tree after a transition
    *
    * @param gen
    *   Büchi automaton
    * @param tree
    *   Current Safra tree
    * @param letter
    *   Event to transition on
    * @return
    *   Next Safra tree
    */
   extern FAUDES_API SafraTree ComputeTransition(const vGenerator& gen, const SafraTree& tree, Idx letter);
   
   } // namespace faudes
   
   #endif