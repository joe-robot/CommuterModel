/* Infrastructure.h */

#ifndef INFRASTRUCTURE
#define INFRASTRUCTURE

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"


/* Agents */
class Infrastructure{
	
private:
    repast::AgentId   id_;
    double              safety;
    double          thresh;
    int             Transtype;
    
	
public:
    Infrastructure(repast::AgentId id);
	Infrastructure(){}
    Infrastructure(repast::AgentId id, double newSafe, double newThresh, int newTranstype);
	
    ~Infrastructure();
	
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
    void commute(double Gsafety,repast::SharedContext<Infrastructure>* context,
              repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space);    // Choose three other agents from the given context and see if they cooperate or not
    void move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space);
    
};


#endif

