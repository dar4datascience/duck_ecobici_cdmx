#pragma once

#include "duckdb.hpp"
#include <string>
#include <vector>

namespace duckdb {

class EcobiciAPIClient {
private:
	std::string gbfs_base_url = "https://gbfs.mex.lyftbikes.com/gbfs/en/";
	std::string historical_base_url = "https://ecobici.cdmx.gob.mx/wp-content/uploads/";

public:
	EcobiciAPIClient();

	std::string FetchGBFSFeed(const std::string &feed_name);
	std::string FetchHistoricalCSV(int year, int month);
	std::vector<std::string> FetchHistoricalCSVRange(int start_year, int start_month, int end_year, int end_month);
};

} // namespace duckdb
