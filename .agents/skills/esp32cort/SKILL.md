```markdown
# esp32cort Development Patterns

> Auto-generated skill from repository analysis

## Overview
This skill teaches you the core development patterns and conventions used in the `esp32cort` TypeScript codebase. You will learn how to structure files, write imports and exports, and follow the project's coding style. Additionally, you'll discover how to identify and run tests, and leverage suggested commands for common workflows.

## Coding Conventions

### File Naming
- Use **camelCase** for all file names.
  - Example: `deviceManager.ts`, `wifiConfig.ts`

### Import Style
- Use **relative imports** for referencing other modules.
  - Example:
    ```typescript
    import { connectWifi } from './wifiConfig';
    ```

### Export Style
- Use **named exports** for all exported functions, classes, or constants.
  - Example:
    ```typescript
    // In deviceManager.ts
    export function initializeDevice() { ... }
    ```

### Commit Messages
- Freeform commit messages, typically averaging 61 characters in length.
  - Example: `fix wifi reconnect logic for unstable networks`

## Workflows

### Adding a New Module
**Trigger:** When you need to add a new feature or utility.
**Command:** `/add-module`

1. Create a new file using camelCase naming (e.g., `newFeature.ts`).
2. Implement your logic using TypeScript.
3. Use relative imports to include dependencies.
4. Export your functions or classes using named exports.
5. Add or update tests in a corresponding `*.test.*` file.

### Running Tests
**Trigger:** When you want to verify your code changes.
**Command:** `/run-tests`

1. Identify test files by the `*.test.*` pattern (e.g., `deviceManager.test.ts`).
2. Use the project's test runner (framework unknown; check project docs or package scripts).
3. Run all tests and ensure they pass before committing.

### Refactoring Code
**Trigger:** When improving or restructuring existing code.
**Command:** `/refactor`

1. Rename files using camelCase if needed.
2. Update relative imports to match new file paths.
3. Maintain named exports for all modules.
4. Update or add tests as necessary.

## Testing Patterns

- Test files follow the `*.test.*` naming convention (e.g., `wifiConfig.test.ts`).
- The specific testing framework is not detected; check the repository for further details.
- Place tests alongside or near the modules they test.

## Commands
| Command        | Purpose                                      |
|----------------|----------------------------------------------|
| /add-module    | Scaffold and implement a new module          |
| /run-tests     | Run all test files in the codebase           |
| /refactor      | Refactor code while following conventions    |
```