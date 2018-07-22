[TOC]

# GIBS

*Generally In-source Build System*

# Intro

GIBS (in-source build system) is a build tool that makes it easier to build C++
projects by using the information from source code to compile it.

In short, to compile a project it is enough to run:

    gibs main.cpp

Instead of "the old way":

    qmake project.pro
    make

# How to get it

Precompiled gibs releases can be found on https://github.com/sierdzio/gibs/releases

Alternatively, get the source code and compile gibs using Qt Creator. Any recent
Qt version would do. C++11 is required.

# Usage

A typical usage of gibs would be to call it on your main source file (of your
app or libary):

    gibs main.cpp

To clean the project, run:

    gibs --clean main.cpp

Specifying the source file is not necessary - it will be extracted from gibs
cache file generated during compilation.

## Command line flags

To see all available commands, type:

    gibs --help

Most notable options are:
C++ in-source project builder. Compile your projects without all the hassle connected with preparing a project file. Just run 'gibs main.cpp' and enjoy your compiled binary! More info: https://github.com/sierdzio/gibs

Options:
```
  -h, --help                  Displays this help.
  -v, --version               Displays version information.
  -d, --debug                 Compile in debug mode. By default, gibs compiles
                              release binaries
  -r, --run                   Run the executable immediately after building
  --qt-dir <Qt dir>           Specify Qt directory for Qt apps
  -m, --auto-qt-modules       Automatically guess Qt modules used by the
                              project. This is done using internal dictionary
                              mapping Qt classes to modules.
  --clean                     Clear build directory
  -q, --quick                 'Convention over configuration' mode - parse
                              files only up to first line of 'concrete code'. Do
                              not check file checksums when doing incremental
                              builds.
  -a, --auto-include          Automatically scan source directory for include
                              paths. This can be used instead of gibs command
                              'include some/path' if the path is below input
                              file.
  -j, --jobs <threads>        Max number of threads used to compile and process
                              the sources. If not specified, gibs will use max
                              possible number of threads. If a fraction is
                              specified, it will use given percentage of
                              available cores (-j 0.5 means half of all CPU
                              cores)
  -c, --commands <commands>   gibs syntax commands - same you can specify in
                              c++ commends. All commands are suppored on the
                              command line as well
  -w, --parse-whole-files     Parse whole files instead of just their
                              beginning. By default, only code up to first class
                              declaration or function definition is parsed.
  --deploy-tool <path>        path to deployment tool to use, for example
                              linuxdeployqt.AppImage
  --compiler <compiler name>  compiler name. If specified, Gibs will search for
                              compiler definitions in $HOME/.gibs/compilers.
                              Built-in compiler definitions are: gcc, clang.
                              Once Gibs learns to parse mkspecs, it will be a
                              source of compiler configs as well

Arguments:
  inputFile                   Input file, usually main.cpp
```

Gibs builds in *release* mode by default. If you want to compile a debug build,
use `--debug` or `-d`.

## Path config / cache

To save you typing, gibs will remember paths between runs, so you need to
specify them only once.

Remembered paths are:
* Qt dir
* deployer path
* sysroot path
* toolchain path
* Android NDK path
* Android NDK API level
* Android NDK ABI level
* Android SDK path
* Android SDK API level
* jdk (Java) path

Paths precedence is:
1. Paths passed on command line.
2. Paths saved in current directory.
3. Paths saved in $HOME/.config/gibs

# Commands

## Syntax

To get additional functionality and specify external libraries, sources etc.
you can use gibs commands inside comments of your C++ code.

You can use one-line command syntax:

    //i some command

Or comment scope:

    /*i
    some command
    other command
    */

## Available commands

### Specific source files

You can specify an extra source file to compile like this:

    //i source my_source_file.cpp

This is especially useful for cases where gibs cannot guess the source file for
given header.

### Target

Target command can be used to define 3 things:

* executable / library name
```
    //i target name MyAwesomeApp
```

* target type: either a library or an application
```
    //i target type app
    //i target type lib
```

* libraries can additionally be marked as static or dynamic. Dynamic libraries
are the default value, it can be skipped
```
    //i target type lib static
    //i target type lib dynamic
```

### Defines

To pass custom defines to the compiler, use the following command:

    //i define MY_DEFINE

### Include paths

Include paths are specified using `include` command:

    //i include some/path

### Libraries

To include and link to an external library, you need to specify both the include
and lgibs commands.

    //i include some/path
    //i lib -Lsome/path -llibrary1 -llibrary2

### Qt support

Ibs is written in Qt, and it makes it easier to build other Qt projects, too.
In order to compile a Qt application or library, you need to specify Qt
directory by runnig gibs with `--qt-dir` flag:

    gibs --qt-dir /home/qt/5.9.4/gcc_64 main.cpp

This will tell gibs which Qt version should it use. Then, in the source code of
your application or library, you need to specify Qt modules which should be
loaded.

    //i qt core network

To run Qt tools, use the `tools` command:

    //i tool rcc myResource.qrc myOtherResource.qrc

You do not need to run MOC manually, gibs will run it automatically when needed.

### Tools

Ibs can run external tools, applications and processes. To do this, use the `tool`
command, followed by executable path and any necessary arguments.

There are some tools which are pre-configured (like Qt's tools: `rcc` and `uic`)
and you don't need to specify their paths.

    //i tool myexecutable.exe --some -a -r -g -s

### Subprojects

Ibs can compile collections of projects in one go. This is especially useful
in bigger projects, where you - for example - might have an application and a
library used by that app.

If a subproject is a library, it will be automatically linked with your main
app. There is no need to manually specify include paths or lgibs in that case.

(not implemented) If a subproject is another application, it will not be
connected to the "main" app. The executables will be placed in the same
directory.

(not implemented) If a subproject is a plugin, gibs will create the plugin and
copy it to the same directory as the main app. No further linking will be
preformed.

Specifying a subproject is extremely easy:

    //i subproject path/to/subproject.h

Ibs will not synchronise until it is absolutely necessary: main app and the
subproject will be compiled in parallel. Only when linking, gibs will wait for
the library to be ready before linking the app.

### Features

Feature is a compile-time piece of functionality that can be turned on or off.
Internally, features are just Subprojects with some extra bits attached.

A feature can be easily turned on or off via a command line flag (during
compilation). Internally, feature always adds a compiler define when it is turned
on. For example, if feature named `tts-support` is added, your C++ code will see
`TTS_SUPPORT` ifdef as being true. You can use this in code to respond to the
feature being active or not.

A feature typically comes with it's own set of C++ files. These are pointed to
by feature definition.

To define a feature, use this syntax:

    //i feature tts-support [default on|off]

Then you can select the feature next time you build your project, like this:

    gibs main.cpp -- --tts-support

Or unselect it using:

    gibs main.cpp -- --no-tts-support

# Recommendations

It's best to put gibs commands early in .cpp or .h file, so that they can be
parsed before the rest of includes. Otherwise it may happen that you define
include folder after it was needed.
