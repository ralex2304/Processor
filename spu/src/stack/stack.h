#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include <math.h>

#include "config.h"
#include "lib/utils/html.h"
#include "lib/utils/console.h"
#include "hash/crc32.h"
#include "log/log.h"
#include "lib/utils/ptr_valid.h"
#include "lib/utils/macros.h"

struct VarCodeData {
    const char* name = "Not specified";
    const char* file = "Not specified";
          int   line = -1;
    const char* func = "Not specified";
};

#define VAR_CODE_DATA(name) {# name, __FILE__, __LINE__, __func__}
#define VAR_CODE_DATA_PTR(name) {(# name) + 1, __FILE__, __LINE__, __func__}

/**
 * @brief Stack data struct
 */
struct Stack {
    static const ssize_t DEFAULT_CAPACITY = 1024;
    static const ssize_t UNITIALISED_CAPACITY = -1;
    static const Elem_t POISON; //< initialised in stack.cpp

    enum Results {
        OK                   = 0x00000,
        NOTHING_TO_POP       = 0x00001,
        ALLOC_ERR            = 0x00002,

        ALREADY_INITIALISED  = 0x00004,
        UNITIALISED          = 0x00008,
        DATA_INVALID_PTR     = 0x00010,
        POISON_VAL_FOUND     = 0x00020,
        NON_POISON_EMPTY     = 0x00040,
        STRUCT_HASH_ERR      = 0x00080,
        DATA_HASH_ERR        = 0x00100,
        LOW_CAPACITY         = 0x00200,
        NEGATIVE_CAPACITY    = 0x00400,
        INVALID_CAPACITY     = 0x00800,
        NEGATIVE_SIZE        = 0x01000,
        STRUCT_L_CANARY_ERR  = 0x02000,
        STRUCT_R_CANARY_ERR  = 0x04000,
        DATA_L_CANARY_ERR    = 0x08000,
        DATA_R_CANARY_ERR    = 0x10000,
    };

#ifdef CANARY_PROTECT
    Canary_t canary_left = CANARY_VAL;
#endif // #ifdef CANARY_PROTECT

    Elem_t* data = nullptr;
    ssize_t capacity = UNITIALISED_CAPACITY;
    ssize_t size = UNITIALISED_CAPACITY;

#ifdef HASH_PROTECT
    CRC32_t struct_hash = 0;
    CRC32_t data_hash   = 0;
#endif // #ifdef HASH_PROTECT

#ifdef DEBUG
    VarCodeData var_data = {};
#endif // #ifdef DEBUG

#ifdef CANARY_PROTECT
    Canary_t canary_right = CANARY_VAL;
#endif // #ifdef CANARY_PROTECT
};

/**
 * @brief Stack constructor
 * @attention Use with macros STK_CTOR or STK_CTOR_CAP
 *
 * @param stk
 * @param init_capacity
 * @return int
 */
int stk_ctor(Stack* stk, const size_t init_capacity = Stack::DEFAULT_CAPACITY);

/**
 * @brief Checks if stack is initialised
 *
 * @param stk
 * @return true
 * @return false
 */
inline bool stk_is_initialised(const Stack* stk) {
    return stk->size != stk->UNITIALISED_CAPACITY ||
           stk->capacity != stk->UNITIALISED_CAPACITY ||
           stk->data != nullptr;
}

#ifdef DEBUG
/**
 * @brief stk_ctor macros
 */
#define STK_CTOR(stk)   stk_ctor_debug(stk, VAR_CODE_DATA_PTR(stk))

/**
 * @brief stk_ctor macros with capacity
 */
#define STK_CTOR_CAP(stk, capacity)   stk_ctor_debug(stk, VAR_CODE_DATA_PTR(stk), capacity)

#else  // #ifndef DEBUG

/**
 * @brief stk_ctor macros
 */
#define STK_CTOR(stk)   stk_ctor(stk)

/**
 * @brief stk_ctor macros with capacity
 */
#define STK_CTOR_CAP(stk, capacity)   stk_ctor(stk, capacity)

#endif // #ifdef DEBUG



/**
 * @brief Pushes elem to stack
 *
 * @param stk
 * @param elem
 * @return int
 */
int stk_push(Stack* stk, const Elem_t elem);

/**
 * @brief Pops elem from stack
 *
 * @param stk
 * @param res
 * @return int
 */
int stk_pop(Stack* stk, Elem_t *const res);

/**
 * @brief Resizes stack
 *
 * @param stk
 * @param new_size
 * @return int
 */
int stk_resize(Stack* stk, const size_t new_size);

/**
 * @brief Calcs new stk size, when size is rising
 *
 * @param stk
 * @return ssize_t new stk capacity
 */
inline ssize_t stk_resize_calc_up(Stack* stk) {
    assert(stk);

    ssize_t new_cap = stk->capacity;

    while (stk->size >= new_cap)
        new_cap *= 2;

    return new_cap;
}

/**
 * @brief Calcs new stk size, when size is falling
 *
 * @param stk
 * @return ssize_t new stk capacity
 */
inline ssize_t stk_resize_calc_down(Stack* stk) {
    assert(stk);

    ssize_t new_cap = stk->capacity;

    if (new_cap >= (ssize_t)stk->DEFAULT_CAPACITY * 2 && stk->size * 4 <= new_cap)
        new_cap /= 2;

    return new_cap;
}

/**
 * @brief Prints errors to log file
 *
 * @param err_code
 */
void stk_print_error(const int err_code);

/**
 * @brief Prints errors to console
 *
 * @param err_code
 */
void stk_print_error_to_user(const int err_code);

/**
 * @brief Stack destructor
 *
 * @param stk
 * @return int
 */
int stk_dtor(Stack* stk);

/**
 * @brief Dumps stk.data and some information about it
 *
 * @param stk
 */
void stk_data_dump(const Stack* stk);

#ifdef DEBUG

    /**
     * @brief hidden ifdef macros for DEBUG
     */
    #define ON_DEBUG(...) __VA_ARGS__

    /**
     * @brief Stack constructor in debug mode (calls stk_ctor() inside)
     *
     * @param stk
     * @param var_data initialising place
     * @param min_capacity
     * @return int
     */
    int stk_ctor_debug(Stack* stk, const VarCodeData var_data,
                       const size_t min_capacity = Stack::DEFAULT_CAPACITY);

    /**
     * @brief Dumps all stack data to log
     *
     * @param stk
     * @param call_data
     */
    void stk_dump(const Stack* stk, const VarCodeData call_data);

    /**
     * @brief Verifies stack
     *
     * @param stk
     * @return int error code
     */
    int stk_verify(Stack* stk);

    /**
     * @brief Used in the beginning of stk_ function
     */
    #define STK_ASSERT(stk) \
        STK_VERIFY(stk);    \
        STK_OK(stk, res);   \
                            \
        if (res != stk->OK) \
            return res

    /**
     * @brief fills stk data array with poison
     */
    #define STK_FILL_POISON(stk, begin, end) \
                if (end > begin)             \
                    fill(stk->data + begin, end - begin, &stk->POISON, sizeof(Elem_t))

    /**
     * @brief stk_verify() enables in DEBUG mode
     */
    #define STK_VERIFY(stk) stk_verify(stk)

    /**
     * @brief calls stk_print_error() and STK_DUMP() if stack is not OK
     */
    #define STK_OK(stk, res)    do {                            \
                                    if (res != stk->OK) {       \
                                        stk_print_error(res);   \
                                        STK_DUMP(stk);          \
                                    }                           \
                                } while (0)

    /**
     * @brief stk_dump macros
     */
    #define STK_DUMP(stk)   stk_dump(stk, VAR_CODE_DATA())
#else // #ifndef DEBUG
    /**
     * @brief hidden ifdef macros for DEBUG
     */
    #define ON_DEBUG(...)

    /**
     * @brief Used in the beginning of stk_ function
     */
    #define STK_ASSERT(stk) Stack::OK

    /**
     * @brief disabled in debug mode
     */
    #define STK_FILL_POISON(stk, begin, end) void (0)

    /**
     * @brief stk_verify() returns OK in no DEBUG mode
     */
    #define STK_VERIFY(stk) Stack::OK;

    #define STK_OK(stk, res) void (0)

    #define STK_DUMP(stk) void (0)
#endif // #ifdef DEBUG

#ifdef HASH_PROTECT

/**
 * @brief hidden ifdef macros for HASH_PROTECT
 */
#define ON_HASH_PROTECT(...) __VA_ARGS__

/**
 * @brief Calculates hash for Stack struct
 *
 * @param stk
 * @return CRC32_t
 */
inline CRC32_t stk_struct_hash_calc(Stack* stk) {
    assert(stk);

    stk->struct_hash = 0;
    return crc_add(0, (char*)stk, sizeof(stk));
}

/**
 * @brief Calculates hash for stk.data
 *
 * @param stk
 * @return CRC32_t
 */
inline CRC32_t stk_data_hash_calc(Stack* stk) {
    assert(stk);

    return crc_add(0, (char*)stk->data, sizeof(Elem_t) * stk->size);
}

/**
 * @brief Addes one elemnt to stk.data hash
 *
 * @param stk
 * @param elem
 * @param crc
 * @return CRC32_t
 */
inline CRC32_t stk_data_hash_push(Stack* stk, Elem_t elem, CRC32_t crc) {
    assert(stk);

    return crc_add(crc, (char*)(&elem), sizeof(elem));
}
#else // #ifndef HASH_PROTECT

/**
 * @brief hidden ifdef macros for HASH_PROTECT
 */
#define ON_HASH_PROTECT(...)

#endif // #ifdef HASH_PROTECT

#ifdef CANARY_PROTECT

/**
 * @brief hidden ifdef macros for CANARY_PROTECT
 */
#define ON_CANARY_PROTECT(...) __VA_ARGS__

/**
 * @brief Returns pointer to stk.data left canary
 *
 * @param stk
 * @return Canary_t*
 */
inline Canary_t* left_data_canary_ptr(const Stack* stk) {
    assert(stk);
    assert(stk->data);

    return (Canary_t*)stk->data - 1;
}

/**
 * @brief Returns pointer to stk.data right canary
 *
 * @param stk
 * @return Canary_t*
 */
inline Canary_t* right_data_canary_ptr(const Stack* stk) {
    assert(stk);
    assert(stk->data);

    return (Canary_t*)((char*)stk->data + stk->capacity * sizeof(Elem_t)
                     + sizeof(Canary_t) - stk->capacity * sizeof(Elem_t) % sizeof(Canary_t));
}

#else  // #ifndef CANARY_PROTECT

/**
 * @brief hidden ifdef macros for CANARY_PROTECT
 */
#define ON_CANARY_PROTECT(...)

#endif // #ifdef CANARY_PROTECT



/**
 * @brief Calcs stk.data size in bytes
 *
 * @param len data array len
 * @return size_t
 */
inline size_t calc_stk_data_size(size_t len) {
    return len * sizeof(Elem_t)
           ON_CANARY_PROTECT(+ sizeof(Canary_t) - len * sizeof(Elem_t) % sizeof(Canary_t)
                             + 2 * sizeof(Canary_t));
}

#endif // #ifndef STACK_H_
