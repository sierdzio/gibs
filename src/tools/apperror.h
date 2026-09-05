#pragma once

enum class AppError
{
    NoError = 0,
    WrongInputPath = -1,
    IncorrectInputFileType = -2,
    EntryPointNotFound = -3,
    ConfigurationError = -4,
};