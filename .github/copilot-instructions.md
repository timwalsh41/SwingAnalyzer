---
applyTo: "**/*.cpp,**/*.h"
---
# Project general coding standards

## Naming Conventions
- Use PascalCase for class names, interfaces, and type aliases
- Use camelCase for functions and methods
- Use snake_case for variables and parameters
- Class/struct member variables should be prefixed with m_
- Use ALL_CAPS for constants

## C++ coding style
- Use Allman style for braces
- Use spaces around operators and after commas
- For includes at the beginning of the file:
    - Use angle brackets for all headers
    - Start with project includes in alphabetical order
    - Next come library includes in alphabetical order
    - Finally, include standard library headers in alphabetical order
    - Leave a blank line between each section

## Other instructions
- Make sure to refer to a current version of the code, not just saved searches and prior context
- Any documentation, both code comments and markdowns, should reference the current implementation only, and not refer to changes or differences from prior state