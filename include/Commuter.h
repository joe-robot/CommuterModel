/* Commuter.h */

//By Joseph Cresswell	Reg No. 150148395

#ifndef COMMUTER
#define COMMUTER

//including required files
#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "Infrastructure.h"

/* Agents */
class Commuter {

private:
	//Defininf private variable of commuters
	repast::AgentId id_;
    double safety;
    double thresh;
    int TransMode;
	double Health;
	double CycleAbility;
	double CycleCost=500;	//initialising cycling cost as 500
	double EconomicPosition;
	int TravelDist;
    int timestep = 0;
	int TransModeUsage=0;
	std::vector<Infrastructure*> InfinRange={};

public:
	//Defining commuter public functions
    Commuter(repast::AgentId id,double InitialCar,double InitialBike, double InitialWalk, double InitialPTrans);	//Commuter constructor
    Commuter() {}
    Commuter(repast::AgentId id, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition);

    ~Commuter(); //Commuter destuctor

    /* Required Getters */
    virtual repast::AgentId& getId() { return id_; }
    virtual const repast::AgentId& getId() const { return id_; }

    /* Getters specific to this kind of Agent */
    double getSafe() { return safety; }
    int getMode() { return TransMode; }
	double getHealth() { return Health;}
	double getCycleAbility() {return CycleAbility;}

    /* Setter */
    void set(int currentRank, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition);

    /* Actions */
    int ChooseMode(double TransCost);
    void Travel(double Gsafety,double TransCost, repast::SharedContext<Commuter>* context,
        repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace); 
};

#endif
