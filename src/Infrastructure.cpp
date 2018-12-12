/* Infrastructure.cpp */

#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id, int newCap, int newReach, double newPSafe): id_(id), Capacity(newCap), Reach(newReach), ProvSafety(newPSafe), OldProvSafety(newPSafe){ }

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank, int newCap, int newReach, double newPSafe){
    id_.currentRank(currentRank);
    Capacity     = newCap;
    Reach = newReach;			//Got a problem with reach as the reach circle is not included in the query!
	if(ProvSafety!=-1)
	{	
		OldProvSafety=ProvSafety;
	}
	else
	{
		OldProvSafety=newPSafe;
	}
    ProvSafety = newPSafe;
    
}


int Infrastructure::use(repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){
	OldProvSafety=ProvSafety;
	if(ProvSafety !=0)
	{
	ProvSafety = ProvSafety - ProvSafety*(Capacity/Capacity+1);
	}	//Need a better algorithm here it is making
	Capacity ++;
	return ProvSafety;

}


/*void Infrastructure::move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}*/



