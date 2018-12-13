/* Infrastructure.cpp */

#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int InfTemplate,int newReach, int Pvar)
{
	id_=id;
	InfType=newInfType;
	Reach=newReach;
	if(InfType==0)		//Templates for cycle saftey infrastructure
	{
		CostPerAgent=0;
		if(InfTemplate==0)	//Template for a segregated cycleway
		{
			InfCost = ((double)Reach/10)*1.15;	//1.15 million per km
			ProvVar=1;
			OldProvVar=1;
			maxCapacity=40*Reach;
			Capacity=0;

		}
		if(InfTemplate==1)  //Template for a non-segregate cycleway
		{
			InfCost = ((double)Reach/10)*0.5;	//0.5 million per km
			ProvVar=0.9;
			OldProvVar=1;
			maxCapacity=30*Reach;
			Capacity=0;
		}
		if(InfTemplate==2)  //Template for a 20mph zone with speed bumps
		{
			InfCost =  ((double)Reach/10)*0.01;	//0.01 million per km
			ProvVar=0.8;
			OldProvVar=1;
			maxCapacity=20*Reach;
			Capacity=0;

		}
		if(InfTemplate==3) //Template for a cycle crossing (typically very small reach..just a crossing)
		{
			InfCost =  ((double)Reach/10)*0.14;	//0.14 million per crossing
			ProvVar=1;
			OldProvVar=1;
			maxCapacity=5*Reach;
			Capacity=0;
		}
	}
	if(InfType==1)	//Templates for economic subsidising infrastructure
	{
		if(InfTemplate==0)	//Pay per cycle scheme
		{
			ProvVar=5*4;	//5 pounds per week
			OldProvVar=5*4;
			maxCapacity=Pvar;
			CostPerAgent=(ProvVar/1000);
			Capacity= 0;
			InfCost=(double)Capacity*CostPerAgent;
		}
		if(InfTemplate==1)	//pay per cycle scheme - more expensive
		{
			ProvVar=10*4;	//10 pounds per week
			OldProvVar=10*4;
			maxCapacity=Pvar;
			Capacity= 0;
			CostPerAgent=(ProvVar/1000);
			InfCost=(double)Capacity*CostPerAgent;	//Cost depends on how many end up using it (initialised as 0)
		}
	}
	if(InfType==2) //Templates for Health improving infrastructure (health campaigns and provided fitness classes)
	{
		if(InfTemplate==0)		//A get healthy living campaign
		{	
			ProvVar=1 + (repast::Random::instance()->getGenerator("duni")->next())*0.05;		//Provides a randomly chosen amount of health for those who take it up with 5% improvement as max
			OldProvVar=ProvVar;
			maxCapacity=Reach*50;	//large capacity
			CostPerAgent=0;
			InfCost= 0.20*((double)Reach/10);	//200K per Km campaign
			
		}
		if(InfTemplate==1)		//Providing free fitness classes
		{
			ProvVar=1.05;		//Provides a 5% improvement in health
			OldProvVar=1.05;
			maxCapacity=Pvar;
			Capacity=0;
			CostPerAgent=0.02*4;
			InfCost=Capacity*CostPerAgent;	//20 pounds per person per class depend how many take it up(each agent is 1000 people)
		}

	}
	if(InfType==3) //Templates for Cycle ability improving infrastructure (cycle lessons and campaigns)
	{
		if(InfTemplate==0)	//Cycling Lessons
		{
			ProvVar=1.15;		//Provides a 15% improvement in cycling ability (a month of cycle training)
			OldProvVar=1.15;
			maxCapacity=Pvar;
			Capacity=0;
			CostPerAgent=0.02*4;
			InfCost=Capacity*CostPerAgent;	//20 pounds per person per class depend how many take it up(each agent is 1000 people)
		}


	}
}

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int newCap, int newMaxCap, int newReach, double newPVar, double newInfCost,double newCostPerAgent): id_(id), InfType(newInfType),Capacity(newCap), maxCapacity(newMaxCap), Reach(newReach), ProvVar(newPVar), OldProvVar(newPVar), InfCost(newInfCost), CostPerAgent(newCostPerAgent){ }

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank,int newInfType, int newCap, int newMaxCap, int newReach, double newPVar, double newInfCost,double	newCostPerAgent){
    id_.currentRank(currentRank);
	InfType=newInfType;
    Capacity     = newCap;
	maxCapacity = newMaxCap;
    Reach = newReach;			//Got a problem with reach as the reach circle is not included in the query!
	InfCost=newInfCost;
	CostPerAgent=newCostPerAgent;
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
			ProvVar=0.75+(-1*exp((log((ProvVar-0.75)+1)/maxCapacity)*Capacity))+1+ProvVar-0.75;	//Calculating the provided saftey with capacity minimum saftey being 0.7
		}
		else
		{
			ProvVar=0.75;
		}
	}
	else if(InfType==1)	//If infrastucture is economic subsidising infrastructure
	{	
		InfCost=InfCost+CostPerAgent;
		if(Capacity<maxCapacity&&InfCost>=maxCapacity*CostPerAgent)
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
		InfCost=InfCost+CostPerAgent;
		if(Capacity<maxCapacity&&InfCost>=maxCapacity*CostPerAgent)
		{
			ProvVar=ProvVar;
		}
		else
		{
			ProvVar=1;
		}
	}
	else if(InfType==3)	//If Infrastructure is Cycle training infrastructure
	{
		InfCost=InfCost+CostPerAgent;
		if(Capacity<maxCapacity&&InfCost>=maxCapacity*CostPerAgent)
		{
			ProvVar=ProvVar;
		}
		else
		{
			ProvVar=1;
		}
	}
	
	/*if(ProvVar !=0)
	{
	ProvVar = ProvVar - ProvVar*(Capacity/Capacity+1);
	}	//Need a better algorithm here it is making*/
	

	Capacity ++;
	return ProvVar;

}





