#include "cli/cli.h"
#include "libudp/logger.h"

int main(int argc, char* argv[]) {
  try {
    cli::Options options;

    options.add_option<bool>({
      .short_name = "v",
      .long_name = "verbose",
      .description = "Enable verbose output",
      .default_value = "false"
    });

    options.add_option<int>({
      .short_name = "p",
      .long_name = "port",
      .description = "Port number",
      .required = true,
      .default_value = "8080",
      .env_var = "APP_PORT"
    });

    options.add_option<std::string>({
      .short_name = "h",
      .long_name = "host",
      .description = "Host address",
      .default_value = "localhost"
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

    return 0;
  } catch (const std::exception& e) {
    log_error(e.what());
    return 1;
  }
}
