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
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	values->bool_value = default_value;
	string_val[0] = default_value ? std::string("true") : std::string("false");
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	values->int_value = default_value;
	string_val[0] = std::to_string(default_value);
}
LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level)
{
	strcpy_s(string_value[0], MAX_STRING_LENGTH, default_value);
	string_val[0] = std::string(default_value);
}

LWM2M_Resource::LWM2M_Resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)()) : resource_id(resource_id), type(type), permissions(permissions), multi_level(multi_level), execute_cb(execute_func)
{
	string_val[0] = std::string("Executable");
	string_val[0] = "";
}

std::string LWM2M_Resource::getValue(int depth)
{
	return string_val[depth];
}