/* Demo_01_Agent.h */

#ifndef DEMO_01_AGENT
#define DEMO_01_AGENT

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"


/* Agents */
class Commuter{
	
private:
    repast::AgentId   id_;
    double              safety;
    double          thresh;
    bool	Transtype;
	
public:
    Commuter(repast::AgentId id);
	
    Commuter(repast::AgentId id, double newSafe, double newThresh, bool newTranstype);
	
    ~Commuter();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    double getSafe(){                                      return safety;      }
    double getThresh(){                                  return thresh;  }
    bool getTrans(){					return Transtype; }
	
    /* Setter */
    void set(int currentRank, double newSafe, double newThresh, bool newTranstype);
	
    /* Actions */
    bool choosetrans();                                                 // Will decide trans method
    void commute(repast::SharedContext<Commuter>* context);    // Choose three other agents from the given context and see if they cooperate or not
	
};

/* Serializable Agent Package */
struct CommuterPackage {
	
public:
    int    id;
    int    rank;
    int    type;
    int    currentRank;
    double safety;
    double thresh;
    bool  Transtype;
	
    /* Constructors */
    CommuterPackage(); // For serialization
    CommuterPackage(int _id, int _rank, int _type, int _currentRank, double _safety, double _thresh, bool _Transtype);
	
    /* For archive packaging */
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version){
        ar & id;
        ar & rank;
        ar & type;
        ar & currentRank;
        ar & safety;
        ar & thresh;
	ar & Transtype;
    }
	
};


#endif
