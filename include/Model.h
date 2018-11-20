/* Demo_01_Model.h */

#ifndef MODEL
#define MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"

#include "Commuter.h"

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

class CommuterModel{
	int stopAt;
	int countOfAgents;
	int NumCycle;
	int NumCar;
	repast::Properties* props;
	repast::SharedContext<Commuter> context;

	CommuterPackageProvider* provider;
	CommuterPackageReceiver* receiver;

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
