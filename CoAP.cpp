#pragma once
#include "CoAP.hpp"


using namespace std;

//#define VERBOSE

string convertToString(char* a, int size)
{
	int i;
	string s = "";
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}
	
uint8_t CoAP_init(CoAP_message_t* coap_struct) {
	coap_struct->header.type = COAP_ACK;
	coap_struct->header.token_length = 8;
	coap_struct->header.returnCode = COAP_SUCCESS_CONTENT;
	coap_struct->header.messageID = 44444;
	coap_struct->options.number_of_options = 0;


	return 0;
}

uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode) {

	srand((unsigned int) time(NULL));
	uint16_t mid = rand() % 65536;
	CoAP_tx_setup(coap_struct, type, tkl, returnCode, mid);
	return 0;
}

uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint16_t mid) {

	uint8_t token[8];
	srand(time(NULL));
	for (uint8_t i = 0; i < tkl; i++) {
		token[i] = rand() % 256;
		//cout << token[i] << endl;
	}
	CoAP_tx_setup(coap_struct, type, tkl, returnCode, mid, token);
	return 0;
}

uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint8_t token[8]) {

	srand((unsigned int)time(NULL));
	uint16_t mid = rand() % 65536;
	CoAP_tx_setup(coap_struct, type, tkl, returnCode, mid, token);
	return 0;
}



uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint16_t mid, uint8_t token[8]) {
	coap_struct->header.type = type;
	coap_struct->header.token_length = tkl;
	coap_struct->header.returnCode = returnCode;
	coap_struct->header.messageID = mid;

	//set tokens

	for (uint8_t i = 0; i < 8; i++) {
		coap_struct->header.token[i] = token[i];
	}

	return 0;
}


uint8_t CoAP_assemble_header(char buffer[], CoAP_message_t* coap_struct) {

	buffer[0] = (COAP_VERSION << 6) | (coap_struct->header.type << 4) | (coap_struct->header.token_length);
	buffer[1] = coap_struct->header.returnCode;
	buffer[2] = coap_struct->header.messageID >> 8;
	buffer[3] = coap_struct->header.messageID & 0xff;

	return 4;
}

uint8_t CoAP_assemble_token(char buffer[], CoAP_message_t* coap_struct) {

	uint8_t tk[8];
	srand(time(NULL));
	for (uint8_t i = 0; i < coap_struct->header.token_length; i++) {
		tk[i] = rand() % 256;
	}

	CoAP_assemble_token(buffer, coap_struct ,tk);

	return coap_struct->header.token_length;
}

uint8_t CoAP_assemble_token(char buffer[], CoAP_message_t* coap_struct, uint8_t token[8]) {
	if (coap_struct->header.token_length == 0) return 0;
	for (uint8_t i = 0; i < coap_struct->header.token_length; i++) {
		buffer[i] = token[i];
	}
	return coap_struct->header.token_length;
}

uint8_t CoAP_assemble_options(char buffer[], CoAP_message_t* coap_struct) {
	
	//If no options attached
	if (coap_struct->options.number_of_options == 0) return 0;

	//For each option assemble bytes
	uint8_t totalOffset = 0;
	uint8_t numOfBytes = 0;

	for (uint8_t i = 0; i < coap_struct->options.number_of_options; i++) {
		CoAP_option_t currentOption = coap_struct->options.options[i];						//filter current option being assembled
		uint8_t offset = currentOption.option_number - totalOffset;							//calculate option offset
		totalOffset = currentOption.option_number;											//update current option number
		uint8_t control = 0;
		if (!((currentOption.option_number == COAP_OPTIONS_ACCEPT || currentOption.option_number == COAP_OPTIONS_CONTENT_FORMAT) && (currentOption.option_value[0] == 0))) {
			control = ((offset & 0x0f) << 4) | (currentOption.option_length & 0x0f);	//assemble control byte
		}
		else {
			control = ((offset & 0x0f) << 4);											//assemble control byte
		}
		

		buffer[numOfBytes] = control;
		numOfBytes++;
		if (!((currentOption.option_number == COAP_OPTIONS_ACCEPT || currentOption.option_number == COAP_OPTIONS_CONTENT_FORMAT) && (currentOption.option_value[0] == 0))) {
			for (uint8_t m = 0; m < currentOption.option_length; m++) {
				buffer[numOfBytes] = currentOption.option_value[m];
					numOfBytes++;
			}
		}	 
	}

	//buffer[numOfBytes] = 0xff;
	//numOfBytes++;
	
	return numOfBytes;
}



uint8_t CoAP_add_option(CoAP_message_t* coap_struct, uint8_t optionNumber, string optionArgs) {
	CoAP_option_t newOption;
	newOption.option_number = optionNumber;
	uint8_t argssize = optionArgs.length();
	newOption.option_length = argssize;
	
	for (uint8_t i = 0; i < argssize; i++) {
		newOption.option_value[i] = optionArgs[i];
	}

	coap_struct->options.options[coap_struct->options.number_of_options] = newOption;
	coap_struct->options.number_of_options++;
	
	return 0;
}

uint8_t CoAP_add_option(CoAP_message_t* coap_struct, uint8_t optionNumber, int optionArgs) {
	CoAP_option_t newOption;
	newOption.option_number = optionNumber;
	if ((optionNumber == COAP_OPTIONS_ACCEPT) && (optionArgs == 0)) newOption.option_length = 0;
	else if (optionArgs > 255)
	{
		newOption.option_length = 2;
		newOption.option_value[0] = (optionArgs >> 8) & 0xff;
		newOption.option_value[1] = optionArgs & 0xff;
	}
	else
	{
		newOption.option_length = 1;
		newOption.option_value[0] = optionArgs & 0xff;
		
	}
	

	coap_struct->options.options[coap_struct->options.number_of_options] = newOption;
	coap_struct->options.number_of_options++;

	return 0;
}




uint8_t CoAP_set_payload(CoAP_message_t* coap_struct, string payload) {
	coap_struct->payload = payload;
	return 0;
}

uint8_t CoAP_assemble_message(CoAP_message_t* coap_struct) {
	char headerbuffer[4];

	uint16_t packetLength = CoAP_assemble_header(headerbuffer, coap_struct);
	char tokenbuffer[8];
	packetLength += CoAP_assemble_token(tokenbuffer, coap_struct, coap_struct->header.token);

	char optionsBuffer[64];
	uint8_t optionsLength = CoAP_assemble_options(optionsBuffer, coap_struct);
	packetLength += optionsLength;




	string headerString = convertToString(headerbuffer, 4);
	string tokenString = convertToString(tokenbuffer, coap_struct->header.token_length);
	string optionsString = convertToString(optionsBuffer, optionsLength);
	string message = headerString + tokenString + optionsString;

	//add Payload
	if (coap_struct->payload.length() != 0) {
		//char c[1] = { 0xff };
		char cs = 0xff;
		//string endOfOptions = convertToString(c, 1);
		string endOfOptions="";
		endOfOptions += cs;
		//numOfBytes++;
		message += endOfOptions + coap_struct->payload;
		packetLength += coap_struct->payload.length() + 1;
	}

	coap_struct->raw_data.message_total = packetLength;
	coap_struct->raw_data.masg = message;

	return 0;
}


uint8_t CoAP_parse_message(CoAP_message_t* output, char data[], uint16_t msgLen) {

	uint8_t check_message_result = CoAP_is_valid_coap_message(data);
	if (check_message_result != COAP_OK) return check_message_result;

	uint8_t messageType = COAP_HEADER_TYPE(data);
	uint8_t messageTkl = COAP_HEADER_TKL(data);
	uint8_t returnClassCode = COAP_HEADER_CLASS(data) << 5 | COAP_HEADER_CODE(data);

	uint16_t messageID = ((data[2] & 0xff) << 8) | (data[3] & 0xff);


	if (messageType == COAP_RST)
	{
		output->header.type = messageType;
		output->header.returnCode = returnClassCode;
		output->header.token_length = messageTkl;
		output->header.messageID = messageID;
		return COAP_OK;
	}

	uint16_t currentByte = 4;
	uint8_t token[8];
	
#ifdef VERBOSE
	for (uint16_t i = 0; i < messageTkl; i++) {
		token[i] = data[currentByte + i];
}
#endif // VERBOSE

	CoAP_header_t hdr = { messageType ,messageTkl ,returnClassCode , messageID };
	for (uint16_t i = 0; i < messageTkl; i++) {
		hdr.token[i] = data[currentByte++];
	}


	//PARSE AND CHECK OPTIONS
	CoAP_options_t options = {0};

	uint8_t option_offset = 0;
	uint8_t number_of_options = 0;

	while (currentByte < msgLen ) {
		
		//End of options (0xff)
		if ((data[currentByte] & 0xff) == COAP_END_OF_OPTIONS) {
			currentByte++;
			break;
		}
		uint8_t option = (data[currentByte] & 0xf0) >> 4;
		option = option_offset + option;
		option_offset = option;
		const uint8_t option_length = data[currentByte] & 0x0f;

		const bool optionOK = (option == COAP_OPTIONS_IF_MATCH) || (option == COAP_OPTIONS_URI_HOST) || (option == COAP_OPTIONS_ETAG) || (option == COAP_OPTIONS_IF_N_MATCH) || (option == COAP_OPTIONS_URI_PORT) ||
			(option == COAP_OPTIONS_LOCATION_PATH) || (option == COAP_OPTIONS_URI_PATH) || (option == COAP_OPTIONS_CONTENT_FORMAT) || (option == COAP_OPTIONS_MAX_AGE) || (option == COAP_OPTIONS_URI_QUERY) ||
			(option == COAP_OPTIONS_ACCEPT) || (option == COAP_OPTIONS_OBSERVE);
		if (!optionOK) {
#ifdef VERBOSE
			char buffer[10];
			_itoa_s(option, buffer, 10);
			cout << "Error. Invalid option number : " << buffer << "\t";
			_itoa_s(number_of_options, buffer, 10);
			cout << "Option number: " << buffer << endl;

#endif // VERBOSE				
			return COAP_BAD_OPTION;
		}
		currentByte++;

		CoAP_option_t opt = { 0 };
		uint8_t arguments[COAP_MAX_OPTION_LENGTH] = {0};

		if ((option == COAP_OPTIONS_ACCEPT || option == COAP_OPTIONS_CONTENT_FORMAT) && (option_length == 0)) {
			arguments[0] = 0;
		}
		else {
			for (uint8_t m = 0; m < option_length; m++) {
				arguments[m] = data[currentByte];
				currentByte++;
			}
		}

		opt.option_number = option;
		opt.option_length = option_length;
		for (uint8_t m = 0; m < option_length; m++) {
			opt.option_value[m] = arguments[m];
		}
		

		number_of_options++;
		options.number_of_options = number_of_options;
		options.options[number_of_options-1] = opt;
		
	}

	


	//PAYLOAD
	//string payload = "";
	if (currentByte < msgLen) {
		output->payload = convertToString(&data[currentByte], msgLen - currentByte);
	}
	
	output->header = hdr;
	output->options = options;
	//output->payload = payload;

	CoAP_raw_t craw = { 0 };
	craw.message_total = msgLen;
	craw.masg = data;

	output->raw_data = craw;


#ifdef VERBOSE
		char buffer[10];
		_itoa_s(messageType, buffer, 16);
		cout << "Message Type: " << buffer << endl;
		_itoa_s(messageTkl, buffer, 16);
		cout << "Message TKL: " << buffer << endl;
		_itoa_s((returnClassCode & 0xe0) >> 5, buffer, 10);
		cout << "Class and Code: " << buffer << ".";


		_itoa_s((returnClassCode & 0x1f), buffer, 10);
		if ((returnClassCode & 0x1f) < 10) cout << "0" << buffer << endl;
		else cout << buffer << endl;

		_itoa_s(messageID, buffer, 10);
		cout << "Message ID: " << buffer << "\t0x";
		_itoa_s(messageID, buffer, 16);
		cout << buffer << endl;

		cout << "Token: ";
		for (uint16_t i = 0; i < messageTkl; i++) {
			_itoa_s(token[i], buffer, 16);
			cout << buffer << " ";
		}
		cout << endl;

		_itoa_s(options.number_of_options, buffer, 10);
		cout << "Number of Options: " << buffer << endl;
		for (uint8_t n = 0; n < options.number_of_options; n++) {
			_itoa_s(options.options[n].option_number, buffer, 10);
			cout << "\tOption: " << buffer;
			_itoa_s(options.options[n].option_length, buffer, 10);
			cout << "\tLength: " << buffer << "\tArgs: ";

			for (uint8_t p = 0; p < options.options[n].option_length; p++) cout << options.options[n].option_value[p];

			cout << endl;

		}

		if (payload == "") cout << "No Payload" << endl;
		else cout << payload << endl;

	
#endif // VERBOSE

	
	return COAP_OK;
}

uint8_t CoAP_send(SOCKET& sock, CoAP_message_t* coap_struct) {

	CoAP_assemble_message(coap_struct);
	int sendResult = send(sock, coap_struct->raw_data.masg.c_str(), coap_struct->raw_data.message_total, 0);

	return sendResult;
}


string CoAP_get_option_string(CoAP_message_t* coap_struct, uint8_t option_to_get) {
	string output = "";
	int number_of_options = coap_struct->options.number_of_options;
	for (int i = 0; i < number_of_options; i++) {
		CoAP_option_t option = coap_struct->options.options[i];
		if (option.option_number == option_to_get) {
			output += '/';
			for (int m = 0; m < option.option_length; m++) {
				output += option.option_value[m];
			}
			if (option_to_get == COAP_OPTIONS_OBSERVE)
			{
				output += '0';
			}
			else if(option_to_get == COAP_OPTIONS_ACCEPT)
			{
				if (option.option_length == 0)
				{
					output = "0";
					return output;
				}
			}
			else if (option_to_get == COAP_OPTIONS_CONTENT_FORMAT)
			{
				if (option.option_length == 0)
				{
					output = "0";
					return output;
				}
			}
		}
	}
	output.erase(0, 1);
	return output;
}

void CoAP_send_error_response(SOCKET& sock, CoAP_message_t* message, uint8_t response_code)
{
	CoAP_message_t error_message;
	int message_type = message->header.type == COAP_CON ? COAP_ACK : COAP_NON;
	CoAP_tx_setup(&error_message, message_type, message->header.token_length, response_code, message->header.messageID, message->header.token);
	CoAP_send(sock, &error_message);
}

uint8_t CoAP_is_valid_coap_message(char data[])
{
	//CHECK VERSION
	if (COAP_HEADER_VERSION(data) != COAP_VERSION) return COAP_BAD_VERSION;

	//PARSE AND CHECK HEADER
	uint8_t messageType = COAP_HEADER_TYPE(data);
	bool messageTypeOK = messageType >= 0x00 && messageType <= 0x03;
	if (!messageTypeOK) return COAP_BAD_TYPE;


	uint8_t messageTkl = COAP_HEADER_TKL(data);
	bool messageTklOK = messageTkl <= 0x08;
	if (!messageTklOK) return COAP_BAD_TKL;

	uint8_t returnClassCode = COAP_HEADER_CLASS(data) << 5 | COAP_HEADER_CODE(data);
	bool classMethodValid = returnClassCode == COAP_METHOD_GET || returnClassCode == COAP_METHOD_POST || returnClassCode == COAP_METHOD_PUT || returnClassCode == COAP_METHOD_DELETE ||
		returnClassCode == COAP_METHOD_FETCH || returnClassCode == COAP_METHOD_PATCH || returnClassCode == COAP_METHOD_IPATCH || returnClassCode == COAP_EMPTY_MESSAGE;

	bool classSuccessValid = returnClassCode == COAP_SUCCESS_CREATED || returnClassCode == COAP_SUCCESS_DELETED || returnClassCode == COAP_SUCCESS_VALID ||
		returnClassCode == COAP_SUCCESS_CHANGED || returnClassCode == COAP_SUCCESS_CONTENT;

	bool classClientErrorValid = returnClassCode == COAP_C_ERR_BAD_REQUEST || returnClassCode == COAP_C_ERR_UNAUTHORIZED || returnClassCode == COAP_C_ERR_BAD_OPT ||
		returnClassCode == COAP_C_ERR_FORBIDDEN || returnClassCode == COAP_C_ERR_NOT_FOUND;

	bool classServerErrorValid = returnClassCode == COAP_S_ERR_INTERNAL_ERR || returnClassCode == COAP_S_ERR_NOT_IMPLEMENTED || returnClassCode == COAP_S_ERR_BAD_GATEWAY ||
		returnClassCode == COAP_S_ERR_SERVICE_UNAVAILABLE || returnClassCode == COAP_S_ERR_GATEWAY_TIMEOUT;
	bool returnClassCodeOK = classMethodValid || classSuccessValid || classClientErrorValid || classServerErrorValid;

	if (!returnClassCodeOK) return COAP_BAD_METHOD;

	return COAP_OK;
}


void CoAP_get_option_chars(CoAP_message_t* coap_struct, uint8_t option_to_get, char* output) {

	uint16_t current_byte = 0;
	int number_of_options = coap_struct->options.number_of_options;
	for (int i = 0; i < number_of_options; i++) {
		CoAP_option_t option = coap_struct->options.options[i];
		if (option.option_number == option_to_get) {
			output[current_byte++]= '/';
			
			for (int m = 0; m < option.option_length; m++) {
				output[current_byte++] = option.option_value[m];
			}
			if (option_to_get == COAP_OPTIONS_OBSERVE)
			{
				output[current_byte++] = '0';
			}
		}
	}
	output[current_byte] = 0;
}

URI_Path_t CoAP_get_URI(CoAP_message_t* coap_struct){

	uint16_t current_byte = 0;
	int number_of_options = coap_struct->options.number_of_options;
	URI_Path_t path;
	for (int i = 0; i < number_of_options; i++) {
		CoAP_option_t option = coap_struct->options.options[i];
		if (option.option_number == COAP_OPTIONS_URI_PATH) {
			uint16_t mpl = 1;
			uint16_t number = 0;
			for(int8_t m = option.option_length-1 ; m >=0  ; m--)
			{
				number += (option.option_value[m] - 0x30) * mpl;
				mpl *= 10;
			}
			path.data[path.path_depth++] = number;

		}
	}
	return path;
}

