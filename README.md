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
./build/bible-app --book "1 Chronicles" --chapter 1 --verse 5
./build/bible-app --version en-kjv --book "1 Chronicles" --chapter 1 --verse 5
```

- `-l, --list`: List versions with version and description.
- `-v, --validate`: Validate the resources directory structure.
- `-p, --path <dir>`: Override the resources root (default: `resources`).
- `--book <name> --chapter <n> --verse <n>`: Print requested verse text.
- `--version <id>`: Select a specific version (default: `en-kjv`).
- `-h, --help`: Show help.

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

