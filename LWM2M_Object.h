#pragma once
#include <cstdint>
#include "LWM2M_Defines.h"
#include "LWM2M_Resource.h"

class LWM2M_Object
{
private:

	uint16_t obj_id;
	uint8_t instance_id;

	uint8_t next_resource_ptr = 0;

	uint16_t resource_ids[MAX_RESOURCES];
	LWM2M_Resource resources[MAX_RESOURCES];

public:
	LWM2M_Object(uint16_t obj_id, uint8_t instance_id = 0);

	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)());




	void setInstance_id(uint8_t inst_id)
	{
		instance_id = inst_id;
	}

	uint16_t getObject_id()
	{
		return obj_id;
	}

	uint16_t getInstance_id()
	{
		return instance_id;
	}


};
