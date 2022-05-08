#pragma once

#include <cstdint>
#include "LWM2M_Defines.h"

#define TYPE_INT		0x00
#define TYPE_STRING		0x01
#define TYPE_FLOAT		0x02
#define TYPE_BOOLEAN	0x04
#define TYPE_EXECUTABLE	0x05
#define TYPE_TIME		0x06
#define TYPE_OPAQUE		0x07

#define EXECUTABLE		0x00
#define READ_ONLY		0x01
#define WRITE_ONLY		0x02
#define READ_WRITE		0x04



union Value_t
{
	float float_value;
	bool  bool_value;
	int int_value;
};

class LWM2M_Resource
{
private:
	uint16_t resource_id;
	bool multi_level;
	uint8_t type;
	uint8_t permissions;
	Value_t values[MAX_RESOURCE_PARTS];
	char string_value[MAX_RESOURCE_PARTS][MAX_STRING_LENGTH];
	uint8_t(*execute_cb)();
	


public:
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level);
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)());

	uint16_t getResource_id()
	{
		return resource_id;
	}

	uint8_t getType()
	{
		return type;
	}

	uint8_t getPermissions()
	{
		return permissions;
	}

	auto getValue(int depth);
	
};