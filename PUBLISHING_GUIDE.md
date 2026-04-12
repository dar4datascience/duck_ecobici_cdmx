# Publishing Guide: Getting Featured on DuckDB Community Extensions

## Overview

To have your extension featured on https://duckdb.org/community_extensions/list_of_extensions, you need to submit it to the official DuckDB community extensions repository.

## Step 1: Prepare Your Extension

### Requirements Checklist

✅ **Repository Structure**
- [ ] Clean repository with proper README.md
- [ ] LICENSE file (MIT recommended for DuckDB extensions)
- [ ] Working build system (CMakeLists.txt)
- [ ] Tests passing on all platforms (Linux, macOS, Windows)
- [ ] CI/CD configured (GitHub Actions)

✅ **Extension Metadata**
- [ ] Extension name: `ecobici`
- [ ] Extension version: `0.1.0` (semantic versioning)
- [ ] DuckDB version compatibility: `v1.4.4`
- [ ] Brief description of functionality
- [ ] Author/maintainer information

✅ **Documentation**
- [ ] Installation instructions (README.md)
- [ ] Usage examples
- [ ] API documentation
- [ ] Data source information
- [ ] Known limitations

### Sign Your Extension (Recommended)

DuckDB requires signed extensions for the official repository. To sign your extension:

1. **Request signing keys from DuckDB team**
   - Email: extensions@duckdb.org
   - Include: Your extension name, purpose, and GitHub repository
   - They will provide you with signing keys

2. **Sign the extension binary**
   ```bash
   # After building the extension
   # Use DuckDB's signing tool (provided by the DuckDB team)
   duckdb_sign build/release/extension/ecobici/ecobici.duckdb_extension
   ```

3. **Update CI to sign automatically**
   - Add signing step to GitHub Actions
   - Use secrets for signing keys

**Note:** For now, users can load your extension with `-unsigned` flag. Signing is required for official community repository.

## Step 2: Create GitHub Release

1. **Tag your release**
   ```bash
   git tag -a v0.1.0 -m "Initial release with GBFS and historical data support"
   git push origin v0.1.0
   ```

2. **Create GitHub Release**
   - Go to: https://github.com/dar4datascience/duck_ecobici_cdmx/releases/new
   - Tag: `v0.1.0`
   - Title: `v0.1.0 - Initial Release`
   - Description: Include release notes
   - Upload binaries:
     - `ecobici.duckdb_extension-linux-amd64`
     - `ecobici.duckdb_extension-osx-universal`
     - `ecobici.duckdb_extension-windows-amd64`

3. **Automate Release Creation** (Optional)
   - Add to `.github/workflows/`:
   ```yaml
   - name: Create Release
     uses: softprops/action-gh-release@v1
     with:
       files: |
         build/release/extension/ecobici/ecobici.duckdb_extension
       draft: false
       prerelease: false
   ```

## Step 3: Submit to DuckDB Community Extensions

### Repository: https://github.com/duckdb/community-extensions

### Required Files to Add

1. **`extensions/ecobici/extension.json`**
   ```json
   {
     "name": "ecobici",
     "description": "Query Ecobici CDMX bike-sharing data directly from SQL",
     "version": "0.1.0",
     "min_duckdb_version": "v1.4.4",
     "license": "MIT",
     "author": "dar4datascience",
     "repository": "https://github.com/dar4datascience/duck_ecobici_cdmx",
     "website": "https://github.com/dar4datascience/duck_ecobici_cdmx",
     "platform_specific": false
   }
   ```

2. **`extensions/ecobici/extension_version.json`**
   ```json
   {
     "version": "0.1.0",
     "duckdb_version": "v1.4.4",
     "extensions": [
       {
         "name": "ecobici",
         "platform": "linux_amd64",
         "url": "https://github.com/dar4datascience/duck_ecobici_cdmx/releases/download/v0.1.0/ecobici.duckdb_extension-linux-amd64",
         "sha256": "<hash>",
         "size": <size_in_bytes>
       },
       {
         "name": "ecobici",
         "platform": "osx_amd64",
         "url": "https://github.com/dar4datascience/duck_ecobici_cdmx/releases/download/v0.1.0/ecobici.duckdb_extension-osx-universal",
         "sha256": "<hash>",
         "size": <size_in_bytes>
       },
       {
         "name": "ecobici",
         "platform": "windows_amd64",
         "url": "https://github.com/dar4datascience/duck_ecobici_cdmx/releases/download/v0.1.0/ecobici.duckdb_extension-windows-amd64",
         "sha256": "<hash>",
         "size": <size_in_bytes>
       }
     ]
   }
   ```

3. **Calculate SHA256 hashes**
   ```bash
   sha256sum build/release/extension/ecobici/ecobici.duckdb_extension
   ```

### Submit Pull Request

1. **Fork the repository**
   ```bash
   # Fork https://github.com/duckdb/community-extensions
   git clone https://github.com/YOUR_USERNAME/community-extensions.git
   cd community-extensions
   ```

2. **Add your extension files**
   ```bash
   mkdir -p extensions/ecobici
   # Create extension.json and extension_version.json
   ```

3. **Submit PR**
   - Title: `Add ecobici extension`
   - Description: Brief description of your extension
   - Link to your repository
   - Mention which DuckDB version it supports

## Step 4: Review Process

### What the DuckDB team checks:
- ✅ Extension builds and loads correctly
- ✅ Tests pass on all platforms
- ✅ Documentation is complete
- ✅ License is compatible (MIT recommended)
- ✅ Extension is useful and well-maintained
- ✅ No security vulnerabilities

### Timeline:
- Review typically takes 1-2 weeks
- You may be asked for changes
- Once approved, it will appear on the community extensions list

## Step 5: After Approval

### Users can now install your extension with:
```sql
INSTALL ecobici;
LOAD ecobici;
```

### Monitor for issues:
- Respond to GitHub issues promptly
- Update extension for new DuckDB versions
- Maintain backward compatibility when possible
- Release updates regularly

## Alternative: Self-Hosted Installation

If you don't want to submit to the official repository, users can still install your extension:

```sql
INSTALL ecobici FROM 'https://github.com/dar4datascience/duck_ecobici_cdmx/releases';
LOAD ecobici;
```

This works without official approval but requires users to specify your repository URL.

## Current Status for Ecobici Extension

### ✅ Ready:
- Repository structure
- README with installation instructions
- Tests passing
- CI/CD configured
- GitHub releases can be created

### ⚠️ To Do:
- [ ] Sign extension (contact DuckDB team)
- [ ] Create GitHub Release v0.1.0
- [ ] Calculate SHA256 hashes
- [ ] Submit PR to duckdb/community-extensions
- [ ] Wait for review and approval

### 📝 Contact Information:
- DuckDB Extensions Email: extensions@duckdb.org
- Community Extensions Repo: https://github.com/duckdb/community-extensions
- Documentation: https://duckdb.org/docs/extensions/overview

## Quick Start Script

```bash
# 1. Tag and push release
git tag -a v0.1.0 -m "Initial release"
git push origin v0.1.0

# 2. Calculate hashes
sha256sum build/release/extension/ecobici/ecobici.duckdb_extension > hashes.txt

# 3. Create GitHub Release manually or via GitHub UI
# 4. Fork community-extensions repo
# 5. Add extension files
# 6. Submit PR
```

## Resources

- [DuckDB Extensions Documentation](https://duckdb.org/docs/extensions/overview)
- [DuckDB Community Extensions](https://github.com/duckdb/community-extensions)
- [Extension Template](https://github.com/duckdb/extension-template)
- [DuckDB Discord](https://discord.gg/duckdb) - Ask questions in #extensions channel
