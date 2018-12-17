/* Main.cpp */

//By Joseph Cresswell	Reg No. 150148395

#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "CommuterModel.h"


int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	std::string propsFile  = argv[2]; // The name of the properties file is Arg 2
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	repast::RepastProcess::init(configFile);	//intialising repast
	
	CommuterModel* model = new CommuterModel(propsFile, argc, argv, &world);	//creating th model
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner(); //intialising the repast schedule runner
	
	model->init();	//intialising the model
	model->initSchedule(runner);	//intialising model schedule
	
	runner.run();	//running schedule
	
	delete model;	//delete model
	
	repast::RepastProcess::instance()->done();	//finsih repast process
	
}

