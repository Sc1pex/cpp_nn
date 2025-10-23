#pragma once

#include "db.h"

struct State {
    State(std::string_view db_path) : db(db_path) {
    }

    DB db;
};
