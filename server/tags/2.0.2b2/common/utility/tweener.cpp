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

// The following code is based on Tweener for actionscript, http://code.google.com/p/tweener/
//
//Disclaimer for Robert Penner's Easing Equations license:
//
//TERMS OF USE - EASING EQUATIONS
//
//Open source under the BSD License.
//
//Copyright � 2001 Robert Penner
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "../stdafx.h"

#include "tweener.h"

#include <boost/assign/list_of.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include <unordered_map>
#include <string>
#include <locale>
#include <functional>
#include <vector>

namespace caspar {

typedef std::function<double(double, double, double, double)> tweener_t;
			
static const double PI = std::atan(1.0)*4.0;
static const double H_PI = std::atan(1.0)*2.0;

double ease_none (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*t/d + b;
}

double ease_in_quad (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*(t/=d)*t + b;
}
	
double ease_out_quad (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return -c *(t/=d)*(t-2) + b;
}	

double ease_in_out_quad (double t, double b, double c, double d, const std::vector<double>& params)
{
	if ((t/=d/2) < 1) 
		return c/2*t*t + b;

	return -c/2 * ((--t)*(t-2) - 1) + b;
}	

double ease_out_in_quad (double t, double b, double c, double d, const std::vector<double>& params)
{
	if (t < d/2) 
		return ease_out_quad (t*2, b, c/2, d, params);

	return ease_in_quad((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_in_cubic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*(t/=d)*t*t + b;
}	

double ease_out_cubic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*((t=t/d-1)*t*t + 1) + b;
}
	
double ease_in_out_cubic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if ((t/=d/2) < 1) 
		return c/2*t*t*t + b;

	return c/2*((t-=2)*t*t + 2) + b;
}
	
double ease_out_in_cubic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) return ease_out_cubic (t*2, b, c/2, d, params);
	return ease_in_cubic((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_in_quart (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*(t/=d)*t*t*t + b;
}
	
double ease_out_quart (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return -c * ((t=t/d-1)*t*t*t - 1) + b;
}	

double ease_in_out_quart (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if ((t/=d/2) < 1)
		return c/2*t*t*t*t + b;

	return -c/2 * ((t-=2)*t*t*t - 2) + b;
}	

double ease_out_in_quart (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2)
		return ease_out_quart (t*2, b, c/2, d, params);

	return ease_in_quart((t*2)-d, b+c/2, c/2, d, params);
}	

double ease_in_quint (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*(t/=d)*t*t*t*t + b;
}
	
double ease_out_quint (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c*((t=t/d-1)*t*t*t*t + 1) + b;
}
	
double ease_in_out_quint (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if ((t/=d/2) < 1) 
		return c/2*t*t*t*t*t + b;

	return c/2*((t-=2)*t*t*t*t + 2) + b;
}
	
double ease_out_in_quint (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) 
		return ease_out_quint (t*2, b, c/2, d, params);

	return ease_in_quint((t*2)-d, b+c/2, c/2, d, params);
}	

double ease_in_sine (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return -c * std::cos(t/d * (PI/2)) + c + b;
}	

double ease_out_sine (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c * std::sin(t/d * (PI/2)) + b;
}	

double ease_in_out_sine (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return -c/2 * (std::cos(PI*t/d) - 1) + b;
}	

double ease_out_in_sine (double t, double b, double c, double d, const std::vector<double>& params)
{
	if (t < d/2) 
		return ease_out_sine (t*2, b, c/2, d, params);
	
	return ease_in_sine((t*2)-d, b+c/2, c/2, d, params);
}	

double ease_in_expo (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return (t==0) ? b : c * std::pow(2, 10 * (t/d - 1)) + b - c * 0.001;
}	

double ease_out_expo (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return (t==d) ? b+c : c * 1.001 * (-std::pow(2, -10 * t/d) + 1) + b;
}
	
double ease_in_out_expo (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t==0) 
		return b;
	if (t==d) 
		return b+c;
	if ((t/=d/2) < 1) 
		return c/2 * std::pow(2, 10 * (t - 1)) + b - c * 0.0005;

	return c/2 * 1.0005 * (-std::pow(2, -10 * --t) + 2) + b;
}
	
double ease_out_in_expo (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) 
		return ease_out_expo (t*2, b, c/2, d, params);

	return ease_in_expo((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_in_circ (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return -c * (std::sqrt(1 - (t/=d)*t) - 1) + b;
}
	
double ease_out_circ (double t, double b, double c, double d, const std::vector<double>& params) 
{
	return c * std::sqrt(1 - (t=t/d-1)*t) + b;
}
	
double ease_in_out_circ (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if ((t/=d/2) < 1) 
		return -c/2 * (std::sqrt(1 - t*t) - 1) + b;

	return c/2 * (std::sqrt(1 - (t-=2)*t) + 1) + b;
}
	
double ease_out_in_circ (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) return ease_out_circ(t*2, b, c/2, d, params);
	return ease_in_circ((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_in_elastic (double t, double b, double c, double d, const std::vector<double>& params)
{
	if (t==0) return b;
	if ((t/=d)==1) return b+c;
	//var p:Number = !Boolean(p_params) || isNaN(p_params.period) ? d*.3 : p_params.period;
	//var s:Number;
	//var a:Number = !Boolean(p_params) || isNaN(p_params.amplitude) ? 0 : p_params.amplitude;
	double p = params.size() > 0 ? params[0] : d*0.3;
	double s;
	double a = params.size() > 1 ? params[1] : 0.0;
	if (a == 0.0 || a < std::abs(c)) 
	{
		a = c;
		s = p/4;
	} 
	else 
		s = p/(2*PI) * std::asin (c/a);
	
	return -(a*std::pow(2,10*(t-=1)) * std::sin( (t*d-s)*(2*PI)/p )) + b;
}
	
double ease_out_elastic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t==0) 
		return b;
	if ((t/=d)==1) 
		return b+c;
	//var p:Number = !Boolean(p_params) || isNaN(p_params.period) ? d*.3 : p_params.period;
	//var s:Number;
	//var a:Number = !Boolean(p_params) || isNaN(p_params.amplitude) ? 0 : p_params.amplitude;
	double p = params.size() > 0 ? params[0] : d*0.3;
	double s;
	double a = params.size() > 1 ? params[1] : 0.0;
	if (a == 0.0 || a < std::abs(c))
	{
		a = c;
		s = p/4;
	} 
	else 
		s = p/(2*PI) * std::asin (c/a);
	
	return (a*std::pow(2,-10*t) * std::sin( (t*d-s)*(2*PI)/p ) + c + b);
}	

double ease_in_out_elastic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t==0)
		return b;
	if ((t/=d/2)==2) 
		return b+c;
	//var p:Number = !Boolean(p_params) || isNaN(p_params.period) ? d*(.3*1.5) : p_params.period;
	//var s:Number;
	//var a:Number = !Boolean(p_params) || isNaN(p_params.amplitude) ? 0 : p_params.amplitude;
	double p = params.size() > 0 ? params[0] : d*0.3*1.5;
	double s;
	double a = params.size() > 1 ? params[1] : 0.0;
	if (a == 0.0 || a < std::abs(c)) 
	{
		a = c;
		s = p/4;
	}
	else
		s = p/(2*PI) * std::asin (c/a);
	
	if (t < 1) 
		return -.5*(a*std::pow(2,10*(t-=1)) * std::sin( (t*d-s)*(2*PI)/p )) + b;

	return a*std::pow(2,-10*(t-=1)) * std::sin( (t*d-s)*(2*PI)/p )*.5 + c + b;
}
	
double ease_out_in_elastic (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) return ease_out_elastic (t*2, b, c/2, d, params);
	return ease_in_elastic((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_in_back (double t, double b, double c, double d, const std::vector<double>& params) 
{
	//var s:Number = !Boolean(p_params) || isNaN(p_params.overshoot) ? 1.70158 : p_params.overshoot;
	double s = params.size() > 0 ? params[0] : 1.70158;
	return c*(t/=d)*t*((s+1)*t - s) + b;
}
	
double ease_out_back (double t, double b, double c, double d, const std::vector<double>& params)
{
	//var s:Number = !Boolean(p_params) || isNaN(p_params.overshoot) ? 1.70158 : p_params.overshoot;
	double s = params.size() > 0 ? params[0] : 1.70158;
	return c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
}
	
double ease_in_out_back (double t, double b, double c, double d, const std::vector<double>& params)
{
	//var s:Number = !Boolean(p_params) || isNaN(p_params.overshoot) ? 1.70158 : p_params.overshoot;
	double s = params.size() > 0 ? params[0] : 1.70158;
	if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525))+1)*t - s)) + b;
	return c/2*((t-=2)*t*(((s*=(1.525))+1)*t + s) + 2) + b;
}
	
double ease_out_int_back (double t, double b, double c, double d, const std::vector<double>& params)
{
	if (t < d/2) return ease_out_back (t*2, b, c/2, d, params);
	return ease_in_back((t*2)-d, b+c/2, c/2, d, params);
}
	
double ease_out_bounce (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if ((t/=d) < (1/2.75))
		return c*(7.5625*t*t) + b;
	else if (t < (2/2.75))
		return c*(7.5625*(t-=(1.5/2.75))*t + .75) + b;
	else if (t < (2.5/2.75))
		return c*(7.5625*(t-=(2.25/2.75))*t + .9375) + b;
	else 
		return c*(7.5625*(t-=(2.625/2.75))*t + .984375) + b;	
}
	
double ease_in_bounce (double t, double b, double c, double d, const std::vector<double>& params)
{
	return c - ease_out_bounce (d-t, 0, c, d, params) + b;
}

double ease_in_out_bounce (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) return ease_in_bounce (t*2, 0, c, d, params) * .5 + b;
	else return ease_out_bounce (t*2-d, 0, c, d, params) * .5 + c*.5 + b;
}
	

double ease_out_in_bounce (double t, double b, double c, double d, const std::vector<double>& params) 
{
	if (t < d/2) return ease_out_bounce (t*2, b, c/2, d, params);
	return ease_in_bounce((t*2)-d, b+c/2, c/2, d, params);
}

tweener_t get_tweener(std::wstring name)
{
	std::transform(name.begin(), name.end(), name.begin(), std::tolower);

	if(name == L"linear")
		return [](double t, double b, double c, double d){return ease_none(t, b, c, d, std::vector<double>());};
	
	std::vector<double> params;
	
	static const boost::wregex expr(L"(?<NAME>\\w*)(:(?<V0>\\d+\\.?\\d?))?(:(?<V1>\\d+\\.?\\d?))?"); // boost::regex has no repeated captures?
	boost::wsmatch what;
	if(boost::regex_match(name, what, expr))
	{
		name = what["NAME"].str();
		if(what["V0"].matched)
			params.push_back(boost::lexical_cast<double>(what["V0"].str()));
		if(what["V1"].matched)
			params.push_back(boost::lexical_cast<double>(what["V1"].str()));
	}
		
	typedef std::function<double(double, double, double, double, const std::vector<double>&)> tween_t;	
	static const std::unordered_map<std::wstring, tween_t> tweens = boost::assign::map_list_of	
		(L"",					ease_none		   )	
		(L"linear",				ease_none		   )	
		(L"easenone",			ease_none		   )
		(L"easeinquad",			ease_in_quad	   )
		(L"easeoutquad",		ease_out_quad	   )
		(L"easeinoutquad",		ease_in_out_quad   )
		(L"easeoutinquad",		ease_out_in_quad   )
		(L"easeincubic",		ease_in_cubic	   )
		(L"easeoutcubic",		ease_out_cubic	   )
		(L"easeinoutcubic",		ease_in_out_cubic  )
		(L"easeoutincubic",		ease_out_in_cubic  )
		(L"easeinquart", 		ease_in_quart 	   )
		(L"easeoutquart",		ease_out_quart	   )
		(L"easeinoutquart",		ease_in_out_quart  )
		(L"easeoutinquart",		ease_out_in_quart  )
		(L"easeinquint",		ease_in_quint	   )
		(L"easeoutquint",		ease_out_quint	   )
		(L"easeinoutquint",		ease_in_out_quint  )
		(L"easeoutinquint",		ease_out_in_quint  )
		(L"easeinsine",			ease_in_sine	   )
		(L"easeoutsine",		ease_out_sine	   )
		(L"easeinoutsine",		ease_in_out_sine   )
		(L"easeoutinsine",		ease_out_in_sine   )
		(L"easeinexpo",			ease_in_expo	   )
		(L"easeoutexpo",		ease_out_expo	   )
		(L"easeinoutexpo",		ease_in_out_expo   )
		(L"easeoutinexpo",		ease_out_in_expo   )
		(L"easeincirc",			ease_in_circ	   )
		(L"easeoutcirc",		ease_out_circ	   )
		(L"easeinoutcirc",		ease_in_out_circ   )
		(L"easeoutincirc",		ease_out_in_circ   )
		(L"easeinelastic",		ease_in_elastic	   )
		(L"easeoutelastic",		ease_out_elastic   )
		(L"easeinoutelastic",	ease_in_out_elastic)
		(L"easeoutinelastic",	ease_out_in_elastic)
		(L"easeinback",			ease_in_back	   )
		(L"easeoutback",		ease_out_back	   )
		(L"easeinoutback",		ease_in_out_back   )
		(L"easeoutintback",		ease_out_int_back  )
		(L"easeoutbounce",		ease_out_bounce	   )
		(L"easeinbounce",		ease_in_bounce	   )
		(L"easeinoutbounce",	ease_in_out_bounce )
		(L"easeoutinbounce",	ease_out_in_bounce );

	auto it = tweens.find(name);
	if(it == tweens.end())
		it = tweens.find(L"linear");
	
	return [=](double t, double b, double c, double d)
	{
		return it->second(t, b, c, d, params);
	};
};

}