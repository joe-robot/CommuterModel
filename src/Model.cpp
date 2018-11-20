/* Demo_01_Model.cpp */

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

#include "Model.h"

CommuterPackageProvider::CommuterPackageProvider(repast::SharedContext<Commuter>* agentPtr): agents(agentPtr){ }

void CommuterPackageProvider::providePackage(Commuter * agent, std::vector<CommuterPackage>& out){
    repast::AgentId id = agent->getId();
    CommuterPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(), agent->getSafe(), agent->getThresh(), agent->getTrans());
    out.push_back(package);
}

void CommuterPackageProvider::provideContent(repast::AgentRequest req, std::vector<CommuterPackage>& out){
    std::vector<repast::AgentId> ids = req.requestedAgents();
    for(size_t i = 0; i < ids.size(); i++){
        providePackage(agents->getAgent(ids[i]), out);
    }
}


CommuterPackageReceiver::CommuterPackageReceiver(repast::SharedContext<Commuter>* agentPtr): agents(agentPtr){}

Commuter * CommuterPackageReceiver::createAgent(CommuterPackage package){
    repast::AgentId id(package.id, package.rank, package.type, package.currentRank);
    return new Commuter(id, package.safety, package.thresh, package.Transtype);
}

void CommuterPackageReceiver::updateAgent(CommuterPackage package){
    repast::AgentId id(package.id, package.rank, package.type);
    Commuter * agent = agents->getAgent(id);
    agent->set(package.currentRank, package.safety, package.thresh,package.Transtype);
}




CommuterModel::CommuterModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	initializeRandom(*props, comm);
	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");

	repast::Point<double> origin(-100,-100);
        repast::Point<double> extent(200, 200);
    
        repast::GridDimensions gd(origin, extent);
    
        std::vector<int> processDims;
        processDims.push_back(2);
        processDims.push_back(2);
    
        discreteSpace = new repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >("AgentDiscreteSpace", gd, processDims, 2, comm);
	
        std::cout << "RANK " << repast::RepastProcess::instance()->rank() << " BOUNDS: " << discreteSpace->bounds().origin() << std::endl;
    
   	context.addProjection(discreteSpace);
}

CommuterModel::~CommuterModel(){
		delete props;
        delete provider;
        delete receiver;
}

void CommuterModel::init(){
	int rank = repast::RepastProcess::instance()->rank();
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
	int whichRank=0;
	std::vector<Commuter*> agents;
    context.selectAgents(repast::SharedContext<Commuter>::LOCAL, countOfAgents, agents);
	std::vector<Commuter*>::iterator it = agents.begin();
	while(it != agents.end()){
		(*it)->commute(&context, discreteSpace);
		it++;
    }
    int r=1;
//	for(int r = 0; r < 4; r++){
			for(int i = 0; i < 10; i++){
				repast::AgentId toDisplay(i, r, 0);
				Commuter* agent = context.getAgent(toDisplay);
					if((agent != 0) && (agent->getId().currentRank() == whichRank)){
                                    	std::vector<int> agentLoc;
                                    	discreteSpace->getLocation(agent->getId(), agentLoc);
                                   	 repast::Point<int> agentLocation(agentLoc);
                                    	std::cout << agent->getId() << " " << agent->getSafe() << " " << agent->getThresh() << " AT " << agentLocation << std::endl;
                               		}
			}
	//}

	it = agents.begin();
        while(it != agents.end()){
	    (*it)->move(discreteSpace);
	    it++;
        }

	NumCycle=0;
	NumCar=0;
	//for(int r=0; r< 4; r++)
	//{
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

	//}
    discreteSpace->balance();
    repast::RepastProcess::instance()->synchronizeAgentStatus<Commuter, CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(context, *provider, *receiver, *receiver);
    
    repast::RepastProcess::instance()->synchronizeProjectionInfo<Commuter, CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(context, *provider, *receiver, *receiver);
    
    repast::RepastProcess::instance()->synchronizeAgentStates<CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(*provider, *receiver);
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


///Pretty sure i don't need these but trying to figure out why it dosen't work k thanks

void CommuterModel::requestAgents(){
    int rank = repast::RepastProcess::instance()->rank();
    int worldSize= repast::RepastProcess::instance()->worldSize();
    repast::AgentRequest req(rank);
    for(int i = 0; i < worldSize; i++){                     // For each process
        if(i != rank){                                      // ... except this one
            std::vector<Commuter*> agents;
            context.selectAgents(5, agents);                // Choose 5 local agents randomly
            for(size_t j = 0; j < agents.size(); j++){
                repast::AgentId local = agents[j]->getId();          // Transform each local agent's id into a matching non-local one
                repast::AgentId other(local.id(), i, 0);
                other.currentRank(i);
                req.addRequest(other);                      // Add it to the agent request
            }
        }
    }
    repast::RepastProcess::instance()->requestAgents<Commuter, CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(context, req, *provider, *receiver, *receiver);
}

void CommuterModel::cancelAgentRequests(){
    int rank = repast::RepastProcess::instance()->rank();
    if(rank == 0) std::cout << "CANCELING AGENT REQUESTS" << std::endl;
    repast::AgentRequest req(rank);
    
    repast::SharedContext<Commuter>::const_state_aware_iterator non_local_agents_iter  = context.begin(repast::SharedContext<Commuter>::NON_LOCAL);
    repast::SharedContext<Commuter>::const_state_aware_iterator non_local_agents_end   = context.end(repast::SharedContext<Commuter>::NON_LOCAL);
    while(non_local_agents_iter != non_local_agents_end){
        req.addCancellation((*non_local_agents_iter)->getId());
        non_local_agents_iter++;
    }
    repast::RepastProcess::instance()->requestAgents<Commuter, CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(context, req, *provider, *receiver, *receiver);
    
    std::vector<repast::AgentId> cancellations = req.cancellations();
    std::vector<repast::AgentId>::iterator idToRemove = cancellations.begin();
    while(idToRemove != cancellations.end()){
        context.importedAgentRemoved(*idToRemove);
        idToRemove++;
    }
}

void CommuterModel::removeLocalAgents(){
    int rank = repast::RepastProcess::instance()->rank();
    if(rank == 0) std::cout << "REMOVING LOCAL AGENTS" << std::endl;
    for(int i = 0; i < 5; i++){
        repast::AgentId id(i, rank, 0);
        repast::RepastProcess::instance()->agentRemoved(id);
        context.removeAgent(id);
    }
    repast::RepastProcess::instance()->synchronizeAgentStatus<Commuter, CommuterPackage, CommuterPackageProvider, CommuterPackageReceiver>(context, *provider, *receiver, *receiver);
}
