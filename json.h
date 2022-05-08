#pragma once

#include <iostream>
#include "LWM2M_Object.h"
#include "LWM2M_Resource.h"
#include "CoAP.hpp"
#include <algorithm>

namespace json {

	std::string createJSON_Object(LWM2M_Object& object);
	std::string createJSON_Instance(LWM2M_Object& object);
	std::string createJSON_Resource(URI_Path_t* uri, LWM2M_Resource& resource);
	/*std::string createJSON_Object(int obj_id, LWM2M_Object::LWM2M_Instance& instance);
	std::string createJSON_Object(int obj_id, int inst_id, LWM2M_Object::LWM2M_Instance::LWM2M_Resource& resource);
	LWM2M_Object& parseJSON(std::string jsonString);*/
}
