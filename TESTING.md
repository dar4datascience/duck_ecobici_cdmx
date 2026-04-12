# Testing Guide - Ecobici DuckDB Extension

Complete guide to testing the Ecobici extension at all levels.

## Quick Start

```bash
# 1. Build the extension
make release

# 2. Run all tests
make test

# 3. Run integration smoke test (requires DuckDB CLI)
./test/integration/smoke_test.sh
```

## Test Levels

### Level 1: Unit Tests (SQL Tests)
**Location**: `test/sql/*.test`  
**Run with**: `make test`  
**Purpose**: Test individual functions in isolation

**Test files**:
- `ecobici_realtime.test` - GBFS real-time functions
- `ecobici_historical.test` - Historical trip data functions

**Example**:
```sql
# Test station status
query I
SELECT COUNT(*) > 0 FROM ecobici_station_status();
----
true
```

### Level 2: Integration Tests (Smoke Test)
**Location**: `test/integration/smoke_test.sh`  
**Run with**: `./test/integration/smoke_test.sh`  
**Purpose**: Test extension with real DuckDB installation

**What it tests**:
1. Extension loading
2. All GBFS functions
3. Historical data functions
4. JOIN operations
5. Error handling
6. Complex queries

**Prerequisites**:
- DuckDB CLI installed
- Extension built (`make release`)
- Internet connection (for live data)

### Level 3: GitHub Actions CI/CD
**Location**: `.github/workflows/`  
**Runs**: Automatically on push/PR  
**Purpose**: Multi-platform testing and quality checks

**Workflows**:
1. **MainDistributionPipeline.yml**
   - Builds on Linux, macOS, Windows
   - Runs unit tests
   - Code quality checks

2. **integration-test.yml** (NEW!)
   - Installs DuckDB on each platform
   - Loads extension
   - Runs real queries
   - Tests database persistence

## Running Tests Locally

### Prerequisites

```bash
# Install DuckDB CLI (choose one):

# macOS
brew install duckdb

# Linux
wget https://github.com/duckdb/duckdb/releases/download/v1.4.4/duckdb_cli-linux-amd64.zip
unzip duckdb_cli-linux-amd64.zip

# Windows
# Download from https://github.com/duckdb/duckdb/releases/download/v1.4.4/duckdb_cli-windows-amd64.zip
```

### Build and Test

```bash
# 1. Build extension
make release

# 2. Run unit tests
make test

# 3. Run integration tests
./test/integration/smoke_test.sh

# 4. Manual testing
duckdb
> LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';
> SELECT * FROM ecobici_station_status() LIMIT 10;
```

## Test Coverage

### GBFS Real-time Functions ✅

| Function | Unit Test | Integration Test | Manual Test |
|----------|-----------|------------------|-------------|
| `ecobici_station_status()` | ✅ | ✅ | ✅ |
| `ecobici_station_information()` | ✅ | ✅ | ✅ |
| `ecobici_system_information()` | ✅ | ✅ | ✅ |

### Historical Data Functions ✅

| Function | Unit Test | Integration Test | Manual Test |
|----------|-----------|------------------|-------------|
| `ecobici_historical_trips(year, month)` | ✅ | ✅ | ✅ |

### Query Patterns ✅

| Pattern | Tested |
|---------|--------|
| Simple SELECT | ✅ |
| WHERE filtering | ✅ |
| JOIN operations | ✅ |
| GROUP BY aggregation | ✅ |
| ORDER BY sorting | ✅ |
| LIMIT pagination | ✅ |
| Complex analytics | ✅ |
| Error handling | ✅ |

## Test Scenarios

### Scenario 1: Find Available Bikes
```sql
SELECT 
    info.name,
    status.num_bikes_available
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
ON info.station_id = status.station_id
WHERE status.num_bikes_available > 5
ORDER BY status.num_bikes_available DESC
LIMIT 10;
```

### Scenario 2: Station Utilization
```sql
SELECT 
    name,
    capacity,
    num_bikes_available,
    ROUND(100.0 * num_bikes_available / capacity, 2) as utilization_pct
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
ON info.station_id = status.station_id
WHERE capacity > 0
ORDER BY utilization_pct DESC;
```

### Scenario 3: Historical Analysis
```sql
SELECT 
    Genero_Usuario,
    COUNT(*) as trip_count,
    AVG(Edad_Usuario) as avg_age
FROM ecobici_historical_trips(2024, 1)
WHERE Edad_Usuario IS NOT NULL
GROUP BY Genero_Usuario;
```

### Scenario 4: Geospatial Query
```sql
SELECT 
    name,
    lat,
    lon,
    num_bikes_available
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
ON info.station_id = status.station_id
WHERE lat BETWEEN 19.40 AND 19.45
  AND lon BETWEEN -99.18 AND -99.13;
```

## Error Handling Tests

### Invalid Parameters
```sql
-- Should fail: Invalid year
SELECT * FROM ecobici_historical_trips(1900, 1);
-- Error: Year must be between 2010 and 2100

-- Should fail: Invalid month
SELECT * FROM ecobici_historical_trips(2024, 13);
-- Error: Month must be between 1 and 12
```

### Network Errors
```sql
-- May fail if network is down
SELECT * FROM ecobici_station_status();
-- Error: Failed to fetch GBFS feed: station_status - Connection error
```

### Missing Data
```sql
-- May fail if data doesn't exist
SELECT * FROM ecobici_historical_trips(2025, 12);
-- Error: Failed to fetch historical CSV for 2025-12 - File not found
```

## Performance Testing

### Benchmark Queries

```sql
-- Measure query time
.timer on

-- Test 1: Simple select
SELECT COUNT(*) FROM ecobici_station_status();

-- Test 2: JOIN operation
SELECT COUNT(*) 
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
ON info.station_id = status.station_id;

-- Test 3: Complex aggregation
SELECT 
    Genero_Usuario,
    COUNT(*) as trips,
    AVG(Edad_Usuario) as avg_age
FROM ecobici_historical_trips(2024, 1)
GROUP BY Genero_Usuario;
```

### Expected Performance
- GBFS queries: < 2 seconds
- Historical CSV fetch: 5-15 seconds (depending on file size)
- JOIN operations: < 1 second (in-memory)

## Continuous Integration

### Workflow Triggers
```yaml
on:
  push:              # Every push
  pull_request:      # Every PR
  workflow_dispatch: # Manual trigger
```

### Test Matrix
```
Platform    | Build | Unit Tests | Integration Tests
------------|-------|------------|------------------
Linux       | ✅    | ✅         | ✅
macOS       | ✅    | ✅         | ✅
Windows     | ✅    | ✅         | ✅
```

### Viewing Results
1. Go to GitHub Actions tab
2. Click on workflow run
3. Expand job logs
4. Download artifacts (if needed)

## Troubleshooting

### Tests Fail Locally

**Problem**: `Extension could not be loaded`  
**Solution**: Build the extension first: `make release`

**Problem**: `DuckDB not found`  
**Solution**: Install DuckDB CLI or provide path to smoke_test.sh

**Problem**: `Connection error`  
**Solution**: Check internet connection, Ecobici API may be down

### Tests Pass Locally but Fail in CI

**Problem**: Format check fails  
**Solution**: Run `make format-fix` before committing

**Problem**: Different behavior on different platforms  
**Solution**: Check platform-specific code, test on all platforms

**Problem**: Network timeout in CI  
**Solution**: Increase timeout values in API client

### Historical Data Tests Fail

**Problem**: Data not available for test month  
**Solution**: Update test to use a month with known data

**Problem**: URL pattern changed  
**Solution**: Update FetchHistoricalCSV() in ecobici_api_client.cpp

## Adding New Tests

### Add a Unit Test

1. Create `test/sql/your_test.test`:
```sql
# name: test/sql/your_test.test
# description: Test your feature
# group: [ecobici]

require ecobici

statement ok
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

query I
SELECT COUNT(*) FROM your_function();
----
expected_result
```

2. Run: `make test`

### Add an Integration Test

1. Edit `test/integration/smoke_test.sh`
2. Add test case:
```bash
echo -e "${YELLOW}Test X: Testing your_feature...${NC}"
RESULT=$($DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT your_query();" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Your feature works${NC}"
else
    echo -e "${RED}✗ Your feature failed${NC}"
    exit 1
fi
```

3. Run: `./test/integration/smoke_test.sh`

### Add a CI Test

1. Edit `.github/workflows/integration-test.yml`
2. Add step to appropriate job:
```yaml
- name: Test your feature
  run: |
    ./duckdb << 'EOF'
    LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';
    SELECT your_query();
    EOF
```

## Best Practices

### Writing Tests
- ✅ Test one thing at a time
- ✅ Use descriptive names
- ✅ Include both positive and negative cases
- ✅ Test error handling
- ✅ Document expected behavior

### Test Data
- ✅ Use real Ecobici data when possible
- ✅ Handle missing/unavailable data gracefully
- ✅ Don't hardcode specific values that may change
- ✅ Test with various date ranges

### CI/CD
- ✅ Keep tests fast (< 5 minutes per platform)
- ✅ Make tests deterministic (no random failures)
- ✅ Clean up resources after tests
- ✅ Upload artifacts for debugging

## Resources

- **DuckDB Testing**: https://duckdb.org/dev/testing
- **SQL Test Format**: https://duckdb.org/dev/sqllogictest
- **GitHub Actions**: https://docs.github.com/en/actions
- **Ecobici API**: https://ecobici.cdmx.gob.mx/datos-abiertos/

## Summary

The Ecobici extension has comprehensive test coverage at three levels:

1. **Unit Tests**: Fast, isolated function tests
2. **Integration Tests**: Real DuckDB installation tests
3. **CI/CD**: Automated multi-platform testing

All tests run automatically on every push and pull request, ensuring the extension works correctly across all platforms and use cases.
