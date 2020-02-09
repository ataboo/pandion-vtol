#include <iostream>
#include "AtaLogger.h"

#define ATALOGGER_IOSTREAM

int main() {
    AtaLogger logger = AtaLogger();
    
    logger.info("TAG1", "Tag 1 works");
    logger.info("TAG2", "Tag 2 works");

    logger.setDefaultLogLevel(ERROR);

    logger.info("TAG1", "Tag 1 doesn't work");
    logger.info("TAG2", "Tag 2 doesn't work");

    logger.setTagLogLevel("TAG1", DEBUG);

    logger.info("TAG1", "Tag 1 works");
    logger.info("TAG2", "Tag 2 doesn't work");


}