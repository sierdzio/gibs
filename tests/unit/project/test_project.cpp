#include <gtest/gtest.h>

#include <exceptions/processexception.h>
#include <logger/log.h>
#include <parsing/syntax.h>
#include <processing/processor.h>
#include <project/command.h>
#include <project/project.h>

#include <memory>
#include <sstream>
#include <vector>

namespace
{
const Paths DefaultPaths = {std::filesystem::current_path(),
                            std::filesystem::current_path(),
                            {},
                            {},
                            std::filesystem::current_path() / "build",
                            {}};
} // namespace

// Helper to create a valid source command
Command makeSourceCommand(const std::string &filename)
{
    Command cmd;
    cmd.append("source");
    cmd.append(filename);
    cmd.finalize({}, DefaultPaths);
    return cmd;
}

// Helper to create a valid library command
Command makeLibraryCommand(const std::string &libname)
{
    Command cmd;
    cmd.append("library");
    cmd.append("name");
    cmd.append(libname);
    cmd.append("type");
    cmd.append("static");
    cmd.finalize({}, DefaultPaths);
    return cmd;
}

// Helper to create a valid executable command
Command makeExecutableCommand(const std::string &exename)
{
    Command cmd;
    cmd.append("executable");
    cmd.append("name");
    cmd.append(exename);
    cmd.finalize({}, DefaultPaths);
    return cmd;
}

Command makeToolCommand(const std::string &executable, const StringList &arguments = {})
{
    Command cmd;
    cmd.append("tool");
    cmd.append(executable);
    for (const auto &argument : arguments)
    {
        cmd.append(argument);
    }
    cmd.finalize({}, DefaultPaths);
    return cmd;
}

class ProjectDependencyTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        _processor = std::make_shared<Processor>();
        _processor->setDryRun(true);
        _project = std::make_shared<Project>(_processor);
    }

    void TearDown() override
    {
        _processor->waitForFinished();
    }

    std::shared_ptr<Processor> _processor;
    std::shared_ptr<Project> _project;
};

TEST_F(ProjectDependencyTest, ExecutableNameOverrideUpdatesWholeCommand)
{
    Command exec;
    exec.append("executable");
    exec.append("samples");
    exec.finalize({}, DefaultPaths);

    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    _project->commandRef(execId).setExecutableName("MultipleFiles");

    EXPECT_EQ(_project->commandRef(execId).whole(), "executable MultipleFiles");
    EXPECT_EQ(_project->commandRef(execId).path(), "MultipleFiles");
}

TEST_F(ProjectDependencyTest, SourceCommandsScheduledImmediately)
{
    Command source = makeSourceCommand("main.cpp");
    const auto sourceId = source.id();

    _project->addCommand(source);

    EXPECT_TRUE(_project->commandRef(sourceId).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, ToolCommandsAreScheduledImmediately)
{
    auto tool = makeToolCommand("true", {"--ignored-by-true"});
    const auto toolId = tool.id();

    _project->addCommand(tool);

    EXPECT_TRUE(_project->commandRef(toolId).isReadyToExecute());
    _processor->waitForFinished();
}

TEST(ProjectToolExecution, ExecutesToolArguments)
{
    auto processor = std::make_shared<Processor>();
    auto tool = makeToolCommand("true");

    processor->schedule(tool);
    EXPECT_NO_THROW(processor->waitForFinished());
}

TEST(ProjectToolExecution, DryRunDoesNotRequireExecutable)
{
    auto processor = std::make_shared<Processor>();
    processor->setDryRun(true);
    auto tool = makeToolCommand("executable-that-does-not-exist");

    processor->schedule(tool);
    EXPECT_NO_THROW(processor->waitForFinished());
}

TEST(ProjectToolExecution, ReportsFailedTool)
{
    auto processor = std::make_shared<Processor>();
    auto tool = makeToolCommand("executable-that-does-not-exist");

    processor->schedule(tool);
    EXPECT_THROW(processor->waitForFinished(), ProcessException);
}

TEST_F(ProjectDependencyTest, LibraryScheduledAfterSourcesToComplete)
{
    Command exec = makeExecutableCommand("myexe");
    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    Command source1 = makeSourceCommand("file1.cpp");
    _project->addCommand(source1);
    const auto source1Id = _project->commands.back().id();
    _project->commandRef(source1Id).parentId = execId;

    Command library = makeLibraryCommand("mylib");
    _project->addCommand(library);
    auto &libRef = _project->commands.back();
    libRef.parentId = execId;
    libRef.addLinkObject("file1.o");

    EXPECT_TRUE(_project->commandRef(source1Id).isReadyToExecute());
    EXPECT_FALSE(_project->commandRef(_project->commands.back().id()).isReadyToExecute());

    _project->onParsingFinished();

    EXPECT_TRUE(_project->commandRef(_project->commands.back().id()).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, ExecutableScheduledAfterLibraryAndSourcesComplete)
{
    Command exec = makeExecutableCommand("myapp");
    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    Command source = makeSourceCommand("main.cpp");
    _project->addCommand(source);
    const auto sourceId = _project->commands.back().id();
    _project->commandRef(sourceId).parentId = execId;

    Command library = makeLibraryCommand("helper");
    _project->addCommand(library);
    auto &libRef = _project->commands.back();
    const auto libId = libRef.id();
    libRef.parentId = execId;
    libRef.addLinkObject("main.o");

    _project->commandRef(execId).addLinkObject("helper.a");
    _project->commandRef(execId).addLinkObject("main.o");

    EXPECT_FALSE(_project->commandRef(libId).isReadyToExecute());
    EXPECT_FALSE(_project->commandRef(execId).isReadyToExecute());

    _project->onParsingFinished();

    EXPECT_TRUE(_project->commandRef(libId).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(execId).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, LibraryWithMultipleDependentSourcesScheduledWhenAllComplete)
{
    Command exec = makeExecutableCommand("app");
    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    Command source1 = makeSourceCommand("src1.cpp");
    _project->addCommand(source1);
    const auto source1Id = _project->commands.back().id();
    _project->commandRef(source1Id).parentId = execId;

    Command source2 = makeSourceCommand("src2.cpp");
    _project->addCommand(source2);
    const auto source2Id = _project->commands.back().id();
    _project->commandRef(source2Id).parentId = execId;

    Command library = makeLibraryCommand("mylib");
    _project->addCommand(library);
    auto &libRef = _project->commands.back();
    const auto libId = libRef.id();
    libRef.parentId = execId;
    libRef.addLinkObject("src1.o");
    libRef.addLinkObject("src2.o");

    _project->commandRef(execId).addLinkObject("mylib.a");
    _project->commandRef(execId).addLinkObject("src1.o");
    _project->commandRef(execId).addLinkObject("src2.o");

    EXPECT_TRUE(_project->commandRef(source1Id).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(source2Id).isReadyToExecute());
    EXPECT_FALSE(_project->commandRef(libId).isReadyToExecute());

    _project->onParsingFinished();

    EXPECT_TRUE(_project->commandRef(libId).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, AllLinkCommandsEventuallyScheduled)
{
    Command exec = makeExecutableCommand("target");
    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    Command source = makeSourceCommand("test.cpp");
    _project->addCommand(source);
    const auto sourceId = _project->commands.back().id();
    _project->commandRef(sourceId).parentId = execId;

    Command lib = makeLibraryCommand("testlib");
    _project->addCommand(lib);
    auto &libRef = _project->commands.back();
    const auto libId = libRef.id();
    libRef.parentId = execId;
    libRef.addLinkObject("test.o");

    _project->commandRef(execId).addLinkObject("testlib.a");
    _project->commandRef(execId).addLinkObject("test.o");

    _project->onParsingFinished();

    EXPECT_TRUE(_project->commandRef(sourceId).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(libId).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(execId).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, ComplexDAGWithMultipleLibrariesAndExecutables)
{
    // Create executable
    Command exec = makeExecutableCommand("final");
    _project->addCommand(exec);
    const auto execId = _project->commands.back().id();

    // Create lib1 with source1
    Command source1 = makeSourceCommand("lib1.cpp");
    _project->addCommand(source1);
    const auto source1Id = _project->commands.back().id();
    _project->commandRef(source1Id).parentId = execId;

    Command lib1 = makeLibraryCommand("lib1");
    _project->addCommand(lib1);
    auto &lib1Ref = _project->commands.back();
    const auto lib1Id = lib1Ref.id();
    lib1Ref.parentId = execId;
    lib1Ref.addLinkObject("lib1.o");

    // Create lib2 with source2
    Command source2 = makeSourceCommand("lib2.cpp");
    _project->addCommand(source2);
    const auto source2Id = _project->commands.back().id();
    _project->commandRef(source2Id).parentId = execId;

    Command lib2 = makeLibraryCommand("lib2");
    _project->addCommand(lib2);
    auto &lib2Ref = _project->commands.back();
    const auto lib2Id = lib2Ref.id();
    lib2Ref.parentId = execId;
    lib2Ref.addLinkObject("lib2.o");

    // Executable depends on both libraries and main.o
    Command sourceMain = makeSourceCommand("main.cpp");
    _project->addCommand(sourceMain);
    const auto sourceMainId = _project->commands.back().id();
    _project->commandRef(sourceMainId).parentId = execId;

    _project->commandRef(execId).addLinkObject("lib1.a");
    _project->commandRef(execId).addLinkObject("lib2.a");
    _project->commandRef(execId).addLinkObject("main.o");

    // Before scheduling link commands
    EXPECT_FALSE(_project->commandRef(lib1Id).isReadyToExecute());
    EXPECT_FALSE(_project->commandRef(lib2Id).isReadyToExecute());
    EXPECT_FALSE(_project->commandRef(execId).isReadyToExecute());

    _project->onParsingFinished();

    // After scheduling
    EXPECT_TRUE(_project->commandRef(lib1Id).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(lib2Id).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(execId).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(source1Id).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(source2Id).isReadyToExecute());
    EXPECT_TRUE(_project->commandRef(sourceMainId).isReadyToExecute());
}

TEST_F(ProjectDependencyTest, LogCommandTreeShowsDefines)
{
    Command compile = makeSourceCommand("main.cpp");
    compile.objectReference().defines = {"DEBUG", "MY_FEATURE"};
    _project->addCommand(compile);

    Log::setLogLevel(Log::Type::Information);
    Log::setUseColorfulLogs(false);

    std::ostringstream buffer;
    const auto previous = std::cout.rdbuf(buffer.rdbuf());
    _project->logCommandTree();
    std::cout.rdbuf(previous);

    const auto output = buffer.str();
    EXPECT_NE(output.find("[defines: DEBUG MY_FEATURE]"), std::string::npos);
}
