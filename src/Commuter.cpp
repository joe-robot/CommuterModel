/* Demo_03_Agent.cpp */

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

Commuter::Commuter(repast::AgentId id, double newSafe, double newThresh, int newTranstype): id_(id), safety(newSafe), thresh(newThresh), Transtype(newTranstype){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank, double newSafe, double newThresh, int newTranstype){
    id_.currentRank(currentRank);
    safety     = newSafe;
    thresh = newThresh;
    Transtype= newTranstype;
}

int Commuter::choosetrans(){
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
    std::vector<int> agentLoc;
    space ->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 1, false, agentsToPlay);
    
    double safetyPayoff     = 0;
    double threshPayoff = 0;
    double numAgentsPlay=0;
    std::vector<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);
        repast::Point<int> otherPoint(otherLoc);
        std::cout << " Agent " << id_ << " AT " << center << " PLAYING " << ((*agentToPlay)->getId().currentRank() == id_.currentRank() ? "LOCAL" : "NON-LOCAL") << " AGENT " << (*agentToPlay)->getId() << " AT " << otherPoint << std::endl;
        
        safetyPayoff = safetyPayoff + ((*agentToPlay)->getSafe());
        numAgentsPlay++;
        agentToPlay++;
    }
    safety      = (safetyPayoff+safety)/numAgentsPlay;
    choosetrans();
    
}


void Commuter::move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}



