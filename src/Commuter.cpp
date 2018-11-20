/* Demo_01_Agent.cpp */

#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Commuter::Commuter(repast::AgentId id): id_(id), safety(repast::Random::instance()->nextDouble()), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)
{
	if(repast::Random::instance()->nextDouble()<0.5)
	{
		Transtype= 1;
	} 
	else
	{
		Transtype = 0;
	}

}

Commuter::Commuter(repast::AgentId id, double newSafe, double newThresh, bool newTranstype): id_(id), safety(newSafe), thresh(newThresh), Transtype(newTranstype){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank, double newSafe, double newThresh, bool newTranstype){
    id_.currentRank(currentRank);
    safety     = newSafe;
    thresh = newThresh;
    Transtype= newTranstype;
}

bool Commuter::choosetrans(){
	if(thresh<safety)
	{	
		Transtype=1;
	}
	else
	{	
		Transtype=0;
	}
	return Transtype;
}

void Commuter::commute(repast::SharedContext<Commuter>* context, repast:: SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){
    std::vector<Commuter*> agentsToPlay;
    //std::set<Commuter*> agentsToPlay;
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 1, false, agentsToPlay);
	
    //agentsToPlay.insert(this); // Prohibit playing against self
	
    //context->selectAgents(3, agentsToPlay, true);
	
    double safetyPayoff     = 0;
    double threshPayoff = 0;
    std::vector<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);
        repast::Point<int> otherPoint(otherLoc);
        std::cout << " Agent " << id_ << " AT " << center << " PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT " << (*agentToPlay)->getId() << " AT " << otherPoint << std::endl;
        
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());
		
        agentToPlay++;
    }
    safety      = (safetyPayoff+safety)/4;
    choosetrans())
	/*{
		std::cout<<"Look ma I'm cycling" << id_ <<std::endl;
		
	}
	else
	{
	std::cout<<"What is cycle ??? " << id_ <<std::endl;
	}*/
	
}



void Commuter::move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}



/* Serializable Agent Package Data */

CommuterPackage::CommuterPackage(){ }

CommuterPackage::CommuterPackage(int _id, int _rank, int _type, int _currentRank, double _safety, double _thresh, bool _Transtype):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), safety(_safety), thresh(_thresh), Transtype(_Transtype){ }
