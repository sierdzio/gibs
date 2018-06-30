#include "compiler.h"
#include "tags.h"

Compiler::Compiler()
{
}

QJsonObject Compiler::toJson() const
{
    QJsonObject object;
    //object.insert(Tags::libs, QJsonArray::fromStringList(mCustomLibs));

    return object;
}

Compiler Compiler::fromJson(const QJsonObject &json)
{
    Q_UNUSED(json);
    Compiler compiler;
    // json.value(Tags::scopeTargetName)
    return compiler;
}
