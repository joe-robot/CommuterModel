/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "CommuterModel.h"

CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initializeRandom(*props, comm);
	
    repast::Point<double> origin(-100,-100);
    repast::Point<double> extent(200, 200);
    
    repast::GridDimensions gd(origin, extent);
    
	//sets to run on just 1 core
    std::vector<int> processDims;
    processDims.push_back(1);
    processDims.push_back(1);
    
    discreteSpace = new repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >("AgentDiscreteSpace", gd, processDims, 2, comm);
	
    std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << " " << discreteSpace->bounds().extents() << std::endl;
    
   	context.addProjection(discreteSpace);
    

	
}

CommuterModel::~CommuterModel(){
	delete props;


}

void CommuterModel::init(){
	int rank = repast::RepastProcess::instance()->rank();
	for(int i = 0; i < countOfAgents; i++){
        repast::Point<int> initialLocation((int)discreteSpace->dimensions().origin().getX() + i,(int)discreteSpace->dimensions().origin().getY() + i);
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id);
		context.addAgent(agent);
        discreteSpace->moveTo(id, initialLocation);
	}
}



void CommuterModel::doSomething(){
	/*int whichRank = 0;
	if(repast::RepastProcess::instance()->rank() == whichRank) std::cout << " TICK " << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;

	if(repast::RepastProcess::instance()->rank() == whichRank){
		std::cout << "LOCAL AGENTS:" << std::endl;
		for(int r = 0; r < 4; r++){
			for(int i = 0; i < 10; i++){
				repast::AgentId toDisplay(i, r, 0);
				Commuter* agent = context.getAgent(toDisplay);
				if((agent != 0) && (agent->getId().currentRank() == whichRank)){
                    std::vector<int> agentLoc;
                    discreteSpace->getLocation(agent->getId(), agentLoc);
                    repast::Point<int> agentLocation(agentLoc);
                    std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << " AT " << agentLocation << std::endl;
                }
			}
		}
		
		std::cout << "NON LOCAL AGENTS:" << std::endl;
		for(int r = 0; r < 4; r++){
			for(int i = 0; i < 10; i++){
				repast::AgentId toDisplay(i, r, 0);
				Commuter* agent = context.getAgent(toDisplay);
				if((agent != 0) && (agent->getId().currentRank() != whichRank)){
                    std::vector<int> agentLoc;
                    discreteSpace->getLocation(agent->getId(), agentLoc);
                    repast::Point<int> agentLocation(agentLoc);
                    std::cout << agent->getId() << " " << agent->getC() << " " << agent->getTotal() << " AT " << agentLocation << std::endl;
                }
			}
		}
	}*/

	
	
	std::vector<Commuter*> agents;
	context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
        (*it)->play(&context, discreteSpace);
		it++;
    }

    it = agents.begin();
    while(it != agents.end()){
		(*it)->move(discreteSpace);
		it++;
    }

	discreteSpace->balance();
  
}

void CommuterModel::initSchedule(repast::ScheduleRunner& runner){

	runner.scheduleEvent(2, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::doSomething)));

	runner.scheduleStop(stopAt);

}

void CommuterModel::recordResults(){
	if(repast::RepastProcess::instance()->rank() == 0){
		props->putProperty("Result","Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("./output/results.csv", keyOrder);
    }
}


	
