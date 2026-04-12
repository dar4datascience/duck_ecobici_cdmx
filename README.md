# DuckDB Ecobici Extension

A DuckDB extension for querying Ecobici (CDMX bike-sharing system) data directly from SQL. This extension provides access to both real-time GBFS (General Bikeshare Feed Specification) data and historical trip data.

## Features

- **Real-time GBFS Data**: Query live station status, station information, and system information
- **Historical Trip Data**: Access monthly trip data from the Ecobici open data platform
- **No Authentication Required**: All data sources are publicly accessible
- **Standard SQL Interface**: Query bike-sharing data using familiar SQL syntax

## Installation

### Option 1: Download Pre-built Extension (Recommended)

Download the extension binary from the [GitHub Releases](https://github.com/dar4datascience/duck_ecobici_cdmx/releases/latest) page.

```bash
# Download the latest release
wget https://github.com/dar4datascience/duck_ecobici_cdmx/releases/download/v0.1.0/ecobici.duckdb_extension

# Load in DuckDB (requires -unsigned flag for unsigned extensions)
duckdb -unsigned
> LOAD 'ecobici.duckdb_extension';
```

**Platform-specific downloads:**
- Linux: `ecobici.duckdb_extension-linux-amd64`
- macOS: `ecobici.duckdb_extension-osx-universal`
- Windows: `ecobici.duckdb_extension-windows-amd64`

### Option 2: Build from Source

```bash
# Clone the repository with submodules
git clone --recurse-submodules https://github.com/dar4datascience/duck_ecobici_cdmx.git
cd duck_ecobici_cdmx

# Install dependencies (Ubuntu/Debian)
sudo apt-get install -y libcpp-httplib-dev nlohmann-json3-dev libssl-dev

# Build the extension
make release

# The extension will be available at:
# build/release/extension/ecobici/ecobici.duckdb_extension
```

### Loading the Extension

```sql
-- Load the extension (requires -unsigned flag for unsigned extensions)
LOAD 'ecobici.duckdb_extension';

-- Or load with full path
LOAD '/path/to/ecobici.duckdb_extension';
```

**Important:** Since this extension is not signed, you must start DuckDB with the `-unsigned` flag:
```bash
duckdb -unsigned
```

### Option 3: Install from GitHub Release (Future)

Once the extension is published to the DuckDB community registry, you can install it directly:

```sql
INSTALL ecobici FROM 'https://github.com/dar4datascience/duck_ecobici_cdmx/releases';
LOAD ecobici;
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

### Historical Trip Data Functions

#### `ecobici_historical_trips(year, month)`

Fetches historical trip data for a specific month from the Ecobici open data platform.

**Parameters:**
- `year` (INTEGER): Year (2010-2100)
- `month` (INTEGER): Month (1-12)

**Columns:**
- `Genero_Usuario` (VARCHAR): User gender
- `Edad_Usuario` (INTEGER): User age
- `Bici` (VARCHAR): Bike ID
- `Ciclo_Estacion_Retiro` (VARCHAR): Start station name
- `Fecha_Retiro` (VARCHAR): Start date/time
- `Ciclo_Estacion_Arribo` (VARCHAR): End station name
- `Fecha_Arribo` (VARCHAR): End date/time

**Example:**
```sql
-- Get all trips from January 2024
SELECT * FROM ecobici_historical_trips(2024, 1) LIMIT 100;

-- Count trips by gender
SELECT Genero_Usuario, COUNT(*) as trip_count
FROM ecobici_historical_trips(2024, 1)
GROUP BY Genero_Usuario;

-- Find most popular routes
SELECT 
    Ciclo_Estacion_Retiro as start_station,
    Ciclo_Estacion_Arribo as end_station,
    COUNT(*) as trip_count
FROM ecobici_historical_trips(2024, 1)
GROUP BY start_station, end_station
ORDER BY trip_count DESC
LIMIT 10;
```

**Note:** Historical CSV data is available from 2023 onwards. The URL pattern follows:
`https://ecobici.cdmx.gob.mx/wp-content/uploads/{YYYY}/{MM}/{YYYY-MM}.csv`

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

### Historical trip analysis

```sql
-- Trip duration analysis by age group
SELECT 
    CASE 
        WHEN Edad_Usuario < 20 THEN 'Under 20'
        WHEN Edad_Usuario BETWEEN 20 AND 30 THEN '20-30'
        WHEN Edad_Usuario BETWEEN 31 AND 40 THEN '31-40'
        WHEN Edad_Usuario BETWEEN 41 AND 50 THEN '41-50'
        ELSE 'Over 50'
    END as age_group,
    COUNT(*) as trip_count,
    AVG(Edad_Usuario) as avg_age
FROM ecobici_historical_trips(2024, 1)
WHERE Edad_Usuario IS NOT NULL
GROUP BY age_group
ORDER BY trip_count DESC;
```

### Combining real-time and historical data

```sql
-- Find stations that are currently busy and were popular last month
WITH popular_stations AS (
    SELECT 
        Ciclo_Estacion_Retiro as station_name,
        COUNT(*) as historical_trips
    FROM ecobici_historical_trips(2024, 1)
    GROUP BY station_name
    ORDER BY historical_trips DESC
    LIMIT 20
)
SELECT 
    info.name,
    info.address,
    status.num_bikes_available,
    status.num_docks_available,
    ps.historical_trips
FROM ecobici_station_information() info
JOIN ecobici_station_status() status ON info.station_id = status.station_id
JOIN popular_stations ps ON info.name LIKE '%' || ps.station_name || '%'
WHERE status.num_bikes_available < 3
ORDER BY ps.historical_trips DESC;
```

## Data Sources

- **GBFS Real-time Data**: https://gbfs.mex.lyftbikes.com/gbfs/en/
  - Live station status, station information, system information
  - Updates in real-time following GBFS specification
- **GBFS Specification**: https://github.com/NABSA/gbfs
- **Ecobici Historical Data**: https://ecobici.cdmx.gob.mx/datos-abiertos/
  - Monthly CSV files with trip data
  - Available from 2023 onwards
  - Includes user demographics, trip times, and station information

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

### Integration Testing

We provide integration tests that verify the extension works with a real DuckDB installation:

```bash
# Quick smoke test (requires DuckDB CLI installed)
./test/integration/smoke_test.sh

# Or specify path to DuckDB
./test/integration/smoke_test.sh /path/to/duckdb
```

The smoke test verifies:
- ✅ Extension loading
- ✅ All GBFS functions (real-time data)
- ✅ Historical data functions
- ✅ JOIN operations
- ✅ Error handling
- ✅ Complex analytical queries

See `test/integration/README.md` for more details.

### Continuous Integration

All tests run automatically on GitHub Actions for every push and pull request:
- **Code Quality**: Format and tidy checks
- **Unit Tests**: SQL test files on all platforms
- **Integration Tests**: Real DuckDB installation tests on Linux, macOS, and Windows

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Publishing

See [PUBLISHING_GUIDE.md](PUBLISHING_GUIDE.md) for instructions on how to publish this extension to the DuckDB community extensions list.

## Acknowledgments

- Built using the [DuckDB Extension Template](https://github.com/duckdb/extension-template)
- Inspired by [duckdb-eurostat](https://github.com/ahuarte47/duckdb-eurostat)
- Data provided by [Ecobici CDMX](https://ecobici.cdmx.gob.mx/)
