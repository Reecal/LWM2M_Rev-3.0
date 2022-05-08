#pragma once
#include <cstdint>
#include "LWM2M_Defines.h"

class LWM2M_Object
{
private:

	uint16_t obj_id;
	uint8_t instance_id;

	uint8_t next_resource_ptr = 0;

	uint16_t resource_id[MAX_RESOURCES];

public:
	LWM2M_Object(uint16_t obj_id, uint8_t instance_id);

	void add_resource();




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
