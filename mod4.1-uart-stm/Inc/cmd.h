#ifndef __CMD_H
#define __CMD_H

#include <stdbool.h>

// Executes the command received over UART. Returns true if the command was recognized and processed, false otherwise.
bool cmd_process(const char *cmd, size_t sz);

#endif /* __CMD_H */
