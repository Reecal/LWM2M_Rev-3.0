#include "LWM2M_Resource.h"

#include <string.h>

LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	string_val[0] = "";
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	values->float_value = default_value;
	string_val[0] = std::to_string(default_value);
	next_value_ptr++;
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	values->bool_value = default_value;
	string_val[0] = default_value ? std::string("1") : std::string("0");
	next_value_ptr++;
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	values->int_value = default_value;
	string_val[0] = std::to_string(default_value);
	next_value_ptr++;
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	strcpy_s(string_value[0], MAX_STRING_LENGTH, default_value);
	string_val[0] = std::string(default_value);
	next_value_ptr++;
}

LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)()) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level), execute_cb(execute_func)
{
	string_val[0] = std::string("Executable");
}

std::string LWM2M_Resource::getValue(int depth)
{
	return string_val[depth];
}

bool LWM2M_Resource::value_exists(uint8_t value_depth)
{
	if (value_depth < next_value_ptr) return true;
	return false;
}

void LWM2M_Resource::update_resource(std::string res_val, uint8_t depth)
{
	uint8_t idx = depth;
	if (!multi_level)
	{
		idx = 0;
	}

	if (type == TYPE_INT)
	{
		values[idx].int_value = stoi(res_val);
		string_val[idx] = res_val;
	}
	else if (type == TYPE_FLOAT)
	{
		values[idx].float_value = stof(res_val);
		string_val[idx] = res_val;
	}
	else if (type == TYPE_BOOLEAN)
	{
		values[idx].bool_value = res_val == "1" ? true : false;
		string_val[idx] = res_val;
	}
	else
	{
		strcpy_s(string_value[idx], MAX_STRING_LENGTH, res_val.data());
		string_val[idx] = res_val;
	}

	if (idx == next_value_ptr) next_value_ptr++;

	
}