/**
 * @file fileSize.h
 * @brief File contains the header for the fileSize function.
 * @author Hannes Braun
 * @date 16.05.2019
 */

#ifndef fileSize_h
#define fileSize_h

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

int64_t calculateFileSize(char* pcFilePath);

#endif
