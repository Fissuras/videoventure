#include "StdAfx.h"

#include "Expression.h"
#include "ExpressionTime.h"
#include "ExpressionEntity.h"
#include "ExpressionConvert.h"
#include "ExpressionConfigure.h"

template<typename T> static void ConfigureWorldTime(const TiXmlElement *element, std::vector<unsigned int> &buffer, const char * const names[], const float defaults[])
{
	Expression::Convert<T, float>::Append(buffer);
	Expression::Append(buffer, EvaluateWorldTime);
}

static Expression::Loader<float>::Auto worldtimefloat(0xf667bf8a /* "worldtime" */, ConfigureWorldTime<float>);
static Expression::Loader<__m128>::Auto worldtimevector(0xf667bf8a /* "worldtime" */, ConfigureWorldTime<__m128>);

template<typename T> static void ConfigureTime(const TiXmlElement *element, std::vector<unsigned int> &buffer, const char * const names[], const float defaults[])
{
	Expression::Convert<T, float>::Append(buffer);
	Expression::Append(buffer, EvaluateTime);
}

static Expression::Loader<float>::Auto timefloat(0x5d3c9be4 /* "time" */, ConfigureTime<float>);
static Expression::Loader<__m128>::Auto timevector(0x5d3c9be4 /* "time" */, ConfigureTime<__m128>);

//
// TIME EXPRESSION
//

// world time
float EvaluateWorldTime(Expression::Context &aContext)
{
	return (sim_turn + sim_fraction) / sim_rate;
}

// entity local time
float EvaluateTime(EntityContext &aContext)
{
	return aContext.mParam;
}
