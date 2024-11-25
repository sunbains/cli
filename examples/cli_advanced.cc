#include <cstdlib>

#include "cli/cli.h"
#include "ut/logger.h"

int main(int argc, char* argv[]) {
  try {
    cli::Options options;

    using Hosts = cli::Array_value<std::string>;

    /* Array options */
    options.add_option<Hosts>({
      .m_long_name = "hosts",
      .m_description = "List of host addresses",
      .m_default_value = Hosts({"localhost:8080", "localhost:8081"}),
      .m_env_var = "APP_HOSTS"
    });

    using Ports = cli::Array_value<int>;

    options.add_option<Ports>({
      .m_long_name = "ports",
      .m_description = "List of ports",
      .m_default_value = Ports({8080, 8081, 8082})
    });

    /* Map options */

    using Limits = cli::Map_value<std::string, int>;

    options.add_option<Limits>({
      .m_long_name = "limits",
      .m_description = "Resource limits",
      .m_default_value = Limits({{"cpu", 4}, {"memory", 1024}, {"connections", 100}})
    });

    using Metadata = cli::Map_value<std::string, std::string>;

    options.add_option<Metadata>({
      .m_long_name = "metadata",
      .m_description = "Additional metadata",
      .m_default_value = Metadata({{"env", "prod"}, {"region", "us-west"}, {"tier", "premium"}})
    });

    if (!options.parse(argc, argv)) {
      options.print_help(argv[0]);
      return EXIT_FAILURE;
    }

    /* Access array values */
    if (auto hosts = options.get<Hosts>("hosts")) {
      log_info("Configured hosts:");
      for (const auto& host : *hosts) {
        log_info("  - ", host);
      }
    }

    if (auto ports = options.get<Ports>("ports")) {
      log_info("Configured ports:");
      for (const auto& port : *ports) {
        log_info("  - ", port);
      }
    }

    /* Access map values */
    if (auto limits = options.get<Limits>("limits")) {
      log_info("Resource limits:");
      for (const auto& [resource, limit] : limits->values()) {
        log_info("  ", resource, ": ", limit);
      }
    }

    if (auto metadata = options.get<Metadata>("metadata")) {
      log_info("Metadata:");
      for (const auto& [k, v] : *metadata) {
        log_info("  ", k, ": ", v);
      }
    }

    return EXIT_SUCCESS;

  } catch (const std::exception& e) {
    log_error(e.what());
    return EXIT_FAILURE;
  }
}
