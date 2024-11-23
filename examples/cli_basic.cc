#include <cstdlib>

#include "cli/cli.h"
#include "ut/logger.h"

int main(int argc, char* argv[]) {
  try {
    cli::Options options;

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

    if (!options.parse(argc, argv)) {
      options.print_help(argv[0]);
      return 1;
    }

    if (options.get<bool>("verbose").value_or(false)) {
      log_info("Verbose mode enabled");
    }

    auto port = options.get<int>("port").value_or(8080);
    log_info("Port: ", port);

    auto host = options.get<std::string>("host").value_or("localhost");
    log_info("Host: ", host);

    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    log_error(e.what());
    return EXIT_FAILURE;
  }
}
