/* Demo_03_Agent.cpp */

#include "CommuterModel.h"
#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"
#include <boost/math/distributions/beta.hpp>	//Including the BETA sistribution from boost libraries

#define PI 3.141592653

Commuter::Commuter(repast::AgentId id, double InitialCar,double InitialBike, double InitialWalk, double InitialPTrans ): id_(id),safety(repast::Random::instance()->nextDouble()), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)
{

	//Initialising agent trasport mode randomly
	double RandTrasp=repast::Random::instance()->getGenerator("duni")->next();
    if(RandTrasp<(InitialCar)&&RandTrasp>=0)
    {
        TransMode= 0;
    }
    else if(RandTrasp<((InitialCar+InitialBike))&&RandTrasp>=(InitialCar))
    {
        TransMode = 1;
    }
	else if(RandTrasp<((InitialCar+InitialBike+InitialWalk))&&RandTrasp>((InitialCar+InitialBike)))
	{
		TransMode=2;
	}
	else if(RandTrasp<1&&RandTrasp>=((InitialCar+InitialBike+InitialWalk)))
	{
		TransMode=3;
	}
	TransModeUsage=1;
	//Initialising health variable using uniform distribution
	double newRand =  repast::Random::instance()->getGenerator("duni")->next();
	boost::math::beta_distribution<> vars(6,2);
	Health = boost::math::quantile(vars,newRand);

	//Initialising the Cycle Ability variable with a uniform distribution
	CycleAbility= repast::Random::instance()->getGenerator("duni")->next();	
	//Intialising the Economic Position using a uniform distribution (look up research in this I feel like it should be mid shifted in the sheffield region as more of average wealth than low income)
	EconomicPosition=  repast::Random::instance()->getGenerator("duni")->next();
}

Commuter::Commuter(repast::AgentId id, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition): id_(id), TravelDist(newTravelDist),safety(newSafe), thresh(newThresh), TransMode(newMode), Health(newHealth), CycleAbility(newCycleAbility), EconomicPosition(newEconomicPosition){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank,int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition){
    id_.currentRank(currentRank);
	TravelDist=newTravelDist;
    safety     = newSafe;
    thresh = newThresh;
    TransMode= newMode;
	Health=newHealth;
	CycleAbility=newCycleAbility;
	EconomicPosition=newEconomicPosition;
}

int Commuter::ChooseMode(int TransCost){
	double TimeCar;
	double TimeBike;
	double TimeWalk;
	double TimePTrans;
	double CTimeP;
	double BTimeP;
	double WTimeP;
	double PTTimeP;
	double CSafeP;
	double BSafeP;
	double WSafeP;
	double PTSafeP;
	double Cyclethresh;
	bool incBike;
	bool incCar;
	bool incPTrans;
	double PTransCost;
	double CarP;
	double CycleP;
	double WalkP;
	double PTransP;
	double Habit;
	int StartTransMode=TransMode;
	int CostP;
	int SafeP;
	int TimeP;

	//Calculating probability form habit (more likely to to stick to current transport method)
	Habit=(-1*exp(-0.2*TransModeUsage)+1)*0.25;

	thresh= (1- exp(-0.01*TravelDist)*Health);
	if(StartTransMode==1)	//Making the threshold lower if in the habit of cycling, only effecting it by maximum 10%
	{
		thresh= (1- exp(-0.01*TravelDist)*Health)-(Habit*(10/25));
	}
	Cyclethresh = thresh * CycleAbility;



	if(safety>thresh&&EconomicPosition>0.1&&CycleAbility!=0)
	{
		incBike=1;
	}
	else
	{
		incBike=0;
	}
	if(EconomicPosition>0.2)
	{
		incCar=1;
	}
	else
	{
		incCar=0;
	}
	if(EconomicPosition>0.1)
	{
		incPTrans=1;
	}
	else
	{
		incPTrans=0;
	}
	
	
	
	TimeCar= 1*TravelDist;
	TimePTrans= 1.5*TravelDist;
	TimeBike = 2*TravelDist;
	TimeWalk = 9*TravelDist;

	CTimeP= ((incCar/TimeCar)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	BTimeP= ((incBike/TimeBike)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	WTimeP= ((1/TimeWalk)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	PTTimeP= ((incPTrans/TimePTrans)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	
	double CycleSafety=safety;
	double WalkSafety=safety;
	double CarSafety=0.99;
	double PTransSafety=1;

	CSafeP= (incCar*CarSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	BSafeP= (incBike*CycleSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	WSafeP= (WalkSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	PTSafeP= (incPTrans*PTransSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	
	double CarCost = 2500+TravelDist*0.1*48*5*2;//based on cost of car and distance
	double CycleCost = 500;	//based on cost of bike
	double WalkCost = 1;
	if(TravelDist<5)
	{
		PTransCost = TransCost*48*5; ///Based on distance
	}
	else
	{
		PTransCost = TransCost*48*5; //need to include rising price with distance
	}
	
	double CCostP= ((incCar/CarCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double BCostP= ((incBike/CycleCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double WCostP= ((1/ WalkCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double PTCostP= ((incPTrans/PTransCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/WalkCost)));
	
	//Calculating the probabilities of choosing certain types of transport
	if(EconomicPosition<0.3) //if lower 30% of money is more important factor in the decision
	{
		CostP = 4;
		SafeP = 3;
		TimeP = 3;
	}
	else
	{
		CostP = 1;
		SafeP = 4;
		TimeP = 5;	
	}

	if(StartTransMode==0)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP)/10)*(1-Habit)+Habit;
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP)/10)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP)/10)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP)/10)*(1-Habit);
	}
	else if(StartTransMode==1)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP)/10)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP)/10)*(1-Habit)+Habit;
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP)/10)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP)/10)*(1-Habit);
	}
	else if(StartTransMode==2)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP)/10)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP)/10)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP)/10)*(1-Habit)+Habit;
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP)/10)*(1-Habit);
	}
	else if(StartTransMode==3)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP)/10)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP)/10)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP)/10)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP)/10)*(1-Habit)+Habit;
	}
		

	
	//std::cout<<id_<<"   Distance: "<<TravelDist<< "Activity Threshold: " << thresh << "Health: " << Health<<std::endl;
	double TransChoice=  repast::Random::instance()->getGenerator("duni")->next();

    if(CarP>TransChoice)
    {
        TransMode=0;
    }
    else if((CarP+CycleP)>TransChoice)
    {
        TransMode=1;
    }
	else if((CarP+CycleP+WalkP)>TransChoice)
	{
		TransMode=2;
	}
	else if((CarP+CycleP+WalkP+PTransP)>TransChoice)
	{
		TransMode=3;
	}

	if(StartTransMode==TransMode)
	{
		TransModeUsage++;
	}


    return TransMode;
	}

void Commuter::Travel(double Gsafety,int TransCost,repast::SharedContext<Commuter>* context, repast:: SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace){
	timestep++;
    std::vector<Commuter*> agentsToPlay;
    std::vector<Infrastructure*> Infrastruct;
	std::vector<Infrastructure*> InfinRange;
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Point<int> Ccenter(0,0);
    int CommuteDistance = ceil(space ->getDistance(agentLoc,Ccenter));  
	TravelDist=CommuteDistance;
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 3, true, agentsToPlay);
    repast::Moore2DGridQuery<Infrastructure> moore2DInfQuery(Infspace);
    moore2DInfQuery.query(center,CommuteDistance , true, Infrastruct);	

    double safetyPayoff     = 0;
    double threshPayoff = 0;
    int numAgentsPlay=0;
    int numInf=0;
    int XSearch[2]={};
    int YSearch[2]={};
    std::vector<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);
        repast::Point<int> otherPoint(otherLoc);
        //std::cout << " Agent " << id_ << " AT " << center << " PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT " << (*agentToPlay)->getId() << " AT " << otherPoint << std::endl;
        
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());
        numAgentsPlay++;
        agentToPlay++;
    }

	//std::cout <<"Hey old safety "<< id_ << " is " << safety;
	if(numAgentsPlay!=0)
	{
    		safety = safety*0.6+0.3*(safetyPayoff/(numAgentsPlay))+0.1*Gsafety;
	}
	else
	{
		safety = safety*0.8+0.2*Gsafety;
	}
	//std::cout <<"and my new is " << safety << " my threshold is " << thresh  << std::endl;
	//std::cout<<"Global safety is " << Gsafety <<" At: "<<timestep<< std::endl;

   //Calculating min and max coordinates for infrastructure search
   if(center.getX()>=Ccenter.getX())
   {
	XSearch[0]=Ccenter.getX();
	XSearch[1]=center.getX();
   }
   else
   {
	XSearch[0]=center.getX();
	XSearch[1]=Ccenter.getX();
   }

   if(center.getY()>=Ccenter.getY())
   {
	YSearch[0]=Ccenter.getY();
	YSearch[1]=center.getY();
   }
   else
   {
	YSearch[0]=center.getY();
	YSearch[1]=Ccenter.getY();
   }
	
	//std::cout<<id_<<"Xmin: "<<XSearch[0]<<"Xmax: "<<XSearch[1]<<"Ymin: "<<YSearch[0]<<" Ymax: "<<YSearch[1]<<"    "<<Ccenter.getX()<<"   "<<center.getX()<<std::endl;
   std::vector<Infrastructure*>::iterator Infrastr = Infrastruct.begin();
   while(Infrastr != Infrastruct.end()){
		std::vector<int> InfLoc;
		Infspace->getLocation((*Infrastr)->getId(), InfLoc);
		repast::Point<int> InfPoint(InfLoc);
		//std::cout<<timestep<<"  Distance: "<<CommuteDistance<<"   -WOW, infrastructure was found it is "<<	(*Infrastr)->getId() <<" at location " << InfPoint << " by "<< id_ <<" at "<< center <<std::endl;		
	//If infrastructure within commuting range bettween agent and commuting point 

if((InfPoint.getX()>=XSearch[0])&&(InfPoint.getX()<=XSearch[1])&&(InfPoint.getY()>=YSearch[0])&&(InfPoint.getY()<=YSearch[1]))
		{
			int InfReach = (*Infrastr)->getReach();	//finding area of range
			int InfArea  = PI*(InfReach)*(InfReach);
			if((*Infrastr)->getInfType()==0)
			{
				InfinRange.push_back(*Infrastr);
				double InfSafe = (*Infrastr)->getProvVar();
				int CommuteArea=(XSearch[1]-XSearch[0])*(YSearch[1]-YSearch[0]);
				if(CommuteArea!=0)
				{
					safety=(1-((InfArea/2)/CommuteArea))*safety+(((InfArea/2)/CommuteArea)*InfSafe);
				}
				else
				{
					safety=(1-(InfReach/TravelDist))*safety+((InfReach/TravelDist)*InfSafe);
				}
			//safetyPayoff = safetyPayoff +((*Infrastr) -> use(Infspace));
			}
			else if((*Infrastr)->getInfType()==1)
			{
				InfinRange.push_back(*Infrastr);
				CycleCost=CycleCost-((*Infrastr)->getProvVar()*48);
			}
			else if((*Infrastr)->getInfType()==2)
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1)
				{
					if(Health!=0)
					{
						InfinRange.push_back(*Infrastr);
						Health=Health*1.01;
					}
				}
			}
			else if((*Infrastr)->getInfType()==3)
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1)
				{
					InfinRange.push_back(*Infrastr);
					CycleAbility=CycleAbility*1.01;
					if(CycleAbility==0)
					{
						CycleAbility=0.05;
					}
				}
			}
		numInf++;
		}
	Infrastr++;
	}

	//Calling function to choose transport method
    ChooseMode(TransCost);
	
	//If cycling chosen use the infrastructure
	if(TransMode==1)
	{
		Infrastr = InfinRange.begin();
  		while(Infrastr != InfinRange.end()){
			(*Infrastr)-> use(Infspace);
			Infrastr++;
		}
	}

    
}


/*void Commuter::move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0]+1);
    agentNewLoc.push_back(agentLoc[1] +1);
    space->moveTo(id_,agentNewLoc);
    
}*/



