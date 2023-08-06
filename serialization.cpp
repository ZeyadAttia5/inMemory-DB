
#include "serialization.h"
#include <string>

/*
    packs a nil into the response string
    Package Format:
    ** SER_NIL *
    ___________________________
    i/p: res
    o/p: res
    return: void
*/
void res_ser_nil(std::string &res)
{
    res.push_back(SER_NIL);
}

/*
    packs an error into the response string
    Package Format:

    ** SER_ERR - Error Code - Length - Error Message *
    ___________________________
    i/p: res, err_code, err_msg
    o/p: res
    return: void
*/
void res_ser_err(std::string &res, int32_t err_code, const std::string &err_msg)
{
    res.push_back(SER_ERR);

    res.append((char *)err_code, 4); // 4 bytes

    /*
        client reads character by character,
        so we need to send the length of the string
        and send the string itself as a char array of length 4
    */
    uint32_t len = (uint32_t)err_msg.size();
    res.append((char *)&len, 4);

    res.append(err_msg);
}

/*
    packs a string into the response string
    Package Format:
    ** SER_STR - Length - String *
    ___________________________
    i/p: res, str
    o/p: res
    return: void
*/
void res_ser_str(std::string &res, std::string &str)
{
    res.push_back(SER_STR);

    /*
        client reads character by character,
        so we need to send the length of the string
        and send the string itself as a char array of length 4
    */
    uint32_t len = (uint32_t)str.size();
    res.append((char *)&len, 4);

    res.append(str);
}

/*
    packs an integer into the response string
    Package Format:
    ** SER_INT - Number *
    ___________________________
    i/p: res, num
    o/p: res
    return: void
*/
void res_ser_int(std::string &res, int64_t num)
{
    res.push_back(SER_INT);
    res.append((char *)num, 8);
}

/*
    packs an array into the response string
    Package Format:
    ** SER_ARR - Length *
    ___________________________
    i/p: res, len
    o/p: res
    return: void
*/
void res_ser_arr(std::string &res, int len)
{
    res.push_back(SER_ARR);
    res.append((uint32_t)len, 4);
}