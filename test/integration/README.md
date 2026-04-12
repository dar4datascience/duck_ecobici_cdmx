# Integration Tests

This directory contains integration tests that verify the Ecobici extension works correctly with a real DuckDB installation.

## Smoke Test

The smoke test (`smoke_test.sh`) is a quick verification script that tests all major functionality of the extension.

### Prerequisites

1. **Build the extension**:
   ```bash
   make release
   ```

2. **Install DuckDB CLI** (one of the following):
   - Download from https://github.com/duckdb/duckdb/releases/tag/v1.4.4
   - Install via package manager:
     ```bash
     # macOS
     brew install duckdb
     
     # Linux (using wget)
     wget https://github.com/duckdb/duckdb/releases/download/v1.4.4/duckdb_cli-linux-amd64.zip
     unzip duckdb_cli-linux-amd64.zip
     ```

### Running the Smoke Test

```bash
# If duckdb is in PATH
./test/integration/smoke_test.sh

# Or specify path to duckdb
./test/integration/smoke_test.sh /path/to/duckdb
```

### What the Smoke Test Checks

1. ✅ Extension loading
2. ✅ `ecobici_station_status()` - Live station status
3. ✅ `ecobici_station_information()` - Station metadata
4. ✅ `ecobici_system_information()` - System info
5. ✅ JOIN operations between functions
6. ✅ `ecobici_historical_trips()` - Historical data (if available)
7. ✅ Error handling (invalid parameters)
8. ✅ Complex analytical queries

### Expected Output

```
=== Ecobici Extension Smoke Test ===
DuckDB: duckdb
Extension: build/release/extension/ecobici/ecobici.duckdb_extension

Test 1: Loading extension...
✓ Extension loaded successfully
Test 2: Testing ecobici_station_status()...
✓ Station status: 480 stations
Test 3: Testing ecobici_station_information()...
✓ Station information: 480 stations
Test 4: Testing ecobici_system_information()...
✓ System information: ecobici_mx
Test 5: Testing JOIN operations...
✓ JOIN operations: 480 joined rows
Test 6: Testing ecobici_historical_trips()...
✓ Historical trips function works
Test 7: Testing error handling...
✓ Error handling works (invalid year rejected)
Test 8: Testing complex analytical query...
✓ Complex analytical query works

=== All smoke tests passed! ===
```

## GitHub Actions Integration Tests

The integration tests also run automatically on GitHub Actions for every push and pull request.

See `.github/workflows/integration-test.yml` for the full test suite that runs on:
- ✅ Linux (Ubuntu)
- ✅ macOS
- ✅ Windows

### What GitHub Actions Tests

1. **Build the extension** on each platform
2. **Install DuckDB CLI** (v1.4.4)
3. **Load and test** all extension functions
4. **Run real-world queries**:
   - Find stations with available bikes
   - Calculate system utilization
   - Join real-time and historical data
5. **Test error handling**
6. **Create and persist** a test database
7. **Upload artifacts** for debugging

### Viewing Test Results

1. Go to the **Actions** tab in GitHub
2. Click on the latest workflow run
3. Expand the test steps to see detailed output
4. Download artifacts if needed

## Manual Testing

You can also test the extension manually:

```bash
# Build the extension
make release

# Start DuckDB
duckdb

# In DuckDB:
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';

-- Test real-time data
SELECT * FROM ecobici_station_status() LIMIT 10;

-- Test historical data
SELECT * FROM ecobici_historical_trips(2024, 1) LIMIT 10;

-- Test complex query
SELECT 
    info.name,
    status.num_bikes_available,
    status.num_docks_available
FROM ecobici_station_information() info
JOIN ecobici_station_status() status 
ON info.station_id = status.station_id
WHERE status.num_bikes_available > 5
ORDER BY status.num_bikes_available DESC
LIMIT 10;
```

## Troubleshooting

### Extension fails to load

**Error**: `IO Error: Extension ... could not be loaded`

**Solution**: Make sure you built the extension first:
```bash
make release
```

### Network errors

**Error**: `Failed to fetch GBFS feed: ... - Connection error`

**Possible causes**:
- No internet connection
- Ecobici API is down
- Firewall blocking HTTPS requests

**Solution**: Check your internet connection and try again.

### Historical data not found

**Error**: `Failed to fetch historical CSV for 2024-1 - File not found`

**Possible causes**:
- Data for that month doesn't exist yet
- URL pattern changed
- Ecobici website is down

**Solution**: Try a different month that you know has data, or check the Ecobici open data website.

## CI/CD Pipeline

The complete testing pipeline:

```
Push/PR → GitHub Actions
    ↓
    ├─→ Code Quality Check (format, tidy)
    │
    ├─→ Build Extension (Linux, macOS, Windows)
    │   └─→ Run unit tests (test/sql/*.test)
    │
    └─→ Integration Tests (NEW!)
        ├─→ Install DuckDB
        ├─→ Load extension
        ├─→ Test all functions
        ├─→ Run real queries
        └─→ Verify results
```

All tests must pass before merging to main branch.
