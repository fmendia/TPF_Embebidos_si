#include "database.h"
#include <string.h>

/* Database storage */
static DB_User_t user_db[] = {
    { "00000000", "0000", 4 , false,0},
    { "00000001", "0000", 4 , false,0},
    { "00000002", "0000", 4 , false,0},
    { "00000003", "0000", 4 , false,0},
    { "11111111", "0000", 5, false,1 },
    { "11111112", "0000", 5, false,1 },
    { "11111113", "0000", 5, false,1 },
    { "11111114", "0000", 5, false,1 },
    { "22222222", "0000", 4 , false,2},
    { "22222223", "0000", 4 , false,2},
    { "22222224", "0000", 4 , false,2},
    { "22222225", "0000", 4 , false,2},
	{ "33333333", "0000", 4 , false,3},
	{ "63663900", "0000", 4 , false,1},
	{ "42783600", "0000", 4 , false,2},
	{ "45947300", "0000", 4 , false,3},
	{ "60612683", "0000", 4 , false,1},
	{ "60612686", "0000", 4 , false,2},
    { DB_ADMIN_ID, DB_ADMIN_PIN, 4 , false, 2}
};

/*static DB_User_t user_db[] = {
    { "00000000", "0000", 4 , false,0},
    { "87654321", "11111", 5, false,0 },
    { "00000001", "0000", 4 , false,0},
	{ "63913050", "0000", 4 , false,0},
	{ "63663900", "0000", 4 , false,1},
	{ "42783600", "0000", 4 , false,2},
	{ "45947300", "0000", 4 , false,3},
	{ "60612683", "0000", 4 , false,1},
	{ "60612685", "0000", 4 , false,2},
    { DB_ADMIN_ID, DB_ADMIN_PIN, 4 , false, 2}
};*/

static const int DB_USER_COUNT = sizeof(user_db) / sizeof(user_db[0]);

/* ============ Implementation ============ */

void DB_Init(void)
{
   // DB_DeleteUser(2); // delete user at index 2
    //DB_DeleteUser(3); // delete user at index 3
}

int DB_FindUserById(const char *id)
{
    if (!id) return -1;

    for (int i = 0; i < DB_USER_COUNT; ++i) {
        /* Check if slot is not deleted (ID not empty) */
        if (user_db[i].id[0] != '\0') {
            if (strncmp(user_db[i].id, id, DB_ID_LENGTH) == 0) {
                return i;
            }
        }
    }
    return -1;
}

DB_User_t* DB_GetUserByIndex(int idx)
{
    if (idx < 0 || idx >= DB_USER_COUNT) {
        return NULL;
    }
    /* Check if slot is deleted */
    if (user_db[idx].id[0] == '\0') {
        return NULL;
    }
    return &user_db[idx];
}

uint8_t DB_GetUserPinLength(int user_idx)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return 0;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return 0;
    }
    return user_db[user_idx].pin_len;
}

bool DB_VerifyPin(int user_idx, const char *pin, uint8_t pin_len)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return false;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return false;
    }
    if (!pin || pin_len < 4 || pin_len > 5) {
        return false;
    }
    if (strlen(user_db[user_idx].pin) != pin_len) {
        return false;
    }

    return (strncmp(user_db[user_idx].pin, pin, pin_len) == 0);
}

bool DB_AddUser(const char *id, const char *pin, uint8_t pin_len, uint8_t user_floor)
{
    if (!id || !pin) {
        return false;
    }
    if (pin_len < 4 || pin_len > 5) {
        return false;
    }

    /* Check if ID already exists */
    if (DB_FindUserById(id) >= 0) {
        return false;
    }

    /* Find empty slot */
    int empty_idx = DB_FindEmptySlot();
    if (empty_idx < 0) {
        return false;  /* Database full */
    }

    /* Add user */
    strncpy(user_db[empty_idx].id, id, DB_ID_LENGTH);
    user_db[empty_idx].id[DB_ID_LENGTH] = '\0';
    strncpy(user_db[empty_idx].pin, pin, pin_len);
    user_db[empty_idx].pin[pin_len] = '\0';
    user_db[empty_idx].pin_len = pin_len;
    user_db[empty_idx].in_building = false;
    user_db[empty_idx].user_floor = user_floor;
    return true;
}

bool DB_DeleteUser(int user_idx)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return false;
    }

    /* Prevent deletion of admin user */
    if (DB_IsAdmin(user_idx)) {
        return false;
    }

    /* Check if already deleted */
    if (user_db[user_idx].id[0] == '\0') {
        return false;
    }

    /* Clear ID with nulls (8 nulls) */
    for (int i = 0; i < DB_ID_LENGTH; ++i) {
        user_db[user_idx].id[i] = '\0';
    }
    user_db[user_idx].id[DB_ID_LENGTH] = '\0';

    /* Optional: clear PIN as well for security */
    memset(user_db[user_idx].pin, 0, sizeof(user_db[user_idx].pin));
    user_db[user_idx].pin_len = 0;

    return true;
}

bool DB_IsAdmin(int user_idx)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return false;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return false;
    }

    return (strncmp(user_db[user_idx].id, DB_ADMIN_ID, DB_ID_LENGTH) == 0);
}

int DB_GetTotalSlots(void)
{
    return DB_USER_COUNT;
}

int DB_FindEmptySlot(void)
{
    for (int i = 0; i < DB_USER_COUNT; ++i) {
        if (user_db[i].id[0] == '\0') {
            return i;
        }
    }
    return -1;
}

int DB_ChangeUserPin(int user_idx, const char *new_pin, uint8_t new_pin_len)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return -1;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return -1;
    }
    if (!new_pin || new_pin_len < 4 || new_pin_len > 5) {
        return -1;
    }
    memset(user_db[user_idx].pin, 0, sizeof(user_db[user_idx].pin));   // deja todo en '\0'

    /* Update PIN */
    strncpy(user_db[user_idx].pin, new_pin, new_pin_len);
    user_db[user_idx].pin[new_pin_len] = '\0';
    user_db[user_idx].pin_len = new_pin_len;

    return 0;
}


void DB_SetUserInBuilding(int user_idx, bool in_building)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return;
    }
    user_db[user_idx].in_building = in_building;
}

bool DB_IsUserInBuilding(int user_idx)
{
    if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
        return false;
    }
    if (user_db[user_idx].id[0] == '\0') {
        return false;
    }
    return user_db[user_idx].in_building;
}

uint8_t DB_GetUserFloor(int user_idx)
{
	if (user_idx < 0 || user_idx >= DB_USER_COUNT) {
	        return -1;
	    }
	    if (user_db[user_idx].id[0] == '\0') {
	        return -1;
	    }
	    return user_db[user_idx].user_floor;
}

void DB_Reset(void)
{
    for (int i=0; i< DB_USER_COUNT; i++)
    {
        DB_SetUserInBuilding(i, false);
    }
}
