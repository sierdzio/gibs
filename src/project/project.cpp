#include "project.h"

#include "command.h"
#include "targetid.h"

void Project::addDependency(const TargetId& target,
                            const TargetId& dependency)
{
    for (auto& currentTarget : targets)
    {
        if (currentTarget.id == target)
        {
            bool shouldAppend = true;

            for (const auto& currentDependency : currentTarget.dependencies)
            {
                if (currentDependency == dependency) {
                    shouldAppend = false;
                    break;
                }
            }

            if (shouldAppend)
            {
                currentTarget.dependencies.push_back(dependency);
                break;
            }
        }
    }
}

bool Project::addCommand(const Command& command, const TargetId& id,
                         const Stage stage)
{
  for (auto& target : targets)
  {
    if (target.id == id)
    {
      switch (stage)
      {
        case Stage::Unknown:
            return false;
        case Stage::First:
          target.stageOne.commands.push_back(command);
          return true;
        case Stage::Second:
          target.stageTwo.commands.push_back(command);
          return true;
      }
    }
  }

  return false;
}
