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
	
	/*std::sort(object.instances[0].resources.begin(), object.instances[0].resources.end(), [](LWM2M_Object::LWM2M_Instance::LWM2M_Resource r1, LWM2M_Object::LWM2M_Instance::LWM2M_Resource r2) {

		if (r1.resource_id < r2.resource_id) return true;
		else return false;
		
		});
	*/
	std::string output = "";
	output += "{\"bn\":\"/";
	output += to_string(object.getObject_id());
	output += "/";
	output += to_string(object.getInstance_id());
	output += "/\",\"e\":[";

	int offset = 1;
	//LWM2M_Resource& lastResource = object.instances[0].resources[object.instances[0].resources.size() - offset];
	LWM2M_Resource& lastResource = object.resources[object.next_resource_ptr - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources[object.next_resource_ptr - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();

	for (auto& resource : object.resources)
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
	/*std::sort(instance.resources.begin(), instance.resources.end(), [](LWM2M_Object::LWM2M_Instance::LWM2M_Resource r1, LWM2M_Object::LWM2M_Instance::LWM2M_Resource r2) {

		if (r1.resource_id < r2.resource_id) return true;
		else return false;

		});
	*/
	std::string output = "";
	output += "{\"bn\":\"/";
	output += to_string(object.getObject_id());
	output += "/";
	output += to_string(object.getInstance_id());
	output += "/\",\"e\":[";

	int offset = 1;
	LWM2M_Resource& lastResource = object.resources[object.next_resource_ptr - offset];
	while (lastResource.getType() == TYPE_EXECUTABLE)
	{
		lastResource = object.resources[object.next_resource_ptr - offset];
		offset++;
	}
	int lastResourceID = lastResource.getResource_id();

	for (auto& resource : object.resources)
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

std::string json::createJSON_Resource(URI_Path_t* uri, LWM2M_Resource& resource)
{
	if (resource.getPermissions() == WRITE_ONLY || resource.getPermissions() == EXECUTABLE) return "";
	std::string output = "";
	output += "{\"bn\":\"/";
	output += std::to_string(uri->obj_id);
	output += "/";
	output += std::to_string(uri->instance_id);
	output += "/";
	output += std::to_string(resource.getResource_id());
	output += "\",\"e\":[{";
	
		std::string value_modifier;
		std::string resource_value;
		if (resource.getType() == TYPE_STRING)
		{
			value_modifier = "sv";
			resource_value = resource.getValue();
		}
		else if (resource.getType() == TYPE_INT)
		{
			value_modifier = "v";
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
			resource_value = resource.getValue();
		}

		output += "\"";
		output += value_modifier;
		if (resource.getType() == TYPE_STRING)
		{
			output += "\":\"" + resource_value + "\"}";
		}
		else
		{
			output += "\":" + resource_value + "}";
		}

	output += "]}";

	LOG_INFO("Assembled json: " +  output);
	return output;
}


std::string json::createJSON_MVResource(URI_Path_t* uri, LWM2M_Resource& resource)
{
	if (resource.getPermissions() == WRITE_ONLY || resource.getPermissions() == EXECUTABLE) return "";
	std::string output = "";
	output += "{\"bn\":\"/";
	output += std::to_string(uri->obj_id);
	output += "/";
	output += std::to_string(uri->instance_id);
	output += "/";
	output += std::to_string(resource.getResource_id());
	
	output += "/\",\"e\":[";

	for (uint8_t index = 0; index < resource.next_value_ptr; index++)
	{
		std::string value_modifier;
		std::string resource_value;

		if (resource.getType() == TYPE_STRING)
		{
			value_modifier = "sv";
			resource_value = resource.getValue(index);
		}
		else if (resource.getType() == TYPE_INT)
		{
			value_modifier = "v";
			resource_value = resource.getValue(index);
		}
		else if (resource.getType() == TYPE_BOOLEAN)
		{
			value_modifier = "bv";
			if (resource.getValue(index) == "1")
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
			resource_value = resource.getValue(index);
		}

		output += "{\"n\":\"";
		output += to_string(index);
		output += "\",\"";
		//output += "\"";
		output += value_modifier;
		if (resource.getType() == TYPE_STRING)
		{
			output += "\":\"" + resource_value + "\"}";
		}
		else
		{
			output += "\":" + resource_value + "}";
		}

		if (index != resource.next_value_ptr-1)
		{
			output += ",";
		}
		else
		{
			break;
		}

	}
	
	

	output += "]}";

	LOG_INFO("Assembled json: " + output);
	return output;
}

/*LWM2M_Object& json::parseJSON(std::string jsonString)
{
	LWM2M_Object o;
	return o;
}*/