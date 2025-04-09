# C++ Task Tracker CLI

Task Tracker is a simple command-line interface (CLI) application built entirely in C++ (using C++11 standards) to help you manage your tasks. It allows you to add new tasks, update existing ones, mark their progress (todo, in-progress, done), delete tasks, and list them based on their status.

This project serves as a practical exercise in C++ programming, covering:
*   Command-line argument parsing (`argc`, `argv`)
*   File I/O using standard C++ libraries (`<fstream>`)
*   Basic manual JSON parsing and serialization (without external libraries)
*   Object-Oriented Programming principles (using a `Task` class)
*   Working with standard containers (`std::vector`) and algorithms (`<algorithm>`)
*   Date and time handling (`<chrono>`)

## Features

*   **Add Tasks:** Add new tasks with a description.
*   **Update Tasks:** Modify the description of an existing task.
*   **Delete Tasks:** Remove tasks by their unique ID.
*   **Track Status:** Mark tasks as `todo`, `in-progress`, or `done`.
*   **List Tasks:**
    *   List all tasks.
    *   List tasks filtered by status (`todo`, `in-progress`, `done`).
*   **Data Persistence:** Tasks are saved to a `tasks.json` file in the current directory.
*   **No Dependencies:** Uses only standard C++ libraries.

## Prerequisites

*   A C++ compiler supporting C++11 or later (e.g., g++, Clang).
*   A command-line terminal or shell (Linux, macOS, Windows with MinGW/WSL/Developer Command Prompt).

## Building the Application

**To Compile the program:** Open your terminal, navigate to the directory containing `task-cli.cpp`, and run the command:

   ```bash
    g++ task-cli.cpp -o task-cli -std=c++20
   ```    
If successful, you will find an executable file named `task-cli` (or `task-cli.exe`) in the same directory.

## Usage

Run the application from your terminal using the compiled executable (`./task-cli` on Linux/macOS, `task-cli.exe` or `.\task-cli.exe` on Windows).

**Command Line Syntax:**

```bash
./task-cli <command> [arguments...]
```

### Available Commands

*   `add <"description">`
    *   Adds a new task with the given description.
    *   **Important:** Enclose the description in double quotes (`"`) if it contains spaces.
    *   *Example:* `./task-cli add "Buy groceries and cook dinner"`

*   `update <id> <"description">`
    *   Updates the description of the task with the specified `<id>`.
    *   **Important:** Enclose the new description in double quotes (`"`) if it contains spaces.
    *   *Example:* `./task-cli update 1 "Buy groceries only"`

*   `delete <id>`
    *   Deletes the task with the specified `<id>`.
    *   *Example:* `./task-cli delete 3`

*   `mark-in-progress <id>`
    *   Marks the task with the specified `<id>` as 'in-progress'.
    *   *Example:* `./task-cli mark-in-progress 2`

*   `mark-done <id>`
    *   Marks the task with the specified `<id>` as 'done'.
    *   *Example:* `./task-cli mark-done 1`

*   `mark-todo <id>`
    *   Marks the task with the specified `<id>` as 'todo'.
    *   *Example:* `./task-cli mark-todo 2`

*   `list [filter]`
    *   Lists tasks.
    *   If no `[filter]` is provided or `all` is used, lists all tasks.
    *   Optional `[filter]` can be one of: `todo`, `in-progress`, `done`.
    *   *Examples:*
        *   `./task-cli list` (Lists all tasks)
        *   `./task-cli list done` (Lists only completed tasks)
        *   `./task-cli list todo` (Lists only tasks yet to be started)

*   `help` or `--help`
    *   Displays the usage instructions and available commands.
    *   *Example:* `./task-cli help`

### Data Storage

*   Tasks are stored in a JSON file named `tasks.json`.
*   This file is created automatically in the **same directory where you run the `task-cli` executable** if it doesn't already exist.
*   The file contains a JSON array of task objects, each having `id`, `description`, `status`, `createdAt`, and `updatedAt` fields.

### Limitations

*   **Basic JSON Handling:** The JSON parsing and serialization are implemented manually without external libraries. This makes the handling less robust than using a dedicated library. It may fail if the `tasks.json` file is manually edited incorrectly or contains complex escaped characters not handled by the basic escaping/unescaping logic.
*   **Error Handling:** Basic error handling is implemented, but more complex edge cases might not be covered.
*   **Concurrency:** This application is not designed for simultaneous use by multiple processes or users modifying the same `tasks.json` file.

### Project Page URL

*  Project hosted at: [https://github.com/Mayokun-Sofowora/Task-Tracker-cpp](https://roadmap.sh/projects/task-tracker)
