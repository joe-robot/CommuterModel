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
    int         		Reach;
    double           	ProvVar=-1;
	double				OldProvVar; //<-might not need now tbh
	int					InfType;
	
public:
    //Infrastructure(repast::AgentId id);
	Infrastructure(){}
    Infrastructure(repast::AgentId id, int InfType, int newCap, int newReach, double newPVar);
	
    ~Infrastructure();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    int getCapacity(){                               return Capacity;  }
    int getReach(){                                  return Reach;  }
    double getProvVar(){                          return ProvVar;  }
	double getOldProvVar(){						 return OldProvVar; }
	int getInfType(){								 return InfType;}
	
    /* Setter */
    void set(int currentRank, int InfType,int newCap, int newReach, double newPVar);
	
    /* Actions */
    int use(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space);    
    //void move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space); //Don't want this to move
    
};


#endif

