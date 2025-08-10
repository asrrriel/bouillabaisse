# Bouillabaisse Styling Guide

## Naming Conventions
* **Use descriptive names with clear meanings.**

### Fixed Meaning Wordoids
#### Prefix class names based on their subsystem:
  * `au` for audio subsystem (e.g., `auDevice`, `auInputDevice`)
  * `ap` for application subsystem
  * `io` for input/output subsystem
  * `mi` for MIDI subsystem
#### Constants use uppercase letters to indicate meaning:
  * `D` = data (e.g. in `auDType`)
  * `S` = sample (e.g. in `auSFormat`)

### Classes

* Use **camelCase** for class names.

### Variables

* Use **snake\_case** consistently for variables.

### Function and Method Names

* Use **snake\_case** for function names(except.: constructors).
* Use the `get_` prefix for getters: `get_card_number()`, `get_dev_name()`.

## Formatting
* Use 4 spaces for indentation.
* Limit line length to 80-100 characters.
* Use braces on the same line for class and function definitions.
* Group related methods together.
* Use inline comments sparingly to explain non-obvious logic.
* Use `snprintf` carefully, with proper formatting strings.

## Header Files
* Header files must be located in `src/include`.
* Use `#pragma once` instead of header guards.

## Logging
* Use `spdlog` for logging.