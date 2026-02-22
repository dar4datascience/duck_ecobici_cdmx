# DuckDB Ecobici Extension

A DuckDB extension for querying Ecobici (CDMX bike-sharing system) data directly from SQL. This extension provides access to both real-time GBFS (General Bikeshare Feed Specification) data and historical trip data.

## Features

- **Real-time GBFS Data**: Query live station status, station information, and system information
- **Historical Trip Data**: Access monthly trip data from the Ecobici open data platform
- **No Authentication Required**: All data sources are publicly accessible
- **Standard SQL Interface**: Query bike-sharing data using familiar SQL syntax

## Installation

### Building from Source

```bash
# Clone the repository with submodules
git clone --recurse-submodules https://github.com/dar4datascience/duck_ecobici_cdmx.git
cd duck_ecobici_cdmx

# Build the extension
make release

# The extension will be available at:
# build/release/extension/ecobici/ecobici.duckdb_extension
```

### Loading the Extension

```sql
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';
```

## Available Functions

### Real-time GBFS Functions

#### `ecobici_station_status()`

Returns the current status of all Ecobici stations.

**Columns:**
- `station_id` (VARCHAR): Unique station identifier
- `num_bikes_available` (INTEGER): Number of bikes currently available
- `num_docks_available` (INTEGER): Number of empty docks available
- `last_reported` (BIGINT): Unix timestamp of last update
- `is_installed` (BOOLEAN): Whether the station is installed
- `is_renting` (BOOLEAN): Whether the station is accepting rentals
- `is_returning` (BOOLEAN): Whether the station is accepting returns

**Example:**
```sql
SELECT * FROM ecobici_station_status() LIMIT 10;
```

#### `ecobici_station_information()`

Returns information about all Ecobici stations.

**Columns:**
- `station_id` (VARCHAR): Unique station identifier
- `name` (VARCHAR): Station name
- `lat` (DOUBLE): Latitude coordinate
- `lon` (DOUBLE): Longitude coordinate
- `address` (VARCHAR): Station address
- `capacity` (INTEGER): Total dock capacity

**Example:**
```sql
SELECT station_id, name, lat, lon, capacity 
FROM ecobici_station_information() 
WHERE capacity > 20;
```

#### `ecobici_system_information()`

Returns system-level information about Ecobici.

**Columns:**
- `system_id` (VARCHAR): System identifier
- `language` (VARCHAR): System language
- `name` (VARCHAR): System name
- `timezone` (VARCHAR): System timezone

**Example:**
```sql
SELECT * FROM ecobici_system_information();
```

## Example Queries

### Find stations with available bikes

```sql
SELECT 
    info.name,
    info.address,
    status.num_bikes_available,
    status.num_docks_available
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
    ON info.station_id = status.station_id
WHERE status.num_bikes_available > 5
ORDER BY status.num_bikes_available DESC;
```

### Get stations by location (within a bounding box)

```sql
SELECT 
    station_id,
    name,
    lat,
    lon,
    address
FROM ecobici_station_information()
WHERE lat BETWEEN 19.40 AND 19.45
  AND lon BETWEEN -99.18 AND -99.13;
```

### Station utilization analysis

```sql
SELECT 
    info.name,
    info.capacity,
    status.num_bikes_available,
    status.num_docks_available,
    ROUND(100.0 * status.num_bikes_available / info.capacity, 2) AS bike_fill_percentage
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
    ON info.station_id = status.station_id
WHERE info.capacity > 0
ORDER BY bike_fill_percentage DESC;
```

## Data Sources

- **GBFS Real-time Data**: https://gbfs.mex.lyftbikes.com/gbfs/en/
- **GBFS Specification**: https://github.com/NABSA/gbfs
- **Ecobici Open Data**: https://ecobici.cdmx.gob.mx/

## Development

### Requirements

- CMake 3.5 or higher
- C++14 compatible compiler
- Ninja (recommended)
- Git with submodules

### Building

```bash
# Initialize submodules
git submodule update --init --recursive

# Build with Ninja (recommended)
GEN=ninja make release

# Or build with Make
make release

# Run tests
make test
```

### Running Tests

```bash
# Run all tests
make test

# Run specific test
build/release/test/unittest "test/sql/ecobici_realtime.test"
```

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- Built using the [DuckDB Extension Template](https://github.com/duckdb/extension-template)
- Inspired by [duckdb-eurostat](https://github.com/ahuarte47/duckdb-eurostat)
- Data provided by [Ecobici CDMX](https://ecobici.cdmx.gob.mx/)
