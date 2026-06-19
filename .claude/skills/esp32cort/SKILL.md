```markdown
# esp32cort Development Patterns

> Auto-generated skill from repository analysis

## Overview
This skill teaches the core development patterns and conventions used in the `esp32cort` TypeScript codebase. You'll learn how to structure files, write imports and exports, follow commit message conventions, and create and run tests. This guide is ideal for contributors looking to maintain consistency and quality in their work.

## Coding Conventions

### File Naming
- Use **camelCase** for all file names.
  - Example: `myModule.ts`, `userController.ts`

### Import Style
- Use **relative imports** for referencing other modules.
  - Example:
    ```typescript
    import { helperFunction } from './utils';
    ```

### Export Style
- Use **named exports** rather than default exports.
  - Example:
    ```typescript
    // In myModule.ts
    export function doSomething() { ... }

    // In another file
    import { doSomething } from './myModule';
    ```

### Commit Messages
- Commit types are **mixed**, but often use the `docs` prefix for documentation updates.
- Keep commit messages concise (average ~53 characters).
  - Example:
    ```
    docs: update README with setup instructions
    ```

## Workflows

### Documentation Update
**Trigger:** When updating or adding documentation files.
**Command:** `/update-docs`

1. Edit or add documentation files as needed.
2. Use the `docs` prefix in your commit message.
3. Commit your changes.
4. Push to the repository.

### Code Contribution
**Trigger:** When adding or modifying TypeScript code.
**Command:** `/contribute-code`

1. Create or update `.ts` files using camelCase naming.
2. Use relative imports and named exports.
3. Write clear, concise commit messages.
4. Push your changes to the repository.

### Testing
**Trigger:** When writing or updating tests for code.
**Command:** `/run-tests`

1. Create test files matching the `*.test.*` pattern (e.g., `myModule.test.ts`).
2. Write tests according to the project's testing style.
3. Run the tests using the project's preferred test runner (framework unknown; check project documentation or scripts).

## Testing Patterns

- Test files are named using the `*.test.*` pattern (e.g., `feature.test.ts`).
- The specific testing framework is **unknown**; check the repository for further details.
- Place tests alongside the code they verify or in a dedicated test directory, following the existing structure.

## Commands
| Command         | Purpose                                   |
|-----------------|-------------------------------------------|
| /update-docs    | Update or add documentation files         |
| /contribute-code| Add or modify TypeScript source code      |
| /run-tests      | Run the test suite on your changes        |
```