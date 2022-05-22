#ifndef SHARED_UTILS
#define SHARED_UTILS

#define OK  (0)
#define NOK (1)
#define NEGATIVE_ERR (-1)

#define CHECK_ERR(val, err, msg_ok, msg_err) ({(val == err) ? DBG_PRINT(msg_ok) : (perror(msg_err), exit(1));})
#define CHECK_NO_ERR(val, err, msg_ok, msg_err) ({(val != err) ? DBG_PRINT(msg_ok) : (perror(msg_err), exit(1));})

#define CHECK_SET_NO_ERR(check_val, err, _set, msg_err)   ({if(check_val != err) _set = check_val; else {perror(msg_err); exit(1);}})
#define CHECK_BREAK_ON_ERR(val, err) ({if(val == err) break;})
// #define CHECK_DO_OK()

#define DBG_PRINT_ENABLED
#if (defined(DBG_PRINT_ENABLED))
#define DBG_PRINT(...) printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

#endif /* SHARED_UTILS */
