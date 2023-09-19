#pragma once

#include <string>
#include <vector>

#include "GopherIdentifier.h"
#include "GopherResponse.h"

bool isResponseDirectory(const std::vector<std::string> responseLines);
std::string gopherResponseToString(const GopherResponse& response);
std::vector<std::shared_ptr<GopherResponse>> parseDirectoryResponse(const std::vector<std::string> responseLines);
void requestAndParseResponse(GopherIdentifier identifier, GopherResponse& response, GopherType type = unknown);