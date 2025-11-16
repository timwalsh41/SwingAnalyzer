---
applyTo: "**/*.cpp,**/*.h"
---
# Project general coding standards

## Design principles
- Keep it simple! Choose straightforward solutions over complex ones whenever possible. Simple solutions are easier to understand, maintain, and debug.
- Prioritize readability. Write code that is easy to read and understand, even if it means writing a few extra lines.
- Avoid building functionality on speculation. Implement features only when they are needed, not when you anticipate they might be useful in the future.
- Dependency Inversion: High-level modules should not depend on low-level modules. Both should depend on abstractions.
- Open/Closed Principle: Software entities should be open for extension but closed for modification.
- Single Responsibility: Each function, class, and module should have one clear purpose.
- Fail Fast: Check for potential errors early and implement appropriate error handling to avoid cascading failures.
- Consistent Style: Follow a consistent coding style throughout the project to enhance readability and maintainability

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