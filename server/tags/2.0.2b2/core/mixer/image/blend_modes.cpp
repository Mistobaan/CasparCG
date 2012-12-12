#include "../../StdAfx.h"

#include "blend_modes.h"

#include <boost/algorithm/string.hpp>

namespace caspar { namespace core {
		
blend_mode::type get_blend_mode(const std::wstring& str)
{
	if(boost::iequals(str, L"normal"))
		return blend_mode::normal;
	else if(boost::iequals(str, L"lighten"))
		return blend_mode::lighten;
	else if(boost::iequals(str, L"darken"))
		return blend_mode::darken;
	else if(boost::iequals(str, L"multiply"))
		return blend_mode::multiply;
	else if(boost::iequals(str, L"average"))
		return blend_mode::average;
	else if(boost::iequals(str, L"add"))
		return blend_mode::add;
	else if(boost::iequals(str, L"subtract"))
		return blend_mode::subtract;
	else if(boost::iequals(str, L"difference"))
		return blend_mode::difference;
	else if(boost::iequals(str, L"negation"))
		return blend_mode::negation;
	else if(boost::iequals(str, L"exclusion"))
		return blend_mode::exclusion;
	else if(boost::iequals(str, L"screen"))
		return blend_mode::screen;
	else if(boost::iequals(str, L"overlay"))
		return blend_mode::overlay;
	else if(boost::iequals(str, L"soft_light"))
		return blend_mode::soft_light;
	else if(boost::iequals(str, L"hard_light"))
		return blend_mode::hard_light;
	else if(boost::iequals(str, L"color_dodge"))
		return blend_mode::color_dodge;
	else if(boost::iequals(str, L"color_burn"))
		return blend_mode::color_burn;
	else if(boost::iequals(str, L"linear_dodge"))
		return blend_mode::linear_dodge;
	else if(boost::iequals(str, L"linear_burn"))
		return blend_mode::linear_burn;
	else if(boost::iequals(str, L"linear_light"))
		return blend_mode::linear_light;
	else if(boost::iequals(str, L"vivid_light"))
		return blend_mode::vivid_light;
	else if(boost::iequals(str, L"pin_light"))
		return blend_mode::pin_light;
	else if(boost::iequals(str, L"hard_mix"))
		return blend_mode::hard_mix;
	else if(boost::iequals(str, L"reflect"))
		return blend_mode::reflect;
	else if(boost::iequals(str, L"glow"))
		return blend_mode::glow;
	else if(boost::iequals(str, L"phoenix"))
		return blend_mode::phoenix;
	else if(boost::iequals(str, L"contrast"))
		return blend_mode::contrast;
	else if(boost::iequals(str, L"saturation"))
		return blend_mode::saturation;
	else if(boost::iequals(str, L"color"))
		return blend_mode::color;
	else if(boost::iequals(str, L"luminosity"))
		return blend_mode::luminosity;
		
	return blend_mode::normal;
}

}}