/* Demo_03_Model.cpp */

#include <stdio.h>
#include <vector>
#include <math.h>
#include <ios>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"
#include <boost/math/distributions/beta.hpp>	//Including the BETA sistribution from boost libraries

#include "CommuterModel.h"

#define PI 3.141592653

CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm),Infcontext(comm) {
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	EndcountOfAgents = repast::strToInt(props->getProperty("count.of.agents.end"));
	countOfInfAgents = repast::strToInt(props->getProperty("initial.count.of.infrastructure.agents"));
	initializeRandom(*props, comm);
	Gsafety=0;
	TransCost=repast::strToInt(props->getProperty("public.transport.cost"));
	TransCostIncrease=((double)repast::strToInt(props->getProperty("public.transport.cost.increase")))/100;
   	repast::Point<double> origin(-300,-300);
   	repast::Point<double> extent(601, 601);
    newAgent=floor(1/(((double)EndcountOfAgents-(double)countOfAgents)/((double)stopAt-(double)timeinsteps)));
	newInfAgent=repast::strToInt(props->getProperty("Inf.Agent.Add.Rate"));
	newInfAgenttype=repast::strToInt(props->getProperty("Inf.Agent.Type"));
	newInfAgenttemplate=repast::strToInt(props->getProperty("Inf.Agent.Template"));
    	repast::GridDimensions gd(origin, extent);
    
	//sets to run on just 1 core
   	std::vector<int> processDims;
    	processDims.push_back(1);
    	processDims.push_back(1);
    
    	discreteSpace = new repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, 		repast::SimpleAdder<Commuter> >("AgentDiscreteSpace", gd, processDims, 2, comm);
    
    	discreteInfSpace = new repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >("AgentDiscreteSpace", gd, processDims, 2, comm);
    
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
	
	DataSource_AgentTotalWalkers* AgentTotalWalkers_DataSource = new DataSource_AgentTotalWalkers(&context);
	builder.addDataSource(createSVDataSource("Total Walkers", AgentTotalWalkers_DataSource, std::plus<int>()));

	DataSource_AgentTotalPTrans* AgentTotalPTrans_DataSource = new DataSource_AgentTotalPTrans(&context);
	builder.addDataSource(createSVDataSource("Total Pub Transport", AgentTotalPTrans_DataSource, std::plus<int>()));

	DataSource_AgentAvgHealth* AgentAvgHealth_DataSource = new DataSource_AgentAvgHealth(&context);
	builder.addDataSource(createSVDataSource("Average Health", AgentAvgHealth_DataSource, std::plus<int>()));
	
	DataSource_AgentAvgCAbility* AgentAvgCAbility_DataSource = new DataSource_AgentAvgCAbility(&context);
	builder.addDataSource(createSVDataSource("Average Cycle Ability", AgentAvgCAbility_DataSource, std::plus<int>()));

	DataSource_AgentAvgSafe* AgentAvgSafe_DataSource = new DataSource_AgentAvgSafe(&context);
	builder.addDataSource(createSVDataSource("Average Safety Perception", AgentAvgSafe_DataSource, std::plus<int>()));
    
	// Use the builder to create the data set
	agentValues = builder.createDataSet();

	
}

CommuterModel::~CommuterModel(){
	delete props;
	delete agentValues;
}

void CommuterModel::init(){
	double initialCar=double(repast::strToInt(props->getProperty("initial.car.proportion")))/100;
	double initialBike=double(repast::strToInt(props->getProperty("initial.bike.proportion")))/100;
	double initialWalk=double(repast::strToInt(props->getProperty("initial.walk.proportion")))/100;
	double initialPTrans=double(repast::strToInt(props->getProperty("initial.ptrans.proportion")))/100;

	int rank = repast::RepastProcess::instance()->rank();
	timeinsteps=0;
	for(int i = 0; i < countOfAgents; i++){ 
		double newRand =  repast::Random::instance()->getGenerator("duni")->next();
		boost::math::beta_distribution<> vars(1.1,3);
		double dist = boost::math::quantile(vars,newRand)*300;
		double angle =  repast::Random::instance()->getGenerator("duni")->next()*2*PI;
       		repast::Point<int> initialLocation(dist*sin(angle),dist*cos(angle));	//Distributing agents randomly
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id,initialCar,initialBike,initialWalk,initialPTrans);
		context.addAgent(agent);
       	discreteSpace->moveTo(id, initialLocation);
	}

}



void CommuterModel::commute(){
	int TransMode;	
	int InfVar=repast::strToInt(props->getProperty("Inf.Agent.PVar"));
	getGSafe();
	std::vector<Commuter*> agents;
	std::vector<Infrastructure*> Infagents;
	//Adding new agents if needed
	if(((double)timeinsteps+1.0)/newAgent==ceil(((double)timeinsteps+1.0)/newAgent))
	{
		addAgents(1);
	}
	//Adding Infrastructure Agents
	if(((double)timeinsteps+1.0)/newInfAgent==ceil(((double)timeinsteps+1.0)/newInfAgent)&&timeinsteps>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(newInfAgenttype,newInfAgenttemplate,10,InfVar);
	}

	context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
        (*it)->Travel(Gsafety, TransCost,&context, discreteSpace, discreteInfSpace);
		it++;
    	}
	NumCycle=0;
	NumWalk=0;
	NumCar=0;
	NumPTrans=0;
 	it = agents.begin();
   	while(it != agents.end()){  //Need similar for recivieing infrastructure information
		TransMode= (*it)->getMode();
		double safezz=(*it)->getSafe();
		if(TransMode ==1)
		{
			std::cout<<"just another cycleist: " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumCycle++;
		}
		else if(TransMode==0)
		{
			std::cout<<"just another driver: " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumCar++;
		}
		else if(TransMode==2)
		{
			std::cout<<"just a walking " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumWalk++;
		}
		else if(TransMode==3)
		{
			std::cout<<"just riding public transport: " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumPTrans++;
		}
		it++;
    }

	//Retriving infrastructure information
	if(countOfInfAgents!=0)
	{
		Infcontext.selectAgents(repast::SharedContext<Infrastructure>::LOCAL, countOfInfAgents, Infagents);
		std::vector<Infrastructure*>::iterator Infit = Infagents.begin();
		while(Infit != Infagents.end()) //what info needs to be retrived though? cost maybe?
		{
		
			Infit++;

		}
	}
		discreteSpace->balance();
  		timeinsteps++;

}

void CommuterModel::initSchedule(repast::ScheduleRunner& runner){

	runner.scheduleEvent(0, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::commute)));
	runner.scheduleEvent(11, 12, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::IncreaseTransCost)));	//increasing public transport cost each year
	runner.scheduleEvent(0, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordResults)));
	runner.scheduleEvent(0, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordAgentPositions)));//Record agent positions at start and at end of the simulation
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordAgentPositions)));
	runner.scheduleEvent(0,12, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordInfAgentPositions)));//record infrastructure positions ever 12 months
	

	runner.scheduleStop(stopAt);

	//Data Collection
	runner.scheduleEvent(0.5, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEvent(10.6, 10, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
	
}


int CommuterModel::CalcCosts(){
	int Cost = NumCar;

	return Cost;

}

void CommuterModel::getGSafe()	//Global saftey is taken form the average saftey of all agents in the model
{
	double SafeTotal=0;
	std::vector<Commuter*> agents;
	context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);
	std::vector<Commuter*>::iterator Git = agents.begin();
	while(Git != agents.end()){
		SafeTotal = (SafeTotal + (*Git)->getSafe());
		Git++;
   	 }
	Gsafety=SafeTotal/countOfAgents;

}

void CommuterModel::addAgents(int NumAgents) //function to add agents to the model
{
	int rank = repast::RepastProcess::instance()->rank();
	for(int i = 0; i < NumAgents; i++){ 
		double newRand =  repast::Random::instance()->getGenerator("duni")->next();
		boost::math::beta_distribution<> vars(1.1,3);
		double dist = boost::math::quantile(vars,newRand)*300;
		double angle =  repast::Random::instance()->getGenerator("duni")->next()*2*PI;
       	repast::Point<int> initialLocation(dist*sin(angle),dist*cos(angle));	//Distributing agents randomly
		repast::AgentId id(countOfAgents, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id,(double)NumCar/(double)countOfAgents,(double)NumCycle/(double)countOfAgents,(double)NumWalk/(double)countOfAgents,(double)NumPTrans/(double)countOfAgents);

		context.addAgent(agent);
       	discreteSpace->moveTo(id, initialLocation);
		countOfAgents++;
	}
}

void CommuterModel::addInfAgents(int type, int temp,int range, int pvar) //function to add infrastrucure agents to the model
{
		repast::Point<int> initialLocation(0,0);
		//int rank = repast::RepastProcess::instance()->rank();
		if(type==1)	//If economic subsidising infrastructure...place at centre
		{
			repast::Point<int> initialLocation(0,0);
		}
		else
		{
			double Infdist =  (repast::Random::instance()->getGenerator("duni")->next())*50;
			double Infangle  = (repast::Random::instance()->getGenerator("duni")->next())*2*PI;
        	repast::Point<int> initialLocation(Infdist*sin(Infangle),Infdist*cos(Infangle));
		}
		repast::AgentId id(countOfInfAgents, 0, 1);
		id.currentRank(0);
		Infrastructure* agent = new Infrastructure(id,type,temp,range,pvar);
		Infcontext.addAgent(agent);
       	discreteInfSpace->moveTo(id, initialLocation);
		countOfInfAgents++;
}

void CommuterModel::IncreaseTransCost()
{
	TransCost=TransCost*(1+TransCostIncrease);
} 

void CommuterModel::recordResults(){
		double infCost=0;
		std::string seperator=",";
		std::vector<std::string> key = {"Month","P Transport Cost","Count of Inf","Inf Cost"};
		std::string fileName="./output/Modelresults.csv";
		 bool writeHeader =  !boost::filesystem::exists(fileName);
 		 std::ofstream outFile;

  		outFile.open(fileName.c_str(), std::ios::app);
		if(writeHeader)
		{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				i++;
    			}
   			outFile << std::endl;
		}
		std::vector<Infrastructure*> Allagents;
		Infcontext.selectAgents(repast::SharedContext<Infrastructure>::LOCAL, countOfAgents, Allagents);
		std::vector<Infrastructure*>::iterator it = Allagents.begin();
   		while(it != Allagents.end())
		{ 
			infCost=infCost+(*it)->getInfCost();
			it++;			
		}

    	outFile << std::fixed << timeinsteps << (seperator);
	outFile << std::fixed << TransCost	<<(seperator);
	outFile << std::fixed <<countOfInfAgents<<(seperator);
	outFile << std::fixed <<infCost	<<(" ");

 	outFile << std::endl;

  	outFile.close();
}

void CommuterModel::recordAgentPositions(){
		std::string seperator=",";
		std::vector<std::string> key = {"Month","Xposition","YPosition"};
		std::string fileName="./output/AgentPositions.csv";
		 bool writeHeader =  !boost::filesystem::exists(fileName);
 		 std::ofstream outFile;

  		outFile.open(fileName.c_str(), std::ios::app);
		if(writeHeader)
		{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				 i++;
    			}
    			outFile << std::endl;
		}
		std::vector<int> agentLoc;
		std::vector<Commuter*> Allagents;
		context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, Allagents);
		std::vector<Commuter*>::iterator it = Allagents.begin();
   		while(it != Allagents.end()){ 
			discreteSpace->getLocation((*it)->getId(), agentLoc);
			repast::Point<int> AgentsPoint(agentLoc);
			outFile << std::fixed << timeinsteps << (seperator);
	 		outFile << std::fixed << AgentsPoint.getX()<<(seperator);
			outFile << std::fixed <<AgentsPoint.getY()<<(" ");

  			outFile << std::endl;
			it++;

		}

  outFile.close();
}

void CommuterModel::recordInfAgentPositions(){
		std::string seperator=",";
		std::vector<std::string> key = {"Month","Xposition","YPosition","Type","Help Provided"};
		if(countOfInfAgents!=0)
		{
			std::string fileName="./output/InfAgentPositions.csv";
			 bool writeHeader =  !boost::filesystem::exists(fileName);
 		 	std::ofstream outFile;

  			outFile.open(fileName.c_str(), std::ios::app);
			if(writeHeader)
			{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				 i++;
    			}
    			outFile << std::endl;
			}
			std::vector<int> agentLoc;
			std::vector<Infrastructure*> Allagents;
			Infcontext.selectAgents(repast::SharedContext<Infrastructure>::LOCAL, countOfInfAgents, Allagents);
			std::vector<Infrastructure*>::iterator it = Allagents.begin();
   			while(it != Allagents.end()){ 
				discreteInfSpace->getLocation((*it)->getId(), agentLoc);
				repast::Point<int> AgentsPoint(agentLoc);
				outFile << std::fixed << timeinsteps << (seperator);
	 			outFile << std::fixed << AgentsPoint.getX()<<(seperator);
				outFile << std::fixed << AgentsPoint.getY()<<(seperator);
				outFile << std::fixed << (*it)->getInfType()<<(seperator);
				outFile << std::fixed <<(*it)->getProvVar()<<(" ");

  				outFile << std::endl;
				it++;

			}

  	outFile.close();
	}
}




//Data source classes



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

DataSource_AgentTotalWalkers::DataSource_AgentTotalWalkers(repast::SharedContext<Commuter>* Walk) : context(Walk){ }

int DataSource_AgentTotalWalkers::getData(){
	double sum = 0;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		if((*iter)->getMode()==2)
		{
			sum+= 1;
		}
		iter++;
	}
	return sum;
}

DataSource_AgentTotalPTrans::DataSource_AgentTotalPTrans(repast::SharedContext<Commuter>* PTrans) : context(PTrans){ }

int DataSource_AgentTotalPTrans::getData(){
	double sum = 0;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		if((*iter)->getMode()==3)
		{
			sum+= 1;
		}
		iter++;
	}
	return sum;
}

DataSource_AgentAvgHealth::DataSource_AgentAvgHealth(repast::SharedContext<Commuter>* Health) : context(Health){ }

double DataSource_AgentAvgHealth::getData(){
	double sum = 0;
	double totalhealth=0;
	double AvgHealth;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		totalhealth =totalhealth + (*iter)->getHealth();	
		sum++;
		iter++;
	}
	if(sum!=0)
	{
		AvgHealth=totalhealth/(double)sum;
	}
	else
	{
		AvgHealth=0;
	}
	return AvgHealth;
}

DataSource_AgentAvgCAbility::DataSource_AgentAvgCAbility(repast::SharedContext<Commuter>* CAbility) : context(CAbility){ }

double DataSource_AgentAvgCAbility::getData(){
	double sum = 0;
	double totalca=0;
	double AvgCAbility;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		totalca =totalca+(*iter)->getCycleAbility();
		sum++;
		iter++;
	}
	if(sum!=0)
	{
		AvgCAbility=totalca/sum;
	}
	else
	{
		AvgCAbility=0;
	}
	return AvgCAbility;
}

DataSource_AgentAvgSafe::DataSource_AgentAvgSafe(repast::SharedContext<Commuter>* safe) : context(safe){ }

double DataSource_AgentAvgSafe::getData(){
	double sum = 0;
	double totalsafe=0;
	double AvgSafe;
	repast::SharedContext<Commuter>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<Commuter>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		totalsafe =totalsafe+(*iter)->getSafe();	
		sum++;
		iter++;
	}
	if(sum!=0)
	{
		AvgSafe=totalsafe/sum;
	}
	else
	{
		AvgSafe=0;
	}
	return AvgSafe;
}


	
