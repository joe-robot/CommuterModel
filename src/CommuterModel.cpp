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
	Gsafety=0;
	
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
     	// Data collection
	// Create the data set builder
	std::string fileOutputName("./output/EndSimCommuterData.csv");
	repast::SVDataSetBuilder builder(fileOutputName.c_str(), ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
	
	// Create the individual data sets to be added to the builder
	DataSource_AgentTotalCars* AgentTotalCars_DataSource = new DataSource_AgentTotalCars(&context);
	builder.addDataSource(createSVDataSource("Total Cars", AgentTotalCars_DataSource, std::plus<int>()));
    
	DataSource_AgentTotalBikes* AgentTotalBikes_DataSource = new DataSource_AgentTotalBikes(&context);
	builder.addDataSource(createSVDataSource("Total Cyclists", AgentTotalBikes_DataSource, std::plus<int>()));
    
	// Use the builder to create the data set
	agentValues = builder.createDataSet();

	
}

CommuterModel::~CommuterModel(){
	delete props;

	delete agentValues;
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
		if(Gsafety==0)
		{
			Gsafety =(*it)->getSafe();
		}
		else
		{
        	Gsafety = (Gsafety + (*it)->getSafe())/2;
		}
		it++;
   	 }


	it = agents.begin();
	while(it != agents.end()){
        (*it)->commute(Gsafety,&context, discreteSpace);
		it++;
    }

 	it = agents.begin();
    while(it != agents.end()){
		Transtype= (*it)->getTrans();
		if(Transtype ==1)
		{
			std::cout<<"just another cyleist " << (*it)->getId() << " at "<< timeinsteps << std::endl;
		}
		else
		{
			std::cout<<"just another driver" << std::endl;
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


	//Data Collection
	runner.scheduleEvent(1.5, 5, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEvent(10.6, 10, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));


	
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



DataSource_AgentTotalCars::DataSource_AgentTotalCars(repast::SharedContext<Commuter>* car) : context(car){ }

int DataSource_AgentTotalCars::getData(){
	double sum = 0;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		if((*iter)->getTrans()==0)
		{
			sum+= 1;
		}
		iter++;
	}
	return sum;
}

DataSource_AgentTotalBikes::DataSource_AgentTotalBikes(repast::SharedContext<Commuter>* bike) : context(bike){ }

int DataSource_AgentTotalBikes::getData(){
	double sum = 0;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		if((*iter)->getTrans()==1)
		{
			sum+= 1;
		}
		iter++;
	}
	return sum;
}


	
