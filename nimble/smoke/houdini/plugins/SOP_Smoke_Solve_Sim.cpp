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
			new OP_Operator("smoke_solve_sim", "Smoke Solve Sim",
					SOP_Smoke_Solve_Sim::myConstructor,
					SOP_Smoke_Solve_Sim::myTemplateList, 1, // Min required sources
					1,	// Maximum sources
					0));
}

PRM_Template SOP_Smoke_Solve_Sim::myTemplateList[] =
{ PRM_Template(), };

OP_Node *
SOP_Smoke_Solve_Sim::myConstructor(OP_Network *net, const char *name,
		OP_Operator *op)
{
	return new SOP_Smoke_Solve_Sim(net, name, op);
}
SOP_Smoke_Solve_Sim::SOP_Smoke_Solve_Sim(OP_Network *net, const char *name,
		OP_Operator *op) :
		SOP_Node(net, name, op)
{
//	sourceGDP = NULL;
	simDataPtr = NULL;
	myLastCookTime = 0;
}
SOP_Smoke_Solve_Sim::~SOP_Smoke_Solve_Sim()
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
	//Access Blind Data
	smoke::houdini::utils::BlindDataManager blindDataManager;
	simDataPtr = blindDataManager.extractSimDataPtr(gdp);
	chman = OPgetDirector()->getChannelManager();
	currframe = chman->getSample(context.getTime());
	reset = simDataPtr->getResetFrame();

	if (currframe <= reset)
	{
		myLastCookTime = reset;
	}
//	currframe += 0.05;	// Add a bit to avoid floating point error

	//Access Blind Data
	smoke::houdini::adapter::SolveSimAdapter adapter(simDataPtr);

	while (myLastCookTime < currframe)
	{
		fpreal fps = OPgetDirector()->getChannelManager()->getSamplesPerSec();
		adapter.stepForward(fps / simDataPtr->getSimulationTimeScale(),
				simDataPtr->getSubSteps());
		myLastCookTime += 1;
//		std::cout << "myLastCookTime : " << myLastCookTime << std::endl;
	}

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
