#pragma once

#include <QLatin1String>

namespace Tags {
// IBS comment scope
const QLatin1String scopeOneLine("//i ");
const QLatin1String scopeBegin("/*i");
const QLatin1String scopeEnd("*/");
// IBS commands
const QLatin1String source("source");
const QLatin1String targetCommand("target");
const QLatin1String targetName("name");
const QLatin1String targetType("type");
const QLatin1String targetApp("app");
const QLatin1String targetLib("lib");
const QLatin1String targetLibStatic("static");
const QLatin1String targetLibDynamic("dynamic");
const QLatin1String includes("includes ");
const QLatin1String libs("libs ");
const QLatin1String tool("tool");
// Qt support
//    modules
const QLatin1String qtModules("qt");
const QLatin1String core("core");
const QLatin1String gui("gui");
const QLatin1String network("network");
const QLatin1String concurrent("concurrent");
const QLatin1String widgets("widgets");
const QLatin1String qml("qml");
const QLatin1String quick("quick");
const QLatin1String quickcontrols2("quickcontrols2");
const QLatin1String quickwidgets("quickwidgets");
const QLatin1String svg("svg");
const QLatin1String sql("sql");
const QLatin1String xml("xml");
const QLatin1String dbus("dbus");
const QLatin1String script("script");
const QLatin1String multimedia("multimedia");
const QLatin1String location("location");
const QLatin1String gamepad("gamepad");
const QLatin1String bluetooth("bluetooth");
//    tools
const QLatin1String rcc("rcc");
const QLatin1String uic("uic");
}
