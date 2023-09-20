#include <iostream>
#include <fstream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "gofer.h"
#include "GopherResponse.h"
#include "GopherIdentifier.h"
#include "SocketWrapper.h"

#define DEFAULT_PORT "70"

std::vector<std::string> breakResponseIntoLines(const std::string response);
void displayResponse(const GopherResponse& response);
void displayResponseFile(const std::string response);
void saveResponseFile(const std::string response);
void displayDirectory(const GopherResponse& dir);

int main() {

	if (initialize() != 0) {
		cleanup();
		return 1;
	}

	std::unordered_map<GopherIdentifier, std::shared_ptr<GopherResponse>> responseCache;
	std::vector<std::shared_ptr<GopherResponse>> backStack;

	bool runInputLoop = true;
	bool selectFromCurrentPage = false;
	bool onStack = false;

	std::string domain, selector, port;
	GopherType type = unknown;

	std::shared_ptr<GopherResponse> response;

	while (runInputLoop) {

		if (!selectFromCurrentPage) {

			port = DEFAULT_PORT;

			std::cout << "Enter the desired domain: ";
			std::getline(std::cin, domain);

			std::cout << "Enter a selector, or nothing to see the domain listing: ";
			std::getline(std::cin, selector);

			response = std::shared_ptr<GopherResponse>(new GopherResponse());
			backStack.push_back(response);
		}
		else {
			domain = backStack.back()->domain;
			port = backStack.back()->port;
			selector = backStack.back()->selector;
			type = backStack.back()->type;

			if (type == search) {
				selector = selector.substr(0, selector.find("\t"));
				std::string searchString;
				std::cout << "Enter your search string: ";
				std::getline(std::cin, searchString);
				selector = selector + "\t" + searchString;
				//std::cout << "Selector: " << selector << std::endl;
			}
		} // At this point it is assumed that port, domain, and selector are set and that backStack.back() contains the pointer to the GopherResponse representing the requested resource

		if (!onStack) {
			GopherIdentifier gi = { domain, port, selector };
			auto cacheResult = responseCache.find(gi);
			if (cacheResult == responseCache.end()) { //current identifier is not in response cache
				//std::cout << "Response not found in cache!" << std::endl;
				//std::cout << gopherResponseToString(*backStack.back()) << std::endl;
				responseCache.insert({ gi, backStack.back() });

				gi.selector += "\r\n";

				requestAndParseResponse(gi, *backStack.back(), type);
				if (backStack.back()->response.size() == 0) {
					//Either no connection established or no response
					backStack.pop_back();
					continue;
				}

			}
			else {
				//std::cout << "Response found in cache!" << std::endl;
				backStack.back() = cacheResult->second;
			}
		}
		onStack = false;

		std::string nextInput;
		bool usableInputGot = false;
		displayResponse(*backStack.back());
		while (!usableInputGot) {
			// At this point we have a parsed response at the back of the stack

			if (backStack.back()->type == directory) {
				std::cout << "Enter a line number to access resource, \"back\" to go back, or hit enter to choose a different domain:" << std::endl;
			}
			else {
				std::cout << "Type \"back\" to go back, \"save\" to save the current resource to your disk, or hit enter to choose a different domain:" << std::endl;
			}
			std::getline(std::cin, nextInput);
			if (backStack.back()->type == directory && !nextInput.empty() && nextInput.find_first_not_of("0123456789") == std::string::npos) {
				const int i{ std::stoi(nextInput) };
				if (i >= backStack.back()->referents.size()) {
					std::cout << "The number you entered does not correspond with an entry in this directory; please try again." << std::endl;
				}
				else {
					usableInputGot = true;
					selectFromCurrentPage = true;
					response = backStack.back()->referents[i];
					backStack.push_back(response);
				}
			}
			else if (nextInput == "back") {
				if (backStack.size() == 1) {
					std::cout << "Nothing to go back to! Choose a different option." << std::endl;
				}
				else {
					selectFromCurrentPage = true;
					usableInputGot = true;
					backStack.pop_back();
					onStack = true;
				}
			}
			else if (nextInput == "save") {
				std::string filePath;
				std::cout << "Enter the location you want to save this resource." << std::endl;
				std::getline(std::cin, filePath);
				std::ofstream file;
				file.open(filePath);
				for (auto line = backStack.back()->response.begin(); line < backStack.back()->response.end(); line++) {
					file << line->c_str();
				}
				file.close();
			}
			else if (nextInput == "") {
				usableInputGot = true;
				selectFromCurrentPage = false;
			}
			else if (nextInput == "exit") {
				usableInputGot = true;
				runInputLoop = false;
			}
			else {
				std::cout << "Input not recognized, please try again." << std::endl;
			}
		}
	}
	cleanup();
	return 0;
}

void displayResponse(const GopherResponse& response) {
	if (response.type == directory || response.type == search) {
		displayDirectory(response);
	}
	else {
		for (auto ptr = response.response.begin(); ptr < response.response.end(); ptr++) {
			printf("%s", ptr->c_str());
		}
		printf("\n");
	}
}

void displayDirectory(const GopherResponse& dir) {
	int referenceCount = 0;
	for (auto dirEntry = dir.response.begin(); dirEntry < dir.response.end(); dirEntry++) {
		std::string typeString = "";
		std::string type = dirEntry->substr(0, 1);
		if (type == "i") {
			typeString = "Info";
		}
		else if (type == "0") {
			typeString = "Text";
		}
		else if (type == "1") {
			typeString = "Dir";
		}
		else if (type == "2") {
			typeString = "Phone";
		}
		else if (type == "3") {
			//referent->type = error;
			typeString = "Error";
		}
		else if (type == "4") {
			//referent->type = macfile;
			typeString = "MacFile";
		}
		else if (type == "5") {
			//referent->type = dosfile;
			typeString = "DOS";
		}
		else if (type == "6") {
			//referent->type = uuencoded;
			typeString = "Uuenc";
		}
		else if (type == "7") {
			//referent->type = search;
			typeString = "Search";
		}
		else if (type == "8") {
			//referent->type = telnet;
			typeString = "Telnet";
		}
		else if (type == "9") {
			//referent->type = binfile;
			typeString = "Bin";
		}
		else if (type == "+") {
			//referent->type = redundant;
			typeString = "Red";
		}
		else if (type == "T") {
			//referent->type = tn3270;
			typeString = "TN3270";
		}
		else if (type == "g") {
			//referent->type = gif;
			typeString = "GIF";
		}
		else if (type == "I") {
			//referent->type = image;
			typeString = "Image";
		}
		else {
			//referent->type = unknown;
			typeString = "Unknown";
		}


		std::string label = dirEntry->substr(1, (dirEntry->find('\t') - 1));

		if (typeString == "Info" || typeString == "Error") {
			std::cout << " \t\t" + label << std::endl;
		}
		else {
			std::cout << std::to_string(referenceCount) + "\t" + typeString + "\t" + label << std::endl;
			referenceCount++;
		}
	}
}
