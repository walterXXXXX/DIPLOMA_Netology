#pragma once 
#include <string>
#include <regex>

enum class ProtocolType
{
	HTTP = 0,
	HTTPS = 1
};

struct Link
{
	Link() = default;
	Link(const std::string& linkStr);

	std::string toString() const;

	ProtocolType protocol;
	std::string hostName;
	std::string query;

	bool operator==(const Link& l) const
	{
		return protocol == l.protocol
			&& hostName == l.hostName
			&& query == l.query;
	}
};

