# Modern C++ Command Line Parser

A type-safe, modern C++ library for parsing command line arguments with support for environment variables, validation, and rich data types.

## Features

- Type-safe option parsing with automatic type deduction
- Support for basic types (bool, int, double, string)
- Array values (vectors of basic types)
- Map values (string-keyed maps of basic types)
- Environment variable fallbacks
- Custom validation rules
- Required option enforcement
- Automatic help message generation
- Short (-v) and long (--verbose) option formats

## Requirements

- C++20 compiler
- Standard library

## Usage

### Basic Example

Here's a simple example showing basic option types:

```cpp
#include "cli/cli.h"

int main(int argc, char* argv[]) {
    cli::Options options;

    // Add command line options
    options.add_option<bool>({
        .m_short_name = "v",
        .m_long_name = "verbose",
        .m_description = "Enable verbose output",
        .m_default_value = false
    });

    options.add_option<int>({
        .m_short_name = "p",
        .m_long_name = "port",
        .m_description = "Port number",
        .m_required = true,
        .m_default_value = 8080,
        .m_env_var = "APP_PORT"
    });

    options.add_option<std::string>({
        .m_short_name = "h",
        .m_long_name = "host",
        .m_description = "Host address",
        .m_default_value = std::string{"localhost"}
    });

    // Parse arguments
    if (!options.parse(argc, argv)) {
        options.print_help(argv[0]);
        return 1;
    }

    // Access values
    if (options.get<bool>("verbose").value_or(false)) {
        std::cout << "Verbose mode enabled\n";
    }

    auto port = options.get<int>("port").value_or(8080);
    auto host = options.get<std::string>("host").value_or("localhost");
}
```

### Advanced Types

The library supports array and map values for more complex configurations:

```cpp
#include "cli/cli.h"

int main(int argc, char* argv[]) {
    cli::Options options;
    using Strings = cli::Array_value<std::string>;
    using Limits = cli::Map_value<std::string, int>;

    // Array of strings
    options.add_option<Strings>({
        .m_long_name = "hosts",
        .m_description = "List of host addresses",
        .m_default_value = Strings({"localhost:8080", "localhost:8081"}),
        .m_env_var = "APP_HOSTS"
    });

    // Map of string to int
    options.add_option<Limits>({
        .m_long_name = "limits",
        .m_description = "Resource limits",
        .m_default_value = Limits({
            {"cpu", 4}, 
            {"memory", 1024}, 
            {"connections", 100}
        })
    });

    // Parse and access values
    if (!options.parse(argc, argv)) {
        options.print_help(argv[0]);
        return 1;
    }

    if (auto hosts = options.get<Strings>("hosts")) {
        for (const auto& host : *hosts) {
            std::cout << "Host: " << host << '\n';
        }
    }

    if (auto limits = options.get<Limits>("limits")) {
        for (const auto& [resource, limit] : limits->values()) {
            std::cout << resource << ": " << limit << '\n';
        }
    }
}
```

## Command Line Format

Options can be specified in multiple formats:

```bash
# Boolean flags
--verbose
-v

# Options with values
--port 8080
-p 8080
--port=8080

# Array values
--hosts localhost:8080,localhost:8081
--ports 8080,8081,8082

# Map values
--limits cpu=4,memory=1024,connections=100
--metadata env=prod,region=us-west,tier=premium
```

## Environment Variables

Options can fall back to environment variables if specified:

```cpp
options.add_option<int>({
    .m_long_name = "port",
    .m_description = "Port number",
    .m_default_value = 8080,
    .m_env_var = "APP_PORT"  // Will check APP_PORT env var
});
```

## Validation

Add custom validation rules to options:

```cpp
options.add_option<int>({
    .m_long_name = "port",
    .m_description = "Port number",
    .m_default_value = 8080
});

// Add port range validation
options.add_validation("port", [](const cli::Value_variant& value) {
    auto port = std::get<int>(value);
    return port >= 1024 && port <= 65535;
});
```

## Help Output

The library automatically generates formatted help output:

```
Usage: ./program [OPTIONS] [ARGUMENTS]
Options:
  -v, --verbose                [default: false]           Enable verbose output
  -p, --port     (required)    [default: 8080]            Port number
  -h, --host                   [default: localhost]       Host address
      --hosts                  [default: localhost:8080]  List of host addresses
      --limits                 [default: cpu=4,...]       Resource limits
```

## License

See LICENCE file.

