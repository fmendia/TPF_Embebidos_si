#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>
#include <stdbool.h>

/* Constants */
#define DB_ID_LENGTH        8
#define DB_PIN_MAX_LENGTH   5
#define DB_ADMIN_ID         "12121212"
#define DB_ADMIN_PIN        "1234"

/* User entry structure */
typedef struct {
    char id[DB_ID_LENGTH + 1];      /* 8 chars + null terminator */
    char pin[DB_PIN_MAX_LENGTH + 1]; /* up to 5 digits + null */
    uint8_t pin_len;                 /* actual PIN length (4 or 5) */
    bool in_building;              /* whether user is currently in building */
    uint8_t user_floor;
} DB_User_t;

/* ============ Database Functions ============ */

/**
 * @brief Initialize database with default users
 */
void DB_Init(void);

/**
 * @brief Find user by ID
 * @param id User ID (8 characters)
 * @return User index (0+) if found, -1 if not found or deleted
 */
int DB_FindUserById(const char *id);

/**
 * @brief Get user entry by index
 * @param idx User index
 * @return Pointer to user entry, or NULL if invalid index
 */
DB_User_t* DB_GetUserByIndex(int idx);

/**
 * @brief Get PIN length for user
 * @param user_idx User index
 * @return PIN length (4 or 5), or 0 if invalid
 */
uint8_t DB_GetUserPinLength(int user_idx);

/**
 * @brief Verify user PIN
 * @param user_idx User index
 * @param pin PIN to verify
 * @param pin_len PIN length
 * @return true if correct, false otherwise
 */
bool DB_VerifyPin(int user_idx, const char *pin, uint8_t pin_len);

/**
 * @brief Add new user to database
 * @param id User ID (8 characters)
 * @param pin PIN (4 or 5 digits)
 * @param pin_len PIN length
 * @return true if success, false if ID exists or DB full
 */
bool DB_AddUser(const char *id, const char *pin, uint8_t pin_len);

/**
 * @brief Delete user by index (clears ID with nulls)
 * @param user_idx User index
 * @return true if success, false if invalid or admin user
 */
bool DB_DeleteUser(int user_idx);

/**
 * @brief Check if user is admin
 * @param user_idx User index
 * @return true if admin user, false otherwise
 */
bool DB_IsAdmin(int user_idx);

/**
 * @brief Get total user count (including deleted slots)
 * @return Total slots in database
 */
int DB_GetTotalSlots(void);

/**
 * @brief Find first empty slot in database
 * @return Slot index if found, -1 if DB full
 */
int DB_FindEmptySlot(void);


int DB_ChangeUserPin(int user_idx, const char *new_pin, uint8_t new_pin_len);

/**
 * @brief Set user's in_building status
 * @param user_idx User index
 * @param in_building true if user is in building, false otherwise
 */
void DB_SetUserInBuilding(int user_idx, bool in_building);

/**
 * @brief Check if user is currently in the building
 * @param user_idx User index
 * @return true if in building, false otherwise
 */
bool DB_IsUserInBuilding(int user_idx);

uint8_t DB_GetUserFloor(int user_idx);
#endif /* DATABASE_H */
