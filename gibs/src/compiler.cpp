#include "compiler.h"
#include "tags.h"
#include "gibs.h"

#include <QJsonArray>

Compiler::Compiler()
{
}

QJsonObject Compiler::toJson() const
{
    QJsonObject object;
    object.insert(Tags::compilerName, name);
    object.insert(Tags::compiler, compiler);
    object.insert(Tags::ccompiler, ccompiler);
    object.insert(Tags::compilerFlags, QJsonArray::fromStringList(flags));
    object.insert(Tags::compilerDebugFlags, QJsonArray::fromStringList(debugFlags));
    object.insert(Tags::compilerReleaseFlags, QJsonArray::fromStringList(releaseFlags));
    object.insert(Tags::linker, linker);
    object.insert(Tags::staticArchiver, staticArchiver);
    object.insert(Tags::librarySuffix, librarySuffix);
    object.insert(Tags::staticLibrarySuffix, staticLibrarySuffix);
    object.insert(Tags::linkerFlags, QJsonArray::fromStringList(linkerFlags));
    object.insert(Tags::linkerStaticFlags, QJsonArray::fromStringList(linkerStaticFlags));
    object.insert(Tags::linkerDynamicFlags, QJsonArray::fromStringList(linkerDynamicFlags));
    return object;
}

Compiler Compiler::fromJson(const QJsonObject &json)
{
    Compiler compiler;
    compiler.name = json.value(Tags::compilerName).toString();
    compiler.compiler = json.value(Tags::compiler).toString();
    compiler.ccompiler = json.value(Tags::ccompiler).toString();
    compiler.flags = Gibs::jsonArrayToStringList(
                json.value(Tags::compilerFlags).toArray());
    compiler.debugFlags = Gibs::jsonArrayToStringList(
                json.value(Tags::compilerDebugFlags).toArray());
    compiler.releaseFlags = Gibs::jsonArrayToStringList(
                json.value(Tags::compilerReleaseFlags).toArray());
    compiler.linker = json.value(Tags::linker).toString();
    compiler.staticArchiver = json.value(Tags::staticArchiver).toString();
    compiler.librarySuffix = json.value(Tags::librarySuffix).toString();
    compiler.staticLibrarySuffix = json.value(
                Tags::staticLibrarySuffix).toString();
    compiler.linkerFlags = Gibs::jsonArrayToStringList(
                json.value(Tags::linkerFlags).toArray());
    compiler.linkerStaticFlags = Gibs::jsonArrayToStringList(
                json.value(Tags::linkerStaticFlags).toArray());
    compiler.linkerDynamicFlags = Gibs::jsonArrayToStringList(
                json.value(Tags::linkerDynamicFlags).toArray());
    return compiler;
}

