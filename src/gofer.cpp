#include "gofer.h"

#include "SocketWrapper.h"

#define DEFAULT_BUFFLEN 4096
#define MAX_RESPONSE_LEN 2097152

std::vector<std::string> breakResponseIntoLines(std::string response) {
	std::vector<std::string> responseLines;

	std::size_t index = response.find('\n');
	while ((index != std::string::npos) && (index != response.length() - 1)) {
		responseLines.push_back(response.substr(0, index + 1));
		response = response.substr(index + 1);
		index = response.find('\n');
	}
	responseLines.push_back(response);

	return responseLines;
}

bool isResponseDirectory(const std::vector<std::string> responseLines) {
	const int numLinesToCheck = 4; // FIXME: Magic heuristic number.

	std::regex tabRegex("(\\t)");
	std::regex crlfRegex("(\\r\\n)");

	bool hasDirEntryFormat = true;
	for (int i = 0; i < numLinesToCheck && i < (responseLines.size() - 1); i++) {
		auto tabsBegin = std::sregex_iterator(responseLines[i].begin(), responseLines[i].end(), tabRegex);
		auto tabsEnd = std::sregex_iterator();

		auto tabNum = std::distance(tabsBegin, tabsEnd);
		bool crlfFound = std::regex_search(responseLines[i], crlfRegex);
		bool lastLine = (responseLines[i] == ".\r\n");

		if ((tabNum != 3 || !crlfFound) && !lastLine) {
			hasDirEntryFormat = false;
			break;
		}
	}

	return hasDirEntryFormat;
}

std::string gopherResponseToString(const GopherResponse& response) {
	std::string stringifiedResponse = "";
	stringifiedResponse = stringifiedResponse + "Type: " + std::to_string(response.type) + "\n";
	stringifiedResponse = stringifiedResponse + "Domain: " + response.domain + "\n";
	stringifiedResponse = stringifiedResponse + "Port: " + response.port + "\n";
	stringifiedResponse = stringifiedResponse + "Selector: " + response.selector + "\n";
	stringifiedResponse = stringifiedResponse + "Label: " + response.label + "\n";
	stringifiedResponse = stringifiedResponse + "Lines in body: " + std::to_string(response.response.size()) + "\n";
	stringifiedResponse = stringifiedResponse + "Number of referents: " + std::to_string(response.referents.size()) + "\n";
	return stringifiedResponse;
}

std::vector<std::shared_ptr<GopherResponse>> parseDirectoryResponse(const std::vector<std::string> responseLines) {
	std::vector<std::shared_ptr<GopherResponse>> referents;

	for (auto line = responseLines.begin(); line != responseLines.end(); ++line) {
		std::string subLine;
		if (line->substr(0) != ".\r\n") {
			std::shared_ptr<GopherResponse> referent(new GopherResponse());

			std::string type = line->substr(0, 1);
			if (type == "0") {
				referent->type = textfile;
			}
			else if (type == "1") {
				referent->type = directory;
			}
			else if (type == "2") {
				referent->type = phonebook;
			}
			else if (type == "3") {
				referent->type = error;
			}
			else if (type == "4") {
				referent->type = macfile;
			}
			else if (type == "5") {
				referent->type = dosfile;
			}
			else if (type == "6") {
				referent->type = uuencoded;
			}
			else if (type == "7") {
				referent->type = search;
			}
			else if (type == "8") {
				referent->type = telnet;
			}
			else if (type == "9") {
				referent->type = binfile;
			}
			else if (type == "+") {
				referent->type = redundant;
			}
			else if (type == "T") {
				referent->type = tn3270;
			}
			else if (type == "g") {
				referent->type = gif;
			}
			else if (type == "I") {
				referent->type = image;
			}
			else {
				referent->type = unknown;
			}
			if (type != "i") {
				referent->label = line->substr(1, (line->find('\t') - 1));
				subLine = line->substr(line->find('\t') + 1);
				referent->selector = subLine.substr(0, subLine.find('\t'));
				subLine = subLine.substr(subLine.find('\t') + 1);
				referent->domain = subLine.substr(0, subLine.find('\t'));
				subLine = subLine.substr(subLine.find('\t') + 1);
				referent->port = subLine.substr(0, subLine.find("\r\n"));

				referents.push_back(referent);
			}
		}
	}
	return referents;
}

void requestAndParseResponse(GopherIdentifier gi, GopherResponse& gResponse, GopherType type) {

	std::shared_ptr<SocketWrapper> sock = connectSocket(gi.domain.c_str(), gi.port.c_str());

	if (sock->GetSocketDescriptor() < 0) {
		printf("That domain or port could not be connected to!\n");
		cleanup();
		return;
	}

	int dataSent;
	if ((dataSent = send(sock, gi.selector.c_str())) < 0) {
		printf("Error sending data!\n");
		cleanup();
		return;
	}

	char receivingBuffer[DEFAULT_BUFFLEN] = { 0 };
	std::string response = "";

	int bytesReceived;
	while ((bytesReceived = receive(sock, receivingBuffer, DEFAULT_BUFFLEN)) > 0) {
		response.append(receivingBuffer, bytesReceived);
		//printf("%s", receivingBuffer);
		std::memset(receivingBuffer, 0, DEFAULT_BUFFLEN);
	}

	gResponse.response = breakResponseIntoLines(response);

	if (gResponse.type == directory || gResponse.type == search || isResponseDirectory(gResponse.response)) {
		gResponse.referents = parseDirectoryResponse(gResponse.response);
		if (gResponse.type != search) {
			gResponse.type = directory;
		}
	}

	gResponse.domain = gi.domain;
	gResponse.port = gi.port;
	gResponse.selector = gi.selector.substr(0, gi.selector.find("\r\n"));
}