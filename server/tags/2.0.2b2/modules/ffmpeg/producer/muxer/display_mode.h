#pragma once

#include <core/video_format.h>

namespace caspar { namespace ffmpeg {
	
struct display_mode
{
	enum type
	{
		simple,
		duplicate,
		half,
		interlace,
		deinterlace_bob,
		deinterlace_bob_reinterlace,
		deinterlace,
		count,
		invalid
	};

	static std::wstring print(display_mode::type value)
	{
		switch(value)
		{
			case simple:						return L"simple";
			case duplicate:						return L"duplicate";
			case half:							return L"half";
			case interlace:						return L"interlace";
			case deinterlace_bob:				return L"deinterlace_bob";
			case deinterlace_bob_reinterlace:	return L"deinterlace_bob_reinterlace";
			case deinterlace:					return L"deinterlace";
			default:							return L"invalid";
		}
	}
};

display_mode::type get_display_mode(const core::field_mode::type in_mode, double in_fps, const core::field_mode::type out_mode, double out_fps)
{		
	static const auto epsilon = 2.0;

	if(in_fps < 20.0 || in_fps > 80.0)
	{
		//if(out_mode != core::field_mode::progressive && in_mode == core::field_mode::progressive)
		//	return display_mode::interlace;
		
		if(out_mode == core::field_mode::progressive && in_mode != core::field_mode::progressive)
		{
			if(in_fps < 35.0)
				return display_mode::deinterlace;
			else
				return display_mode::deinterlace_bob;
		}
	}

	if(std::abs(in_fps - out_fps) < epsilon)
	{
		if(in_mode != core::field_mode::progressive && out_mode == core::field_mode::progressive)
			return display_mode::deinterlace;
		//else if(in_mode == core::field_mode::progressive && out_mode != core::field_mode::progressive)
		//	simple(); // interlace_duplicate();
		else
			return display_mode::simple;
	}
	else if(std::abs(in_fps/2.0 - out_fps) < epsilon)
	{
		if(in_mode != core::field_mode::progressive)
			return display_mode::invalid;

		if(out_mode != core::field_mode::progressive)
			return display_mode::interlace;
		else
			return display_mode::half;
	}
	else if(std::abs(in_fps - out_fps/2.0) < epsilon)
	{
		if(out_mode != core::field_mode::progressive)
			return display_mode::invalid;

		if(in_mode != core::field_mode::progressive)
			return display_mode::deinterlace_bob;
		else
			return display_mode::duplicate;
	}

	return display_mode::invalid;
}

}}