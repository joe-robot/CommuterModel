/* Demo_03_Agent.h */

#ifndef COMMUTER
#define COMMUTER

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "Infrastructure.h"


/* Agents */
class Commuter{
	
private:
    repast::AgentId   id_;
    double              safety;
    double          thresh;
    int             TransMode;
	int timestep=0;
    
	
public:
    Commuter(repast::AgentId id);
	Commuter(){}
    Commuter(repast::AgentId id, double newSafe, double newThresh, int newMode);
	
    ~Commuter();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    double getSafe(){                                      return safety;  }
    double getThresh(){                                  return thresh;  }
    int getMode(){                                  return TransMode;  }
	
    /* Setter */
    void set(int currentRank, double newSafe, double newThresh, int newMode);
	
    /* Actions */
    int ChooseMode();
    // Will indicate whether the agent cooperates or not; probability determined by = c / total
    void Travel(double Gsafety,repast::SharedContext<Commuter>* context,
              repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace);    // Choose three other agents from the given context and see if they cooperate or not
    //void move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space);
    
};


#endif

