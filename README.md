# GIBS

[TOC]

[![Build Status](https://travis-ci.org/sierdzio/gibs.svg?branch=master)](https://travis-ci.org/sierdzio/gibs)

GIBS - *Generally In-source Build System*

Note: *This branch contains gibs written in pure C++ and is a WORK IN PROGRESS*

If you look for something that can be used, check out *master_qt* branch, although it is heavily outdated!

## Intro

GIBS (in-source build system) is a build tool that makes it easier to build C++
projects by using the information from source code to compile it.

In short, to compile a project it is enough to run:

```bash
gibs main.cpp
```

Instead of "the old way":

```bash
qmake project.pro
make
```

## How to get it

Precompiled gibs releases can be found on [releases page](https://github.com/sierdzio/gibs/releases)

Alternatively, get the source code and compile gibs using cmake. C++14 is
required.

## Online docs

Apart from this README here, full doxygen documentation... is not available at this time.

## Usage

A typical usage of gibs would be to call it on your main source file (of your
app or libary):

```bash
gibs main.cpp
```

To clean the project, run:

```bash
gibs --clean main.cpp
```

Specifying the source file is not necessary - it will be extracted from gibs
cache file generated during compilation, or default to main.cpp when the cache
is empty.

### Command line flags

To see all available flags, type:

```bash
gibs --help
```

Options:

```text
   -h, --help                      Displays this help information and
                                  exits.
  -v, --version                   Displays gibs version info and exits.
  -r, --run                       Run the executable immediately after
                                  building.
  -d, --debug                     Compile in debug mode. By default,
                                  gibs compiles release binaries.
  -q, --quick                     'Convention over configuration' mode
                                  - parse files only up to first line of
                                  'concrete code'. Do not check file checksums
                                  when doing incremental builds.
  --verbose                       Sets log level to 'Verbose'. If more
                                  than one log level is specified, or log
                                  level is combined with --verbose, only
                                  the last flag is taken nto account
  -l, --log-level                 Sets log level to one of: silent,
                                  error, warning, information, debug,
                                  verbose. Logs are printed for selected
                                  level and all levels above it. For example,
                                  when information is set, all error,
                                  warning and information logs will be
                                  printed, but no debug or verbose ones.
                                  'silent' setting will not print any logs
                                  at all. Log level parser is case-sensitive,
                                  please make sure to provide log levels
                                  in lower case. If more than one log
                                  level is specified, or log level is
                                  combined with --verbose, only the last
                                  flag is taken into account.
  --log-file-path                 Duplicates all console logs into this
                                  file. If the file does not exist, it will
                                  be created. If the file does exist, it
                                  will be cleared and written to. If a path
                                  to a directory is provided, a log file
                                  called 'gibs-<datetime>.log will be
                                  created.
  --no-color                      Disables color in log messages.
  --dry-run                       Does not actually run any compilation
                                  or linking commands. Commands are only
                                  printed out but not executed.
  --log-process-output            Prints standard and error outputs
                                  from spawned processes (compiler, linker
                                  etc.).
  -c, --compiler, --compiler-set  Selects the compiler set to use: gcc,
                                  clang, apple-clang. Defaults to apple-clang
                                  on macOS and gcc elsewhere.
  input                           Path to the input file or directory
                                  to build. If a directory is provided,
                                  gibs will look for files with supported
                                  extensions in it and its subdirectories.
                                  If a file is provided, gibs will try to
                                  build it. If no input is provided, gibs
                                  will try to build a file called 'main'
                                  with a supported extension in the current
                                  directory.
  --                              Other, user-defined arguments passed
                                  to the program should be placed after
                                  this separator. For example: 'gibs
                                  --main.cpp -- --my-flag' will enable feature
                                  'my-flag', and 'gibs --main.cpp --
                                  --my-flag=OFF' will disable it when compiling
                                  main.cpp.
```

Gibs builds in *release* mode by default. If you want to compile a debug build,
use `--debug` or `-d`.

### Path config / cache

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

## Commands

### Syntax

To get additional functionality and specify external libraries, sources etc.
you can use gibs commands inside comments of your C++ code.

You can use one-line command syntax:

```text
//i some command
```

Or comment scope:

```text
/*i
some command
other command
*/
```

### Available commands

#### Specific source files

You can specify an extra source file to compile like this:

```text
//i source my_source_file.cpp
```

This is especially useful for cases where gibs cannot guess the source file for
given header.

#### Targets

Targets are applications or libraries that gibs is compiling and linking.

* to create an executable or library with a given name:

```text
//i executable name MyAwesomeApp
//i library name MyAwesomeLibrary
```

If no other information is provided, gibs will assume it is compiling
an executable and it will be named the same as parent directory name.

* libraries can additionally be marked as static or dynamic. Libraries are dynamic by default

```text
//i library MyAwesomeLib type static
//i library MyAwesomeLib type dynamic
```

#### Defines

To pass custom defines to the compiler, use the following command:

```text
//i define MY_DEFINE
```

#### Include paths

Include paths are specified using `include` command:

```text
//i include some/path
```

If a path to a directory is provided, it will be added to C++ include paths (`-I`).

If a path to gibs project file is provided, the file will be loaded and parsed.

If a path to a C++ header file is provided, it will be parsed by gibs and potentially
the corresponding source file will be compiled.

#### Include libraries

To include and link to an external library, you need to specify both the include
and gibs commands.

```text
//i include library some/path
//i include library -Lsome/path -llibrary1 -llibrary2
```

#### Configuration files

This command allows to generate an output file (which can be compiled if
necessary) based on an input schema - with predefined variables replacing
templates with real values.

This is a very similar concept to CMake's
[configure_file()](https://cmake.org/cmake/help/latest/command/configure_file.html).

```text
//i configure file input something.h.in output something.h
//i replace "some-text" with target.main.version()
//i replace "another_text" with "hello, config file!"
```

The first line above specifies the input file and the generated file path.
The following lines (read until the end of comment block, first empty line
or detection of a different command) specify which text should be replaced
with which values.

`replace` will match exact strings from input file and replace them with either
hardcoded values (provided in quotes) or with built-in gibs methods.

Built-in gibs values and functions are matched and replaced automatically, so
there is no need to write for example:

```text
//i replace ${target.gibs.name()} with target.gibs.name()
```

because the value will be replaced automatically.

##### Built-in values

* `target.name()` - name of current target (library or executable)
* `target.version()` - version of current target (library or executable)
* `target.<name>.name()` - name of a specified target (library or executable)
* `target.<name>.version()` - version of a specified target (library or executable)
* TODO: more...

#### Qt support

Gibs makes it easier to build other Qt projects.
In order to compile a Qt application or library, you need to specify Qt
directory by running gibs with `--qt-dir` flag:

```bash
gibs --qt-dir /home/qt/6.8.1/gcc_64 main.cpp
```

This will tell gibs which Qt version should it use. Then, in the source code of
your application or library, you need to specify Qt modules which should be
loaded.

```text
//i qt core network
```

To run Qt tools, use the `tools` command:

```text
//i tool rcc myResource.qrc myOtherResource.qrc
```

You do not need to run MOC manually, gibs will run it automatically when needed.

#### Tools

Gibs can run external tools, applications and processes. To do this, use the `tool`
command, followed by executable path and any necessary arguments.

There are some tools which are pre-configured (like Qt's tools: `rcc` and `uic`)
and you don't need to specify their paths.

```text
//i tool myexecutable.exe --some -a -r -g -s
```

#### Subprojects

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

```text
//i subproject path/to/subproject.h
```

Ibs will not synchronise until it is absolutely necessary: main app and the
subproject will be compiled in parallel. Only when linking, gibs will wait for
the library to be ready before linking the app.

#### Options

See: Features.

#### Features

Feature (also called Option) is a compile-time piece of functionality that can be turned on or off.

A feature can be easily turned on or off via a command line flag (during
compilation). Internally, feature always adds a compiler define when it is turned
on. For example, if feature named `tts-support` is added, your C++ code will see
`TTS_SUPPORT` ifdef as being true. You can use this in code to respond to the
feature being active or not.

A feature can come with it's own set of C++ files. These are pointed to by feature definition.

**TODO: how to define which files belong to a feature?**

To define a feature, use this syntax:

```text
//i feature tts-support [default on|off]
```

Or:

```text
//i option tts-support [default on|off]
```

Then you can select the feature next time you build your project, like this:

```bash
gibs main.cpp -- --tts-support
```

Or unselect it using:

```bash
gibs main.cpp -- --no-tts-support
```

On / off syntax is also supported:

```bash
gibs main.cpp -- --tts-support=OFF
gibs main.cpp -- --tts-support=ON
```

## Recommendations

It's best to put gibs commands early in .cpp or .h file, so that they can be
parsed before the rest of includes. Otherwise it may happen that you define
include folder after it was needed.
