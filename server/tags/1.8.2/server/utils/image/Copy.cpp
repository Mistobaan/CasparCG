/*
* copyright (c) 2010 Sveriges Television AB <info@casparcg.com>
*
*  This file is part of CasparCG.
*
*    CasparCG is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    CasparCG is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.

*    You should have received a copy of the GNU General Public License
*    along with CasparCG.  If not, see <http://www.gnu.org/licenses/>.
*
*/
 
#include "..\..\stdafx.h"

#include "Copy.hpp"

#include <intrin.h>
#include <functional>

#include "../Types.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace std::tr1::placeholders;

namespace caspar{
namespace utils{
namespace image{

static const size_t STRIDE = sizeof(__m128i)*4;

void DoCopyParallel(const tbb::blocked_range<size_t>& r, const std::tr1::function<void(void*, const void*, size_t)>& func, void* dest, const void* source)
{
	size_t offset = r.begin()*STRIDE;
	size_t size = r.size()*STRIDE;
	func(reinterpret_cast<s8*>(dest) + offset, reinterpret_cast<const s8*>(source) + offset, size);
}

void CopyParallel(const std::tr1::function<void(void*, const void*, size_t)>& func, void* dest, const void* source, size_t size)
{
	tbb::parallel_for(tbb::blocked_range<size_t>(0, size/STRIDE), std::tr1::bind(&DoCopyParallel, _1, func, dest, source));	
}

CopyFun GetCopyFun(SIMD simd)
{
	if(simd >= SSE2)
		return CopyParallel_SSE2;
	else
		return CopyParallel_REF;
}

// TODO: (R.N) optimize => prefetch and cacheline loop unroll
void Copy_SSE2(void* dest, const void* source, size_t size)
{
	__m128i val = _mm_setzero_si128();
	__m128i* pD = reinterpret_cast<__m128i*>(dest);
	const __m128i* pS = reinterpret_cast<const __m128i*>(source);

	int times = size / 16;
	for(int i=0; i < times; ++i) 
	{
		val = _mm_load_si128(pS);
		_mm_stream_si128(pD, val);

		++pD;
		++pS;
	}
	_mm_mfence();	//ensure last WC buffers get flushed to memory
}

void CopyParallel_SSE2(void* dest, const void* source, size_t size)
{
	CopyParallel(&Copy_SSE2, dest, source, size);
}

void Copy_REF(void* dest, const void* source, size_t size)
{
	__movsd(reinterpret_cast<unsigned long*>(dest), reinterpret_cast<const unsigned long*>(source), size/4);
}

void CopyParallel_REF(void* dest, const void* source, size_t size)
{
	CopyParallel(&Copy_REF, dest, source, size);
}

}
}
}