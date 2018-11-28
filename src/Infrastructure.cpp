/* Infrastructure.cpp */

#include "Infrastructure.h"
#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Point.h"

Infrastructure::Infrastructure(repast::AgentId id): id_(id), safety(repast::Random::instance()->nextDouble()), thresh(((repast::Random::instance()->nextDouble())/2)+0.5)
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

Infrastructure::Infrastructure(repast::AgentId id, double newSafe, double newThresh, int newTranstype): id_(id), safety(newSafe), thresh(newThresh), Transtype(newTranstype){ }

Infrastructure::~Infrastructure(){ }


void Infrastructure::set(int currentRank, double newSafe, double newThresh, int newTranstype){
    id_.currentRank(currentRank);
    safety     = newSafe;
    thresh = newThresh;
    Transtype= newTranstype;
}


void Infrastructure::commute(double Gsafety,repast::SharedContext<Infrastructure>* context, repast:: SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){
    
}


void Infrastructure::move(repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* space){

    std::vector<int> agentLoc;
    space->getLocation(id_, agentLoc);
    std::vector<int> agentNewLoc;
    agentNewLoc.push_back(agentLoc[0] + (id_.id() < 7 ? -1 : 1));
    agentNewLoc.push_back(agentLoc[1] + (id_.id() > 3 ? -1 : 1));
    space->moveTo(id_,agentNewLoc);
    
}



