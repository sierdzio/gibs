#include <logger/log.h>

namespace
{
const bool g_loggerSilent = []() {
    Log::setLogLevel(Log::Type::Silent);
    return true;
}();
} // namespace
