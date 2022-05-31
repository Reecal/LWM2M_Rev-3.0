#include "json.h"
#include "Logger_xdvora2g.h"


#if LOG_OUTPUT == 1
#define LOG_ENTITY "\x1B[34m    JSON_LIB\033[0m"
#define LOG_DATA(x,y)   LOG123(x, std::string(LOG_ENTITY), std::string(y))
#define LOG_INFO(x)     LOG123(LOG_INFO_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#define LOG_WARNING(x)  LOG123(LOG_WARNING_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#define LOG_ERROR(x)    LOG123(LOG_ERROR_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#else
#define LOG_DATA(x, y) 
#define LOG_INFO(x)
#define LOG_WARNING(x)
#define LOG_ERROR(x) 
#endif





std::string json::createJSON_Object(LWM2M_Object& object)
{
	std::string output = "";
	output += "{\"bn\":\"/";
	output += to_string(object.getObject_id());
	output += "/";
	output += to_string(object.getInstance_id());
	output += "/\",\"e\":[";

	int offset = 1;
	//LWM2M_Resource& lastResource = object.instances[0].resources[object.instances[0].resources.size() - offset];
#if defined(USE_VECTORS)
	LWM2M_Resource& lastResource = object.resources_vector[object.resources_vector.size() - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources_vector[object.resources_vector.size() - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();
	for (auto& resource : object.resources_vector)
#else
	LWM2M_Resource& lastResource = object.resources[object.next_resource_ptr - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources[object.next_resource_ptr - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();

	for (auto& resource : object.resources)
#endif
	
	{
		std::string value_modifier;
		std::string resource_value;
		if (resource.getPermissions() == WRITE_ONLY || resource.getPermissions() == EXECUTABLE) continue;
		if (resource.getType() == TYPE_STRING)
		{
			value_modifier = "sv";
			resource_value = resource.getValue();
		}
		else if (resource.getType() == TYPE_INT)
		{
			value_modifier = "v";
			//resource_value = to_string(resource.value_int);
			resource_value = resource.getValue();
		}
		else if (resource.getType() == TYPE_BOOLEAN)
		{
			value_modifier = "bv";
			if (resource.getValue() == "1")
			{
				resource_value = "true";
			}
			else
			{
				resource_value = "false";
			}
			
		}
		else if (resource.getType() == TYPE_FLOAT)
		{
			value_modifier = "v";
			//resource_value = to_string(resource.value_float);
			resource_value = resource.getValue();
		}
		else continue;

		output += "{\"n\":\"";
		output += to_string(resource.getResource_id());
		output += "\",\"";
		output += value_modifier;
		if (resource.getType() == TYPE_STRING)
		{
			output += "\":\"" + resource_value + "\"}";
		}
		else
		{
			output += "\":" + resource_value + "}";
		}
		


		if (resource.getResource_id() != lastResourceID)
		{
			output += ",";
		}
		else
		{
			break;
		}
	}
	

	output += "]}";
	
	return output;
}

std::string json::createJSON_Instance(LWM2M_Object& object)
{
	std::string output = "";
	output += "{\"bn\":\"/";
	output += to_string(object.getObject_id());
	output += "/";
	output += to_string(object.getInstance_id());
	output += "/\",\"e\":[";

	int offset = 1;
#if defined(USE_VECTORS)
	LWM2M_Resource& lastResource = object.resources_vector[object.resources_vector.size() - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources_vector[object.resources_vector.size() - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();
	for (auto& resource : object.resources_vector)
#else
	LWM2M_Resource& lastResource = object.resources[object.next_resource_ptr - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources[object.next_resource_ptr - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();

	for (auto& resource : object.resources)
#endif
	{
		if (resource.getType() == TYPE_EXECUTABLE) continue;
		URI_Path_t up = { object.getObject_id(), object.getInstance_id(), 0 , 0, 2 };
		output += createJSON_Resource(&up, resource);
		//output += "}";



		if (resource.getResource_id() != lastResourceID)
		{
			output += ",";
		}
		else
		{
			break;
		}
	}


	output += "]}";

	return output;
}

std::string json::createJSON_Resource(URI_Path_t* uri, LWM2M_Resource& resource)
{
	if (resource.getPermissions() == WRITE_ONLY || resource.getPermissions() == EXECUTABLE) return "";
	std::string output = "";
	if (uri->path_depth == REQUEST_RESOURCE)
	{
		output += "{\"bn\":\"/";
		output += std::to_string(uri->obj_id);
		output += "/";
		output += std::to_string(uri->instance_id);
		output += "/";
		output += std::to_string(resource.getResource_id());
		//output += "/\",\"e\":[";
		if (resource.getMultiLevel()) output += "/";
		output += "\",\"e\":[";
	}
	if (resource.getMultiLevel())
	{
		output += createJSON_MVResource(uri, resource);
	}
	else
	{
		if (uri->path_depth == REQUEST_RESOURCE)
		{
			output += "{";
			output += getPartialResourceString(uri, resource);
			output += "}";
		}
		else
		{
			output += "{\"n\":\"";
			output += to_string(resource.getResource_id());
			output += "\",";
			output += getPartialResourceString(uri, resource);
			output += "}";
		}
		
	}
	

	if (uri->path_depth == REQUEST_RESOURCE) output += "]}";

	//LOG_INFO("Assembled json: " +  output);
	return output;
}

std::string json::createJSON_MVResource(URI_Path_t* uri, LWM2M_Resource& resource)
{
	if (resource.getPermissions() == WRITE_ONLY || resource.getPermissions() == EXECUTABLE) return "";
	std::string output = "";
	if (uri->path_depth == REQUEST_MV_RESOURCE)
	{
		output += "{\"bn\":\"/";
		output += std::to_string(uri->obj_id);
		output += "/";
		output += std::to_string(uri->instance_id);
		output += "/";
		output += std::to_string(resource.getResource_id());
		output += "/\",\"e\":[";
	}

	
	for (uint8_t index = 0; index < resource.next_value_ptr; index++)
	{
		std::string value_modifier;
		std::string resource_value;

		output += "{\"n\":\"";
		if(uri->path_depth == REQUEST_INSTANCE)
		{
			output += to_string(resource.getResource_id()) + "/" +  to_string(index);
		}
		else
		{
			output += to_string(index);
		}

		
		output += "\",";
		output += getPartialResourceString(uri, resource, index);
		output += "}";

		if (index != resource.next_value_ptr-1)
		{
			output += ",";
		}
		else
		{
			break;
		}

	}
	
	

	if (uri->path_depth == REQUEST_MV_RESOURCE) output += "]}";

	//LOG_INFO("Assembled json: " + output);

	//LOG_INFO(getPartialResourceString(uri, resource));
	return output;
}

std::string json::getPartialResourceString(URI_Path_t* uri, LWM2M_Resource& resource, uint8_t depth)
{
	std::string output = "\"";
	std::string value_modifier;
	std::string resource_value;
	if (resource.getType() == TYPE_STRING)
	{
		value_modifier = "sv";
		resource_value = resource.getValue(depth);
	}
	else if (resource.getType() == TYPE_INT)
	{
		value_modifier = "v";
		resource_value = resource.getValue(depth);
	}
	else if (resource.getType() == TYPE_BOOLEAN)
	{
		value_modifier = "bv";
		if (resource.getValue(depth) == "1")
		{
			resource_value = "true";
		}
		else
		{
			resource_value = "false";
		}
	}
	else if (resource.getType() == TYPE_FLOAT)
	{
		value_modifier = "v";
		resource_value = resource.getValue(depth);
	}

	output += value_modifier;
	if (resource.getType() == TYPE_STRING)
	{
		output += "\":\"" + resource_value + "\"";
	}
	else
	{
		output += "\":" + resource_value;
	}
	resource.clearValueChanged();
	return output;
}

uint8_t json::parseJsonAndUpdate_Resource(URI_Path_t* uri, std::string json_string, LWM2M_Resource& resource)
{
	json_string.erase(0, json_string.find("[{\"")); // get rid of the beginning. We all know of it from URI
	int var_start = json_string.find("{")+1;				//Find the beginning of a resource
	int var_end = json_string.find("}");					//Find the end of a resource
	std::string value_string = json_string.substr(var_start, var_end-var_start);

	int value_end = value_string.find("\"", 1);
	std::string variable_type = value_string.substr(1, value_end-1);

	if (variable_type == "bv")
	{
		if (resource.getType() != TYPE_BOOLEAN) return JSON_INVALID_VALUE_TYPE;
		int value_start = value_string.find(":") + 1;
		std::string value = value_string.erase(0, value_start);
		
		if (value == "false")
		{
			value = "0";
		}
		else if (value == "true")
		{
			value = "1";
		}
		else
		{
			return JSON_INVALID_VALUE_FORMAT;
		}
		resource.update_resource(value, uri->path_depth);
	}
	else if (variable_type == "sv")
	{
		if (resource.getType() != TYPE_STRING) return JSON_INVALID_VALUE_TYPE;
		int value_start = value_string.find(":") + 2;
		std::string value = value_string.erase(0, value_start);
		value = value.substr(0, value.length() - 1);
		resource.update_resource(value, uri->path_depth);
	}
	else if (variable_type == "v")
	{
		if (resource.getType() != TYPE_INT || resource.getType() != TYPE_FLOAT) return JSON_INVALID_VALUE_TYPE;
		int value_start = value_string.find(":") + 1;
		std::string value = value_string.erase(0, value_start);
		if (!isInteger(value) || !isFloat(value)) return JSON_INVALID_VALUE_FORMAT;
		resource.update_resource(value, uri->path_depth);
	}
	else
	{
		return JSON_INVALID_VALUE_FORMAT;
	}


	return JSON_SUCCESS;
	
}

LWM2M_Resource json::parseJson_Instance(URI_Path_t* uri, std::string json_string)
{
	//{"bn":"/1/","e":[{"n":"1/1","v":34},{"n":"1/6","bv":true},{"n":"1/7","sv":"U"}]}
	return 0;
}
LWM2M_Resource json::parseJson_Object(URI_Path_t* uri, std::string json_string)
{
	return 0;
}
/*LWM2M_Object& json::parseJSON(std::string jsonString)
{
	LWM2M_Object o;
	return o;
}*/