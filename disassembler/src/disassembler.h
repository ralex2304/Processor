#ifndef DISASSEMBLER_H_
#define DISASSEMBLER_H_

#include "lib/utils/statuses.h"
#include "cmd.h"
#include "lib/utils/console.h"
#include "lib/file/file.h"

/**
 * @brief disassembles data to output file
 *
 * @param data
 * @param size
 * @param output_filename
 * @param debug_mode
 * @return Status::Statuses
 */
Status::Statuses disasm_parse(const char* data, const size_t size, const char* output_filename,
                              const bool debug_mode);

/**
 * @brief Writes header to file
 *
 * @param file
 * @param header
 * @return int
 */
int disasm_write_header(FILE* file, const FileHeader header);

#endif // #ifndef DISASSEMBLER_H_
