/* Infrastructure.cpp */

#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int newCap, int newMaxCap, int newReach, double newPVar): id_(id), InfType(newInfType),Capacity(newCap), maxCapacity(newMaxCap), Reach(newReach), ProvVar(newPVar), OldProvVar(newPVar){ }

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank,int newInfType, int newCap, int newMaxCap, int newReach, double newPVar){
    id_.currentRank(currentRank);
	InfType=newInfType;
    Capacity     = newCap;
	maxCapacity = newMaxCap;
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
		if(Capacity<maxCapacity)
		{
			ProvVar=0.7+(-1*exp((log((ProvVar-0.7)+1)/maxCapacity)*Capacity))+1+ProvVar-0.7;	//Calculating the provided saftey with capacity minimum saftey being 0.7
		}
		else
		{
			ProvVar=0.7;
		}
	}
	else if(InfType==1)	//If infrastucture is economic subsidising infrastructure
	{	
		if(Capacity<maxCapacity)
		{
			ProvVar=ProvVar;
		}
		else
		{
			ProvVar=0;
		}
		
	}
	else if(InfType==2)	//If infrastructure is Health improving infrastructure
	{
		if(Capacity<maxCapacity)
		{
			ProvVar=ProvVar;
		}
		else
		{
			ProvVar=0;
		}
	}
	else if(InfType==3)	//If Infrastructure is Cycle training infrastructure
	{
		if(Capacity<maxCapacity)
		{
			ProvVar=ProvVar;
		}
		else
		{
			ProvVar=0;
		}
	}
	
	/*if(ProvVar !=0)
	{
	ProvVar = ProvVar - ProvVar*(Capacity/Capacity+1);
	}	//Need a better algorithm here it is making*/
	

	Capacity ++;
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



