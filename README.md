# Bible App CLI (C++20)

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Usage

```bash
./build/bible-app --help
./build/bible-app --list
./build/bible-app --validate
./build/bible-app --list --path /path/to/resources
./build/bible-app -v -p ./resources
./build/bible-app --book "1 Chronicles" --chapter 1
./build/bible-app --book "1 Chronicles" --chapter 1 --verse 5
./build/bible-app --book "1 Chronicles" --chapter 1 --verse "23, 27"
./build/bible-app --book "1 Chronicles" --chapter 1 --verse "23, 34-40"
./build/bible-app --version en-kjv --book "1 Chronicles" --chapter 1 --verse 5
```

- `-l, --list`: List versions with version and description.
- `-v, --validate`: Validate the resources directory structure.
- `-p, --path <dir>`: Override the resources root (default: `resources`, or from config file).
- `--book <name> --chapter <n> [--verse <spec>]`: Print requested verse(s) or entire chapter.
  - `--verse` is optional. If omitted, the entire chapter is printed.
  - Verse spec can be: single number (`5`), range (`34-40`), or comma-separated list (`23, 27, 34-40`).
- `--version <id>`: Select a specific version (default: `en-kjv`, or from config file).
- `--config <path>`: Path to config file (default: `~/.config/bible-app/config.ini` on Unix, `%APPDATA%/bible-app/config.ini` on Windows).
- `-h, --help`: Show help.

## Configuration File

The app supports an INI-format configuration file to store default settings. The config file is checked on each invocation for fast loading.

### Config File Location

- **Unix/Linux/macOS**: `~/.config/bible-app/config.ini` (or `$XDG_CONFIG_HOME/bible-app/config.ini` if set)
- **Windows**: `%APPDATA%/bible-app/config.ini`

### Config File Format

Create a config file with the following format:

```ini
# Bible App Configuration
# Resources directory path
resources_dir = /path/to/resources

# Default Bible version
default_version = en-kjv
```

### Config File Options

- `resources_dir` (or `resources_directory`, `resources`): Path to the resources directory containing Bible versions.
- `default_version` (or `version`, `default_bible_version`): Default Bible version to use when `--version` is not specified.

### Config vs CLI Arguments

Command-line arguments always override config file settings. If a config file doesn't exist or a setting is missing, the app falls back to defaults:
- Resources directory: `resources` (current directory)
- Default version: `en-kjv`

**Note**: If neither the config file nor CLI provides a resources directory, and the default `resources` directory doesn't exist, the app will require you to specify `--path`.

## Expected resources structure

```
resources/
  <translationId>/
    <translationId>.json
    books/
      <bookId>/
        chapters/
          1.json
          2.json
          ...
```

Validation ensures:
- `<translationId>.json` exists in each translation/version directory.
- `books/` exists, each book has `chapters/`.
- `chapters/` contains files named `^[1-9][0-9]*\.json$`, contiguous from 1..N.

## Exit codes
- 0: Success
- 1: CLI usage or unknown argument error
- 2: Validation failures
- 3: I/O error (e.g., resources path missing when listing)

