/* Commuter.cpp */

//By Joseph Cresswell	Reg No. 150148395

//Including required files
#include "CommuterModel.h"
#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"
#include <boost/math/distributions/beta.hpp>	//Including the BETA sistribution from boost libraries

#define PI 3.141592653	//defining pi for reach area calculations (as circular)

Commuter::Commuter(repast::AgentId id, double InitialCar,double InitialBike, double InitialWalk, double InitialPTrans ): id_(id), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)	//Constructor for creating a commuter agent
{

	//Initialising agent trasport mode randomly
	double RandTrasp=repast::Random::instance()->getGenerator("duni")->next();	//Randomly selecting transport mode using a unifrom random number and proportions given on intialising the agent
    if(RandTrasp<(InitialCar)&&RandTrasp>=0)
    {
        TransMode= 0;	//Agent is a driver
    }
    else if(RandTrasp<((InitialCar+InitialBike))&&RandTrasp>=(InitialCar))
    {
        TransMode = 1;	//Agent is a cyclist
    }
	else if(RandTrasp<((InitialCar+InitialBike+InitialWalk))&&RandTrasp>((InitialCar+InitialBike)))
	{
		TransMode=2;	//Agent is a walker
	}
	else if(RandTrasp<1&&RandTrasp>=((InitialCar+InitialBike+InitialWalk)))
	{
		TransMode=3;	//Agent uses public transport
	}
	TransModeUsage=1;
	//Initialising health variable using beta distribution and cycle abiliity
	double newRand =  repast::Random::instance()->getGenerator("duni")->next();
	boost::math::beta_distribution<> vars(6,2);	//defining beta distribution for health
	if(TransMode==1) //if chosen cycling health generally higher and cycling ability better
	{
		CycleAbility= 0.2+(repast::Random::instance()->getGenerator("duni")->next())*0.8; //Allocating cycle ability (minimum 0.2 as already a cyclist)

	}
	else
	{
		CycleAbility= (repast::Random::instance()->getGenerator("duni")->next())*0.9;//Allcoating cycle ability

	}


	Health = boost::math::quantile(vars,newRand); //Allocating health using beta distribution

	//Initialising the Cycle Ability variable with a uniform distribution
	CycleAbility= repast::Random::instance()->getGenerator("duni")->next();	
	//Intialising the Economic Position using a uniform distribution 
	if(TransMode==0)
	{
		EconomicPosition=  0.15+(repast::Random::instance()->getGenerator("duni")->next())*(0.85);	//If driving min eco positon is 0.15
	}
	else if(TransMode==3)
	{
		EconomicPosition=  0.1+(repast::Random::instance()->getGenerator("duni")->next())*(0.85); // if using public transport min eco position is 0.1
	}
	else
	{
		EconomicPosition=  (repast::Random::instance()->getGenerator("duni")->next())*(0.9);	//Otherwise use uniform distribution maximum of 0.9
	}

	//Intialising safety Perception making it skewed to 0.5
	newRand =  repast::Random::instance()->getGenerator("duni")->next();
	boost::math::beta_distribution<> vars2(3,2.1);	//Allocating intial safety using a beta distribution
	safety = boost::math::quantile(vars2,newRand);
}

Commuter::Commuter(repast::AgentId id, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition): id_(id), TravelDist(newTravelDist),safety(newSafe), thresh(newThresh), TransMode(newMode), Health(newHealth), CycleAbility(newCycleAbility), EconomicPosition(newEconomicPosition){ } //Can initialise agent by defining all parameters immediatly

Commuter::~Commuter(){ } //Commuter destructor


void Commuter::set(int currentRank,int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition){	//Can set new paramters using this function
    id_.currentRank(currentRank);
	TravelDist=newTravelDist;
    safety     = newSafe;
    thresh = newThresh;
    TransMode= newMode;
	Health=newHealth;
	CycleAbility=newCycleAbility;
	EconomicPosition=newEconomicPosition;
}

int Commuter::ChooseMode(double TransCost){	//Funciton for agents to choose transport mode (called by the travel function)
	//Defining variables for calculations
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
	int StartTransMode=TransMode; //recording current transport mode for habit calculation
	int CostP;
	int SafeP;
	int TimeP;
	int ComfP;
	int WeightP;

	//Calculating probability form habit (more likely to to stick to current transport method)
	Habit=0.1+(-1*exp(-0.2*TransModeUsage)+1)*0.3;	//Based on how many times used current transport mode increase habit

	thresh= (1- exp(-0.01*TravelDist)*Health);	//Calculating threshold as funciton fo distance and health
	if(StartTransMode==1)	//Making the threshold lower if in the habit of cycling, only effecting it by maximum 10%
	{
		thresh= (1- exp(-0.01*TravelDist)*Health)-(Habit*(10/40));
	}
	Cyclethresh = thresh * CycleAbility; //calcualting cycling threshold to include cycleability



	if(safety>thresh&&CycleAbility!=0)	//If safety above threshold and cycle ability not 0 - include cycling as an option
	{
		incBike=1;
	}
	else
	{
		incBike=0;
	}
	if(EconomicPosition>0.15)	//If economic position bigger than 0.15 include driving as an option
	{
		incCar=1;
	}
	else
	{
		incCar=0;
	}
	if(EconomicPosition>0.1)	//If economic position bigger than 0.1 then include public tranport as an option
	{
		incPTrans=1;
	}
	else
	{
		incPTrans=0;
	}
	
///Commute time calculations	

	if(TravelDist<50)	//Within 5 Km (congestion areas)
	{
		TimeCar= 1*TravelDist; //time to drive is given as travel distance
		TimePTrans = 2*TravelDist;	//time to use public tranport is double that of a car
	}
	else			//Outside of congetion areas
	{
		TimeCar= 0.5*(TravelDist-50)+50;	//Faster when out of 5Km radius by half (60mph)
		TimePTrans= 50*2+0.4*(TravelDist-50) +	(1.6*exp(-0.001*(TravelDist-50))*TravelDist);						//Gradual decrease in time taken per distance
	}

	TimeBike = 1.5*TravelDist;	//time to cycle takes 1.5 times as long as a car (15mph)
	TimeWalk = 10*TravelDist;	//Walking takes 10 times as long as cycling

//Calculate probabilities of using each transport mode based on time (larger time means lower probabiility)
	CTimeP= ((incCar/TimeCar)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	BTimeP= ((incBike/TimeBike)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	WTimeP= ((1/TimeWalk)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));
	PTTimeP= ((incPTrans/TimePTrans)/((incCar/TimeCar)+(incPTrans/TimePTrans)+(incBike/TimeBike)+(1/TimeWalk)));

///Commute safety calcualtions
	
	double CycleSafety=safety;	//Safety given by model and estimates
	double WalkSafety=safety;
	double CarSafety=0.99;
	double PTransSafety=0.99;
//Calculate probabilities of using each transport mode based on safety provided (larger safety means higher probabiility)
	CSafeP= (incCar*CarSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	BSafeP= (incBike*CycleSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	WSafeP= (WalkSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));
	PTSafeP= (incPTrans*PTransSafety)/((incCar*CarSafety)+(incPTrans*PTransSafety)+(incBike*CycleSafety)+(WalkSafety));

////Commute cost calculations
	
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
//Calculate probabilities of using each transport mode based on cost (larger cost means lower probabiility)
	double CCostP= ((incCar/CarCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double BCostP= ((incBike/CycleCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double WCostP= ((1/ WalkCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/ WalkCost)));
	double PTCostP= ((incPTrans/PTransCost)/((incCar/CarCost)+(incPTrans/PTransCost)+(incBike/CycleCost)+(1/WalkCost)));

////Commute Comfort calculations

	double CycleComf=	0.4;	//Comfort and conveninece provided by each transport method
	double WalkComf =	0.4;
	double PTransComf = 0.7;
	double CarComf= 0.95;
//Calculate probabilities of using each transport mode based on comfort (more comfort means higher probabiility)
	CComfP =(incCar*CarComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	BComfP =(incCar*CycleComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	WComfP =(incCar*WalkComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	PTComfP =(incCar*PTransComf)/((incCar*CarComf)+(incPTrans*PTransComf)+(incBike*CycleComf)+(WalkComf));
	
	
	//Calculating the probabilities of choosing certain types of transport
	if(EconomicPosition<0.3) //for lower 30%, money is more important factor in the decision
	{
		CostP = 4;
		SafeP = 4;
		TimeP = 3;
		ComfP = 2;
	}
	else	//otherwise time and comfort is seen as a more important factor
	{
		CostP = 1;
		SafeP = 4;
		TimeP = 5;	
		ComfP = 3;
		
	}
	WeightP=CostP+SafeP+TimeP+ComfP;

	//Calculating probabilities of using each transport mode (including habit on currently used mode)
	if(StartTransMode==0)	//if currently driving
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit)+Habit;
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==1) //if currently cycling
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit)+Habit;
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==2)	//if currently walking
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit)+Habit;
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit);
	}
	else if(StartTransMode==3)	//if currently using public tranport
	{
		CarP=((CostP*CCostP+SafeP*CSafeP+TimeP*CTimeP+ComfP*CComfP)/WeightP)*(1-Habit);
		CycleP=((CostP*BCostP+SafeP*BSafeP+TimeP*BTimeP+ComfP*BComfP)/WeightP)*(1-Habit);
		WalkP=((CostP*WCostP+SafeP*WSafeP+TimeP*WTimeP+ComfP*WComfP)/WeightP)*(1-Habit);
		PTransP=((CostP*PTCostP+SafeP*PTSafeP+TimeP*PTTimeP+ComfP*PTComfP)/WeightP)*(1-Habit)+Habit;
	}


	double TransChoice=  repast::Random::instance()->getGenerator("duni")->next();	//Decide transport choice using uniform random number

//Setting new commuter mode
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

	if(StartTransMode==TransMode)	//if chosen same transmode again add to transmode usage variable to show habit
	{
		TransModeUsage++;
	}


    return TransMode;
	}

void Commuter::Travel(double Gsafety,double TransCost,repast::SharedContext<Commuter>* context, repast:: SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace){ //Function for agents to commute
	int InfReach = 0;
	int InfArea=0;
    std::vector<Commuter*> agentsToPlay;		//Vectors for storing agents in search range
    std::vector<Infrastructure*> Infrastruct;
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);		//getting location of current agent
    repast::Point<int> center(agentLoc);
    repast::Point<int> Ccenter(0,0);		//Setting commute location as the centre
    int CommuteDistance = ceil(space ->getDistance(agentLoc,Ccenter)); //Getting distance of agent to centre (rounding up)
	//Initialising variables for calculating infrastructure effect
	double infAreasum=0;
	double infSafesum=0;
	double infRSafesum=0;
	double infReachsum=0;
	double CommuteArea=0;
	double InfraSafety=safety;
	double SocialSafety=safety;
	//Setting travel distance as distance to centre
	TravelDist=CommuteDistance;
	//Setting up grid query for Commuters
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 3, true, agentsToPlay);	//Searching within 3 wide circle for other agents
	//Setting up grid query for infrastructure agents
    repast::Moore2DGridQuery<Infrastructure> moore2DInfQuery(Infspace);
    moore2DInfQuery.query(center,CommuteDistance , true, Infrastruct);	//Searching using commute distance as the range

    double safetyPayoff     = 0;
    double threshPayoff = 0;
    int numAgentsPlay=0;
    int numInf=0;
    int XSearch[2]={};
    int YSearch[2]={};
    

   //Calculating min and max coordinates for infrastructure search (as only searching in rectangle where linear distance from agent to centre is the diagonal)
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
	
  		std::vector<Infrastructure*>::iterator Infrastr = InfinRange.begin(); //iterating through all previously found infrastructure
		while(Infrastr != InfinRange.end())				//Removing effects of economic subsidy as may no longer be avalible
//Probelem comes when the safety has decreased 
		{
			if((*Infrastr)->getInfType()==1)
			{
				CycleCost=CycleCost+((*Infrastr)->getOldProvVar()*48);
			}
			Infrastr++;
		}


	InfinRange.clear();		//Clearing infrastructure vector to ensure there aren't duplicates in the vector
	Infrastr = Infrastruct.begin();			//Iterating through all infrastructure found in range
   	while(Infrastr != Infrastruct.end()){
		std::vector<int> InfLoc;	//Getting location of infrastructure
		Infspace->getLocation((*Infrastr)->getId(), InfLoc);
		repast::Point<int> InfPoint(InfLoc);

	//If infrastructure within commuting range bettween agent and commuting point 

		if((InfPoint.getX()>=XSearch[0]) && (InfPoint.getX()<=XSearch[1]) && (InfPoint.getY()>=YSearch[0]) && (InfPoint.getY()<=YSearch[1]))	//If infrastructure in range of rectangle search area then include in calculations
		{
			InfReach = (*Infrastr)->getReach();	//finding range of infrastructure
			InfArea  = PI*(InfReach)*(InfReach);	//Getting area covered by infrastructure
			if((*Infrastr)->getInfType()==0&&((*Infrastr)->getProvVar())>InfraSafety)	//if safety infrastructure and provided safety bigger than agent current safety perception then consider using it
			{
				InfinRange.push_back(*Infrastr);	//Calculating how much are all the safety infrastructure covers (/2 to account for overlapping and out of range range)
				double InfSafe = (*Infrastr)->getProvVar();	//Getting provided safety of agent
				CommuteArea=(XSearch[1]-XSearch[0])*(YSearch[1]-YSearch[0]);
				infAreasum=	infAreasum+(InfArea/2);		
				infSafesum=	infSafesum+(InfArea/2)*InfSafe;
				infRSafesum=infRSafesum+(InfReach/2)*InfSafe;
				infReachsum=infReachsum+(InfReach/2);
	
				
			}
			else if((*Infrastr)->getInfType()==1)	//If economic subsidising agent
			{
				if(EconomicPosition<0.6)	//If in lower economic position then use economic subsidies
				{
					InfinRange.push_back(*Infrastr);
					CycleCost=CycleCost-((*Infrastr)->getProvVar()*12);
				}
			}
			else if((*Infrastr)->getInfType()==2)	//If health infrastructure
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1&&Health<0.9)		//10% chance of using health infrastructure and won't use if above 90% health
				{
					if(Health!=0)	//If health not 0
					{
						InfinRange.push_back(*Infrastr);
						if(Health<1)	//If healht no 1
						{
							Health=Health*(1+((*Infrastr)->getProvVar())*exp(-1*Health));//increase health with larger effect with lower current health
							if(Health>1)
							{
								Health=1;	//Make sure max health is 1
							}
						}
						(*Infrastr)->use(Infspace);		//Use health infrastructure (Don't need to be a cyclist alredy to use this)
					}
				}
			}
			else if((*Infrastr)->getInfType()==3)	//If cycle class infrastructure
			{
				if(repast::Random::instance()->getGenerator("duni")->next()<0.1&&CycleAbility<0.9)		//10% chance of taking up lessons and won't use if above 0.9 ability
				{
					InfinRange.push_back(*Infrastr);
					if(CycleAbility<=1)	//If not already the best at cycling
					{
						CycleAbility=CycleAbility*(*Infrastr)->getProvVar();	//Cycle ability is increased 
					}
					if(CycleAbility<0.05)//if cycle ability very small make it 5%
					{
						CycleAbility=0.05;
					}
					(*Infrastr)->use(Infspace);	// Use cycling infrastructure, don't need to be a cyclist already to use this
				}
			}
		numInf++;
		}
	Infrastr++;
	}
	if((infSafesum*infRSafesum)!=0&&InfinRange.size()!=0)	//If safety infrastrucute total safety not 0
	{
		if(CommuteArea!=0)	//if commuter area bigger than zero
		{
			if(infAreasum>CommuteArea)	//if infrastructure covers whole area
			{
				InfraSafety=(infSafesum/infAreasum); //Make safety due to infrastructure entirely governed by infrastructure
			}
			else	//Otherwise make saftey due to infrastructure improve the safety based on the percentage of the commuter area it covers
			{
				InfraSafety=(infSafesum+((CommuteArea-infAreasum)*InfraSafety))/CommuteArea;
			}

		}
		else if(TravelDist!=0)	//If commuter area is 0 (horizontal or verticle line to centre)
		{
			if(infReachsum>TravelDist)	//if infrastructure covers whole area
			{
				InfraSafety=(infRSafesum/infReachsum);//Make safety due to infrastructure entirely governed by infrastructure
			}
			else
			{
				InfraSafety=(infRSafesum+((TravelDist-infReachsum)*InfraSafety))/TravelDist; //Otherwise make saftey due to infrastructure improve the safety based on the percentage of the commuter range it covers
			}
		}

	}
	

	//Communicatiing with nearby agents and reciving global safety to influence safety perception
	std::vector<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){	//Iterating through all agents in social range
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);	//Get location of agent
        repast::Point<int> otherPoint(otherLoc);
        
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());	//Get safety perception of agent
        numAgentsPlay++;
        agentToPlay++;
    }


	if(numAgentsPlay>0) //if there are agents in range than use it to calculate safety perception
	{
    		SocialSafety = SocialSafety*0.7+0.2*(safetyPayoff/(numAgentsPlay))+0.1*Gsafety;
	}
	else	//If no agents in range then only use global safety to calculate socail safety perception
	{
		SocialSafety = SocialSafety*0.8+0.2*Gsafety;
	}

	//Finding new saftey from average of safety from other agents and from infrastruture
	safety=(InfraSafety+SocialSafety)/2;	//Social safety perception accounts for 50% of safety perception and Infrastrucure safety accounts for the other half

	//Calling function to choose transport method
    ChooseMode(TransCost);
	
	//If cycling chosen use the infrastructure
	if(TransMode==1)
	{
		Infrastr = InfinRange.begin();
  		while(Infrastr != InfinRange.end()){
			if(((*Infrastr)->getInfType())==0||((*Infrastr)->getInfType())==1)
			{
				(*Infrastr)-> use(Infspace);	//Use safety and economic cycling infrastructure found
				
			}
			Infrastr++;
		}
		if(Health<=1)	//increasing health due to chosing active transport  mode
		{
			Health=Health*(1+(0.04*exp(-2*Health)));	//Health increase if chosen cycling larger effect on those with lower health
			if(Health>1)
			{
				Health=1;
			}
		}
	}
	else if(TransMode==2)	//if walker
	{
		if(Health<=1)	//Increase health as active transport mode chosen
		{
			Health=Health*(1+(0.02*exp(-2*Health)));	//Health increas if chosen walking
			if(Health>1)
			{
				Health=1;
			}
		}
	}
	else if(TransMode==0)	//If driving
		if(Health<=1&&Health>=0.5) //Decrease health by small percentage if drove on commute
		{
			Health=Health*(1-(0.003));
		}
		else if(Health<0.5)
		{
         	Health=Health;
		}



    timestep++;
}


