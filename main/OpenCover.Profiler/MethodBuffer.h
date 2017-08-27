//
// OpenCover - S Wilde
//
// This source code is released under the MIT License; see the accompanying license file.
//
#pragma once

namespace Instrumentation
{
	/// <summary>The <c>MethodBuffer</c> encapsulates all the buffer handling</summary>
	class MethodBuffer
	{
	protected:
		MethodBuffer()
			: m_bufferBase(nullptr),
			m_bufferCurrent(nullptr),
			m_position(0L)
		{
		}

		void SetBuffer(BYTE* pBuffer) {
			m_bufferBase = pBuffer;
			m_bufferCurrent = pBuffer;
			m_position = 0;
		}

		unsigned long GetPosition() const { return m_position; }

		template<typename value_type> value_type Read() {
			_ASSERTE(m_bufferCurrent != NULL);
#pragma warning (suppress : 26490) // simplest deserialization
			value_type value = *reinterpret_cast<value_type*>(m_bufferCurrent);
			Advance(sizeof(value_type));
			return value;
		}

		template<typename value_type> void Write(value_type value) {
			_ASSERTE(m_bufferCurrent != NULL);
#pragma warning (suppress : 26490) // simplest serialization
			*reinterpret_cast<value_type*>(m_bufferCurrent) = value;
			Advance(sizeof(value_type));
		}

		template<typename value_type> void Align() {
			_ASSERTE(m_bufferCurrent != NULL);
			const long i = sizeof(value_type) - 1;
			long incr = static_cast<long>(((m_position + i) & ~i) - m_position);
			Advance(incr);
		}

		void Advance(long num) {
			_ASSERTE(m_bufferCurrent != NULL);
			std::advance(m_bufferCurrent, num);
			m_position += num;
		}

	private:
		BYTE * m_bufferBase;
		BYTE * m_bufferCurrent;
		unsigned long m_position;
#pragma warning (suppress : 4820) // 4 bytes padding
	};
}