#include "text_proccessor.h"

Status::Statuses InputData::ctor(const char* filename) {
    assert(filename);

    long file_size = 0;

    Status::Statuses res = file_open_read_close(filename, &text, 1, &file_size);
    if (res != Status::NORMAL_WORK)
        return res;

    assert(text);

    lines_cnt = cntlines(text, file_size);

    lines = (String*)calloc(lines_cnt + 1, sizeof(String));
    if (lines == nullptr) {
        printf("Memory allocation error!\n");
        return Status::MEMORY_EXCEED;
    }

    split_text_to_lines(text, lines);

    return Status::NORMAL_WORK;
}

void InputData::detor() {
    FREE(lines);
    FREE(text);
}
