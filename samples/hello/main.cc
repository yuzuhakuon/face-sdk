#include <future>
#include <iostream>
#include <optional>

#include "gawrs_face/common/logger.h"
#include "gawrs_face/common/version.h"

int main()
{
    std::cout << "Version: " << coreVersion << std::endl;
    std::cout << "Build Date: " << buildDate << std::endl;

    auto logger = Logger::instance().logger();
    logger->info("This is a info message.");
    logger->error("This is an error message.");
    logger->warn("This is a warning message.");
    logger->debug("This is a debug message.");
    logger->trace("This is a trace message.");
    logger->flush();

    return 0;
}