/* CommuterModel.cpp */

//By Joseph Cresswell	Reg No. 150148395

//Including required headers
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
#include <boost/math/distributions/beta.hpp>	//Including the BETA distribution from boost libraries

#include "CommuterModel.h"

#define PI 3.141592653	//Defining pi for trig calculations of position

CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm),Infcontext(comm) {			//Commuter Model constructo
	//Defining intial variables using props file
	props = new repast::Properties(propsFile, argc, argv, comm);	
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	EndcountOfAgents = repast::strToInt(props->getProperty("count.of.agents.end"));
	countOfInfAgents = repast::strToInt(props->getProperty("initial.count.of.infrastructure.agents"));
	initializeRandom(*props, comm);
	Gsafety=0;
	TransCost=repast::strToInt(props->getProperty("public.transport.cost"));
	TransCostIncrease=((double)repast::strToInt(props->getProperty("public.transport.cost.increase")))/100;
	//Setting grid size as 601 by 601 grid with 0,0 as centre
   	repast::Point<double> origin(-300,-300);
   	repast::Point<double> extent(601, 601);	
	//Calcualting how often a new agent needs to be created
    newAgent=floor(1/(((double)EndcountOfAgents-(double)countOfAgents)/((double)stopAt-(double)timeinsteps)));
	//Getting infrastructure information from properties file
	newInfAgent=repast::strToInt(props->getProperty("Inf.Agent.Add.Rate"));
	newInfAgenttype=repast::strToInt(props->getProperty("Inf.Agent.Type"));
	newInfAgenttemplate=repast::strToInt(props->getProperty("Inf.Agent.Template"));
    	repast::GridDimensions gd(origin, extent);
    
	//sets to run on just 1 core
   	std::vector<int> processDims;
    	processDims.push_back(1);
    	processDims.push_back(1);
    	//Defining agents discrete space
    	discreteSpace = new repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, 		repast::SimpleAdder<Commuter> >("AgentDiscreteSpace", gd, processDims, 2, comm);
    	//Defining infrastructure discrete space
    	discreteInfSpace = new repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >("AgentDiscreteSpace", gd, processDims, 2, comm);
    	//Adding discrete space projection to respective contexts
   	context.addProjection(discreteSpace);
    	Infcontext.addProjection(discreteInfSpace);
     	// Data collection
		// Create the data set builder
		std::string fileOutputName("./output/EndSimCommuterData.csv");
		repast::SVDataSetBuilder builder(fileOutputName.c_str(), ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
	
	// Create the individual data sets to be added to the builder
	DataSource_AgentTotalCars* AgentTotalCars_DataSource = new DataSource_AgentTotalCars(&context);							//Recording total number of cars 
	builder.addDataSource(createSVDataSource("Total Cars", AgentTotalCars_DataSource, std::plus<int>()));
    
	DataSource_AgentTotalBikes* AgentTotalBikes_DataSource = new DataSource_AgentTotalBikes(&context);							//Recording total cyclists
	builder.addDataSource(createSVDataSource("Total Cyclists", AgentTotalBikes_DataSource, std::plus<int>()));
	
	DataSource_AgentTotalWalkers* AgentTotalWalkers_DataSource = new DataSource_AgentTotalWalkers(&context);							//Recording total walkers
	builder.addDataSource(createSVDataSource("Total Walkers", AgentTotalWalkers_DataSource, std::plus<int>()));

	DataSource_AgentTotalPTrans* AgentTotalPTrans_DataSource = new DataSource_AgentTotalPTrans(&context);							//Recording number of public transport users
	builder.addDataSource(createSVDataSource("Total Pub Transport", AgentTotalPTrans_DataSource, std::plus<int>()));

	DataSource_AgentAvgHealth* AgentAvgHealth_DataSource = new DataSource_AgentAvgHealth(&context);						//Recording average agent health
	builder.addDataSource(createSVDataSource("Average Health", AgentAvgHealth_DataSource, std::plus<int>()));
	
	DataSource_AgentAvgCAbility* AgentAvgCAbility_DataSource = new DataSource_AgentAvgCAbility(&context);						//Recording average agent cycle ability
	builder.addDataSource(createSVDataSource("Average Cycle Ability", AgentAvgCAbility_DataSource, std::plus<int>()));

	DataSource_AgentAvgSafe* AgentAvgSafe_DataSource = new DataSource_AgentAvgSafe(&context);			//Recording average agent saftey perception
	builder.addDataSource(createSVDataSource("Average Safety Perception", AgentAvgSafe_DataSource, std::plus<int>()));
    
	// Use the builder to create the data set
	agentValues = builder.createDataSet();

	
}

CommuterModel::~CommuterModel(){	//Model destructor
	delete props;
	delete agentValues;
}

void CommuterModel::init(){		//Function called to initialise the model
	//Initial proportions of each transportation mode
	double initialCar=double(repast::strToInt(props->getProperty("initial.car.proportion")))/100;
	double initialBike=double(repast::strToInt(props->getProperty("initial.bike.proportion")))/100;
	double initialWalk=double(repast::strToInt(props->getProperty("initial.walk.proportion")))/100;
	double initialPTrans=double(repast::strToInt(props->getProperty("initial.ptrans.proportion")))/100;

	int rank = repast::RepastProcess::instance()->rank();
	timeinsteps=0;
	//Create number of agents required at start of the model
	for(int i = 0; i < countOfAgents; i++){ 
		double newRand =  repast::Random::instance()->getGenerator("duni")->next();
		boost::math::beta_distribution<> vars(1.1,3);	//Use a beta distribution to distribute agents accross the space randomly
		double dist = boost::math::quantile(vars,newRand)*300;
		double angle =  repast::Random::instance()->getGenerator("duni")->next()*2*PI;
       		repast::Point<int> initialLocation(dist*sin(angle),dist*cos(angle));	//Distributing agents randomly
		repast::AgentId id(i, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id,initialCar,initialBike,initialWalk,initialPTrans);	//Creating the agent
		context.addAgent(agent);
       	discreteSpace->moveTo(id, initialLocation);	//Placing the agent in the discrete space
	}

}



void CommuterModel::commute(){		//Function to get agents to commute and retrive data from agents
	int TransMode;	
	int InfVar=repast::strToInt(props->getProperty("Inf.Agent.PVar")); //Getting parameter for infrastructure
	getGSafe();	//Get the updated global safety
	//Vectors for storing agents
	std::vector<Commuter*> agents;
	std::vector<Infrastructure*> Infagents;
	//Adding new agents if needed
	if(((double)timeinsteps+1.0)/newAgent==ceil(((double)timeinsteps+1.0)/newAgent))
	{
		addAgents(1);
	}
	//Adding Infrastructure Agents that will only begin to add after 9 years (2019)
	if(((double)timeinsteps+1.0)/newInfAgent==ceil(((double)timeinsteps+1.0)/newInfAgent)&&timeinsteps>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(newInfAgenttype,newInfAgenttemplate,10,InfVar);
	}
	/* Functions to include multiple types of infrastructure
	if(((double)timeinsteps+1.0)/12==ceil(((double)timeinsteps+1.0)/12)&&(timeinsteps-6)>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(0,2,10,0);//20mphzone
		addInfAgents(0,3,1,0);//crossing
	}
	if(((double)timeinsteps+1.0)/12==ceil(((double)timeinsteps+1.0)/12)&&timeinsteps>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(0,1,10,0);//non-segregated
	
	}
	if(((double)timeinsteps+1.0)/24==ceil(((double)timeinsteps+1.0)/24)&&(timeinsteps)>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(0,0,10,0);//segrgated
	}
	if(((double)timeinsteps+1.0)/12==ceil(((double)timeinsteps+1.0)/12)&&timeinsteps>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(2,1,10,1);//health classes
	
	}
		if(((double)timeinsteps+1.0)/12==ceil(((double)timeinsteps+1.0)/12)&&timeinsteps>=107)	//only begin adding infrastructure after end of 2018
	{
		addInfAgents(3,0,10,1);//cycling classes
	
	}*/
	context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);	//Iterating through all commuter agents
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
        (*it)->Travel(Gsafety, TransCost,&context, discreteSpace, discreteInfSpace);	//Making all commuter agents commute and passing required variables for this
		it++;
    	}
	NumCycle=0;
	NumWalk=0;
	NumCar=0;
	NumPTrans=0;
 	it = agents.begin();		//Iterating through all commuter agents
   	while(it != agents.end()){
		TransMode= (*it)->getMode();		//Iterating to get mode of each agent and print information to terminal for checking agent behaviour while it is running
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
			std::cout<<"just a walker " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumWalk++;
		}
		else if(TransMode==3)
		{
			std::cout<<"just riding public transport: " << (*it)->getId() << " at "<< timeinsteps <<" Safety: "<<safezz<<" Health: "<<(*it)->getHealth()<< std::endl;
			NumPTrans++;
		}
		it++;
    }

		discreteSpace->balance();	
  		timeinsteps++;	//incrementing timeinsteps

}

void CommuterModel::initSchedule(repast::ScheduleRunner& runner){	//Function creates schedule for model functions

	runner.scheduleEvent(0, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::commute)));	//Commute function run every step from the start
	runner.scheduleEvent(11, 12, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::IncreaseTransCost)));	//increasing public transport cost each year
	runner.scheduleEvent(0, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordResults))); //recording model results each time step from the start
	runner.scheduleEvent(0, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordAgentPositions)));//Record agent positions at start and at end of the simulation
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordAgentPositions)));
	runner.scheduleEvent(0,12, repast::Schedule::FunctorPtr(new repast::MethodFunctor<CommuterModel> (this, &CommuterModel::recordInfAgentPositions)));//record infrastructure positions ever 12 months
	

	runner.scheduleStop(stopAt);	//Stopping simulation at chosen stop time

	//Data Collection
	runner.scheduleEvent(0.5, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));	//Recording chosen data
	runner.scheduleEvent(10.6, 10, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
	
}


int CommuterModel::CalcCosts(){	//Function to calculate costs is just given as number of cars currently
	int Cost = NumCar;

	return Cost;

}

void CommuterModel::getGSafe()	//Global saftey is taken form the average saftey of all agents in the model
{
	double SafeTotal=0;
	std::vector<Commuter*> agents;	//Iterating through all commuter agents adn finding average safety of them all
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
	for(int i = 0; i < NumAgents; i++){ 	//For loop to add multiple agents if required
		double newRand =  repast::Random::instance()->getGenerator("duni")->next();
		boost::math::beta_distribution<> vars(1.1,3);	//Distributing agents randomly using a beta distribution
		double dist = boost::math::quantile(vars,newRand)*300;
		double angle =  repast::Random::instance()->getGenerator("duni")->next()*2*PI;
       	repast::Point<int> initialLocation(dist*sin(angle),dist*cos(angle));	//Distributing agents randomly
		repast::AgentId id(countOfAgents, rank, 0);
		id.currentRank(rank);
		Commuter* agent = new Commuter(id,(double)NumCar/(double)countOfAgents,(double)NumCycle/(double)countOfAgents,(double)NumWalk/(double)countOfAgents,(double)NumPTrans/(double)countOfAgents);		//Setting agent intial proportions to match current proportions of agent types

		context.addAgent(agent);	//Adding agent to context
       	discreteSpace->moveTo(id, initialLocation);	//Placing agent in intial location in context
		countOfAgents++;	//Increasing count of agents
	}
}

void CommuterModel::addInfAgents(int type, int temp,int range, int pvar) //function to add infrastrucure agents to the model
{
		int xpos = 0;
		int ypos=0;
		if(type!=1)	//If economic subsidising infrastructure...place at centre, otherwise distibute infrastructure uniformly within 5km of the centre
		{
			double Infdist =  (repast::Random::instance()->getGenerator("duni")->next())*50;
			double Infangle  = (repast::Random::instance()->getGenerator("duni")->next())*2*PI;
        	xpos= Infdist*sin(Infangle);
			ypos= Infdist*cos(Infangle);
		}
		repast::Point<int> initialLocation(xpos,ypos);	//Creating point to put infrastructure agent in calculated position
		repast::AgentId id(countOfInfAgents, 0, 1);
		id.currentRank(0);
		Infrastructure* agent = new Infrastructure(id,type,temp,range,pvar);	//Creating infrastructure agent
		Infcontext.addAgent(agent);	//Adding agent to infrastructure context
       	discreteInfSpace->moveTo(id, initialLocation);	//Placing agent in chosen position
		countOfInfAgents++;	//Incrementing number of infrastructure agents
}

void CommuterModel::IncreaseTransCost()	//Function to increase transcost based on set annual increase of transport cost
{
	TransCost=TransCost*(1+TransCostIncrease);
} 

void CommuterModel::recordResults(){	//Function to record some results to csv file
		double infCost=0;
		std::string seperator=",";	//Seperator to place data in seperate cells
		std::vector<std::string> key = {"Month","P Transport Cost","Count of Inf","Inf Cost"};	//Defining header of file
		std::string fileName="./output/Modelresults.csv"; //Defining file name
		 bool writeHeader =  !boost::filesystem::exists(fileName);	//Checking if the file exists
 		 std::ofstream outFile;

  		outFile.open(fileName.c_str(), std::ios::app); //opening file
		if(writeHeader)	//if fie dosen't already exists add header file
		{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;	//Iterating through header vector to place in csv file
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				i++;
    			}
   			outFile << std::endl;	//Moving to new row
		}
		std::vector<Infrastructure*> Allagents;	//Iterating through all infrastructure to calculate the total costs
		Infcontext.selectAgents(repast::SharedContext<Infrastructure>::LOCAL, countOfAgents, Allagents);
		std::vector<Infrastructure*>::iterator it = Allagents.begin();
   		while(it != Allagents.end())
		{ 
			infCost=infCost+(*it)->getInfCost();
			it++;			
		}

    	outFile << std::fixed << timeinsteps << (seperator);	//Outputting time to file
	outFile << std::fixed << TransCost	<<(seperator);			//Outputting public transport cost to file
	outFile << std::fixed <<countOfInfAgents<<(seperator);		//Ouputting number of infrastructure agents to file
	outFile << std::fixed <<infCost	<<(" ");	//Outputting total infrastructure cost to file

 	outFile << std::endl;

  	outFile.close();		//Closing the file
}

void CommuterModel::recordAgentPositions(){		//Recording agent positions
		std::string seperator=",";	//Seperator to place data in seperate cells
		std::vector<std::string> key = {"Month","Xposition","YPosition"};	//Defining header vector
		std::string fileName="./output/AgentPositions.csv";	//Defining output file name
		 bool writeHeader =  !boost::filesystem::exists(fileName);
 		 std::ofstream outFile;

  		outFile.open(fileName.c_str(), std::ios::app);	//Opening file
		if(writeHeader)			//If file doesn't yet exist write the header
		{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;		//iterate through header vector to place the header in the file
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				 i++;
    			}
    			outFile << std::endl;	//Move to new row
		}
		std::vector<int> agentLoc;		//Interating through all agents to find positions
		std::vector<Commuter*> Allagents;
		context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, Allagents);
		std::vector<Commuter*>::iterator it = Allagents.begin();
   		while(it != Allagents.end()){ 
			discreteSpace->getLocation((*it)->getId(), agentLoc);
			repast::Point<int> AgentsPoint(agentLoc);
			outFile << std::fixed << timeinsteps << (seperator);
	 		outFile << std::fixed << AgentsPoint.getX()<<(seperator);	//Getting x position and outputting it
			outFile << std::fixed <<AgentsPoint.getY()<<(" ");			//Getting and outputting y position

  			outFile << std::endl;
			it++;

		}

  outFile.close();	//Closing the file
}

void CommuterModel::recordInfAgentPositions(){	//Recording infrastructure agent positions
		std::string seperator=",";	//Seperator to place data in different cells of csv file
		std::vector<std::string> key = {"Month","Xposition","YPosition","Type","Help Provided"}; // Vector to define the header of the csv file
		if(countOfInfAgents!=0)	//Only output to file if there are any infrastructure agents
		{
			std::string fileName="./output/InfAgentPositions.csv";	//Defining file name
			 bool writeHeader =  !boost::filesystem::exists(fileName);
 		 	std::ofstream outFile;

  			outFile.open(fileName.c_str(), std::ios::app);	//Opening the file
			if(writeHeader)		//if file doesn't already exist then ouput the headers
			{
    			std::vector<std::string>::iterator keys      = key.begin();
    			std::vector<std::string>::iterator keysEnd   = key.end();

    			int i = 1;		//iterate through the header vector and to print it
    			while(keys != keysEnd){
     				 outFile << *keys << (i!=key.size() ? seperator:"");
     				 keys++;
      				 i++;
    			}
    			outFile << std::endl;	//Moving to next row of the file
			}
			std::vector<int> agentLoc;		//Defining vectors to store infrastructure position information
			std::vector<Infrastructure*> Allagents;
			Infcontext.selectAgents(repast::SharedContext<Infrastructure>::LOCAL, countOfInfAgents, Allagents);
			std::vector<Infrastructure*>::iterator it = Allagents.begin();	//Iterating though all infrastructure agents to find position, type and provided help
   			while(it != Allagents.end()){ 
				discreteInfSpace->getLocation((*it)->getId(), agentLoc);
				repast::Point<int> AgentsPoint(agentLoc);
				outFile << std::fixed << timeinsteps << (seperator);
	 			outFile << std::fixed << AgentsPoint.getX()<<(seperator);	//Getting and outputting x positon
				outFile << std::fixed << AgentsPoint.getY()<<(seperator);	//Getting and outputting y position
				outFile << std::fixed << (*it)->getInfType()<<(seperator);	//Getting and outputting infrastructure type
				outFile << std::fixed <<(*it)->getProvVar()<<(" ");	//Getting and outputting infrastructure provided help

  				outFile << std::endl;
				it++;

			}

  	outFile.close();	//Closing the file
	}
}




//Data source classes


//Getting and outputting number of driving commuters
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

//Getting and outputting number of cycling commuters
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

//Getting number and outputting of walking commuters
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

//Getting and outputting number of public transport using commuters
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

//Getting and outputting average health of commuter agents
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

//Getting and outputting average cyle ability of commuters
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

//Getting anf outputting average safety of commuter agents
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


	
