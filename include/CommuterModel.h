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

#include "Commuter.h"

/* Agent Package Provider */
class CommuterPackageProvider {
	
private:
    repast::SharedContext<Commuter>* agents;
	
public:
	
    CommuterPackageProvider(repast::SharedContext<Commuter>* agentPtr);
	
    void providePackage(Commuter * agent, std::vector<CommuterPackage>& out);
	
    void provideContent(repast::AgentRequest req, std::vector<CommuterPackage>& out);
	
};

/* Agent Package Receiver */
class CommuterPackageReceiver {
	
private:
    repast::SharedContext<Commuter>* agents;
	
public:
	
    CommuterPackageReceiver(repast::SharedContext<Commuter>* agentPtr);
	
    Commuter * createAgent(CommuterPackage package);
	
    void updateAgent(CommuterPackage package);
	
};


/* Data Collection */
class DataSource_AgentTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<Commuter>* context;

public:
	DataSource_AgentTotals(repast::SharedContext<Commuter>* c);
	int getData();
};
	

class DataSource_AgentCTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<Commuter>* context;
	
public:
	DataSource_AgentCTotals(repast::SharedContext<Commuter>* c);
	int getData();
};

class CommuterModel{
	int stopAt;
	int countOfAgents;
	repast::Properties* props;
	repast::SharedContext<Commuter> context;
	
	CommuterPackageProvider* provider;
	CommuterPackageReceiver* receiver;

	repast::SVDataSet* agentValues;
    repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* discreteSpace;
	
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

#endif
