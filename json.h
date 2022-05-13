#pragma once

#include <iostream>
#include "LWM2M_Object.h"
#include "LWM2M_Resource.h"
#include "CoAP.hpp"
#include <algorithm>
#include "Utils.h"

#define JSON_SUCCESS					0
#define JSON_INVALID_VALUE_FORMAT		1
#define JSON_INVALID_VALUE_TYPE			2


namespace json {



	std::string createJSON_Object(LWM2M_Object& object);
	std::string createJSON_Instance(LWM2M_Object& object);
	std::string createJSON_Resource(URI_Path_t* uri, LWM2M_Resource& resource);
	std::string createJSON_MVResource(URI_Path_t* uri, LWM2M_Resource& resource);

	std::string getPartialResourceString(URI_Path_t* uri, LWM2M_Resource& resource, uint8_t depth = 0);

	uint8_t parseJsonAndUpdate_Resource(URI_Path_t* uri, std::string json_string, LWM2M_Resource& resource);
	LWM2M_Resource parseJson_Instance(URI_Path_t* uri, std::string json_string);
	LWM2M_Resource parseJson_Object(URI_Path_t* uri, std::string json_string);
	


	/*
	LWM2M_Object& parseJSON(std::string jsonString);
	*/
}
