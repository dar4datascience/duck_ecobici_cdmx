#define DUCKDB_EXTENSION_MAIN

#include "ecobici_extension.hpp"
#include "ecobici_api_client.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
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

// Historical Data Functions
static unique_ptr<FunctionData> EcobiciHistoricalTripsBind(ClientContext &context, TableFunctionBindInput &input,
                                                           vector<LogicalType> &return_types, vector<string> &names) {
	auto result = make_uniq<EcobiciHistoricalTripsBindData>();

	// Get year and month parameters
	result->year = input.inputs[0].GetValue<int32_t>();
	result->month = input.inputs[1].GetValue<int32_t>();

	// Validate parameters
	if (result->year < 2010 || result->year > 2100) {
		throw InvalidInputException("Year must be between 2010 and 2100");
	}
	if (result->month < 1 || result->month > 12) {
		throw InvalidInputException("Month must be between 1 and 12");
	}

	// Define CSV schema based on Ecobici open data format
	names.emplace_back("Genero_Usuario");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("Edad_Usuario");
	return_types.emplace_back(LogicalType::INTEGER);
	names.emplace_back("Bici");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("Ciclo_Estacion_Retiro");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("Fecha_Retiro");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("Ciclo_Estacion_Arribo");
	return_types.emplace_back(LogicalType::VARCHAR);
	names.emplace_back("Fecha_Arribo");
	return_types.emplace_back(LogicalType::VARCHAR);

	// Fetch CSV data
	EcobiciAPIClient client;
	try {
		result->csv_data = client.FetchHistoricalCSV(result->year, result->month);
	} catch (const IOException &e) {
		throw IOException("Failed to fetch historical data for " + std::to_string(result->year) + "-" +
		                  std::to_string(result->month) + ": " + std::string(e.what()));
	}

	return std::move(result);
}

static void EcobiciHistoricalTripsFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	auto &data = data_p.bind_data->CastNoConst<EcobiciHistoricalTripsBindData>();

	// Parse CSV data using DuckDB's CSV reader
	// For now, we'll use a simple line-by-line parser
	// In production, should use DuckDB's built-in CSV reader for better performance

	if (data.offset == 0 && !data.csv_data.empty()) {
		// Simple CSV parsing - split by lines and commas
		std::istringstream stream(data.csv_data);
		std::string line;
		bool first_line = true;

		idx_t count = 0;
		while (std::getline(stream, line) && count < STANDARD_VECTOR_SIZE) {
			if (first_line) {
				first_line = false;
				continue; // Skip header
			}

			if (line.empty()) {
				continue;
			}

			// Simple CSV parsing (note: doesn't handle quoted commas properly)
			std::vector<std::string> fields;
			std::istringstream line_stream(line);
			std::string field;

			while (std::getline(line_stream, field, ',')) {
				fields.push_back(field);
			}

			if (fields.size() >= 7) {
				output.SetValue(0, count, Value(fields[0])); // Genero_Usuario
				try {
					output.SetValue(1, count, Value::INTEGER(std::stoi(fields[1]))); // Edad_Usuario
				} catch (...) {
					output.SetValue(1, count, Value()); // NULL if parse fails
				}
				output.SetValue(2, count, Value(fields[2])); // Bici
				output.SetValue(3, count, Value(fields[3])); // Ciclo_Estacion_Retiro
				output.SetValue(4, count, Value(fields[4])); // Fecha_Retiro
				output.SetValue(5, count, Value(fields[5])); // Ciclo_Estacion_Arribo
				output.SetValue(6, count, Value(fields[6])); // Fecha_Arribo

				count++;
			}
		}

		output.SetCardinality(count);
		data.offset = 1; // Mark as processed
	} else {
		output.SetCardinality(0);
	}
}

static void LoadInternal(ExtensionLoader &loader) {
	// Register GBFS real-time data functions
	TableFunction station_status_function("ecobici_station_status", {}, EcobiciStationStatusFunction,
	                                      EcobiciStationStatusBind);
	loader.RegisterFunction(station_status_function);

	TableFunction station_info_function("ecobici_station_information", {}, EcobiciStationInfoFunction,
	                                    EcobiciStationInfoBind);
	loader.RegisterFunction(station_info_function);

	TableFunction system_info_function("ecobici_system_information", {}, EcobiciSystemInfoFunction,
	                                   EcobiciSystemInfoBind);
	loader.RegisterFunction(system_info_function);

	// Register historical data functions
	TableFunction historical_trips_function("ecobici_historical_trips", {LogicalType::INTEGER, LogicalType::INTEGER},
	                                        EcobiciHistoricalTripsFunction, EcobiciHistoricalTripsBind);
	loader.RegisterFunction(historical_trips_function);
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
