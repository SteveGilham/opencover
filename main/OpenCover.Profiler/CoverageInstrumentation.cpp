#include "StdAfx.h"
#include "CoverageInstrumentation.h"

#include <algorithm>

namespace CoverageInstrumentation
{
	using namespace Instrumentation;

    Instruction* InsertInjectedMethod(InstructionList &instructions, mdMethodDef injectedMethodDef, ULONG uniqueId)
    {
#pragma warning (disable : 26409) // new being used in a controlled fashion
		gsl::owner<Instruction*> firstInstruction = new Instruction(CEE_LDC_I4, uniqueId);
        instructions.push_back(firstInstruction);
        instructions.push_back(new Instruction(CEE_CALL, injectedMethodDef));
        return firstInstruction;
#pragma warning (default : 26409)
	}

    Instruction* InsertFunctionCall(InstructionList &instructions, mdSignature pvsig, FPTR pt, ULONGLONG uniqueId)
    {
#pragma warning (disable : 26409) // new being used in a controlled fashion
		gsl::owner<Instruction*> firstInstruction = new Instruction(CEE_LDC_I4, uniqueId);
        instructions.push_back(firstInstruction);
#ifdef _WIN64
		instructions.push_back(new Instruction(CEE_LDC_I8, static_cast<ULONGLONG>(pt)));
#else
		instructions.push_back(new Instruction(CEE_LDC_I4, static_cast<ULONG>(pt)));
#endif
        instructions.push_back(new Instruction(CEE_CALLI, pvsig));
#pragma warning (default : 26409)

        return firstInstruction;
    }
}