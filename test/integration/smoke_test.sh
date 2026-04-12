#!/bin/bash
#
# Smoke test for Ecobici DuckDB Extension
# This script tests the extension with a real DuckDB installation
#
# Usage: ./test/integration/smoke_test.sh [path_to_duckdb]
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Find DuckDB executable
if [ -n "$1" ]; then
    DUCKDB="$1"
elif command -v duckdb &> /dev/null; then
    DUCKDB="duckdb"
else
    echo -e "${RED}Error: DuckDB not found. Please install DuckDB or provide path as argument.${NC}"
    echo "Download from: https://github.com/duckdb/duckdb/releases"
    exit 1
fi

# Find extension
EXTENSION_PATH="build/release/extension/ecobici/ecobici.duckdb_extension"
if [ ! -f "$EXTENSION_PATH" ]; then
    echo -e "${RED}Error: Extension not found at $EXTENSION_PATH${NC}"
    echo "Please build the extension first: make release"
    exit 1
fi

echo -e "${GREEN}=== Ecobici Extension Smoke Test ===${NC}"
echo "DuckDB: $DUCKDB"
echo "Extension: $EXTENSION_PATH"
echo ""

# Test 1: Load extension
echo -e "${YELLOW}Test 1: Loading extension...${NC}"
$DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT 'OK' as status;" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Extension loaded successfully${NC}"
else
    echo -e "${RED}✗ Failed to load extension${NC}"
    exit 1
fi

# Test 2: Station status
echo -e "${YELLOW}Test 2: Testing ecobici_station_status()...${NC}"
RESULT=$($DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT COUNT(*) FROM ecobici_station_status();" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Station status: $RESULT stations${NC}"
else
    echo -e "${RED}✗ Failed to fetch station status${NC}"
    echo "$RESULT"
    exit 1
fi

# Test 3: Station information
echo -e "${YELLOW}Test 3: Testing ecobici_station_information()...${NC}"
RESULT=$($DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT COUNT(*) FROM ecobici_station_information();" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Station information: $RESULT stations${NC}"
else
    echo -e "${RED}✗ Failed to fetch station information${NC}"
    exit 1
fi

# Test 4: System information
echo -e "${YELLOW}Test 4: Testing ecobici_system_information()...${NC}"
RESULT=$($DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT name FROM ecobici_system_information();" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ System information: $RESULT${NC}"
else
    echo -e "${RED}✗ Failed to fetch system information${NC}"
    exit 1
fi

# Test 5: JOIN operations
echo -e "${YELLOW}Test 5: Testing JOIN operations...${NC}"
RESULT=$($DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT COUNT(*) FROM ecobici_station_information() info JOIN ecobici_station_status() status ON info.station_id = status.station_id;" 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ JOIN operations: $RESULT joined rows${NC}"
else
    echo -e "${RED}✗ Failed JOIN operation${NC}"
    exit 1
fi

# Test 6: Historical data (may fail if data not available)
echo -e "${YELLOW}Test 6: Testing ecobici_historical_trips()...${NC}"
$DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT COUNT(*) FROM ecobici_historical_trips(2024, 1) LIMIT 1;" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Historical trips function works${NC}"
else
    echo -e "${YELLOW}⚠ Historical data not available (this is OK if data doesn't exist for 2024-01)${NC}"
fi

# Test 7: Error handling
echo -e "${YELLOW}Test 7: Testing error handling...${NC}"
$DUCKDB -c "LOAD '$EXTENSION_PATH'; SELECT * FROM ecobici_historical_trips(1900, 1);" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${GREEN}✓ Error handling works (invalid year rejected)${NC}"
else
    echo -e "${RED}✗ Error handling failed (should reject invalid year)${NC}"
    exit 1
fi

# Test 8: Complex query
echo -e "${YELLOW}Test 8: Testing complex analytical query...${NC}"
$DUCKDB << 'EOF' > /dev/null 2>&1
LOAD 'build/release/extension/ecobici/ecobici.duckdb_extension';
SELECT 
    info.name,
    status.num_bikes_available,
    status.num_docks_available,
    ROUND(100.0 * status.num_bikes_available / info.capacity, 2) as utilization
FROM ecobici_station_information() info
JOIN ecobici_station_status() status ON info.station_id = status.station_id
WHERE info.capacity > 0
ORDER BY utilization DESC
LIMIT 5;
EOF

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Complex analytical query works${NC}"
else
    echo -e "${RED}✗ Complex query failed${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}=== All smoke tests passed! ===${NC}"
echo ""
echo "The extension is working correctly. You can now use it with:"
echo "  $DUCKDB"
echo "  > LOAD '$EXTENSION_PATH';"
echo "  > SELECT * FROM ecobici_station_status() LIMIT 10;"
