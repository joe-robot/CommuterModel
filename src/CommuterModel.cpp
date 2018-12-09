/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <math.h>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"

#include "CommuterModel.h"

#define PI 3.141592653

CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm),Infcontext(comm) {
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	countOfInfAgents = repast::strToInt(props->getProperty("count.of.infrastructure.agents"));
	initializeRandom(*props, comm);
	Gsafety=0;
   	repast::Point<double> origin(-100,-100);
   	repast::Point<double> extent(201, 201);
    
    	repast::GridDimensions gd(origin, extent);
    
	//sets to run on just 1 core
   	std::vector<int> processDims;
    	processDims.push_back(1);
    	processDims.push_back(1);
    
    	discreteSpace = new repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, 		repast::SimpleAdder<Commuter> >("AgentDiscreteSpace", gd, processDims, 2, comm);
    
    	discreteInfSpace = new repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >("AgentDiscreteSpace", gd, processDims, 2, comm);
	
   	// std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << " " << discreteSpace->bounds().extents() << std::endl;
    
   	context.addProjection(discreteSpace);
    	Infcontext.addProjection(discreteInfSpace);
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
		double val = repast::Random::instance()->getGenerator("lognor")->next();
		double angle = repast::Random::instance()->nextDouble()*2*PI;
       		repast::Point<int> initialLocation(val*10*sin(angle),val*10*cos(angle));
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id);
		context.addAgent(agent);
        discreteSpace->moveTo(id, initialLocation);
	}

	for(int i = 0; i < countOfInfAgents; i++){ 
        repast::Point<int> initialLocation(0,0);
		repast::AgentId id(i, rank, 1);
		id.currentRank(rank);
		Infrastructure* agent = new Infrastructure(id,1,1,0.5);
		Infcontext.addAgent(agent);
       		discreteInfSpace->moveTo(id, initialLocation);
	}
}



void CommuterModel::doSomething(){
	timeinsteps++;
	int TransMode;	
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
        (*it)->Travel(Gsafety,&context, discreteSpace, discreteInfSpace);
		it++;
    	}

 	it = agents.begin();
   	while(it != agents.end()){  //Need similar for recivieing infrastructure information
		TransMode= (*it)->getMode();
		if(TransMode ==1)
		{
			//std::cout<<"just another cyleist " << (*it)->getId() << " at "<< timeinsteps << std::endl;
		}
		else
		{
			//std::cout<<"just another driver" << std::endl;
		}
		it++;
    }

	/*No Longer need to move since 'movement' is done by query
    it = agents.begin();
    while(it != agents.end()){
		(*it)->move(discreteSpace);
		it++;
    }*/

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
		if((*iter)->getMode()==0)
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
		if((*iter)->getMode()==1)
		{
			sum+= 1;
		}
		iter++;
	}
	return sum;
}


	
