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
	std::string createJSON_MVResource(URI_Path_t* uri, LWM2M_Resource& resource);

	std::string getPartialResourceString(URI_Path_t* uri, LWM2M_Resource& resource, uint8_t depth = 0);

	LWM2M_Resource parseJson_Resource(URI_Path_t* uri, std::string json_string);
	


	/*
	LWM2M_Object& parseJSON(std::string jsonString);
	*/
}
