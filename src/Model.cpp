/* Demo_01_Model.cpp */

#include <stdio.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"

#include "Model.h"

CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initializeRandom(*props, comm);
	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");

}

CommuterModel::~CommuterModel(){
		delete props;
}

void CommuterModel::init(){
	int rank = repast::RepastProcess::instance()->rank();
	for(int i = 0; i < countOfAgents; i++){
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id);
		context.addAgent(agent);
	}
}

void CommuterModel::doSomething(){
	std::vector<Commuter*> agents;
	context.selectAgents(countOfAgents, agents);
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
		(*it)->commute(&context);
		it++;
    }
	
	NumCycle=0;
	NumCar=0;
	for(int r=0; r< 4; r++)
	{
		for(int i=0; i<countOfAgents;i++)
		{
			repast::AgentId toDisplay(i,r,0);
			Commuter* agent = context.getAgent(toDisplay);
			if(agent!=0)
			{	
				if(agent->getTrans()==1)
				{

				std::cout << agent->getId() << " is Cycling" <<std::endl;
				NumCycle++;
				}
				else
				{
					std::cout << agent->getId() << " is Driving" <<std::endl;
					NumCar++;
				}

			}
		}

	}
}

void CommuterModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::doSomething)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordResults)));
	runner.scheduleStop(stopAt);
}

void CommuterModel::recordResults(){
		std::vector<std::string> keyOrder;
		props->putProperty("NumCycle",NumCycle);
		props->putProperty("NumCar",NumCar);
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("NumCycle");
		keyOrder.push_back("NumCar");
		props->writeToSVFile("./output/CommutersEnd.csv", keyOrder);
}


