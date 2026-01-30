# Contribution guidelines

This file is mostly aimed at developers, and primarily describes the setup required for development, and project-specific things to think about when contributing code. For general open-source contribution guidelines, see [opensource.guide](//opensource.guide). The guidelines listed under "Basic guidelines" do apply to all forms of contributions, including issues.

This file will not go into detail on how to write issues. Any important details that need to be included (if any) will be part of an issue template, selectable when you create an issue. If none exists for your use-case (or at all), use common sense. I do strongly suggest reading [the section on communicating effectively on opensource.guide](https://opensource.guide/how-to-contribute/#communicating-effectively) if you're wondering how to write good issues. There's nothing anyone could write here that isn't covered there and in thousands of other resources around the internet in far greater detail.

## Basic guidelines

### Use of generative AI is banned

Generative AI uses training data [based on plagiarism and piracy](https://web.archive.org/web/20250000000000*/https://www.theatlantic.com/technology/archive/2025/03/libgen-meta-openai/682093/), has [significant environmental costs associated with it](https://doi.org/10.21428/e4baedd9.9070dfe7), and [generates fundamentally insecure code](https://doi.org/10.1007/s10664-024-10590-1). GenAI is not ethically built, ethical to use, nor safe to use for programming applications. When caught, you will be permanently banned from contributing to the project, and any prior contributions will be checked and potentially reverted. Any and all contributions you've made cannot be trusted if AI slop machines were involved.

## Development setup

The dev requirements are the same as listed in the README. However, for development use, I strongly recommend using Conan. This adds Python and venvs as an extra dependency. If your distro or OS has the up-to-date deps in their package repos, or you've installed the deps locally manually for whatever reasons, you don't have to use conan.

```
# A venv is strongly recommended
python3 -m venv env 
# Install conan
pip3 install conan

# Run CMake
mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_CONAN=ON
cmake --build . 
```

If you're using Visual Studio, a `CMakeSettings.json` is pre-provided. Its functionality is on a best-effort basis, as I do not willingly use MSVC nor Windows myself.

### Running tests

```
cmake --build . --target test
```

#### Running tests with coverage (Linux only)

If you want to run coverage, the stuff to do that been added to the CMakeLists.txt

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_CONAN=ON -DMAGPIE_COVERAGE=ON
```
You may need to remove `CMakeCache.txt` and/or the entire build dir for the cmake command to take effect. If you're not seeing lots of recompiling in the next step, that's probably the case.
```bash
make -j $(nproc) test
make gen-coverage
xdg-open coverage-html/index.html
```

### Testing policy

As much of the code should be tested as possible, within reason.

The primary goal of tests is to ensure there's a support framework that prevents backsliding in code quality. With enough tests, you don't need to worry as much about breaking something unrelated to what you were working on. 100% coverage is a pointless metric, but coverage tools can be useful to tell what critical paths aren't being tested. In real code, many paths may legitimately be unreachable without doing an awful lot of fucking around, particularly in exception handlers. Doing elaborate bullshit to test every possible path in the code, including trivial paths, is a waste of time and effort.

In practice, this means:

* If you're writing new functionality, write tests for the core parts of it
* If you're fixing a bug that was reported, write regression tests
* If you're working with edge-cases, test them

Writing tests isn't always feasible, but it should be attempted whereever possible. However, if any tests break, they must be fixed. Removal should only be done if the corresponding functionality is removed, and not as a way to bypass the test failures to maybe perhaps fix later.

## Naming conventions

### Enum values and constants

To maintain support for Windows, PascalCase is used for enums and constants. `SCREAMING_SNAKE_CASE` is functionally unusable as long as Windows is part of the target operating systems, due to Windows defining fucking everything as a macro. 

This is with the exception of HTTP methods, which use a pseudo-pascalcase where only the first letter is capitalised, and any underscores are kept to be as close to the actual names in HTTP standards as possible. 
