/* Demo_03_Agent.cpp */

#include "CommuterModel.h"
#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"
#include <boost/math/distributions/beta.hpp>	//Including the BETA sistribution from boost libraries

#define PI 3.141592653

Commuter::Commuter(repast::AgentId id, double InitialCar,double InitialBike, double InitialWalk, double InitialPTrans ): id_(id), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)
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
	if(TransMode==0)
	{
		EconomicPosition=  0.15+(repast::Random::instance()->getGenerator("duni")->next())*(0.85);
	}
	else if(TransMode==3)
	{
		EconomicPosition=  0.1+(repast::Random::instance()->getGenerator("duni")->next())*(0.85);
	}
	else
	{
		EconomicPosition=  (repast::Random::instance()->getGenerator("duni")->next())*(0.9);
	}

	//Intialising Saftey Perception making it skewed to 0.5
	newRand =  repast::Random::instance()->getGenerator("duni")->next();
	boost::math::beta_distribution<> vars2(3,2.1);
	safety = boost::math::quantile(vars2,newRand);
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

int Commuter::ChooseMode(double TransCost){
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
	double CComfP;
	double BComfP;
	double WComfP;
	double PTComfP;
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
	int ComfP;
	int WeightP;

	//Calculating probability form habit (more likely to to stick to current transport method)
	Habit=0.1+(-1*exp(-0.2*TransModeUsage)+1)*0.4;

	thresh= (1- exp(-0.01*TravelDist)*Health);
	if(StartTransMode==1)	//Making the threshold lower if in the habit of cycling, only effecting it by maximum 10%
	{
		thresh= (1- exp(-0.01*TravelDist)*Health)-(Habit*(10/50));
	}
	Cyclethresh = thresh * CycleAbility;



	if(safety>thresh&&CycleAbility!=0)
	{
		incBike=1;
	}
	else
	{
		incBike=0;
	}
	if(EconomicPosition>0.15)
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
	
	
	if(TravelDist<50)	//Within 5 Km (congestion areas)
	{
		TimeCar= 1*TravelDist;
		TimePTrans = 2*TravelDist;
	}
	else			//Outside of congetion areas
	{
		TimeCar= 0.5*(TravelDist-50)+50;	//Faster when out of 5Km radius by half (60mph)
		TimePTrans= 50*2+0.4*(TravelDist-50) +	(1.6*exp(-0.001*(TravelDist-50))*TravelDist);						//Gradual decrease in time taken per distance
	}

	TimeBike = 1.5*TravelDist;
	TimeWalk = 10*TravelDist;

	CTimeP= ((incCar/TimeCar)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	BTimeP= ((incBike/TimeBike)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	WTimeP= ((1/TimeWalk)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	PTTimeP= ((incPTrans/TimePTrans)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	
	double CycleSafety=safety;
	double WalkSafety=safety;
	double CarSafety=0.99;
	double PTransSafety=0.99;

	CSafeP= (incCar*CarSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	BSafeP= (incBike*CycleSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	WSafeP= (WalkSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	PTSafeP= (incPTrans*PTransSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	
	double CarCost = 1500+TravelDist*0.09*48*5*2;//based on cost of cheap car and insurance and distance
	double WalkCost = 1;
	if(TravelDist<50)		//In 5km range a sheffield season pass is used so flat rate not based on distance
	{
		PTransCost = TransCost*48; ///Based on sheffield weekly pass
	}
	else
	{
		PTransCost = TransCost*48+(TravelDist-50)*(TransCost)*48; //rising price with distance
	}
	
	double CCostP= ((incCar/CarCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double BCostP= ((incBike/CycleCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double WCostP= ((1/ WalkCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double PTCostP= ((incPTrans/PTransCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/WalkCost)));

	double CycleComf=	0.4;	//Comfort and conveninece provided by each transport method
	double WalkComf =	0.4;
	double PTransComf = 0.7;
	double CarComf= 0.95;

	CComfP =(incCar*CarComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	BComfP =(incCar*CycleComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	WComfP =(incCar*WalkComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	PTComfP =(incCar*PTransComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	
	
	//Calculating the probabilities of choosing certain types of transport
	if(EconomicPosition<0.3) //for lower 30%, money is more important factor in the decision
	{
		CostP = 4;
		SafeP = 3;
		TimeP = 3;
		ComfP = 2;
	}
	else	//otherwise time and comfort is seen as a more important factor
	{
		CostP = 1;
		SafeP = 4;
		TimeP = 5;	
		ComfP = 3;
		WeightP=CostP+SafeP+TimeP+ComfP;
	}

	if(StartTransMode==0)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit)+Habit;
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==1)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit)+Habit;
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==2)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit)+Habit;
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==3)
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit)+Habit;
	}


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

	if(StartTransMode==TransMode)	//if chosen same transmode again add to transmode usage variable
	{
		TransModeUsage++;
	}


    return TransMode;
	}

void Commuter::Travel(double Gsafety,double TransCost,repast::SharedContext<Commuter>* context, repast:: SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace){
	int InfReach = 0;
	int InfArea=0;
    std::vector<Commuter*> agentsToPlay;
    std::vector<Infrastructure*> Infrastruct;
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Point<int> Ccenter(0,0);
    int CommuteDistance = ceil(space ->getDistance(agentLoc,Ccenter));  
	double infAreasum=0;
	double infSafesum=0;
	double infRSafesum=0;
	double infReachsum=0;
	int CommuteArea=0;
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
        
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());
        numAgentsPlay++;
        agentToPlay++;
    }


	if(numAgentsPlay!=0)
	{
    		safety = safety*0.6+0.3*(safetyPayoff/(numAgentsPlay))+0.1*Gsafety;
	}
	else
	{
		safety = safety*0.8+0.2*Gsafety;
	}


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
	

  	std::vector<Infrastructure*>::iterator Infrastr = InfinRange.begin();
	while(Infrastr != InfinRange.end())									//Removing saftey of previously used infastructure to account for any removed
	{
		if((*Infrastr)->getInfType()==0)
		{	
			double InfSafe = (*Infrastr)->getOldProvVar();
			InfReach = (*Infrastr)->getReach();
			InfArea  = PI*(InfReach)*(InfReach);
			infAreasum=	infAreasum+(InfArea/2);		
			infSafesum=	infSafesum+(InfArea/2)*InfSafe;
			infRSafesum=infRSafesum+(InfReach/2)*InfSafe;
			infReachsum=infReachsum+(InfReach/2);
		}
		else if((*Infrastr)->getInfType()==1)
		{
			CycleCost=CycleCost+((*Infrastr)->getOldProvVar()*48);
		}
	}

	if(CommuteArea!=0)
		{
			safety=(infSafesum+((CommuteArea-InfArea)*safety))/CommuteArea;
			safety= (safety*CommuteArea-infSafesum)/(CommuteArea-InfArea);

		}
		else
		{
			safety=(infRSafesum+((TravelDist-infReachsum)*safety))/TravelDist;
			safety = (safety*TravelDist-infRSafesum)/(TravelDist-infReachsum);
		}
	
		InfReach = 0;	//resetting variables to be used for calculating saftey from current infrastructure
		InfArea  = 0;
		infAreasum=	0;		
		infSafesum=	0;
		infRSafesum=0;
		infReachsum=0;

	Infrastr = Infrastruct.begin();
   	while(Infrastr != Infrastruct.end()){
		std::vector<int> InfLoc;
		Infspace->getLocation((*Infrastr)->getId(), InfLoc);
		repast::Point<int> InfPoint(InfLoc);

	//If infrastructure within commuting range bettween agent and commuting point 

		if((InfPoint.getX()>=XSearch[0]) && (InfPoint.getX()<=XSearch[1]) && (InfPoint.getY()>=YSearch[0]) && (InfPoint.getY()<=YSearch[1]))
		{
			InfReach = (*Infrastr)->getReach();	//finding area of range
			InfArea  = PI*(InfReach)*(InfReach);
			if((*Infrastr)->getInfType()==0&&((*Infrastr)->getProvVar())>safety)	//if saftey infrastructure and provided saftey bigger than agent current safety perception
			{
				InfinRange.push_back(*Infrastr);
				double InfSafe = (*Infrastr)->getProvVar();
				CommuteArea=(XSearch[1]-XSearch[0])*(YSearch[1]-YSearch[0]);
				infAreasum=	infAreasum+(InfArea/2);		
				infSafesum=	infSafesum+(InfArea/2)*InfSafe;
				infRSafesum=infRSafesum+(InfReach/2)*InfSafe;
				infReachsum=infReachsum+(InfReach/2);
	
				
			}
			else if((*Infrastr)->getInfType()==1)
			{
				InfinRange.push_back(*Infrastr);
				CycleCost=CycleCost-((*Infrastr)->getProvVar()*12);
			}
			else if((*Infrastr)->getInfType()==2)
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1)
				{
					if(Health!=0)
					{
						InfinRange.push_back(*Infrastr);
						if(Health<1)
						{
							Health=Health*(*Infrastr)->getProvVar();
							if(Health>1)
							{
								Health=1;
							}
						}
					}
				}
			}
			else if((*Infrastr)->getInfType()==3)
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1)
				{
					InfinRange.push_back(*Infrastr);
					if(CycleAbility!=1)
					{
						CycleAbility=CycleAbility*(*Infrastr)->getProvVar();;
					}
					if(CycleAbility<0.05)
					{
						CycleAbility=0.05;
					}
				}
			}
		numInf++;
		}
	Infrastr++;
	}
	if(InfArea!=0&&infSafesum!=0)
	{
		if(CommuteArea!=0)
		{
			safety=(infSafesum+((CommuteArea-InfArea)*safety))/CommuteArea;

		}
		else
		{
			safety=(infRSafesum+((TravelDist-infReachsum)*safety))/TravelDist;
		}

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
		if(Health<=1)
		{
			Health=Health*1.04;	//Health increase if chosen cycling
			if(Health>1)
			{
				Health=1;
			}
		}
	}
	if(TransMode==2)
	{
		if(Health<=1)
		{
			Health=Health*1.02;	//Health increas if chosen walking
			if(Health>1)
			{
				Health=1;
			}
		}
	}

    timestep++;
}


