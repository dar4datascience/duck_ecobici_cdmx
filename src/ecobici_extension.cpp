#define DUCKDB_EXTENSION_MAIN

#include "ecobici_extension.hpp"
#include "ecobici_api_client.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/main/extension_util.hpp"
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

namespace duckdb {

struct EcobiciStationStatusBindData : public TableFunctionData {
	std::vector<std::vector<Value>> rows;
	idx_t offset = 0;
};

struct EcobiciStationInfoBindData : public TableFunctionData {
	std::vector<std::vector<Value>> rows;
	idx_t offset = 0;
};

struct EcobiciSystemInfoBindData : public TableFunctionData {
	std::vector<std::vector<Value>> rows;
	idx_t offset = 0;
};

struct EcobiciHistoricalTripsBindData : public TableFunctionData {
	int year;
	int month;
	std::string csv_data;
	idx_t offset = 0;
};

static unique_ptr<FunctionData> EcobiciStationStatusBind(ClientContext &context, TableFunctionBindInput &input,
                                                          vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<EcobiciStationStatusBindData>();
	
	names.emplace_back("station_id");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("num_bikes_available");
	return_types.emplace_back(LogicalType::INTEGER);
	names.emplace_back("num_docks_available");
	return_types.emplace_back(LogicalType::INTEGER);
	names.emplace_back("last_reported");
	return_types.emplace_back(LogicalType::BIGINT);
	names.emplace_back("is_installed");
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("is_renting");
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("is_returning");
	return_types.emplace_back(LogicalType::BOOLEAN);
	
	EcobiciAPIClient client;
	std::string json_data = client.FetchGBFSFeed("station_status");
	
	auto parsed = json::parse(json_data);
	if (parsed.contains("data") && parsed["data"].contains("stations")) {
		for (const auto &station : parsed["data"]["stations"]) {
			std::vector<Value> row;
			row.push_back(Value(station.value("station_id", "")));
			row.push_back(Value::INTEGER(station.value("num_bikes_available", 0)));
			row.push_back(Value::INTEGER(station.value("num_docks_available", 0)));
			row.push_back(Value::BIGINT(station.value("last_reported", 0)));
			row.push_back(Value::BOOLEAN(station.value("is_installed", 1) == 1));
			row.push_back(Value::BOOLEAN(station.value("is_renting", 1) == 1));
			row.push_back(Value::BOOLEAN(station.value("is_returning", 1) == 1));
			result->rows.push_back(std::move(row));
		}
	}
	
	return std::move(result);
}

static void EcobiciStationStatusFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<EcobiciStationStatusBindData>();
	idx_t count = 0;
	
	while (data.offset < data.rows.size() && count < STANDARD_VECTOR_SIZE) {
		for (idx_t col = 0; col < output.ColumnCount(); col++) {
			output.SetValue(col, count, data.rows[data.offset][col]);
		}
		data.offset++;
		count++;
	}
	
	output.SetCardinality(count);
}

static unique_ptr<FunctionData> EcobiciStationInfoBind(ClientContext &context, TableFunctionBindInput &input,
                                                        vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<EcobiciStationInfoBindData>();
	
	names.emplace_back("station_id");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("name");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("lat");
	return_types.emplace_back(LogicalType::DOUBLE);
	names.emplace_back("lon");
	return_types.emplace_back(LogicalType::DOUBLE);
	names.emplace_back("address");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("capacity");
	return_types.emplace_back(LogicalType::INTEGER);
	
	EcobiciAPIClient client;
	std::string json_data = client.FetchGBFSFeed("station_information");
	
	auto parsed = json::parse(json_data);
	if (parsed.contains("data") && parsed["data"].contains("stations")) {
		for (const auto &station : parsed["data"]["stations"]) {
			std::vector<Value> row;
			row.push_back(Value(station.value("station_id", "")));
			row.push_back(Value(station.value("name", "")));
			row.push_back(Value::DOUBLE(station.value("lat", 0.0)));
			row.push_back(Value::DOUBLE(station.value("lon", 0.0)));
			row.push_back(Value(station.value("address", "")));
			row.push_back(Value::INTEGER(station.value("capacity", 0)));
			result->rows.push_back(std::move(row));
		}
	}
	
	return std::move(result);
}

static void EcobiciStationInfoFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<EcobiciStationInfoBindData>();
	idx_t count = 0;
	
	while (data.offset < data.rows.size() && count < STANDARD_VECTOR_SIZE) {
		for (idx_t col = 0; col < output.ColumnCount(); col++) {
			output.SetValue(col, count, data.rows[data.offset][col]);
		}
		data.offset++;
		count++;
	}
	
	output.SetCardinality(count);
}

static unique_ptr<FunctionData> EcobiciSystemInfoBind(ClientContext &context, TableFunctionBindInput &input,
                                                       vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<EcobiciSystemInfoBindData>();
	
	names.emplace_back("system_id");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("language");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("name");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("timezone");
	return_types.emplace_back(LogicalType::VARCHAR);
	
	EcobiciAPIClient client;
	std::string json_data = client.FetchGBFSFeed("system_information");
	
	auto parsed = json::parse(json_data);
	if (parsed.contains("data")) {
		auto &data_obj = parsed["data"];
		std::vector<Value> row;
		row.push_back(Value(data_obj.value("system_id", "")));
		row.push_back(Value(data_obj.value("language", "")));
		row.push_back(Value(data_obj.value("name", "")));
		row.push_back(Value(data_obj.value("timezone", "")));
		result->rows.push_back(std::move(row));
	}
	
	return std::move(result);
}

static void EcobiciSystemInfoFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<EcobiciSystemInfoBindData>();
	idx_t count = 0;
	
	while (data.offset < data.rows.size() && count < STANDARD_VECTOR_SIZE) {
		for (idx_t col = 0; col < output.ColumnCount(); col++) {
			output.SetValue(col, count, data.rows[data.offset][col]);
		}
		data.offset++;
		count++;
	}
	
	output.SetCardinality(count);
}

static void LoadInternal(ExtensionLoader &loader) {
	// Register GBFS real-time data functions
	TableFunction station_status_function("ecobici_station_status", {}, EcobiciStationStatusFunction, EcobiciStationStatusBind);
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), station_status_function);
	
	TableFunction station_info_function("ecobici_station_information", {}, EcobiciStationInfoFunction, EcobiciStationInfoBind);
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), station_info_function);
	
	TableFunction system_info_function("ecobici_system_information", {}, EcobiciSystemInfoFunction, EcobiciSystemInfoBind);
	ExtensionUtil::RegisterFunction(loader.GetDatabase(), system_info_function);
}

void EcobiciExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}

std::string EcobiciExtension::Name() {
	return "ecobici";
}

std::string EcobiciExtension::Version() const {
#ifdef EXT_VERSION_ECOBICI
	return EXT_VERSION_ECOBICI;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_CPP_EXTENSION_ENTRY(ecobici, loader) {
	duckdb::LoadInternal(loader);
}
}
