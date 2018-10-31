/* Demo_01_Agent.cpp */

#include "Commuter.h"

Commuter::Commuter(repast::AgentId id): id_(id), safety(repast::Random::instance()->nextDouble()), thresh(((repast::Random::instance()->nextDouble())/2)+0.5){ }

Commuter::Commuter(repast::AgentId id, double newSafe, double newThresh): id_(id), safety(newSafe), thresh(newThresh){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank, double newSafe, double newThresh){
    id_.currentRank(currentRank);
    safety     = newSafe;
    thresh = newThresh;
}

bool Commuter::choosetrans(){
	return thresh<safety;
}

void Commuter::commute(repast::SharedContext<Commuter>* context){
    std::set<Commuter*> agentsToPlay;
	
    agentsToPlay.insert(this); // Prohibit playing against self
	
    context->selectAgents(3, agentsToPlay, true);
	
    double safetyPayoff     = 0;
    double threshPayoff = 0;
    std::set<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
      
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());
		
        agentToPlay++;
    }
    safety      = (safetyPayoff+safety)/4;
    if(choosetrans())
	{
		std::cout<<"Look ma I'm cycling" << id_ <<std::endl;
		
	}
	else
	{
	std::cout<<"What is cycle ??? " << id_ <<std::endl;
	}
	
}


/* Serializable Agent Package Data */

CommuterPackage::CommuterPackage(){ }

CommuterPackage::CommuterPackage(int _id, int _rank, int _type, int _currentRank, double _safety, double _thresh):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), safety(_safety), thresh(_thresh){ }
