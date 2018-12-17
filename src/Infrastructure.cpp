/* Infrastructure.cpp */

//By Joseph Cresswell	Reg No. 150148395

//inclusing required files
#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int InfTemplate,int newReach, double Pvar) //Constructer for infrastructure agents where templates are used that are based on real infrastructures and dataa
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
			InfCost =  ((double)Reach)*0.14;	//0.14 million per crossing
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
			CostPerAgent=(ProvVar/1000);	
			maxCapacity=Pvar/CostPerAgent;	//Pvar gives amount of invetment so max is found by dividing invetment by cost per agent
			Capacity= 0;
			InfCost=(double)Capacity*CostPerAgent;
		}
		if(InfTemplate==1)	//pay per cycle scheme - more expensive
		{
			ProvVar=10*4;	//10 pounds per week
			OldProvVar=10*4;
			Capacity= 0;
			CostPerAgent=(ProvVar/1000);
			maxCapacity=Pvar/CostPerAgent;//Pvar gives amount of invetment so max is found by dividing invetment by cost per agent
			InfCost=(double)Capacity*CostPerAgent;	//Cost depends on how many end up using it (initialised as 0)
		}
	}
	if(InfType==2) //Templates for Health improving infrastructure (health campaigns and provided fitness classes)
	{
		if(InfTemplate==0)		//A get healthy living campaign
		{	
			ProvVar=(repast::Random::instance()->getGenerator("duni")->next())*0.05;		//Provides a randomly chosen amount of health for those who take it up with 5% improvement as max
			OldProvVar=ProvVar;
			maxCapacity=Reach*50;	//large capacity
			CostPerAgent=0;
			InfCost= 0.20*((double)Reach/10);	//200K per Km campaign
			
		}
		if(InfTemplate==1)		//Providing free fitness classes
		{
			ProvVar=0.1;		//Provides a maximum 10% improvement in health
			OldProvVar=0.1;
			Capacity=0;
			CostPerAgent=0.01*4;
			maxCapacity=Pvar/CostPerAgent;//Pvar gives amount of invetment so max is found by dividing invetment by cost per agent
			InfCost=Capacity*CostPerAgent;	//20 pounds per person per class depend how many take it up(each agent is 1000 people)
		}

	}
	if(InfType==3) //Templates for Cycle ability improving infrastructure (cycle lessons and campaigns)
	{
		if(InfTemplate==0)	//Cycling Lessons
		{
			ProvVar=1.15;		//Provides a 15% improvement in cycling ability (a month of cycle training)
			OldProvVar=1.15;
			Capacity=0;
			CostPerAgent=0.01*4;
			maxCapacity=Pvar/CostPerAgent;	//Pvar gives amount of invetment so max is found by dividing invetment by cost per agent
			InfCost=Capacity*CostPerAgent;	//20 pounds per person per class depend how many take it up(each agent is 1000 people)
		}


	}
}

Infrastructure::Infrastructure(repast::AgentId id, int newInfType, int newCap, int newMaxCap, int newReach, double newPVar, double newInfCost,double newCostPerAgent): id_(id), InfType(newInfType),Capacity(newCap), maxCapacity(newMaxCap), Reach(newReach), ProvVar(newPVar), OldProvVar(newPVar), InfCost(newInfCost), CostPerAgent(newCostPerAgent){ } //infrastructure contructure where all paramters can be set without templates

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank,int newInfType, int newCap, int newMaxCap, int newReach, double newPVar, double newInfCost,double	newCostPerAgent){	//Setting infrastructure parameters allowerd at anypoint (could increase funding)
    id_.currentRank(currentRank);
	InfType=newInfType;
    Capacity     = newCap;
	maxCapacity = newMaxCap;
    Reach = newReach;			
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


int Infrastructure::use(repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){	//Function for using infrastructure
	OldProvVar=ProvVar;

	if(InfType==0)		//If infrastructure is saftey boosting infrastructure
	{
	
		if(Capacity<maxCapacity)
		{
			ProvVar=0.75+(-1*exp((log((ProvVar-0.75)+1)/maxCapacity)*Capacity))+1+ProvVar-0.75;	//Calculating the provided saftey with capacity minimum saftey being 0.7 so that the larger the capacity the less the safety
		}
		else
		{
			ProvVar=0.75;
		}
	}
	else if(InfType==1)	//If infrastucture is economic subsidising infrastructure
	{	
		
		if(InfCost<=(double)maxCapacity*CostPerAgent) //if budget not fully used
		{
			InfCost=InfCost+CostPerAgent;	//Add another agent to cost
			ProvVar=ProvVar;		//provided help dosen't decrease with amount of people using it until budget fully used
		}
		else
		{
			ProvVar=0;	//if budget used then provide no subsidy
		}
		
	}
	else if(InfType==2)	//If infrastructure is Health improving infrastructure
	{
		if(InfCost<=(double)maxCapacity*CostPerAgent) //If budget not used 
		{
			InfCost=InfCost+CostPerAgent;	//Add another agent to cost
			ProvVar=ProvVar;		//provided help dosen't decrease with amount of people using it until budget fully used
		}
		else
		{
			ProvVar=0;	//if budget used then provide no health improvement
		}
	}
	else if(InfType==3)	//If Infrastructure is Cycle training infrastructure
	{
		
		if(InfCost<=(double)maxCapacity*CostPerAgent)	//if budget not yet used
		{
			InfCost=InfCost+CostPerAgent;	//Add another agent to cost
			ProvVar=ProvVar;		//provided help dosen't decrease with amount of people using it until budget fully used
		}
		else
		{
			ProvVar=1;	//if budget used then provide no cycle ability improvement
		}
	}

	Capacity ++;	//Add to capacity of infrastructure
	return ProvVar;

}





