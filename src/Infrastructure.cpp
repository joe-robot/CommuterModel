/* Infrastructure.cpp */

#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int newCap, int newReach, double newPVar): id_(id), InfType(newInfType),Capacity(newCap), Reach(newReach), ProvVar(newPVar), OldProvVar(newPVar){ }

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank,int newInfType, int newCap, int newReach, double newPVar){
    id_.currentRank(currentRank);
	InfType=newInfType;
    Capacity     = newCap;
    Reach = newReach;			//Got a problem with reach as the reach circle is not included in the query!
	if(ProvVar!=-1)
	{	
		OldProvVar=ProvVar;
	}
	else
	{
		OldProvVar=newPVar;
	}
    ProvVar = newPVar;
    
}


int Infrastructure::use(repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){
	OldProvVar=ProvVar;

	if(InfType==0)		//If infrastructure is saftey boosting infrastructure
	{
		ProvVar=ProvVar;
	}
	else if(InfType==1)	//If infrastucture is economic subsidising infrastructure
	{
		ProvVar=ProvVar;
	}
	else if(InfType==2)	//If infrastructure is Health improving infrastructure
	{
		ProvVar=ProvVar;
	}
	else if(InfType==3)	//If Infrastructure is Cycle training infrastructure
	{
		ProvVar=ProvVar;
	}
	
	/*if(ProvVar !=0)
	{
	ProvVar = ProvVar - ProvVar*(Capacity/Capacity+1);
	}	//Need a better algorithm here it is making
	Capacity ++;*/
	return ProvVar;

}


/*void Infrastructure::move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}*/



