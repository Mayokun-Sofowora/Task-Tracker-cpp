// Project:
// Task tracker is a project used to track and manage your tasks. In this task, you will build a simple command line interface (CLI) to track what you need to do, what you have done, and what you are currently working on. This project will help you practice your programming skills, including working with the filesystem, handling user inputs, and building a simple CLI application.

// Requirements:
// The application should run from the command line, accept user actions and inputs as arguments, and store the tasks in a JSON file. The user should be able to:
// Add, Update, and Delete tasks
// Mark a task as in progress or done
// List all tasks
// List all tasks that are done
// List all tasks that are not done
// List all tasks that are in progress

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <format>
#include <ranges>

// --- Constants ---
const std::string TASKS_FILE = "tasks.json";

// --- Forward Declarations ---
class Task; // Forward declare Task class
std::vector<Task> loadTasks();
void saveTasks(const std::vector<Task> &tasks);
std::string getCurrentTimestamp();
std::string escapeJsonString(const std::string &input);
std::string unescapeJsonString(const std::string &input);
std::string findJsonValue(const std::string &objectStr, const std::string &key);
int getNextId(const std::vector<Task> &tasks);
void printUsage();

// --- Task Class Definition ---
class Task
{
private:
    int id;
    std::string description;
    std::string status; // "todo", "in-progress", "done"
    std::string createdAt;
    std::string updatedAt;

    // Private helper to update the timestamp
    void updateTimestamp()
    {
        updatedAt = getCurrentTimestamp();
    }

public:
    // Constructor for creating new tasks programmatically
    Task(int taskId, const std::string &taskDescription) : id(taskId),
                                                           description(taskDescription),
                                                           status("todo") // New tasks default to 'todo'
    {
        createdAt = getCurrentTimestamp();
        updatedAt = createdAt; // Initially the same
    }

    // Default constructor: Needed for creating Task objects before populating from file
    Task() : id(0), status("todo") {}

    ~Task() {} // Destructor (not strictly necessary here, but good practice)

    // --- Getters (provide read access) ---
    int getID() const { return id; }
    const std::string &getDescription() const { return description; }
    const std::string &getStatus() const { return status; }
    const std::string &getCreatedAt() const { return createdAt; }
    const std::string &getUpdatedAt() const { return updatedAt; }

    // --- Setters (provide controlled write access) ---

    // Sets description and updates the timestamp
    void setDescription(const std::string &newDescription)
    {
        description = newDescription;
        updateTimestamp();
    }

    // Sets status (with validation) and updates the timestamp
    void setStatus(const std::string &newStatus)
    {
        if (newStatus == "todo" || newStatus == "in-progress" || newStatus == "done")
        {
            status = newStatus;
            updateTimestamp();
        }
        else
        {
            // Keep original status or handle error differently if needed
            std::cerr << "Warning: Invalid status '" << newStatus << "' for task " << id
                      << ". Status must be 'todo', 'in-progress', or 'done'. Status not changed." << std::endl;
        }
    }

    // Grant `loadTasks` direct access to private members.
    // This avoids needing public 'internalSet' methods just for loading.
    friend std::vector<Task> loadTasks();
};

// --- Helper Functions ---

// Get current timestamp as string
std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Basic JSON string escaping
std::string escapeJsonString(const std::string &input)
{
    std::string output;
    output.reserve(input.length());
    for (char c : input)
    {
        switch (c)
        {
        case '"':
            output += "\\\"";
            break;
        case '\\':
            output += "\\\\";
            break;
        // Add more escapes if needed (e.g., \n, \r, \t, \b, \f)
        // case '\n': output += "\\n"; break;
        // case '\r': output += "\\r"; break;
        // case '\t': output += "\\t"; break;
        // case '\b': output += "\\b"; break;
        // case '\f': output += "\\f"; break;
        default:
            output += c;
            break;
        }
    }
    return output;
}

// Basic JSON string unescaping
std::string unescapeJsonString(const std::string &input)
{
    std::string output;
    output.reserve(input.length());
    bool escaped = false;
    for (char c : input)
    {
        if (escaped)
        {
            switch (c)
            {
            case '"':
                output += '"';
                break;
            case '\\':
                output += '\\';
                break;
            // Add more unescapes corresponding to escapes above
            // case 'n': output += '\n'; break;
            // case 'r': output += '\r'; break;
            // case 't': output += '\t'; break;
            // case 'b': output += '\b'; break;
            // case 'f': output += '\f'; break;
            default:
                output += '\\';
                output += c;
                break; // Keep unrecognized escapes
            }
            escaped = false;
        }
        else
        {
            if (c == '\\')
            {
                escaped = true;
            }
            else
            {
                output += c;
            }
        }
    }
    return output;
}

// Helper to find the value associated with a key in a JSON object string segment
std::string findJsonValue(const std::string &objectStr, const std::string &key)
{
    std::string keyPattern = "\"" + key + "\":";
    size_t keyPos = objectStr.find(keyPattern);
    if (keyPos == std::string::npos)
    {
        return ""; // Key not found
    }

    size_t valueStart = keyPos + keyPattern.length();

    // Skip whitespace
    while (valueStart < objectStr.length() && std::isspace(static_cast<unsigned char>(objectStr[valueStart])))
    {
        valueStart++;
    }

    if (valueStart >= objectStr.length())
        return ""; // No value found

    if (objectStr[valueStart] == '"')
    { // String value
        size_t valueEnd = valueStart + 1;
        bool inEscape = false;
        while (valueEnd < objectStr.length())
        {
            if (inEscape)
            {
                inEscape = false; // Consume the escaped character
            }
            else if (objectStr[valueEnd] == '\\')
            {
                inEscape = true;
            }
            else if (objectStr[valueEnd] == '"')
            {
                // Found the closing quote
                return unescapeJsonString(objectStr.substr(valueStart + 1, valueEnd - valueStart - 1));
            }
            valueEnd++;
        }
        // If loop finishes without finding closing quote, it's malformed
        std::cerr << "Warning: Malformed JSON string value found for key '" << key << "'" << std::endl;
        return ""; // Malformed string
    }
    else
    { // Assume number (or potentially bool/null, but we only need numbers here)
        size_t valueEnd = valueStart;
        // Find the end based on comma or closing brace
        size_t commaPos = objectStr.find(',', valueStart);
        size_t bracePos = objectStr.find('}', valueStart);

        // Determine the actual end position
        if (commaPos != std::string::npos && bracePos != std::string::npos)
        {
            valueEnd = std::min(commaPos, bracePos);
        }
        else if (commaPos != std::string::npos)
        {
            valueEnd = commaPos;
        }
        else if (bracePos != std::string::npos)
        {
            valueEnd = bracePos;
        }
        else
        {
            // Should not happen in valid JSON within an object, but handle defensively
            valueEnd = objectStr.length();
        }

        // Extract the potential number part
        std::string numStr = objectStr.substr(valueStart, valueEnd - valueStart);

        // Trim trailing whitespace from the extracted part
        size_t lastChar = numStr.find_last_not_of(" \t\n\r\f\v");
        if (lastChar == std::string::npos)
        {
            return ""; // Empty value
        }
        numStr = numStr.substr(0, lastChar + 1);

        // Basic validation: check if it looks like a number (digits, optionally negative)
        bool is_num = !numStr.empty();
        for (size_t i = 0; i < numStr.length(); ++i)
        {
            if (i == 0 && numStr[i] == '-')
                continue; // Allow leading minus
            if (!std::isdigit(static_cast<unsigned char>(numStr[i])))
            {
                is_num = false;
                break;
            }
        }
        if (is_num)
        {
            return numStr;
        }
        else
        {
            std::cerr << "Warning: Non-numeric value found for numeric key '" << key << "': " << numStr << std::endl;
            return "";
        }
    }
}

// --- JSON Loading (using friend access) ---
std::vector<Task> loadTasks()
{
    std::vector<Task> tasks;
    std::ifstream file(TASKS_FILE);

    if (!file.is_open())
    {
        // File doesn't exist is not an error, just means no tasks yet.
        return tasks;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Basic check for empty or just whitespace content
    if (content.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
    {
        return tasks;
    }

    // Trim leading/trailing whitespace just in case
    content.erase(0, content.find_first_not_of(" \t\n\r\f\v"));
    content.erase(content.find_last_not_of(" \t\n\r\f\v") + 1);

    if (content.empty() || content == "[]")
    {
        return tasks; // Empty file or empty JSON array
    }

    // Very basic array parsing: find '[' and ']'
    size_t startPos = content.find('[');
    size_t endPos = content.rfind(']');
    if (startPos == std::string::npos || endPos == std::string::npos || startPos >= endPos)
    {
        std::cerr << "Error: Invalid JSON format in " << TASKS_FILE << " (missing or misplaced array brackets)." << std::endl;
        return tasks; // Return empty on major format error
    }

    size_t currentPos = startPos + 1;
    while (currentPos < endPos)
    {
        size_t objStart = content.find('{', currentPos);
        if (objStart == std::string::npos || objStart >= endPos)
            break; // No more objects

        size_t objEnd = content.find('}', objStart + 1);
        // Basic brace balancing check (doesn't handle nested objects)
        size_t nextObjStart = content.find('{', objStart + 1);
        if (objEnd == std::string::npos || (nextObjStart != std::string::npos && objEnd > nextObjStart) || objEnd >= endPos)
        {
            std::cerr << "Error: Invalid JSON format in " << TASKS_FILE << " (mismatched or nested braces detected by simple check)." << std::endl;
            // Attempt to recover might be complex, safer to stop parsing here
            break;
        }

        std::string objectStr = content.substr(objStart + 1, objEnd - objStart - 1);

        Task task; // Create default task object
        bool taskValid = true;
        try
        {
            std::string idStr = findJsonValue(objectStr, "id");
            std::string descStr = findJsonValue(objectStr, "description");
            std::string statusStr = findJsonValue(objectStr, "status");
            std::string createdStr = findJsonValue(objectStr, "createdAt");
            std::string updatedStr = findJsonValue(objectStr, "updatedAt");

            // Basic validation of extracted values
            if (idStr.empty())
            {
                std::cerr << "Warning: Skipping task due to missing or invalid ID." << std::endl;
                taskValid = false;
            }
            else
            {
                task.id = std::stoi(idStr); // Use friend access
            }

            if (descStr.empty())
            {
                std::cerr << "Warning: Skipping task ID " << task.id << " due to missing description." << std::endl;
                taskValid = false;
            }
            else
            {
                task.description = descStr; // Use friend access
            }

            if (statusStr.empty() || (statusStr != "todo" && statusStr != "in-progress" && statusStr != "done"))
            {
                std::cerr << "Warning: Skipping task ID " << task.id << " due to missing or invalid status: '" << statusStr << "'" << std::endl;
                taskValid = false;
            }
            else
            {
                task.status = statusStr; // Use friend access
            }

            if (createdStr.empty())
            {
                std::cerr << "Warning: Skipping task ID " << task.id << " due to missing createdAt." << std::endl;
                taskValid = false;
            }
            else
            {
                task.createdAt = createdStr; // Use friend access
            }

            if (updatedStr.empty())
            {
                std::cerr << "Warning: Skipping task ID " << task.id << " due to missing updatedAt." << std::endl;
                taskValid = false;
            }
            else
            {
                task.updatedAt = updatedStr; // Use friend access
            }

            if (taskValid)
            {
                tasks.push_back(task); // Add valid task to vector
            }
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Error parsing ID field as integer: " << e.what() << ". Skipping task fragment." << std::endl;
            taskValid = false; // Ensure partially filled task isn't added
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Error parsing ID field (out of range): " << e.what() << ". Skipping task fragment." << std::endl;
            taskValid = false; // Ensure partially filled task isn't added
        }

        currentPos = objEnd + 1; // Move past the parsed object
    }

    return tasks;
}

// --- JSON Saving (using getters) ---
void saveTasks(const std::vector<Task> &tasks)
{
    std::ofstream file(TASKS_FILE);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open " << TASKS_FILE << " for writing." << std::endl;
        // Consider throwing an exception or returning a bool status
        return;
    }

    file << "[\n";
    for (size_t i = 0; i < tasks.size(); ++i)
    {
        const auto &task = tasks[i];
        file << "  {\n";
        // Use getter methods to access task data
        file << "    \"id\": " << task.getID() << ",\n";
        file << "    \"description\": \"" << escapeJsonString(task.getDescription()) << "\",\n";
        file << "    \"status\": \"" << escapeJsonString(task.getStatus()) << "\",\n";
        file << "    \"createdAt\": \"" << escapeJsonString(task.getCreatedAt()) << "\",\n";
        file << "    \"updatedAt\": \"" << escapeJsonString(task.getUpdatedAt()) << "\"\n";
        file << "  }";
        if (i < tasks.size() - 1)
        {
            file << ",\n"; // Comma between objects
        }
        else
        {
            file << "\n"; // No comma after the last object
        }
    }
    file << "]\n";
    file.close();
    if (!file.good())
    { // Check if any write errors occurred
        std::cerr << "Error: An error occurred while writing to " << TASKS_FILE << "." << std::endl;
    }
}

// --- Task Management Logic (using Task class methods and C++20 features) ---
int getNextId(const std::vector<Task> &tasks)
{
    if (tasks.empty())
    {
        return 1;
    }
    auto max_it = std::ranges::max_element(tasks, {}, &Task::getID);
    // It's guaranteed to find an element if tasks is not empty
    int maxId = max_it->getID(); // Dereference iterator to get Task, then get ID

    if (maxId >= std::numeric_limits<int>::max())
    {
        throw std::overflow_error("Cannot generate new task ID, maximum integer value reached.");
    }
    return maxId + 1;
}

void addTask(std::vector<Task> &tasks, const std::string &description)
{
    if (description.empty())
    {
        std::cerr << "Error: Task description cannot be empty." << std::endl;
        return;
    }
    try
    {
        int newId = getNextId(tasks);
        // Use the Task constructor that sets timestamps etc.
        Task newTask(newId, description);
        tasks.push_back(newTask);
        saveTasks(tasks);
        std::cout << "Task added successfully (ID: " << newId << ")" << std::endl;
    }
    catch (const std::overflow_error &e)
    {
        std::cerr << "Error adding task: " << e.what() << std::endl;
    }
}

void updateTask(std::vector<Task> &tasks, int id, const std::string &newDescription)
{
    if (newDescription.empty())
    {
        std::cerr << "Error: New task description cannot be empty." << std::endl;
        return;
    }
    auto it = std::ranges::find_if(tasks, [id](const Task &task)
                                   { return task.getID() == id; });

    if (it != tasks.end())
    {                                       // Check if the iterator is valid (task found)
        it->setDescription(newDescription); // Use setter via iterator (setter updates timestamp)
        saveTasks(tasks);
        std::cout << "Task " << id << " updated successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error: Task with ID " << id << " not found for update." << std::endl;
    }
}

void deleteTask(std::vector<Task> &tasks, int id)
{
    size_t numRemoved = std::erase_if(tasks, [id](const Task &task)
                                      { return task.getID() == id; }); // Use getter in lambda

    if (numRemoved > 0)
    { // Check if any elements were actually removed
        saveTasks(tasks);
        std::cout << "Task " << id << " deleted successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error: Task with ID " << id << " not found for deletion." << std::endl;
    }
}

void markTaskStatus(std::vector<Task> &tasks, int id, const std::string &status)
{
    // Basic validation of the status string remains useful
    if (status != "todo" && status != "in-progress" && status != "done")
    {
        std::cerr << "Error: Invalid status '" << status << "'. Use 'todo', 'in-progress', or 'done'." << std::endl;
        return;
    }
    auto it = std::ranges::find_if(tasks, [id](const Task &task)
                                   { return task.getID() == id; });

    if (it != tasks.end())
    {                          // Check if found
        it->setStatus(status); // Use setter via iterator (setter validates & handles timestamp)
        saveTasks(tasks);
        // The setStatus method now prints warnings, so a simple notification is sufficient
        std::cout << "Task " << id << " status updated." << std::endl; // Message adjusted slightly
    }
    else
    {
        std::cerr << "Error: Task with ID " << id << " not found to mark status." << std::endl;
    }
}

void listTasks(const std::vector<Task> &tasks, const std::string &filter = "all")
{
    std::cout << "\n--- Tasks";
    if (filter != "all")
    {
        std::cout << " (Status: " << filter << ")";
    }
    std::cout << " ---" << std::endl;

    bool tasksDisplayed = false;
    // Looping can stay simple, or could use std::views::filter etc.
    // Sticking to simple loop for clarity comparison with original.
    for (const auto &task : tasks)
    {
        // Use getter for filtering
        bool matchFilter = (filter == "all" || task.getStatus() == filter);
        if (matchFilter)
        {
            tasksDisplayed = true;
            // Use getters for display - using std::format for cleaner output
            std::cout << std::format(
                "ID: {}\n"
                "  Description: {}\n"
                "  Status: {}\n"
                "  Created: {}\n"
                "  Updated: {}\n"
                "-------------\n",
                task.getID(),
                task.getDescription(),
                task.getStatus(),
                task.getCreatedAt(),
                task.getUpdatedAt());
        }
    }

    if (!tasksDisplayed)
    {
        if (filter == "all")
        {
            std::cout << "No tasks found." << std::endl;
        }
        else
        {
            std::cout << "No tasks found with status '" << filter << "'." << std::endl;
        }
        std::cout << "-------------" << std::endl;
    }
}

void printUsage()
{
    // Using std::format with a raw string literal for easier multiline formatting
    std::cout << std::format(R"(
Usage: task-cli <command> [options]

Commands:
  add <"description">        Add a new task (use quotes for descriptions with spaces)
  update <id> <"description">  Update task description (use quotes)
  delete <id>                Delete a task by ID
  mark-in-progress <id>    Mark task as 'in-progress'
  mark-done <id>             Mark task as 'done'
  mark-todo <id>             Mark task as 'todo'
  list [all|todo|in-progress|done]  List tasks (default: all)
  help                       Show this help message

Example:
  ./task-cli add "Submit project report"
  ./task-cli list todo
  ./task-cli mark-in-progress 1

Note: Task descriptions containing spaces must be enclosed in double quotes.
)");
}

// --- Main Application Logic ---
int main(int argc, char *argv[])
{
    // Check for help command or insufficient arguments
    if (argc < 2 || std::string(argv[1]) == "help" || std::string(argv[1]) == "--help")
    {
        printUsage();
        return (argc < 2); // Return 1 if no command given, 0 if 'help' was explicitly asked for
    }

    std::vector<Task> tasks;
    try
    {
        tasks = loadTasks(); // Load tasks at the beginning
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error during task loading: " << e.what() << std::endl;
        return 1; // Exit if loading fails critically
    }

    std::string command = argv[1];
    int exitCode = 0; // Default to success

    try // Main command processing block
    {
        if (command == "add")
        {
            if (argc != 3)
            {
                std::cerr << "Error: 'add' command requires exactly one argument (description)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                addTask(tasks, argv[2]);
            }
        }
        else if (command == "list")
        {
            std::string filter = "all";
            if (argc >= 3)
            { // Allow filter argument
                filter = argv[2];
                // Validate filter
                if (filter != "all" && filter != "todo" && filter != "in-progress" && filter != "done")
                {
                    std::cerr << "Error: Invalid filter '" << filter << "'. Use 'all', 'todo', 'in-progress', or 'done'." << std::endl;
                    printUsage();
                    exitCode = 1;
                }
                else if (argc > 3) // Check for too many arguments
                {
                    std::cerr << "Error: 'list' command takes at most one argument (filter)." << std::endl;
                    printUsage();
                    exitCode = 1;
                }
                else
                {
                    listTasks(tasks, filter); // Call with valid filter
                }
            }
            else
            {                             // No filter provided
                listTasks(tasks, filter); // Call with default "all" filter
            }
        }
        else if (command == "update")
        {
            if (argc != 4)
            {
                std::cerr << "Error: 'update' command requires two arguments (id, description)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                int id = std::stoi(argv[2]); // stoi can throw
                updateTask(tasks, id, argv[3]);
            }
        }
        else if (command == "delete")
        {
            if (argc != 3)
            {
                std::cerr << "Error: 'delete' command requires one argument (id)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                int id = std::stoi(argv[2]); // stoi can throw
                deleteTask(tasks, id);
            }
        }
        else if (command == "mark-in-progress")
        {
            if (argc != 3)
            {
                std::cerr << "Error: 'mark-in-progress' command requires one argument (id)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                int id = std::stoi(argv[2]); // stoi can throw
                markTaskStatus(tasks, id, "in-progress");
            }
        }
        else if (command == "mark-done")
        {
            if (argc != 3)
            {
                std::cerr << "Error: 'mark-done' command requires one argument (id)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                int id = std::stoi(argv[2]); // stoi can throw
                markTaskStatus(tasks, id, "done");
            }
        }
        else if (command == "mark-todo")
        {
            if (argc != 3)
            {
                std::cerr << "Error: 'mark-todo' command requires one argument (id)." << std::endl;
                printUsage();
                exitCode = 1;
            }
            else
            {
                int id = std::stoi(argv[2]); // stoi can throw
                markTaskStatus(tasks, id, "todo");
            }
        }
        // No need for explicit 'help' check here, handled at the top
        else
        {
            std::cerr << "Error: Unknown command '" << command << "'." << std::endl;
            printUsage();
            exitCode = 1; // Unknown command is an error
        }
    }
    // Catch errors from std::stoi
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: Invalid number format provided for task ID. Please use an integer." << std::endl;
        exitCode = 1;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "Error: Provided task ID is too large or too small." << std::endl;
        exitCode = 1;
    }
    // Catch any other standard exceptions during command processing
    catch (const std::exception &e)
    {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        exitCode = 1;
    }

    return exitCode; // Return 0 on success, 1 on error
}
