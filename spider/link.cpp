#include "link.h"

Link::Link(const std::string& linkStr) {
	std::regex urlRegex(R"((https?://)([^/]+)(/.*)?)");
	std::smatch matches;
	if (std::regex_search(linkStr, matches, urlRegex)) {
		Link link;
		std::string protocolStr = matches[1].str();
		if (matches[1].str() == "https://")
			protocol = ProtocolType::HTTPS;
		else
			protocol = ProtocolType::HTTP;
		hostName = matches[2].str();
		query = matches[3].str();
	}
	else {
		throw std::exception("Uncorrect URL");
	}
}

std::string Link::toString() const {
	std::string result = ((protocol == ProtocolType::HTTP) ? "http://" : "https://") + hostName + query;
	return result;
}
