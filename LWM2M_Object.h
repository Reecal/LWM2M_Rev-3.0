#pragma once
#include <cstdint>
#include <vector>
#include "LWM2M_Defines.h"
#include "LWM2M_Resource.h"

class LWM2M_Object
{
private:

	

public:

	uint16_t obj_id;			//Object ID
	uint8_t instance_id;		//Instance ID

#if defined(USE_VECTORS)
	vector<LWM2M_Resource> resources_vector; //Vector that contains all the resources in this object
#else
	uint8_t next_resource_ptr = 0;
	uint16_t resource_ids[MAX_RESOURCES];
	LWM2M_Resource resources[MAX_RESOURCES];
#endif
	LWM2M_Object(uint16_t obj_id = 65535, uint8_t instance_id = 0); //Default constructor

	/**
	 *	Methods used to create a resource within an object
	 *
	 *	INPUT : resource_id - Resource ID
	 *			type		- Type of data held in the resource
	 *			permissions - Permission of access to the data
	 *			multi_level - Whether the resource is a type of field eg. multiple data inside the resource
	 *			default_value - Default value.
	 *
	 */
	//TODO Consolidate these methods to a single method using single argument as data
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	void add_resource(uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	void add_executable_resource(uint16_t resource_id, uint8_t(*execute_func)() = nullptr);

	//Getters & Setters
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

	/**
	 *	Method that checks whether a resource exists within the scope of this object.
	 *
	 *	INPUT : rsrc_id - Resource ID
	 *
	 *	OUTPUT: bool - resource exists within this object
	 */
	bool resource_exists(uint16_t rsrc_id);
	bool resource_exists(uint16_t rsrc_id, uint16_t sub_res_id);

};
