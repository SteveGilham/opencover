#pragma once

#include "method.h"
#include "Messages.h"
#include <algorithm>

#ifdef _WIN64
typedef unsigned __int64 FPTR;
#else
typedef unsigned long FPTR;
#endif

namespace CoverageInstrumentation
{
    template<class IM>
    inline void AddSequenceCoverage(IM instrumentMethod, Instrumentation::Method& method, std::vector<SequencePoint> points)
    {
        if (points.size() == 0) return;
        for (auto it = points.begin(); it != points.end(); ++it)
        {
	        Instrumentation::InstructionList instructions;
            instrumentMethod(instructions, (*it).UniqueId);
            method.InsertInstructionsAtOriginalOffset((*it).Offset, instructions);
        }
    }

    template<class IM>
    void AddBranchCoverage(IM instrumentMethod, Instrumentation::Method& method, std::vector<BranchPoint> points, std::vector<SequencePoint> seqPoints)
    {
        if (points.size() == 0) return;

        for (auto it = method.m_instructions.begin(); it != method.m_instructions.end(); ++it)
        {

            if ((*it)->m_isBranch && ((*it)->m_origOffset != -1))
            {
                const OperationDetails &details = Instrumentation::Operations::m_mapNameOperationDetails[(*it)->m_operation];
                if (details.controlFlow == COND_BRANCH)
                {
					auto *pCurrent = *it;
                    ++it;
					auto *pNext = *it;

                    int idx = 0;

					Instrumentation::InstructionList instructions;

#pragma warning (suppress : 4820) // 4 bytes padding
					const auto bpp = std::find_if(points.begin(), points.end(), [pCurrent, idx](const BranchPoint &bp){return bp.Offset == pCurrent->m_origOffset && bp.Path == idx;});
					if (bpp == points.end()) // we can't find information on a branch to instrument (this may happen if it was skipped/ignored during initial investigation by the host process)
						continue;

                    ULONG uniqueId = (*bpp).UniqueId;
                    ULONG storedId = uniqueId; // store branch 0 ID (default/else)

                    auto pJumpNext = std::make_unique<Instrumentation::Instruction>(CEE_BR);
                    pJumpNext->m_isBranch = true;
                    pJumpNext->m_branches.push_back(pNext);

                    // Yield the owndership of the pointer to the collection
                    Instrumentation::Instruction * pJumpNextNonOwner{ nullptr };
                    instructions.push_back(pJumpNextNonOwner = pJumpNext.release());

                    // collect branches instrumentation
                    for(auto sbit = pCurrent->m_branches.begin(); sbit != pCurrent->m_branches.end(); ++sbit)
                    {
                        idx++;
#pragma warning (suppress : 4820) // 4 bytes padding
						uniqueId = (*std::find_if(points.begin(), points.end(), [pCurrent, idx](const BranchPoint &bp){return bp.Offset == pCurrent->m_origOffset && bp.Path == idx;})).UniqueId;
                        auto pBranchInstrument = instrumentMethod(instructions, uniqueId);
                        auto pBranchJump = std::make_unique<Instrumentation::Instruction>(CEE_BR);
                        pBranchJump->m_isBranch = true;
                        pBranchJump->m_branches.push_back(*sbit);
                        instructions.push_back(pBranchJump.release());
                        *sbit = pBranchInstrument; // rewire conditional branch to instrumentation
                        
                    }
                    
                    // now instrument "default:" or "else" branch
                    // insert all instrumentation at pNext
                    // ----------------------------------------
                    //        IL_xx Conditional Branch instruction with arguments (at BranchPoint.Offset)
                    //        IL_xx BR pNext -> rewired to pElse (Path 0)
                    //        IL_xx Path 1 Instrument
                    //        IL_xx pBranchJump back to original Path 1 Instruction
                    //        IL_xx Path 2 Instrument
                    //        IL_xx pBranchJump back to original Path 2 Instruction
                    //        IL_xx Path N.. Instrument
                    //        IL_xx pBranchJump back to original Path N.. Instruction
                    // pElse: IL_xx Path 0 Instrument 
                    // pNext: IL_xx Whatever it is 
                    
                    auto pElse = instrumentMethod(instructions, storedId);
                    pJumpNextNonOwner->m_branches[0] = pElse; // rewire pJumpNext

                    // ReSharper disable once CppPossiblyErroneousEmptyStatements
                    for (it = method.m_instructions.begin(); *it != pNext; ++it);

                    method.m_instructions.insert(it, instructions.begin(), instructions.end());
                    
                    // restore 'it' position
                    for (it = method.m_instructions.begin(); *it != pNext; ++it);
                }
            }
        }
        method.RecalculateOffsets();
    }

	Instrumentation::Instruction* InsertInjectedMethod(Instrumentation::InstructionList &instructions, mdMethodDef injectedMethodDef, ULONG uniqueId);
	Instrumentation::Instruction* InsertFunctionCall(Instrumentation::InstructionList &instructions, mdSignature pvsig, FPTR pt, ULONGLONG uniqueId);

}

