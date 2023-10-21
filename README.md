# Simple INI
The simple INI is a header only C++20 library for parsing .ini files.
It supports basic INI files without relative nesting.

## Usage
The `SimpleINI` class provides an interface for interracting with .ini configurations.
```cpp
#include "simpleini.h"

std::filesystem::path myfile {"my/config.ini"};
simpleini::SimpleINI myconfig(myfile); // May throw INIException if config file format isn't correct.

std::string value = myconfig["my section name"]["my value"] // May throw std::out_of_range if a key isn't found
```

## Contributing
The header is formatted using `clang-format -i -style="{BasedOnStyle: Mozilla, IndentWidth: 4}`
