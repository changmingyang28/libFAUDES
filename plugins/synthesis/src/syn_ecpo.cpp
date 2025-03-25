/** @file syn_1_rabin.cpp
 * Tutorial: Adding Rabin pair attribute to a simple synthesis example.
 */

 #include "libfaudes.h"
 #include <vector>

 using namespace faudes;
 
 /**
  * AttributeRabinPair class to represent Rabin acceptance conditions for a Generator.
  * A RabinPair is a pair of StateSets (L, U), and this attribute holds a vector of such pairs.
  */
 class AttributeRabinPair : public AttributeVoid {
  public:
   // Define RabinPair as a pair of StateSets
   typedef std::pair<faudes::StateSet, faudes::StateSet> RabinPair;
 
   // FAUDES type declaration
   FAUDES_TYPE_DECLARATION(RabinPair, AttributeRabinPair, AttributeVoid)
 
   // Default constructor
   AttributeRabinPair(void) {}
 
   // Copy constructor
   AttributeRabinPair(const AttributeRabinPair& rOther) {
     DoAssign(rOther);
   }
 
   // Destructor
   virtual ~AttributeRabinPair(void) {}
 
   // Clear the attribute
   void Clear(void) {
     mRabinPairs.clear();
   }
 
   // Add a Rabin pair
   void AddRabinPair(const StateSet& lSet, const StateSet& uSet) {
     mRabinPairs.push_back(RabinPair(lSet, uSet));
   }
 
   // Get the vector of Rabin pairs
   const std::vector<RabinPair>& RabinPairs(void) const {
     return mRabinPairs;
   }
 
   // Get the number of Rabin pairs
   size_t Size(void) const {
     return mRabinPairs.size();
   }
 
   // Get a specific Rabin pair by index
   const RabinPair& At(size_t index) const {
     if (index >= mRabinPairs.size()) {
       std::stringstream err;
       err << "Index out of range: " << index;
       throw Exception("AttributeRabinPair::At()", err.str(), 47);
     }
     return mRabinPairs[index];
   }
 
  protected:
   // Assign from another AttributeRabinPair
   void DoAssign(const AttributeRabinPair& rSrcAttr) {
     mRabinPairs = rSrcAttr.mRabinPairs;
   }
 
   // Test equality with another AttributeRabinPair
   bool DoEqual(const AttributeRabinPair& rOther) const {
     return mRabinPairs == rOther.mRabinPairs;
   }
 
   // Write to TokenWriter
   void DoWrite(TokenWriter& rTw, const std::string& rLabel = "", const Type* pContext = 0) const {
     (void) pContext; // Unused
     std::string label = rLabel.empty() ? "RabinPairs" : rLabel;
 
     rTw.WriteBegin(label);
     for (size_t i = 0; i < mRabinPairs.size(); ++i) {
       Token pairToken;
       pairToken.SetBegin("Pair");
       pairToken.InsAttributeInteger("index", i);
 
       // Write L set
       rTw.WriteBegin("LSet");
       mRabinPairs[i].first.Write(rTw);
       rTw.WriteEnd("LSet");
 
       // Write U set
       rTw.WriteBegin("USet");
       mRabinPairs[i].second.Write(rTw);
       rTw.WriteEnd("USet");
 
       rTw.WriteEnd("Pair");
     }
     rTw.WriteEnd(label);
   }
 
   // Read from TokenReader
   void DoRead(TokenReader& rTr, const std::string& rLabel = "", const Type* pContext = 0) {
     (void) pContext; // Unused
     std::string label = rLabel.empty() ? "RabinPairs" : rLabel;
 
     Clear(); // Reset existing data
     rTr.ReadBegin(label);
     while (!rTr.Eos(label)) {
       Token pairToken;
       rTr.ReadBegin("Pair", pairToken);
 
       // Read L set
       StateSet lSet;
       rTr.ReadBegin("LSet");
       lSet.Read(rTr);
       rTr.ReadEnd("LSet");
 
       // Read U set
       StateSet uSet;
       rTr.ReadBegin("USet");
       uSet.Read(rTr);
       rTr.ReadEnd("USet");
 
       AddRabinPair(lSet, uSet);
       rTr.ReadEnd("Pair");
     }
     rTr.ReadEnd(label);
   }
 
  private:
   // Vector of Rabin pairs
   std::vector<RabinPair> mRabinPairs;
 };
 
 // FAUDES type implementation
 FAUDES_TYPE_IMPLEMENTATION(RabinPair, AttributeRabinPair, AttributeVoid)
 
 /////////////////
 // main program
 /////////////////
 
 int main() {
   // Compose plant dynamics from two very simple machines 
   Generator tempgen, machinea, machineb;
   System cplant; 
  
   tempgen.Read("data/verysimplemachine.gen");
   tempgen.Version("1", machinea);
   tempgen.Version("2", machineb);
   Parallel(machinea, machineb, cplant);
 
   // Declare controllable events
   EventSet contevents;
   contevents.Insert("alpha_1");
   contevents.Insert("alpha_2");
   cplant.SetControllable(contevents);
 
   // Write plant to file
   cplant.Write("tmp_cplant12.gen");
 
   // Report plant to console
   std::cout << "################################\n";
   std::cout << "# tutorial, plant model \n";
   cplant.DWrite();
   std::cout << "################################\n";
 
   // Read specification 
   Generator specification;
   specification.Read("data/buffer2.gen");
   InvProject(specification, cplant.Alphabet()); 
   specification.Name("simple machines specification");
 
   // Write specification to file
   specification.Write("tmp_specification12.gen");
 
   // Report specification to console
   std::cout << "################################\n";
   std::cout << "# tutorial, specification \n";
   specification.DWrite();
   std::cout << "################################\n";
 
   // Run synthesis algorithm
   System supervisor;
   SupConNB(cplant, specification, supervisor);
   supervisor.Name("simple machines supervisor");
   supervisor.Write("tmp_supervisor12.gen");
 
   // Report supervisor to console
   std::cout << "################################\n";
   std::cout << "# tutorial, supervisor\n";
   supervisor.DWrite();
   std::cout << "################################\n";
 
   // Add Rabin pair attribute to supervisor
   AttributeRabinPair rabinAttr;
 
   // Define example Rabin pairs
   StateSet lSet1, uSet1;
   lSet1.Insert(1); // Example state
   uSet1.Insert(2);
   rabinAttr.AddRabinPair(lSet1, uSet1);
 
   StateSet lSet2, uSet2;
   lSet2.Insert(3);
   uSet2.Insert(4);
   rabinAttr.AddRabinPair(lSet2, uSet2);
 
   // Write Rabin attribute to file
   TokenWriter tw("tmp_rabinattr.gen");
   rabinAttr.DoWrite(tw);
   tw.Close();
 
   // Read Rabin attribute from file
   AttributeRabinPair rabinAttrRead;
   TokenReader tr("tmp_rabinattr.gen");
   rabinAttrRead.DoRead(tr);
   tr.Close();
 
   // Report Rabin pairs to console
   std::cout << "################################\n";
   std::cout << "# tutorial, Rabin pairs\n";
   std::cout << "Number of Rabin pairs: " << rabinAttrRead.Size() << "\n";
   for (size_t i = 0; i < rabinAttrRead.Size(); ++i) {
     const AttributeRabinPair::RabinPair& pair = rabinAttrRead.At(i);
     std::cout << "Pair " << i << ": L = " << pair.first.ToString() 
               << ", U = " << pair.second.ToString() << "\n";
   }
   std::cout << "################################\n";
 
   return 0;
 }