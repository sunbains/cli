#include <cstdlib>

#include "cli/cli.h"
#include "ut/logger.h"

int main(int argc, char* argv[]) {
  try {
    cli::Options options;
    using Ints = cli::Array_value<int>;
    using Strings = cli::Array_value<std::string>;
    using Limits = cli::Map_value<std::string, int>;
    using Metadata = cli::Map_value<std::string, std::string>;

    /* Array options */
    options.add_option<Strings>({
      .m_long_name = "hosts",
      .m_description = "List of host addresses",
      .m_default_value = Strings({"localhost:8080", "localhost:8081"}),
      .m_env_var = "APP_HOSTS"
    });

    options.add_option<Ints>({
      .m_long_name = "ports",
      .m_description = "List of ports",
      .m_default_value = Ints({8080, 8081, 8082})
    });

    /* Map options */
    options.add_option<Limits>({
      .m_long_name = "limits",
      .m_description = "Resource limits",
      .m_default_value = Limits({{"cpu", 4}, {"memory", 1024}, {"connections", 100}})
    });

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
    if (auto hosts = options.get<Strings>("hosts")) {
      log_info("Configured hosts:");
      for (const auto& host : *hosts) {
        log_info("  - ", host);
      }
    }

    if (auto ports = options.get<Ints>("ports")) {
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

    return EXIT_SUCCESS;

  } catch (const std::exception& e) {
    log_error(e.what());
    return EXIT_FAILURE;
  }
}
