#include "../include/cli_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

CliParser::CliParser(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {
    // Get the program name from argv[0]
    if (argc > 0) {
        std::filesystem::path programPath(argv[0]);
        m_programName = programPath.filename().string();
    } else {
        m_programName = "program";
    }
    
    // Parse the arguments
    parseArgs(argc, argv);
    
    // Handle help flag
    if (hasOption("help") || hasOption("h")) {
        displayHelp();
        // If help is requested, exit after displaying
        exit(0);
    }
}

void CliParser::parseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg.size() > 1 && arg[0] == '-') {
            // This is an option
            std::string option;
            std::string value;
            
            // Handle --option=value format
            size_t equalPos = arg.find('=');
            if (equalPos != std::string::npos) {
                option = arg.substr(0, equalPos);
                value = arg.substr(equalPos + 1);
            } else {
                option = arg;
                
                // Check if the next argument is a value (not an option)
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    value = argv[i + 1];
                    i++; // Skip the value in the next iteration
                } else {
                    // Boolean flag (presence means true)
                    value = "true";
                }
            }
            
            // Remove leading dashes
            if (option.size() > 2 && option[0] == '-' && option[1] == '-') {
                option = option.substr(2);
            } else if (option.size() > 1 && option[0] == '-') {
                option = option.substr(1);
            }
            
            // Store the option
            m_options[option] = value;
        } else {
            // This is a positional argument
            m_positionalArgs.push_back(arg);
        }
    }
}

bool CliParser::hasOption(const std::string& option) const {
    return m_options.find(option) != m_options.end();
}

std::string CliParser::getStringOption(const std::string& option, const std::string& defaultValue) const {
    auto it = m_options.find(option);
    if (it != m_options.end()) {
        return it->second;
    }
    return defaultValue;
}

int CliParser::getIntOption(const std::string& option, int defaultValue) const {
    auto it = m_options.find(option);
    if (it != m_options.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

double CliParser::getDoubleOption(const std::string& option, double defaultValue) const {
    auto it = m_options.find(option);
    if (it != m_options.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool CliParser::getBoolOption(const std::string& option, bool defaultValue) const {
    auto it = m_options.find(option);
    if (it != m_options.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), 
                      [](unsigned char c) { return std::tolower(c); });
        
        return (value == "true" || value == "1" || value == "yes" || value == "y");
    }
    return defaultValue;
}

std::vector<std::string> CliParser::getPositionalArgs() const {
    return m_positionalArgs;
}

void CliParser::displayHelp() const {
    std::cout << "Usage: " << m_programName << " [options] [arguments]" << std::endl;
    std::cout << std::endl;
    std::cout << "OpenMP Debugging and Performance Analysis Tool" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                Display this help message" << std::endl;
    std::cout << "  --threads=N               Number of threads to use (default: system available)" << std::endl;
    std::cout << "  --iterations=N            Number of iterations for the test" << std::endl;
    std::cout << "  --size=N                  Size parameter for tests (array size, etc.)" << std::endl;
    std::cout << "  --elements=N              Number of elements for tests" << std::endl;
    std::cout << "  --report=FILE             Path to save CSV performance report" << std::endl;
    std::cout << "  --verbose                 Enable verbose output" << std::endl;
    std::cout << "  --quiet                   Minimize output (useful for profiling runs)" << std::endl;
    std::cout << "  --check                   Verify results where applicable" << std::endl;
    std::cout << "  --target=NAME             Target name for diagnostics tools" << std::endl;
    
    // Example specific flags
    if (m_programName.find("race_conditions") != std::string::npos) {
        std::cout << std::endl;
        std::cout << "Race Conditions Options:" << std::endl;
        std::cout << "  --mode=MODE              Race condition type: simple, complex, atomics" << std::endl;
    }
    else if (m_programName.find("false_sharing") != std::string::npos) {
        std::cout << std::endl;
        std::cout << "False Sharing Options:" << std::endl;
        std::cout << "  --padding=N              Padding size between array elements" << std::endl;
        std::cout << "  --cache-line=N           Cache line size to use (default: 64)" << std::endl;
    }
    else if (m_programName.find("load_imbalance") != std::string::npos) {
        std::cout << std::endl;
        std::cout << "Load Imbalance Options:" << std::endl;
        std::cout << "  --schedule=TYPE          Schedule type: static, dynamic, guided, auto" << std::endl;
        std::cout << "  --chunk-size=N           Chunk size for scheduling" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << m_programName << " --threads=4 --iterations=1000" << std::endl;
    std::cout << "  " << m_programName << " --size=1024 --report=results.csv" << std::endl;
    std::cout << std::endl;
}

void CliParser::addOption(const std::string& option, const std::string& defaultValue) {
    // Only add if not already present
    if (m_options.find(option) == m_options.end()) {
        m_options[option] = defaultValue;
    }
}

void CliParser::addOption(const std::string& option, char shortName, const std::string& description, bool required) {
    // This version is used for documentation purposes
    // The actual option handling is done in parseArgs
    // We'll just add the option with an empty default value
    if (m_options.find(option) == m_options.end()) {
        m_options[option] = "";
    }
    
    // We could store the short name, description, and required flag in separate maps
    // but for now we'll just use this method for compatibility
}

void CliParser::parse(int argc, char* argv[]) {
    // This is just an alias for parseArgs
    parseArgs(argc, argv);
}

void CliParser::parse() {
    // Use the arguments provided in the constructor
    parseArgs(m_argc, m_argv);
}

int CliParser::getIntValue(const std::string& option, int defaultValue) const {
    // This is just an alias for getIntOption
    return getIntOption(option, defaultValue);
}

std::string CliParser::getStringValue(const std::string& option, const std::string& defaultValue) const {
    // This is just an alias for getStringOption
    return getStringOption(option, defaultValue);
}