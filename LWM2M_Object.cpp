#include "LWM2M_Object.h"

LWM2M_Object::LWM2M_Object(uint16_t obj_id, uint8_t instance_id) : obj_id(obj_id), instance_id(instance_id)
{
	
}

void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value)
{
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
}

void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value)
{
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
}
void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value)
{
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
}
void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value)
{
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
}
void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)())
{
	LWM2M_Resource res(resource_id, type, permissions, multi_level, execute_func);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
}

