#include <UT/UT_DSOVersion.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include "SOP_Smoke_Solve_Sim.h"
#include <GU/GU_PrimVDB.h>
#include <iostream>
#include "GA/GA_AIFBlindData.h"
#include "smoke/lib/core/SimData.h"
#include "../utils/BlindDataManager.h"

using namespace HDK_Sample;
void newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(
			new OP_Operator("smoke_solve_sim", "Smoke Solve Sim", SOP_Smoke_Solve_Sim::myConstructor,
							SOP_Smoke_Solve_Sim::myTemplateList, 1, // Min required sources
							1,	// Maximum sources
							0));
}
// The names here have to match the inline evaluation functions
static PRM_Name names[] =
{ PRM_Name("reset", "Reset Frame"), PRM_Name("subSteps", "Sub Steps"), PRM_Name("sim_time_scale",
																				"Simulation Time Scale"), };
static PRM_Default defaultSubSteps(1);
PRM_Template SOP_Smoke_Solve_Sim::myTemplateList[] =
{ PRM_Template(PRM_INT, 1, &names[0], PRMoneDefaults), PRM_Template(PRM_INT_J, 1, &names[1],
																	&defaultSubSteps),
	PRM_Template(PRM_FLT, 1, &names[2], PRMoneDefaults), PRM_Template(), };
int * SOP_Smoke_Solve_Sim::myOffsets = 0;
OP_Node *
SOP_Smoke_Solve_Sim::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	return new SOP_Smoke_Solve_Sim(net, name, op);
}
SOP_Smoke_Solve_Sim::SOP_Smoke_Solve_Sim(OP_Network *net, const char *name, OP_Operator *op) :
		SOP_Node(net, name, op)
{
	if (!myOffsets)
		myOffsets = allocIndirect(32);
	sourceGDP = NULL;
	simDataPtr = NULL;
	myLastCookTime = 0;
}
SOP_Smoke_Solve_Sim::~SOP_Smoke_Solve_Sim()
{
}
void SOP_Smoke_Solve_Sim::initSystem()
{

}
OP_ERROR SOP_Smoke_Solve_Sim::cookMySop(OP_Context &context)
{
	fpreal reset, currframe;
	CH_Manager *chman;
	gdp->clearAndDestroy();
	if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();
	OP_Node::flags().timeDep = 1;
	duplicateSource(0, context);
	sourceGDP = inputGeo(0);
	chman = OPgetDirector()->getChannelManager();
	currframe = chman->getSample(context.getTime());
	reset = RESET();

	if (currframe <= reset)
	{
		myLastCookTime = reset;
		initSystem();
	}
//	currframe += 0.05;	// Add a bit to avoid floating point error

	//Access Blind Data
	smoke::houdini::utils::BlindDataManager blindDataManager;
	simDataPtr = blindDataManager.extractSimDataPtr(gdp);
	//Access Blind Data
	smoke::houdini::adapter::SolveSimAdapter adapter(simDataPtr);

	while (myLastCookTime < currframe)
	{
		fpreal fps = OPgetDirector()->getChannelManager()->getSamplesPerSec();
		adapter.stepForward(fps / simDataPtr->getSimulationTimeScale(), simDataPtr->getSubSteps());
		myLastCookTime += 1;
	}

	GU_PrimVDB* vdb = GU_PrimVDB::buildFromGrid((GU_Detail&) *gdp, simDataPtr->getDensityGridPtr(), NULL,
			"gridName");

	unlockInputs();
	return error();
}
const char *
SOP_Smoke_Solve_Sim::inputLabel(unsigned inum) const
{
	switch (inum)
	{
	case 0:
		return "Solid Sim";
	}
	return "Unknown source";
}
