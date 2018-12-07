/* Demo_03_Model.h */

#ifndef COMMUTERMODEL
#define COMMUTERMODEL

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
	int stopAt;
	int countOfAgents;
	int countOfInfAgents;
	int timeinsteps;
	double Gsafety;
	repast::Properties* props;
	repast::SharedContext<Commuter> context;
	repast::SharedContext<Infrastructure> Infcontext;

	repast::SVDataSet* agentValues;


    repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* discreteSpace;
      repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* discreteInfSpace;
	
public:
	CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~CommuterModel();
	void init();
	void requestAgents();
	void cancelAgentRequests();
	void removeLocalAgents();
	void doSomething();
	void initSchedule(repast::ScheduleRunner& runner);
	void recordResults();
};


/* Data Collection */
class DataSource_AgentTotalCars : public repast::TDataSource<int>{
private:
	repast::SharedContext<Commuter>* context;
    
public:
	DataSource_AgentTotalCars(repast::SharedContext<Commuter>* car);
	int getData();
};


class DataSource_AgentTotalBikes : public repast::TDataSource<int>{
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentTotalBikes(repast::SharedContext<Commuter>* bike);
	int getData();
};



#endif
