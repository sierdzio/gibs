#include "paths.h"

std::filesystem::path Paths::absolutePath(const std::filesystem::path &inputPath) const
{
    if (inputPath.is_absolute())
    {
        return inputPath;
    }

    if (std::filesystem::exists(projectDirectory / inputPath))
    {
        return (projectDirectory / inputPath).lexically_normal();
    }

    for (const auto &dir : includePaths)
    {
        const auto path = (workingDirectory / dir / inputPath).lexically_normal();
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }

    return (workingDirectory / inputPath).lexically_normal();
}

std::ostream &operator<<(std::ostream &stream,
                         const std::vector<std::filesystem::path> &paths)
{
    stream << "(";
    for (size_t i = 0; i < paths.size(); ++i)
    {
        stream << paths[i];
        if (i < paths.size() - 1)
        {
            stream << ", ";
        }
    }
    stream << ")";
    return stream;
}