# Documentation Guide

## General Rules

Use Doxygen block comments:

/**
 * @brief One-line summary.
 *
 * Detailed explanation describing intent and behavior.
 *
 * @param name Description.
 * @return Description.
 *
 * @par Thread Safety
 * Statement.
 *
 * @par Complexity
 * Big-O description.
 *
 * @code{.cpp}
 * Example usage.
 * @endcode
 */

Document:
- classes
- structs
- enums
- templates
- public methods

## Functions

Must include:
- @param
- @return
- @throws (if applicable)

Explain:
- ownership semantics
- lifetime rules
- performance characteristics

## Examples

Provide usage examples:

@code{.cpp}
Example example;
example.run();
@endcode

Examples must be minimal and realistic.

## Avoid
- repeating function names
- obvious comments
- documenting trivial getters

