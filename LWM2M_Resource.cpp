#include "LWM2M_Resource.h"

#include <string.h>

LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level) : type(type), permissions(permissions), multi_level(multi_level)
{
	
}
LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, float default_value) : type(type), permissions(permissions), multi_level(multi_level)
{
	values->float_value = default_value;
}
LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, bool default_value) : type(type), permissions(permissions), multi_level(multi_level)
{
	values->bool_value = default_value;
}
LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, int default_value) : type(type), permissions(permissions), multi_level(multi_level)
{
	values->int_value = default_value;
}
LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, char* default_value) : type(type), permissions(permissions), multi_level(multi_level)
{
	strcpy_s(string_value[0], 40, default_value);
}

LWM2M_Resource::LWM2M_Resource(uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)()) : type(type), permissions(permissions), multi_level(multi_level), execute_cb(execute_func)
{

}