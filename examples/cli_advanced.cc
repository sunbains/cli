#include <cstdlib>

#include "cli/cli.h"
#include "libudp/logger.h"

int main(int argc, char* argv[]) {
  try {
    cli::Options options;

    /* Array options */
    options.add_option<cli::Array_value<std::string>>({
      .long_name = "hosts",
      .description = "List of host addresses",
      .default_value = "localhost:8080,localhost:8081",
      .env_var = "APP_HOSTS"
    });

    options.add_option<cli::Array_value<int>>({
      .long_name = "ports",
      .description = "List of ports",
      .default_value = "8080,8081,8082"
    });

    /* Map options */
    options.add_option<cli::Map_value<std::string, int>>({
      .long_name = "limits",
      .description = "Resource limits",
      .default_value = "cpu=4,memory=1024,connections=100"
    });

    options.add_option<cli::Map_value<std::string, std::string>>({
      .long_name = "metadata",
      .description = "Additional metadata",
      .default_value = "env=prod,region=us-west,tier=premium"
    });

    if (!options.parse(argc, argv)) {
      options.print_help(argv[0]);
      return EXIT_FAILURE;
    }

    /* Access array values */
    if (auto hosts = options.get<cli::Array_value<std::string>>("hosts")) {
      log_info("Configured hosts:");
      for (const auto& host : *hosts) {
        log_info("  - ", host);
      }
    }

    if (auto ports = options.get<cli::Array_value<int>>("ports")) {
      log_info("Configured ports:");
      for (const auto& port : *ports) {
        log_info("  - ", port);
      }
    }

    /* Access map values */
    if (auto limits = options.get<cli::Map_value<std::string, int>>("limits")) {
      log_info("Resource limits:");
      for (const auto& [resource, limit] : limits->values()) {
        log_info("  ", resource, ": ", limit);
      }
    }

    return EXIT_SUCCESS;

  } catch (const std::exception& e) {
    log_error(e.what());
    return EXIT_FAILURE;
  }
}
