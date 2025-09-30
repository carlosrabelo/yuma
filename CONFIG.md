# Configuration Management

This project uses a template-based configuration system to avoid committing auto-generated files.

## How it works

- **Template files** (committed to git):
  - `platformio.ini.template` - PlatformIO configuration template
  - `Makefile.template` - Makefile template

- **Generated files** (ignored by git):
  - `platformio.ini` - Auto-generated from template
  - `Makefile` - Auto-generated from template

## Commands

### Auto-detect and configure
```bash
make detect
```
This will:
1. Create `platformio.ini` and `Makefile` from templates (if they don't exist)
2. Detect connected ESP32/ESP8266 boards
3. Update the generated files with correct ports and board types

### Manual setup
If you need to manually create the config files:
```bash
cp platformio.ini.template platformio.ini
cp Makefile.template Makefile
```

### Clean auto-generated files
```bash
rm platformio.ini Makefile *.bak
```

## Benefits

- ✅ No auto-generated config files in git commits
- ✅ Template files preserve the base configuration
- ✅ Each developer can have their own local port settings
- ✅ `make detect` creates and configures files automatically