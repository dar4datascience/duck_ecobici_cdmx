# GitHub Actions Workflows

This directory contains the CI/CD workflows for the Ecobici DuckDB extension.

## Workflows Overview

### 1. MainDistributionPipeline.yml
**Purpose**: Main build and distribution pipeline  
**Triggers**: Push, Pull Request, Manual  
**What it does**:
- Builds extension binaries for all platforms (Linux, macOS, Windows)
- Runs code quality checks (format, tidy)
- Executes unit tests from `test/sql/`
- Can publish releases (when configured)

**Jobs**:
- `duckdb-stable-build`: Builds extension using DuckDB v1.4.4
- `code-quality-check`: Runs format and tidy checks

### 2. integration-test.yml (NEW!)
**Purpose**: End-to-end integration testing with real DuckDB  
**Triggers**: Push to main/develop, Pull Request, Manual  
**What it does**:
- Installs DuckDB CLI on each platform
- Loads the extension in a real DuckDB instance
- Runs actual SQL queries against live Ecobici data
- Tests error handling and edge cases
- Creates and verifies database persistence

**Jobs**:
- `test-linux`: Integration tests on Ubuntu
- `test-macos`: Integration tests on macOS
- `test-windows`: Integration tests on Windows

**Tests performed**:
1. ✅ Extension loading
2. ✅ GBFS real-time functions (station_status, station_information, system_information)
3. ✅ Historical data function (historical_trips)
4. ✅ Data analysis queries (JOINs, aggregations, filtering)
5. ✅ Error handling (invalid parameters)
6. ✅ Database creation and persistence
7. ✅ View creation and querying

### 3. ExtensionTemplate.yml
**Purpose**: Template testing (for duckdb/extension-template repo)  
**Triggers**: Only when `RUN_RENAME_TEST` is enabled  
**Status**: Not used in production (template testing only)

## Workflow Execution Flow

```
┌─────────────────────────────────────────────────────────┐
│                    Push / Pull Request                   │
└────────────────────┬────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
        ▼                         ▼
┌───────────────┐         ┌──────────────────┐
│ Main Pipeline │         │ Integration Test │
└───────┬───────┘         └────────┬─────────┘
        │                          │
        ├─→ Code Quality           ├─→ Build Extension
        │   ├─ Format              │
        │   └─ Tidy                ├─→ Install DuckDB
        │                          │
        ├─→ Build Binaries         ├─→ Load Extension
        │   ├─ Linux               │
        │   ├─ macOS               ├─→ Test Functions
        │   └─ Windows             │   ├─ GBFS Functions
        │                          │   ├─ Historical Data
        ├─→ Run Unit Tests         │   └─ JOINs & Analytics
        │   └─ test/sql/*.test     │
        │                          ├─→ Error Handling
        └─→ Upload Artifacts       │
                                   └─→ Database Tests
```

## Viewing Workflow Results

### In GitHub UI
1. Go to the **Actions** tab
2. Select a workflow run
3. Click on a job to see detailed logs
4. Download artifacts if available

### Status Badges
Add to README.md:
```markdown
![Build Status](https://github.com/dar4datascience/duck_ecobici_cdmx/actions/workflows/MainDistributionPipeline.yml/badge.svg)
![Integration Tests](https://github.com/dar4datascience/duck_ecobici_cdmx/actions/workflows/integration-test.yml/badge.svg)
```

## Local Testing Before Push

To avoid CI failures, run these locally before pushing:

```bash
# 1. Format code
make format-fix

# 2. Build extension
make release

# 3. Run unit tests
make test

# 4. Run integration tests (requires DuckDB CLI)
./test/integration/smoke_test.sh
```

## Troubleshooting CI Failures

### Format Check Failed
```bash
# Fix locally
make format-fix
git add .
git commit -m "Fix formatting"
git push
```

### Tidy Check Failed
- Review the tidy errors in the workflow log
- Fix code issues (usually unused variables, potential bugs)
- Commit and push

### Build Failed
- Check the build log for compilation errors
- Verify dependencies in `vcpkg.json`
- Test locally: `make release`

### Integration Test Failed
- Check which specific test failed
- Test locally with: `./test/integration/smoke_test.sh`
- Common issues:
  - Network connectivity (Ecobici API down)
  - Data not available for test month
  - Extension not built correctly

### Unit Test Failed
- Run the specific test locally:
  ```bash
  build/release/test/unittest "test/sql/failing_test.test"
  ```
- Fix the issue
- Commit and push

## Adding New Tests

### Add a Unit Test
1. Create file in `test/sql/your_test.test`
2. Follow DuckDB test format
3. Tests run automatically in MainDistributionPipeline

### Add an Integration Test
1. Edit `.github/workflows/integration-test.yml`
2. Add test steps to existing jobs
3. Test locally first with smoke_test.sh

### Add a New Workflow
1. Create `.github/workflows/your_workflow.yml`
2. Define triggers, jobs, and steps
3. Test with `workflow_dispatch` trigger first

## Workflow Configuration

### Concurrency Control
Both workflows use concurrency groups to cancel old runs when new commits are pushed:
```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}-...
  cancel-in-progress: true
```

### DuckDB Version
Currently pinned to **v1.4.4**:
```yaml
duckdb_version: v1.4.4
ci_tools_version: v1.4.4
```

To update, change in both workflows.

### Platform Matrix
Integration tests run on:
- **Linux**: `ubuntu-latest`
- **macOS**: `macos-latest`
- **Windows**: `windows-latest`

## Artifacts

### MainDistributionPipeline
- Extension binaries for all platforms
- Test results

### Integration Tests
- `integration-test-artifacts-linux`: Test database and extension
- Available for 90 days after workflow run

## Security

### Secrets
No secrets required - all data sources are public.

### Permissions
Workflows have default permissions:
- Read repository
- Write to Actions (for artifacts)

## Performance

### Typical Run Times
- **MainDistributionPipeline**: 15-25 minutes
- **Integration Tests**: 5-10 minutes per platform
- **Total**: ~30-40 minutes for all checks

### Optimization Tips
- Use caching for vcpkg dependencies
- Parallel job execution
- Skip redundant tests when possible

## Questions?

See the main README.md or open an issue on GitHub.
