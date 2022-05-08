#pragma once

#include <cstdint>
#include <string.h>
#include <string>

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
	uint8_t next_value_ptr = 0;
	Value_t values[MAX_RESOURCE_PARTS];
	char string_value[MAX_RESOURCE_PARTS][MAX_STRING_LENGTH];
	uint8_t(*execute_cb)();
	std::string string_val[MAX_RESOURCE_PARTS];
	


public:
	LWM2M_Resource(uint16_t resource_id = 0, uint8_t type = TYPE_INT, uint8_t permissions = READ_WRITE, bool multi_level = false);
	LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value = 0);
	LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value = false);
	LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value = 0);
	LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value = nullptr);
	LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)());

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

	bool getMultiLevel()
	{
		return multi_level;
	}

	std::string getValue(int depth = 0);

	void setValue(float val, int depth = 0)
	{
		values[depth].float_value = val;
		string_val[depth] = std::to_string(val);
	}

	void setValue(bool val, int depth = 0)
	{
		values[depth].bool_value = val;
		string_val[depth] = std::to_string(val);
	}

	void setValue(int val, int depth = 0)
	{
		values[depth].int_value = val;
		string_val[depth] = std::to_string(val);
	}

	void setValue(char* val, int depth = 0)
	{
		strcpy_s(string_value[depth], MAX_STRING_LENGTH, val);
		string_val[depth] = std::string(val);
	}

	void setValue(uint8_t(*execute_func)(), int depth = 0)
	{
		execute_cb = execute_func;
	}

	bool value_exists(uint8_t value_depth);
	
	
};