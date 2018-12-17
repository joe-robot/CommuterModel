/* CommuterModel.h */

//By Joseph Cresswell	Reg No. 150148395

#ifndef COMMUTERMODEL
#define COMMUTERMODEL

//Including required files
#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"

#include "Infrastructure.h"
#include "Commuter.h"


class CommuterModel{
	//Defining private attributes
	int stopAt;
	int countOfAgents;
	int EndcountOfAgents;
	int countOfInfAgents;
	int timeinsteps=0;	//Setting time in steps intially as 0
	double Gsafety;
	int NumCycle;
	int NumCar;
	int NumWalk;
	int NumPTrans;
	double newInfAgent;
	int newInfAgenttype;
	int newInfAgenttemplate;
	double newAgent=0;
	double TransCost;
	double TransCostIncrease;
	repast::Properties* props;
	repast::SharedContext<Commuter> context;
	repast::SharedContext<Infrastructure> Infcontext;

	repast::SVDataSet* agentValues;


    	repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* discreteSpace;
      	repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* discreteInfSpace;
	
public:
	//Defining public functions of model
	CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~CommuterModel();
	void init();
	void addAgents(int NumAgents);
	void addInfAgents(int type, int temp,int range, int pvar);
	void commute();
	void initSchedule(repast::ScheduleRunner& runner);
	void recordResults();
	void IncreaseTransCost();
	double getTransCost() {return TransCost;};
	void recordAgentPositions();
	void recordInfAgentPositions();
private:
	//Defining private functions of the model
	int CalcCosts();
	void getGSafe();
};


/* Data Collection */
class DataSource_AgentTotalCars : public repast::TDataSource<int>{	//Recording number of drivers
private:
	repast::SharedContext<Commuter>* context;
    
public:
	DataSource_AgentTotalCars(repast::SharedContext<Commuter>* car);
	int getData();
};


class DataSource_AgentTotalBikes : public repast::TDataSource<int>{	//Recording number of cyclists
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentTotalBikes(repast::SharedContext<Commuter>* bike);
	int getData();
};

class DataSource_AgentTotalWalkers : public repast::TDataSource<int>{	//Recording number of walkers
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentTotalWalkers(repast::SharedContext<Commuter>* Walk);
	int getData();
};

class DataSource_AgentTotalPTrans : public repast::TDataSource<int>{	//Recording number of public transport users
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentTotalPTrans(repast::SharedContext<Commuter>* PTrans);
	int getData();
};

class DataSource_AgentAvgHealth : public repast::TDataSource<double>{	//Recording average agent health
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentAvgHealth(repast::SharedContext<Commuter>* Health);
	double getData();
};

class DataSource_AgentAvgCAbility : public repast::TDataSource<double>{ //Recording average agent cycle ability
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentAvgCAbility(repast::SharedContext<Commuter>* CAbility);
	double getData();
};

class DataSource_AgentAvgSafe : public repast::TDataSource<double>{ //Recording average agent safety perception
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentAvgSafe(repast::SharedContext<Commuter>* safe);
	double getData();
};



#endif
