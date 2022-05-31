#pragma once
#include <cstdint>
#include <vector>
#include "LWM2M_Defines.h"
#include "LWM2M_Resource.h"

class LWM2M_Object
{
private:

	

public:

	uint16_t obj_id;
	uint8_t instance_id;

#if defined(USE_VECTORS)
	vector<LWM2M_Resource> resources_vector;
#else
	uint8_t next_resource_ptr = 0;
	uint16_t resource_ids[MAX_RESOURCES];
	LWM2M_Resource resources[MAX_RESOURCES];
#endif
	LWM2M_Object(uint16_t obj_id = 65535, uint8_t instance_id = 0);

	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	void add_executable_resource(uint16_t resource_id, uint8_t(*execute_func)() = nullptr);


	LWM2M_Resource& getResource(uint16_t rsrc_id);

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

	bool resource_exists(uint16_t rsrc_id);
	bool resource_exists(uint16_t rsrc_id, uint16_t sub_res_id);

};
