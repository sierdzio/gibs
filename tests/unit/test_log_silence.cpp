#include <logger/log.h>

namespace
{
const bool makeLoggerSilentInTests = []()
{
    Log::setLogLevel(Log::Type::Silent);
    return true;
}();
} // namespace
