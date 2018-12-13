/* Demo_03_Agent.h */

#ifndef COMMUTER
#define COMMUTER

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "Infrastructure.h"

/* Agents */
class Commuter {

private:
	repast::AgentId id_;
    double safety;
    double thresh;
    int TransMode;
	double Health;
	double CycleAbility;
	double CycleCost=500;
	double EconomicPosition;
	int TravelDist;
    int timestep = 0;
	int TransModeUsage=0;
	std::vector<Infrastructure*> InfinRange;

public:
    Commuter(repast::AgentId id,double InitialCar,double InitialBike, double InitialWalk, double InitialPTrans);
    Commuter() {}
    Commuter(repast::AgentId id, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition);

    ~Commuter();

    /* Required Getters */
    virtual repast::AgentId& getId() { return id_; }
    virtual const repast::AgentId& getId() const { return id_; }

    /* Getters specific to this kind of Agent */
    double getSafe() { return safety; }
    double getThresh() { return thresh; }
    int getMode() { return TransMode; }
	double getHealth() { return Health;}
	double getCycleAbility() {return CycleAbility;}

    /* Setter */
    void set(int currentRank, int newTravelDist, double newSafe, double newThresh, int newMode, double newHealth, double newCycleAbility, double newEconomicPosition);

    /* Actions */
    int ChooseMode(double TransCost);
    // Will indicate whether the agent cooperates or not; probability determined by = c / total
    void Travel(double Gsafety,double TransCost, repast::SharedContext<Commuter>* context,
        repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space, repast::SharedDiscreteSpace<Infrastructure, repast::WrapAroundBorders, repast::SimpleAdder<Infrastructure> >* Infspace); // Choose three other agents from the given context and see if they cooperate or not
    //void move(repast::SharedDiscreteSpace<Commuter, repast::WrapAroundBorders, repast::SimpleAdder<Commuter> >* space);
};

#endif
