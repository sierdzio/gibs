#include "configuration.h"

#include <fstream>
#include <sstream>

namespace
{
void setError(std::string *error, const std::string &message)
{
    if (error)
    {
        *error = message;
    }
}
} //namespace

bool ConfigurationGenerator::generate(
    const std::filesystem::path &input, const std::filesystem::path &output,
    const std::vector<ConfigurationReplacement> &replacements, std::string *error)
{
    std::ifstream inputFile(input, std::ios::binary);
    if (not inputFile)
    {
        setError(error, "Could not open configuration input file: " + input.string());
        return false;
    }

    std::ostringstream contents;
    contents << inputFile.rdbuf();
    auto generated = contents.str();

    for (const auto &replacement : replacements)
    {
        if (replacement.token.empty())
        {
            setError(error, "Configuration replacement token is empty");
            return false;
        }

        std::size_t position = 0;
        bool replaced = false;
        while ((position = generated.find(replacement.token, position)) !=
               std::string::npos)
        {
            generated.replace(position, replacement.token.size(), replacement.value);
            position += replacement.value.size();
            replaced = true;
        }

        if (not replaced)
        {
            setError(error, "Configuration replacement token was not found: " +
                                replacement.token);
            return false;
        }
    }

    if (input == output)
    {
        setError(error, "Configuration input and output files must be different");
        return false;
    }

    std::error_code directoryError;
    const auto parent = output.parent_path();
    if (not parent.empty())
    {
        std::filesystem::create_directories(parent, directoryError);
        if (directoryError)
        {
            setError(error, "Could not create configuration output directory: " +
                                parent.string());
            return false;
        }
    }

    std::ofstream outputFile(output, std::ios::binary | std::ios::trunc);
    if (not outputFile)
    {
        setError(error, "Could not open configuration output file: " + output.string());
        return false;
    }

    outputFile << generated;
    if (not outputFile)
    {
        setError(error, "Could not write configuration output file: " + output.string());
        return false;
    }

    return true;
}
