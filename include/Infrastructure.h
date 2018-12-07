/* Infrastructure.h */

#ifndef INFRASTRUCTURE
#define INFRASTRUCTURE

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"


/* Agents */
class Infrastructure{
	
private:
    repast::AgentId  	id_;
    int             	Capacity;
    int         	Reach;
    double           	ProvSafety;
    
	
public:
    //Infrastructure(repast::AgentId id);
	Infrastructure(){}
    Infrastructure(repast::AgentId id, int newCap, int newReach, double newPSafe);
	
    ~Infrastructure();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    int getCapacity(){                               return Capacity;  }
    int getReach(){                                  return Reach;  }
    double getProvSafety(){                          return ProvSafety;  }
	
    /* Setter */
    void set(int currentRank, int newCap, int newReach, double newPSafe);
	
    /* Actions */
    int use(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space);    
    //void move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space); //Don't want this to move
    
};


#endif

