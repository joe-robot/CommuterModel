/* Demo_03_Agent.cpp */

#include "Commuter.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Commuter::Commuter(repast::AgentId id): id_(id), c(100), total(200){ }

Commuter::Commuter(repast::AgentId id, double newC, double newTotal): id_(id), c(newC), total(newTotal){ }

Commuter::~Commuter(){ }


void Commuter::set(int currentRank, double newC, double newTotal){
    id_.currentRank(currentRank);
    c     = newC;
    total = newTotal;
}

bool Commuter::cooperate(){
	return repast::Random::instance()->nextDouble() < c/total;
}

void Commuter::play(repast::SharedContext<Commuter>* context,
                              repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){
    std::vector<Commuter*> agentsToPlay;
    
    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    repast::Point<int> center(agentLoc);
    repast::Moore2DGridQuery<Commuter> moore2DQuery(space);
    moore2DQuery.query(center, 1, false, agentsToPlay);
    
    
    double cPayoff     = 0;
    double totalPayoff = 0;
    std::vector<Commuter*>::iterator agentToPlay = agentsToPlay.begin();
    while(agentToPlay != agentsToPlay.end()){
        std::vector<int> otherLoc;
        space->getLocation((*agentToPlay)->getId(), otherLoc);
        repast::Point<int> otherPoint(otherLoc);
        std::cout << " AGENT " << id_ << " AT " << center << " PLAYING " << ((*agentToPlay)->getId().currentRank()) << (*agentToPlay)->getId() << " AT " << otherPoint << std::endl;

        bool iCooperated = cooperate();                          // Do I cooperate?
        double payoff = (iCooperated ?
						 ((*agentToPlay)->cooperate() ?  7 : 1) :     // If I cooperated, did my opponent?
						 ((*agentToPlay)->cooperate() ? 10 : 3));     // If I didn't cooperate, did my opponent?
        if(iCooperated) cPayoff += payoff;
        totalPayoff             += payoff;
		
        agentToPlay++;
    }
    c      += cPayoff;
    total  += totalPayoff;
	
}

void Commuter::move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}



