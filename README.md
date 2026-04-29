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
# Using short book names:
./build/bible-app --book "1 Chr" --chapter 1 --verse 5
./build/bible-app --book Gen --chapter 1 --verse 1
./build/bible-app --book Matt --chapter 5 --verse "3-12"
# Using lowercase book names (full names or abbreviations):
./build/bible-app --book genesis --chapter 1 --verse 1
./build/bible-app --book "1 chronicles" --chapter 1 --verse 5
./build/bible-app --book gen --chapter 1 --verse 1
```

- `-l, --list`: List versions with version and description.
- `-v, --validate`: Validate the resources directory structure.
- `-p, --path <dir>`: Override the resources root (default: `resources`, or from config file).
- `--book <name> --chapter <n> [--verse <spec>]`: Print requested verse(s) or entire chapter.
  - `--verse` is optional. If omitted, the entire chapter is printed.
  - Verse spec can be: single number (`5`), range (`34-40`), or comma-separated list (`23, 27, 34-40`).
  - Book names support both full names (e.g., `"1 Chronicles"`) and common abbreviations (e.g., `"1 Chr"`, `"Gen"`, `"Matt"`). Case-insensitive: `"genesis"`, `"GENESIS"`, and `"Genesis"` all work. See examples below.
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

## Short Book Names

The app supports common Bible book abbreviations in addition to full names. Examples:

- **Old Testament**: `Gen` (Genesis), `Ex` (Exodus), `Lev` (Leviticus), `Num` (Numbers), `Deut` (Deuteronomy), `Josh` (Joshua), `Judg` (Judges), `1 Sam` or `1Sam` (1 Samuel), `2 Sam` or `2Sam` (2 Samuel), `1 Kgs` or `1Kgs` (1 Kings), `2 Kgs` or `2Kgs` (2 Kings), `1 Chr` or `1Chr` (1 Chronicles), `2 Chr` or `2Chr` (2 Chronicles), `Neh` (Nehemiah), `Esth` (Esther), `Ps` or `Pss` (Psalms), `Prov` (Proverbs), `Eccl` (Ecclesiastes), `Song` (Song of Solomon), `Isa` (Isaiah), `Jer` (Jeremiah), `Lam` (Lamentations), `Ezek` (Ezekiel), `Dan` (Daniel), `Hos` (Hosea), `Amos` (Amos), `Obad` (Obadiah), `Mic` (Micah), `Nah` (Nahum), `Hab` (Habakkuk), `Zeph` (Zephaniah), `Hag` (Haggai), `Zech` (Zechariah), `Mal` (Malachi)

- **New Testament**: `Matt` (Matthew), `Mark` (Mark), `Luke` (Luke), `John` (John), `Acts` (Acts), `Rom` (Romans), `1 Cor` or `1Cor` (1 Corinthians), `2 Cor` or `2Cor` (2 Corinthians), `Gal` (Galatians), `Eph` (Ephesians), `Phil` (Philippians), `Col` (Colossians), `1 Thess` or `1Thess` (1 Thessalonians), `2 Thess` or `2Thess` (2 Thessalonians), `1 Tim` or `1Tim` (1 Timothy), `2 Tim` or `2Tim` (2 Timothy), `Titus` (Titus), `Phlm` or `Philem` (Philemon), `Heb` (Hebrews), `Jas` (James), `1 Pet` or `1Pet` (1 Peter), `2 Pet` or `2Pet` (2 Peter), `1 John` or `1John` (1 John), `2 John` or `2John` (2 John), `3 John` or `3John` (3 John), `Jude` (Jude), `Rev` (Revelation)

Short names and full names are both case-insensitive. You can use `"genesis"`, `"GENESIS"`, `"Genesis"`, `"gen"`, `"GEN"`, or `"Gen"` - they all work. This applies to both the `--book` option and the `--read` option. If a short name is not recognized, the app will try to use it as a full name (also case-insensitively).

## Exit codes
- 0: Success
- 1: CLI usage or unknown argument error
- 2: Validation failures
- 3: I/O error (e.g., resources path missing when listing)

