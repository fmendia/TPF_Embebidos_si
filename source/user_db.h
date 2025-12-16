#ifndef USER_DB_H
#define USER_DB_H

#include <stdint.h>
#include <stdbool.h>

/* Access to user DB entries (readonly) */
typedef struct {
    char id[9]; /* 8 digits + NUL */
    char pin[6]; /* up to 5 digits + NUL */
    uint8_t pin_len;
    bool valid;
} UserEntry_t;

extern UserEntry_t user_db[];
extern const int USER_DB_COUNT;

#endif /* USER_DB_H */
