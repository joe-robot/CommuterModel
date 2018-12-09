/* Demo_03_Agent.cpp */

#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Commuter::Commuter(repast::AgentId id): id_(id), safety(repast::Random::instance()->nextDouble()), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)
{
    if(repast::Random::instance()->nextDouble()<0.5)
    {
        TransMode= 1;
    }
    else
    {
        TransMode = 0;
    }
    
}

Commuter::Commuter(repast::AgentId id, double newSafe, double newThresh, int newMode): id_(id), safety(newSafe), thresh(newThresh), TransMode(newMode){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank, double newSafe, double newThresh, int newMode){
    id_.currentRank(currentRank);
    safety     = newSafe;
    thresh = newThresh;
    TransMode= newMode;
}

int Commuter::ChooseMode(){
    if(thresh<safety)
    {
        TransMode=1;
    }
    else
    {
        TransMode=0;
    }
    return TransMode;
}

void Commuter::Travel(double Gsafety,repast::SharedContext<Commuter>* context, repast:: SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace){
	timestep++;
    std::vector<Commuter*> agentsToPlay;
    std::vector<Infrastructure*> Infrastruct;
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
     repast::Point<int> Ccenter(0,0);
    int CommuteDistance = ceil(space ->getDistance(agentLoc,Ccenter));  
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 1, false, agentsToPlay);
    repast::Moore2DGridQuery<Infrastructure> moore2DInfQuery(Infspace);
    moore2DInfQuery.query(center,CommuteDistance , true, Infrastruct);	
    
    double safetyPayoff     = 0;
    double threshPayoff = 0;
    int numAgentsPlay=0;
    int numInf=0;
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
   std::vector<Infrastructure*>::iterator Infrastr = Infrastruct.begin();
   while(Infrastr != Infrastruct.end()){
		std::vector<int> InfLoc;
		Infspace->getLocation((*Infrastr)->getId(), InfLoc);
		repast::Point<int> InfPoint(InfLoc);
		std::cout<<timestep<<"  Distance: "<<CommuteDistance<<"   -WOW, infrastructure was found it is "<<	(*Infrastr)->getId() <<" at location " << InfPoint << " by "<< id_ <<" at "<< center <<std::endl;	
		if(
		safetyPayoff = safetyPayoff +((*Infrastr) -> use(Infspace));
		numInf++;
		Infrastr++;
	}
	//std::cout <<"Hey old safety "<< id_ << " is " << safety;
	if(numAgentsPlay!=0)
	{
    		safety = (safety+((safetyPayoff/(numAgentsPlay))/2)+(Gsafety/2))/(2);
	}
	else
	{
		safety = (safety+(Gsafety/2))/(1.5);
	}
	 //std::cout <<"and my new is " << safety << " my threshold is " << thresh  << std::endl;
	//std::cout<<"Global safety is " << Gsafety << std::endl;
    ChooseMode();
    
}


/*void Commuter::move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0]+1);
    agentNewLoc.push_back(agentLoc[1] +1);
    space->moveTo(id_,agentNewLoc);
    
}*/



