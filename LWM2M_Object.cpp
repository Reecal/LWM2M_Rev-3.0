#include "LWM2M_Object.h"
#include "LWM2M_Client.h"

LWM2M_Object::LWM2M_Object(uint16_t obj_id, uint8_t instance_id) : obj_id(obj_id), instance_id(instance_id)
{
	
}

void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value)
{
#if defined(USE_VECTORS)
	resources_vector.push_back(LWM2M_Resource(resource_id, type, permissions, multi_level, default_value));
#else
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
#endif
}

void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value)
{
#if defined(USE_VECTORS)
	resources_vector.push_back(LWM2M_Resource(resource_id, type, permissions, multi_level, default_value));
#else
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
#endif
}
void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value)
{
#if defined(USE_VECTORS)
	resources_vector.push_back(LWM2M_Resource(resource_id, type, permissions, multi_level, default_value));
#else
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
#endif
}
void LWM2M_Object::add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value)
{
#if defined(USE_VECTORS)
	resources_vector.push_back(LWM2M_Resource(resource_id, type, permissions, multi_level, default_value));
#else
	LWM2M_Resource res(resource_id, type, permissions, multi_level, default_value);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
#endif
}
void LWM2M_Object::add_executable_resource(uint16_t resource_id, uint8_t(*execute_func)())
{
#if defined(USE_VECTORS)
	resources_vector.push_back(LWM2M_Resource(resource_id, TYPE_EXECUTABLE, EXECUTABLE, false, execute_func));
#else
	LWM2M_Resource res(resource_id, TYPE_EXECUTABLE, EXECUTABLE, false, execute_func);
	resource_ids[next_resource_ptr] = resource_id;
	resources[next_resource_ptr] = res;
	next_resource_ptr++;
#endif
	
}

LWM2M_Resource& LWM2M_Object::getResource(uint16_t rsrc_id)
{
	//uint8_t ptr = next_resource_ptr;
#if defined(USE_VECTORS)
	for(LWM2M_Resource& r : resources_vector)
	{
		if (r.getResource_id() == rsrc_id)
			return r;
	}
	return resources_vector.front();
#else
	for (uint8_t search_var = 0; search_var < next_resource_ptr; search_var++)
	{
		if (resource_ids[search_var] == rsrc_id)
			return resources[search_var];
	}
	return resources[0];
#endif
}

bool LWM2M_Object::resource_exists(uint16_t rsrc_id)
{
#if defined(USE_VECTORS)
		for (LWM2M_Resource& r : resources_vector)
		{
			if (r.getResource_id() == rsrc_id)
				return true;
		}
		return false;
#else
	for (uint8_t search_var = 0; search_var < next_resource_ptr; search_var++)
	{
		if (resource_ids[search_var] == rsrc_id)
			return true;
	}
	return false;
#endif
}

bool LWM2M_Object::resource_exists(uint16_t rsrc_id, uint16_t sub_res_id)
{

#if defined(USE_VECTORS)
	for (LWM2M_Resource& r : resources_vector)
	{
		if (r.getResource_id() == rsrc_id && r.value_exists(sub_res_id))
			return true;
	}
	return false;
#else
	for (uint8_t search_var = 0; search_var < next_resource_ptr; search_var++)
	{
		//TODO: Check subresource as well
		if (resource_ids[search_var] == rsrc_id)
			return true;
	}
	return false;
#endif

}

