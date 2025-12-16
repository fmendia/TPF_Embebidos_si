#include "user_db.h"

/* Data base example: modify / load from flash as you need */
UserEntry_t user_db[] = {
    { "12345678", "4321", 4, true },
    { "87654321", "11111", 5, true },
    { "00000001", "0000", 4, true }
};
const int USER_DB_COUNT = sizeof(user_db) / sizeof(user_db[0]);
