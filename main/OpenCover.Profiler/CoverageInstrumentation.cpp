#include "StdAfx.h"
#include "CoverageInstrumentation.h"

#include <algorithm>

namespace CoverageInstrumentation
{
	using namespace Instrumentation;

    Instruction* InsertInjectedMethod(InstructionList &instructions, mdMethodDef injectedMethodDef, ULONG uniqueId)
    {
		auto firstInstruction = std::make_unique<Instruction>(CEE_LDC_I4, uniqueId);
        instructions.push_back(firstInstruction.get());
        instructions.push_back(std::make_unique<Instruction>(CEE_CALL, injectedMethodDef).release());
        return firstInstruction.release();
	}

    Instruction* InsertFunctionCall(InstructionList &instructions, mdSignature pvsig, FPTR pt, ULONGLONG uniqueId)
    {
		auto firstInstruction = std::make_unique<Instruction>(CEE_LDC_I4, uniqueId);
        instructions.push_back(firstInstruction.get());
#ifdef _WIN64
		instructions.push_back(std::make_unique<Instruction>(CEE_LDC_I8, static_cast<ULONGLONG>(pt)).release());
#else
		instructions.push_back(std::make_unique<Instruction>(CEE_LDC_I4, static_cast<ULONG>(pt)).release());
#endif
        instructions.push_back(std::make_unique<Instruction>(CEE_CALLI, pvsig).release());
        return firstInstruction.release();
    }
}