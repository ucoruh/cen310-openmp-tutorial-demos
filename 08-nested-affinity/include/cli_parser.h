#pragma once

#include <string>
#include <map>
#include <vector>

/**
 * @class CliParser
 * @brief Command-line argument parser for the application
 * 
 * This class processes command-line arguments and provides a simple
 * interface for accessing option values.
 */
class CliParser {
public:
    /**
     * @brief Construct a new CliParser object
     * @param argc Argument count from main
     * @param argv Argument vector from main
     */
    CliParser(int argc, char* argv[]);

    /**
     * @brief Check if an option was provided on the command line
     * @param option Option name to check
     * @return true if the option exists
     */
    bool hasOption(const std::string& option) const;

    /**
     * @brief Get an option's string value
     * @param option Option name
     * @param defaultValue Value to return if option doesn't exist
     * @return Option value or defaultValue
     */
    std::string getStringOption(const std::string& option, const std::string& defaultValue = "") const;

    /**
     * @brief Get an option's integer value
     * @param option Option name
     * @param defaultValue Value to return if option doesn't exist
     * @return Option value or defaultValue
     */
    int getIntOption(const std::string& option, int defaultValue = 0) const;

    /**
     * @brief Get an option's double value
     * @param option Option name
     * @param defaultValue Value to return if option doesn't exist
     * @return Option value or defaultValue
     */
    double getDoubleOption(const std::string& option, double defaultValue = 0.0) const;

    /**
     * @brief Get an option's boolean value
     * @param option Option name
     * @param defaultValue Value to return if option doesn't exist
     * @return Option value or defaultValue
     */
    bool getBoolOption(const std::string& option, bool defaultValue = false) const;

    /**
     * @brief Get positional arguments (arguments without option flags)
     * @return Vector of positional arguments
     */
    std::vector<std::string> getPositionalArgs() const;

    /**
     * @brief Display help information with available options
     */
    void displayHelp() const;

private:
    std::map<std::string, std::string> m_options;
    std::vector<std::string> m_positionalArgs;
    std::string m_programName;

    /**
     * @brief Parse the argument vector into options and values
     * @param argc Argument count
     * @param argv Argument vector
     */
    void parseArgs(int argc, char* argv[]);
};