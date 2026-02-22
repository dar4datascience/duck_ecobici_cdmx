#include "ecobici_api_client.hpp"
#include "duckdb/common/exception.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <sstream>
#include <iomanip>

namespace duckdb {

EcobiciAPIClient::EcobiciAPIClient() {
}

std::string EcobiciAPIClient::FetchGBFSFeed(const std::string &feed_name) {
	httplib::Client cli("https://gbfs.mex.lyftbikes.com");
	cli.set_follow_location(true);
	cli.set_connection_timeout(10, 0);
	cli.set_read_timeout(30, 0);

	std::string path = "/gbfs/en/" + feed_name + ".json";
	auto res = cli.Get(path.c_str());

	if (!res) {
		throw IOException("Failed to fetch GBFS feed: " + feed_name + " - Connection error");
	}

	if (res->status != 200) {
		throw IOException("Failed to fetch GBFS feed: " + feed_name + " - HTTP " + std::to_string(res->status));
	}

	return res->body;
}

std::string EcobiciAPIClient::FetchHistoricalCSV(int year, int month) {
	httplib::Client cli("https://ecobici.cdmx.gob.mx");
	cli.set_follow_location(true);
	cli.set_connection_timeout(10, 0);
	cli.set_read_timeout(60, 0);

	std::ostringstream path_stream;
	path_stream << "/wp-content/uploads/" << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << year
	            << "-" << std::setfill('0') << std::setw(2) << (month - 1) << ".csv";

	std::string path = path_stream.str();
	auto res = cli.Get(path.c_str());

	if (!res) {
		throw IOException("Failed to fetch historical CSV for " + std::to_string(year) + "-" + std::to_string(month) +
		                  " - Connection error");
	}

	if (res->status == 404) {
		std::ostringstream alt_path_stream;
		alt_path_stream << "/wp-content/uploads/" << year << "/" << std::setfill('0') << std::setw(2) << month << "/"
		                << year << "-" << std::setfill('0') << std::setw(2) << month << ".csv";

		std::string alt_path = alt_path_stream.str();
		res = cli.Get(alt_path.c_str());

		if (!res || res->status != 200) {
			throw IOException("Failed to fetch historical CSV for " + std::to_string(year) + "-" +
			                  std::to_string(month) + " - File not found (tried multiple URL patterns)");
		}
	} else if (res->status != 200) {
		throw IOException("Failed to fetch historical CSV for " + std::to_string(year) + "-" + std::to_string(month) +
		                  " - HTTP " + std::to_string(res->status));
	}

	return res->body;
}

std::vector<std::string> EcobiciAPIClient::FetchHistoricalCSVRange(int start_year, int start_month, int end_year,
                                                                   int end_month) {
	std::vector<std::string> results;

	int current_year = start_year;
	int current_month = start_month;

	while (current_year < end_year || (current_year == end_year && current_month <= end_month)) {
		try {
			results.push_back(FetchHistoricalCSV(current_year, current_month));
		} catch (const IOException &e) {
		}

		current_month++;
		if (current_month > 12) {
			current_month = 1;
			current_year++;
		}
	}

	return results;
}

} // namespace duckdb
