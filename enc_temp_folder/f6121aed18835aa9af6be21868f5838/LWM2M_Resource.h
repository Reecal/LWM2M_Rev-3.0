#pragma once

#include <cstdint>
#include "LWM2M_Defines.h"

union Value_t
{
	float float_value;
	bool  bool_value;
	int int_value;
};

class LWM2M_Resource
{
private:
	bool multi_level;
	uint8_t type;
	uint8_t permissions;
	Value_t values[MAX_RESOURCE_PARTS];
	


public:
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level);

	uint8_t getType()
	{
		return type;
	}

	uint8_t getPermission()
	{
		return permissions;
	}
};