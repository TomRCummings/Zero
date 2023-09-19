#pragma once

#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "SocketWrapper.h"

enum GopherType {directory, textfile, phonebook, macfile, dosfile, uuencoded, telnet, redundant, tn3270, gif, image, binfile, search, info, error, unknown};

struct GopherResponse {
	GopherType type;
	std::string domain = "";
	std::string port = "";
	std::string selector = "";
	std::string label = "";
	std::vector<std::string> response;
	std::vector<std::shared_ptr<GopherResponse>> referents;
};