#include "../include/cli_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

CliParser::CliParser(int argc, char* argv[]) {
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
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                  Display this help message" << std::endl;
    std::cout << "  --demo=N                    Run a specific demo (1-6)" << std::endl;
    std::cout << "  --outer_threads=N           Set the number of threads for outer parallel regions" << std::endl;
    std::cout << "  --inner_threads=N           Set the number of threads for inner parallel regions" << std::endl;
    std::cout << "  --proc_bind=POLICY          Set OpenMP thread affinity policy (master, close, spread)" << std::endl;
    std::cout << "  --max_levels=N              Set the maximum number of active parallel levels" << std::endl;
    std::cout << "  --matrix_size=N             Set the size of matrices for matrix multiplication" << std::endl;
    std::cout << "  --verbose                   Enable verbose output" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << m_programName << " --demo=1                  Run system topology demo" << std::endl;
    std::cout << "  " << m_programName << " --outer_threads=4 --inner_threads=2  Run with 4 outer and 2 inner threads" << std::endl;
    std::cout << "  " << m_programName << " --proc_bind=spread        Run with 'spread' thread affinity policy" << std::endl;
    std::cout << std::endl;
}