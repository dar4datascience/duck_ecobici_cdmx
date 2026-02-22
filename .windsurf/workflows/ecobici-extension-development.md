---
description: Complete development guide for DuckDB-Ecobici extension
auto_execution_mode: 3
---

# DuckDB-Ecobici Extension Development Workflow

This workflow guides you through developing a DuckDB extension that fetches data from the Ecobici (CDMX bike-sharing system) open data platform.

## Project Overview

**Goal**: Create a DuckDB extension that allows users to query Ecobici data using SQL, supporting both real-time and historical data.

**Data Sources**:
- **Live Data**: GBFS (General Bikeshare Feed Specification) JSON feeds
  - Base URL: https://gbfs.mex.lyftbikes.com/gbfs/gbfs.json
  - Standard: https://github.com/NABSA/gbfs
- **Historical Data**: Monthly CSV files with trip data
  - Pattern: `https://ecobici.cdmx.gob.mx/wp-content/uploads/{YYYY}/{MM}/{YYYY-MM}.csv`
  - Note: Upload month (MM) is typically data month + 1
  - Available from 2023-01 onwards

**Reference Project**:
- Structure: https://github.com/ahuarte47/duckdb-eurostat

**Key Requirements**:
- No authentication required (public data)
- Support for GBFS real-time feeds (JSON)
- Support for historical trip data (CSV)
- Handle URL pattern variations in historical data
- Efficient data fetching and caching

## Development Steps

### 1. Set Up Development Environment

Ensure you have the required dependencies:
- CMake 3.5 or higher
- C++14 compatible compiler
- Ninja (recommended): `GEN=ninja`
- Git with submodules

Clone and initialize submodules:
```bash
git submodule update --init --recursive
```

### 2. Plan Core Functions

Implement these core functions for the Ecobici extension:

**a. ECOBICI_StationStatus()**
- Fetch real-time station status from GBFS
- Endpoint: `https://gbfs.mex.lyftbikes.com/gbfs/en/station_status.json`
- Return: station_id, num_bikes_available, num_docks_available, last_reported, is_installed, is_renting, is_returning

**b. ECOBICI_StationInformation()**
- Fetch station information from GBFS
- Endpoint: `https://gbfs.mex.lyftbikes.com/gbfs/en/station_information.json`
- Return: station_id, name, lat, lon, address, capacity, rental_methods

**c. ECOBICI_SystemInformation()**
- Fetch system-level information
- Endpoint: `https://gbfs.mex.lyftbikes.com/gbfs/en/system_information.json`
- Return: system_id, language, name, timezone, phone_number, email

**d. ECOBICI_HistoricalTrips(year INTEGER, month INTEGER)**
- Fetch historical trip data for a specific month
- Construct URL: `https://ecobici.cdmx.gob.mx/wp-content/uploads/{year}/{month:02d}/{year}-{month-1:02d}.csv`
- Handle URL pattern variations (some files have different naming)
- Return: trip data columns (trip_id, start_date, start_station, end_date, end_station, bike_id, user_type, etc.)

**e. ECOBICI_HistoricalTripsRange(start_year INTEGER, start_month INTEGER, end_year INTEGER, end_month INTEGER)**
- Fetch multiple months of historical data
- Iterate through date range and union results
- Return: combined trip data from all months

**f. ECOBICI_FreeBikeStatus()** (if available)
- Fetch free bike status from GBFS
- Endpoint: `https://gbfs.mex.lyftbikes.com/gbfs/en/free_bike_status.json`
- Return: bike_id, lat, lon, is_reserved, is_disabled

### 3. Implement HTTP Client for API Calls

Add HTTP library dependency to vcpkg.json:
- Recommended: cpp-httplib or libcurl
- Update CMakeLists.txt to link the HTTP library

Create API client class:
```cpp
// In src/include/ecobici_api_client.hpp
class EcobiciAPIClient {
private:
    string gbfs_base_url = "https://gbfs.mex.lyftbikes.com/gbfs/en/";
    string historical_base_url = "https://ecobici.cdmx.gob.mx/wp-content/uploads/";
    
public:
    EcobiciAPIClient();
    string FetchGBFSFeed(const string &feed_name);
    string FetchHistoricalCSV(int year, int month);
    vector<string> FetchHistoricalCSVRange(int start_year, int start_month, 
                                           int end_year, int end_month);
};
```

### 4. Implement Data Parsing

**For GBFS JSON feeds**:
- Add JSON parsing library (nlohmann/json recommended)
- Parse GBFS format responses (nested data structure)
- Extract `data.stations` or `data.bikes` arrays
- Convert to DuckDB table format

**For Historical CSV files**:
- Use DuckDB's built-in CSV reader capabilities
- Handle potential schema variations across months
- Parse date/time fields appropriately

Example JSON parsing structure:
```cpp
// GBFS response structure:
// {
//   "last_updated": 1234567890,
//   "ttl": 10,
//   "data": {
//     "stations": [...]
//   }
// }
```

### 5. Handle Historical Data URL Patterns

The historical CSV URLs have inconsistent patterns:
- Standard: `/{YYYY}/{MM}/{YYYY-MM}.csv` (e.g., `/2025/12/2025-11.csv`)
- Variations: Different filenames for some months (e.g., `ecobici_2024_enero.csv`, `datos_abiertos_2024_03-1-1.csv`)

Implementation strategy:
1. Try standard pattern first
2. If 404, try known variations
3. Cache successful URL patterns
4. Log warnings for missing months

### 6. Register Functions in Extension

Update `src/quack_extension.cpp` (rename to `ecobici_extension.cpp`):

```cpp
static void LoadInternal(ExtensionLoader &loader) {
    // GBFS real-time data functions
    auto station_status_function = TableFunction("ECOBICI_StationStatus", 
        {}, EcobiciStationStatusFun);
    loader.RegisterFunction(station_status_function);
    
    auto station_info_function = TableFunction("ECOBICI_StationInformation", 
        {}, EcobiciStationInformationFun);
    loader.RegisterFunction(station_info_function);
    
    auto system_info_function = TableFunction("ECOBICI_SystemInformation", 
        {}, EcobiciSystemInformationFun);
    loader.RegisterFunction(system_info_function);
    
    // Historical data functions
    auto historical_trips_function = TableFunction("ECOBICI_HistoricalTrips", 
        {LogicalType::INTEGER, LogicalType::INTEGER}, EcobiciHistoricalTripsFun);
    loader.RegisterFunction(historical_trips_function);
    
    auto historical_range_function = TableFunction("ECOBICI_HistoricalTripsRange", 
        {LogicalType::INTEGER, LogicalType::INTEGER, 
         LogicalType::INTEGER, LogicalType::INTEGER}, 
        EcobiciHistoricalTripsRangeFun);
    loader.RegisterFunction(historical_range_function);
}
```

### 7. Update Build Configuration

**a. Update CMakeLists.txt**:
- Change TARGET_NAME from "quack" to "ecobici"
- Add HTTP library dependencies
- Add JSON library (nlohmann/json)

**b. Update extension_config.cmake**:
```cmake
duckdb_extension_load(ecobici
    DESCRIPTION "DuckDB extension for querying Ecobici (CDMX bike-sharing) data"
    DONT_LINK
)
```

**c. Update vcpkg.json**:
```json
{
  "dependencies": [
    "nlohmann-json",
    "cpp-httplib"
  ]
}
```

### 8. Write Tests

Create SQL tests in `test/sql/`:

**a. test/sql/ecobici_realtime.test**:
```sql
# Test fetching station status
statement ok
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

# Test station status
query I
SELECT COUNT(*) > 0 FROM ECOBICI_StationStatus();
----
true

# Test station information
query I
SELECT COUNT(*) > 0 FROM ECOBICI_StationInformation();
----
true

# Test system information
query I
SELECT COUNT(*) FROM ECOBICI_SystemInformation();
----
1
```

**b. test/sql/ecobici_historical.test**:
```sql
# Test fetching historical data
statement ok
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

# Test single month
query I
SELECT COUNT(*) > 0 FROM ECOBICI_HistoricalTrips(2024, 1);
----
true

# Test date range
query I
SELECT COUNT(*) > 0 FROM ECOBICI_HistoricalTripsRange(2024, 1, 2024, 3);
----
true

# Test data quality - check for required columns
query I
SELECT COUNT(*) FROM (
    SELECT * FROM ECOBICI_HistoricalTrips(2024, 1) LIMIT 1
) WHERE start_date IS NOT NULL;
----
1
```

**c. test/sql/ecobici_integration.test**:
```sql
# Test joining real-time and historical data
statement ok
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

# Join station info with current status
query I
SELECT COUNT(*) > 0 
FROM ECOBICI_StationInformation() info
JOIN ECOBICI_StationStatus() status 
ON info.station_id = status.station_id;
----
true
```

### 9. Build and Test

Build the extension:
```bash
make clean
make release
```

Run tests:
```bash
make test
```

Test manually:
```bash
./build/release/duckdb
```

Then in DuckDB:
```sql
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

-- Test real-time data
SELECT * FROM ECOBICI_StationStatus() LIMIT 5;
SELECT * FROM ECOBICI_StationInformation() LIMIT 5;

-- Test historical data
SELECT * FROM ECOBICI_HistoricalTrips(2024, 1) LIMIT 10;

-- Analyze trip patterns
SELECT 
    DATE_TRUNC('hour', start_date) as hour,
    COUNT(*) as trips
FROM ECOBICI_HistoricalTrips(2024, 1)
GROUP BY hour
ORDER BY hour;
```

### 10. Documentation

Create comprehensive documentation in `docs/`:

**a. docs/README.md**:
- Overview of the extension
- Installation instructions
- Quick start guide
- Example queries

**b. docs/API.md**:
- Detailed function reference
- Parameter descriptions
- Return value schemas
- Example queries for each function

**c. docs/DATA_SOURCES.md**:
- Information about Ecobici data sources
- GBFS specification details
- Historical data availability and patterns
- Data schema documentation
- Known issues and limitations

**d. docs/EXAMPLES.md**:
- Common use cases
- Analytics queries (busiest stations, peak hours, trip patterns)
- Combining real-time and historical data
- Visualization examples

### 11. Implement Caching (Advanced)

Implement smart caching for better performance:

**For GBFS data**:
- Cache with TTL (time-to-live) from GBFS response
- Typical TTL is 10-60 seconds
- Invalidate cache when TTL expires

**For Historical data**:
- Cache downloaded CSV data
- Historical data doesn't change, so cache indefinitely
- Store in DuckDB temporary tables or files

```cpp
// In src/include/ecobici_cache.hpp
class EcobiciCache {
private:
    struct CacheEntry {
        string data;
        time_t expiry;
    };
    map<string, CacheEntry> cache;
    
public:
    bool Has(const string &key);
    string Get(const string &key);
    void Set(const string &key, const string &data, int ttl_seconds);
    void Clear();
};
```

### 12. Error Handling

Implement robust error handling:

**Network errors**:
- Connection timeouts
- DNS resolution failures
- HTTP error codes (404, 500, etc.)

**Data errors**:
- Invalid JSON responses
- Missing required fields
- Schema mismatches in CSV files

**User input errors**:
- Invalid year/month parameters
- Future dates
- Dates before data availability (pre-2023)

Example error messages:
```cpp
throw InvalidInputException(
    "Historical data only available from 2023-01 onwards. "
    "Requested: %d-%02d", year, month
);

throw IOException(
    "Failed to fetch historical data for %d-%02d. "
    "URL may not exist or network error occurred.", year, month
);
```

### 13. Implement Helper Functions (Optional)

Additional utility functions:

**a. ECOBICI_AvailableMonths()**
- Return list of available historical data months
- Useful for discovering what data exists

**b. ECOBICI_StationMetrics(station_id VARCHAR, year INTEGER, month INTEGER)**
- Aggregate metrics for a specific station
- Return: total_trips_started, total_trips_ended, avg_bikes_available, etc.

**c. ECOBICI_TripDuration()**
- Calculate trip durations from historical data
- Return: trip_id, duration_minutes, start_station, end_station

### 14. Prepare for Distribution

**a. Update README.md**:
```markdown
# DuckDB-Ecobici Extension

Query Ecobici (CDMX bike-sharing) data directly from DuckDB.

## Features
- Real-time station status and availability (GBFS)
- Historical trip data (2023-present)
- No authentication required
- Fast, efficient querying

## Installation
[Installation instructions]

## Quick Start
[Usage examples]
```

**b. Set up CI/CD**:
- GitHub Actions workflows are in `.github/workflows/`
- Ensure tests pass on multiple platforms (Linux, macOS, Windows)
- Test with different DuckDB versions

**c. Version management**:
- Update version in extension_config.cmake
- Follow semantic versioning (MAJOR.MINOR.PATCH)
- Tag releases appropriately

### 15. Performance Optimization

**For Historical Data**:
- Implement parallel downloads for date ranges
- Use DuckDB's parallel CSV reader
- Consider compression for cached data

**For Real-time Data**:
- Minimize API calls with smart caching
- Use connection pooling for HTTP requests
- Implement request batching where possible

**Query Optimization**:
- Implement filter pushdown where applicable
- Use DuckDB's projection pushdown
- Optimize JSON parsing for large responses

## Common Commands Reference

```bash
# Build
make release

# Build with Ninja (faster)
GEN=ninja make release

# Run tests
make test

# Clean build
make clean

# Format code
make format

# Run specific test
./build/release/test/unittest "test/sql/ecobici_realtime.test"

# Debug build
make debug

# Install extension locally
make install
```

## API Endpoints Reference

**GBFS Feeds** (Real-time):
- Discovery: `https://gbfs.mex.lyftbikes.com/gbfs/gbfs.json`
- System Info: `https://gbfs.mex.lyftbikes.com/gbfs/en/system_information.json`
- Station Info: `https://gbfs.mex.lyftbikes.com/gbfs/en/station_information.json`
- Station Status: `https://gbfs.mex.lyftbikes.com/gbfs/en/station_status.json`
- Free Bike Status: `https://gbfs.mex.lyftbikes.com/gbfs/en/free_bike_status.json`

**Historical Data** (CSV):
- Pattern: `https://ecobici.cdmx.gob.mx/wp-content/uploads/{YYYY}/{MM}/{YYYY-MM}.csv`
- Note: Upload month is typically data month + 1
- Example: `https://ecobici.cdmx.gob.mx/wp-content/uploads/2025/12/2025-11.csv`
- Available: 2023-01 to present

## Data Schema Reference

**Station Status** (GBFS):
- `station_id` (VARCHAR): Unique station identifier
- `num_bikes_available` (INTEGER): Available bikes
- `num_docks_available` (INTEGER): Available docks
- `last_reported` (TIMESTAMP): Last update time
- `is_installed` (BOOLEAN): Station installed
- `is_renting` (BOOLEAN): Station renting bikes
- `is_returning` (BOOLEAN): Station accepting returns

**Station Information** (GBFS):
- `station_id` (VARCHAR): Unique station identifier
- `name` (VARCHAR): Station name
- `lat` (DOUBLE): Latitude
- `lon` (DOUBLE): Longitude
- `address` (VARCHAR): Street address
- `capacity` (INTEGER): Total dock capacity

**Historical Trips** (CSV):
- Schema may vary by month, common fields:
- Trip identifiers and timestamps
- Start/end station information
- Bike ID
- User type
- Trip duration

## Example Use Cases

**1. Find busiest stations**:
```sql
SELECT name, num_bikes_available, num_docks_available, capacity
FROM ECOBICI_StationInformation() info
JOIN ECOBICI_StationStatus() status ON info.station_id = status.station_id
WHERE num_bikes_available = 0 OR num_docks_available = 0
ORDER BY capacity DESC;
```

**2. Analyze trip patterns**:
```sql
SELECT 
    HOUR(start_date) as hour_of_day,
    COUNT(*) as total_trips,
    AVG(duration_minutes) as avg_duration
FROM ECOBICI_HistoricalTrips(2024, 1)
GROUP BY hour_of_day
ORDER BY hour_of_day;
```

**3. Station utilization over time**:
```sql
SELECT 
    start_station_name,
    COUNT(*) as trips_started,
    COUNT(DISTINCT DATE(start_date)) as active_days
FROM ECOBICI_HistoricalTripsRange(2024, 1, 2024, 12)
GROUP BY start_station_name
ORDER BY trips_started DESC
LIMIT 20;
```

## Troubleshooting

**Build errors**:
- Ensure all submodules are initialized: `git submodule update --init --recursive`
- Check CMake version: `cmake --version` (>= 3.5 required)
- Verify C++14 compiler support

**Network errors**:
- Check internet connectivity
- Verify URLs are accessible: `curl https://gbfs.mex.lyftbikes.com/gbfs/gbfs.json`
- Check for firewall/proxy issues

**Data errors**:
- Historical data may not be available for very recent months
- Some months may have different CSV schemas
- URL patterns may change over time

**Test failures**:
- Ensure network access during tests
- Check API availability
- Review test expectations vs. actual API responses

## Known Limitations

1. **Historical Data Availability**: Data only available from 2023-01 onwards
2. **URL Pattern Variations**: Some months have non-standard filenames
3. **No Authentication**: Public data only, no user-specific data
4. **Real-time Data TTL**: GBFS data has short TTL (10-60 seconds)
5. **CSV Schema Variations**: Historical data schema may vary across months

## Next Steps After Initial Implementation

1. Add geospatial analysis functions (distance calculations, route mapping)
2. Implement predictive analytics (demand forecasting, availability prediction)
3. Add data quality checks and validation
4. Create visualization examples (maps, charts, dashboards)
5. Implement data export functions (GeoJSON, KML)
6. Add support for other bike-sharing systems using GBFS
7. Create example notebooks/tutorials
8. Submit to DuckDB Community Extensions

## Contributing

[Contribution guidelines]

## License

[License information]
