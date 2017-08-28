//
// OpenCover - S Wilde
//
// This source code is released under the MIT License; see the accompanying license file.
//
#include "StdAfx.h"
#include "Instruction.h"

namespace Instrumentation
{
	Instruction::Instruction() :
		m_offset(-1), m_operation(CEE_NOP), m_operand(0),
		m_isBranch(false), m_origOffset(-1)
	{
	}

	Instruction::Instruction(CanonicalName operation, ULONGLONG operand) : 
		m_offset(-1), m_operation(operation), m_operand(operand),
		m_isBranch(false), m_origOffset(-1)
	{
	}

	Instruction::Instruction(CanonicalName operation) :
		m_offset(-1), m_operation(operation), m_operand(0),
		m_isBranch(false), m_origOffset(-1)
	{
	}

	Instruction& Instruction::operator = (const Instruction& b)
	{
		if (&b != this)
		{
			m_offset = b.m_offset;
			m_operation = b.m_operation;
			m_operand = b.m_operand;
			m_isBranch = b.m_isBranch;
			m_branchOffsets = b.m_branchOffsets;
			m_branches = b.m_branches;
			m_joins = b.m_joins;
			m_origOffset = b.m_origOffset;
		}
		return *this;
	}

	bool Instruction::Equivalent(const Instruction&b)
	{
		if (m_operation != b.m_operation) 
			return false;
		if (m_operand != b.m_operand) 
			return false;
		if (m_branches.size() != b.m_branches.size()) 
			return false;
		auto it2 = b.m_branches.begin();
		for (const auto it : m_branches)
		{
			if (!it->Equivalent(*(*it2)))
				return false;
			++it2;
		}
		return true;
	}
}