#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include "LWM2M_Client.h"
#include "Utils.h"

inline void userInputLWM(LWM2M_Client& client, bool& isFinished, bool& applicationRun) {
    {

        //client.printObjs();

        //std::cout << "Thread run!" << std::endl;
        while (!isFinished)
        {
            std::string input;
            //m.lock();
            std::cout << ">";
            //m.unlock();
            std::getline(std::cin, input);
            std::stringstream ss(input);
            //PerformanceTimer Timer("One while cycle of user input");
            /*while (ss.good())
            {
                std::string token;
                ss >> token;
                //std::cout << token << std::endl;
            }*/

            if (ss.good())
            {
                std::string token;
                ss >> token;
                //-------------------------------------------------------------- PRINT
                if (token == "print" || token == "echo" || token == "out" || token == "show")
                {
                    //std::cout << "The token is print" << std::endl;
                    if (ss.good())
                    {
                        ss >> token;
                        //------------------------------------------------------------------------ PRINT:OBJECT
                        if (token == "object" || token == "obj" || token == "o")
                        {
                            //std::cout << "The token is object" << std::endl;
                            if (ss.good())
                            {
                                ss >> token;

                                //Check if given string is an integer
                                //If not check string input parameters
                                //------------------------------------------------------------------------ PRINT:OBJECT:STR_ARG
                                if (!isIntegerStd(token))
                                {
                                    //------------------------------------------------------------------------ PRINT:OBJECT:STR_ARG:ALL
                                    if (token == "all" || token == "a")
                                    {
                                        std::cout << "The token is all" << std::endl;
                                        //client.printObjs();
                                    }
                                    //------------------------------------------------------------------------ PRINT:OBJECT:STR_ARG:INVALID
                                    else
                                    {
                                        std::cout << "Invalid input parameter. \"" << token << "\" is not an Integer nor it is a valid argument." << std::endl;
                                    }

                                }
                                //If string is integer parse the number and search for the object
                                //------------------------------------------------------------------------ PRINT:OBJECT:INTEGER
                                else
                                {
                                    int num = stoi(token);
                                    std::cout << "The token is " << num << std::endl;

                                    /*if (!client.object_exists(num))
                                    {
                                        std::cout << "No object with that Object ID" << std::endl;
                                        continue;
                                    }

                                    client.printSingleObj(num);*/

                                }
                            }
                            else //if no other args passed
                            {
                                std::cout << "You need to enter at least one other parameter" << std::endl;
                            }

                        }
                        //------------------------------------------------------------------------ PRINT:INSTANCE
                        else if (token == "instance" || token == "in" || token == "i" || token == "inst")
                        {
                            // std::cout << "The token is instance" << std::endl;
                        }
                        //------------------------------------------------------------------------ PRINT:RESOURCE
                        else if (token == "resource" || token == "res" || token == "r")
                        {
                            //std::cout << "The token is resource" << std::endl;


                            //-------------------------------------------------------------- PRINT:RESOURCE:OBJECT_ID
                            if (ss.good())
                            {
                                ss >> token;
                                if (!isIntegerStd(token))
                                {
                                    std::cout << "Invalid object number: " << token << "   " << isIntegerStd(token) << std::endl;
                                    continue;
                                }

                                int object_id = stoi(token);
                                std::cout << "Object ID: " << object_id << std::endl;
                                /*if (!client.object_exists(object_id))
                                {
                                    std::cout << "Object with this object ID does not exist." << std::endl;
                                    continue;
                                }*/

                                if (ss.good())
                                {
                                    //-------------------------------------------------------------- PRINT:RESOURCE:OBJECT_ID:INSTANCE_ID
                                    ss >> token;
                                    if (!isIntegerStd(token))
                                    {
                                        std::cout << "Invalid instance number: " << token << "   " << isIntegerStd(token) << std::endl;
                                        continue;
                                    }


                                    int instance_id = stoi(token);
                                    std::cout << "Instance ID: " << instance_id << std::endl;
                                    /*if (!client.objects[object_id].instance_exists(instance_id))
                                    {
                                        std::cout << "Instance with this instance ID does not exist." << std::endl;
                                        continue;
                                    }*/


                                    if (ss.good())
                                    {
                                        //-------------------------------------------------------------- PRINT:RESOURCE:OBJECT_ID:INSTANCE_ID:RESOURCE_ID
                                        ss >> token;
                                        if (!isIntegerStd(token))
                                        {
                                            std::cout << "Invalid resource number" << std::endl;
                                            continue;
                                        }

                                        int resource_id = stoi(token);
                                        std::cout << "Resource ID: " << resource_id << std::endl;

                                        /*if (!client.objects[object_id].instances[instance_id].resource_exists(resource_id))
                                        {
                                            std::cout << "Resource with this resource ID does not exist." << std::endl;
                                            continue;
                                        }

                                        client.printSingleResource(object_id, instance_id, resource_id);*/

                                    }
                                }
                            }

                        }
                        //------------------------------------------------------------------------ PRINT:CLIENT
                        else if (token == "client")
                        {
                            //std::cout << "The token is client" << std::endl;
                        }
                        //------------------------------------------------------------------------ PRINT:ALL
                        else if (token == "all" || token == "a")
                        {
                            std::cout << "All" << std::endl;
                        }
                        //------------------------------------------------------------------------ PRINT:INVALID
                        else
                        {
                            std::cout << "Invalid input parameter " << token << "." << std::endl;
                        }

                    }
                    else std::cout << "Too few arguments for a " << token << " command." << std::endl;
                }
                //-------------------------------------------------------------- SET
                else if (token == "set" || token == "update")
                {
                    //std::cout << "The token is set" << std::endl;

                    if (ss.good())
                    {
                        ss >> token;

                        //-------------------------------------------------------------- SET:RESOURCE
                        if (token == "resource" || token == "res" || token == "r")
                        {
                            std::cout << "The token is resource" << std::endl;
                            //-------------------------------------------------------------- SET:RESOURCE:OBJECT_ID
                            if (ss.good())
                            {
                                ss >> token;
                                if (!isIntegerStd(token))
                                {
                                    std::cout << "Invalid object number: " << token << "   " << isIntegerStd(token) << std::endl;
                                    continue;
                                }

                                int object_id = stoi(token);
                                std::cout << "Object ID: " << object_id << std::endl;

                                if (ss.good())
                                {
                                    //-------------------------------------------------------------- SET:RESOURCE:OBJECT_ID:INSTANCE_ID
                                    ss >> token;
                                    if (!isIntegerStd(token))
                                    {
                                        std::cout << "Invalid instance number: " << token << "   " << isIntegerStd(token) << std::endl;
                                        continue;
                                    }


                                    int instance_id = stoi(token);
                                    std::cout << "Instance ID: " << instance_id << std::endl;


                                    if (ss.good())
                                    {
                                        //-------------------------------------------------------------- SET:RESOURCE:OBJECT_ID:INSTANCE_ID:RESOURCE_ID
                                        ss >> token;
                                        if (!isIntegerStd(token))
                                        {
                                            std::cout << "Invalid resource number" << std::endl;
                                            continue;
                                        }

                                        int resource_id = stoi(token);
                                        std::cout << "Resource ID: " << resource_id << std::endl;

                                        if (ss.good())
                                        {
                                            //-------------------------------------------------------------- SET:RESOURCE:OBJECT_ID:INSTANCE_ID:RESOURCE_ID:MESSAGE&UPDATE

                                            ss >> token;
                                            /* if (isIntegerStd(token) && client.objects[object_id].instances[instance_id].getResource(resource_id)->value_type == TYPE_INT)
                                            {
                                                bool isLifetime = object_id == 1 && instance_id == 0 && resource_id == 1;
                                                if (isLifetime)
                                                {
                                                    m.lock();
                                                    client.lifetime = stoi(token);
                                                    client.update_resource(object_id, instance_id, resource_id, stoi(token));
                                                    //client.client_send_update();
                                                    client.externalUpdateRequest = true;
                                                    m.unlock();
                                                }
                                                else
                                                {
                                                    client.update_resource(object_id, instance_id, resource_id, stoi(token));
                                                }



                                            }
                                            else if (isFloat(token) && client.objects[object_id].instances[instance_id].getResource(resource_id)->value_type == TYPE_FLOAT)
                                            {
                                                client.update_resource(object_id, instance_id, resource_id, stof(token));

                                            }
                                            else if ((token == "true" || token == "false") && client.objects[object_id].instances[instance_id].getResource(resource_id)->value_type == TYPE_BOOLEAN)
                                            {
                                                if (token == "true")
                                                {
                                                    client.update_resource(object_id, instance_id, resource_id, true);
                                                }
                                                else
                                                {
                                                    client.update_resource(object_id, instance_id, resource_id, false);
                                                }
                                            }
                                            else
                                            {
                                                while (ss.good())
                                                    token += ss.get();

                                                int stringLen = token.length();
                                                if (token[stringLen - 1] == (char)0xff)
                                                {

                                                    token.erase(stringLen - 1, 1);
                                                }
                                                client.update_resource(object_id, instance_id, resource_id, token);
                                            }*/

                                            std::cout << "Resource updated" << std::endl;

                                        }
                                        else
                                        {
                                            std::cout << "Missing argument" << std::endl;
                                        }

                                    }
                                    else
                                    {
                                        std::cout << "Resource number missing" << std::endl;
                                    }

                                }
                                else
                                {
                                    std::cout << "Instance number missing" << std::endl;

                                }
                            }
                            else
                            {
                                std::cout << "Arguments missing" << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "Invalid parameter " + token << std::endl;
                        }
                    }
                }


                //-------------------------------------------------------------- EXIT
                else if (token == "exit")
                {
                    applicationRun = false;
                    isFinished = true;
                    //std::cout << "The token is exit" << std::endl;
                }
                //-------------------------------------------------------------- Enter (empty string)
                else if (token == "") {}
                else if (token == "Hello" || token == "Hi" || token == "hello" || token == "hi" || token == "whatsup") std::cout << "Hello to you too bro!" << std::endl;

                //-------------------------------------------------------------- INVALID
                else
                {
                    std::cout << "Invalid input: No " << token << " command." << std::endl;
                }
            }
        }
        std::cout << "Thread finish!" << std::endl;
    }
}