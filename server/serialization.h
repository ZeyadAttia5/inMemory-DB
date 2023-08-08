


#include <string>
/* 
    Serialization format: Type - Length - Value     (TLV)
    Type: 1 byte
    Length: 4 bytes
    Value: Length bytes

    Types:
        0: Nil
        1: Error
        2: String
        3: Int
        4: Array
    
*/

enum SER_TYPE{
    SER_NIL = 0,    // Like `NULL`
    SER_ERR = 1,    // An error code and message
    SER_STR = 2,    // string
    SER_INT = 3,    // int
    SER_ARR = 4,    // Array
};

void res_ser_nil(std::string &res);
void res_ser_err(std::string &res, int32_t err_code, const std::string &err_msg);
void res_ser_str(std::string &res, const std::string &str);
void res_ser_int(std::string &res, int64_t num);
void res_ser_arr(std::string &res, uint32_t len);


