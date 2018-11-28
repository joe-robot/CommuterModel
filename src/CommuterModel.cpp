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
	timeinsteps=0;
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
	timeinsteps++;
	int Transtype;	
	std::vector<Commuter*> agents;
	context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
        (*it)->commute(&context, discreteSpace);
		it++;
    }

 	it = agents.begin();
    while(it != agents.end()){
		Transtype= (*it)->getTrans();
		if(Transtype ==1)
		{
			std::cout<<"just another cyle boi " << (*it)->getId() << " at "<< timeinsteps << std::endl;
		}
		else
		{
			std::cout<<"just another drivey man" << std::endl;
		}
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

	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::doSomething)));

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


	
