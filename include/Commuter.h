/* Demo_03_Agent.h */

#ifndef COMMUTER
#define COMMUTER

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"


/* Agents */
class Commuter{
	
private:
    repast::AgentId   id_;
    double              safety;
    double          thresh;
    int             Transtype;
    
	
public:
    Commuter(repast::AgentId id);
	Commuter(){}
    Commuter(repast::AgentId id, double newSafe, double newThresh, int newTranstype);
	
    ~Commuter();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    double getSafe(){                                      return safety;  }
    double getThresh(){                                  return thresh;  }
    int getTrans(){                                  return Transtype;  }
	
    /* Setter */
    void set(int currentRank, double newSafe, double newThresh, int newTranstype);
	
    /* Actions */
    int choosetrans();
    // Will indicate whether the agent cooperates or not; probability determined by = c / total
    void commute(double Gsafety,repast::SharedContext<Commuter>* context,
              repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space);    // Choose three other agents from the given context and see if they cooperate or not
    void move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space);
    
};

/* //Serializable Agent Package
struct CommuterPackage {
	
public:
    int    id;
    int    rank;
    int    type;
    int    currentRank;
    double c;
    double total;
	
    //Constructors
    CommuterPackage(); // For serialization
    CommuterPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total);
	
    //For archive packaging
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version){
        ar & id;
        ar & rank;
        ar & type;
        ar & currentRank;
        ar & c;
        ar & total;
    }
	
};

*/
#endif

