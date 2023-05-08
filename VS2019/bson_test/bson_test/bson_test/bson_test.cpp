// bson_test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <bson/bson.h>
#include <httplib.h>

#include <time.h>

std::string getReqTime()
{
    time_t timep;
    time(&timep);
    char tmpTime[256];
    // strftime(tmpTime, sizeof(tmpTime), "%Y%m%d%H%M%S", localtime(&timep));
    strftime(tmpTime, sizeof(tmpTime), "[%Y-%m-%d %H:%M:%S]", localtime(&timep));
    std::string reqTime = tmpTime;
    return reqTime;
}

void ss(const httplib::Request &req, httplib::Response &res)
{

    bson_t *b_bson;
    bson_t *b_arr_doc = bson_new();
    bson_t *b_arr_obj = bson_new();
    bson_error_t _error;
    bson_iter_t iter_bson;
    bson_iter_t iter_array_document;
    bson_iter_t iter_array_object;

    std::string cname;
    std::string cval;
    std::string type;
    int cvalsize;

    const uint8_t *buf;
    uint32_t len;

    b_bson = bson_new_from_json((const uint8_t *)req.body.c_str(), (ssize_t)req.body.length(), &_error);
    bson_iter_init(&iter_bson, b_bson);
    while (bson_iter_next(&iter_bson))
    {
        const char *key = bson_iter_key(&iter_bson);
        if (strcmp(key, "cname") == 0)
        {
            cname = bson_iter_utf8(&iter_bson, NULL);
            std::cout << getReqTime() << cname << "\n";
        }
        else if (strcmp(key, "cval") == 0)
        {
            cval = bson_iter_utf8(&iter_bson, NULL);
            std::cout << getReqTime() << cval << "\n";
        }
        else if (strcmp(key, "cvalsize") == 0)
        {
            cvalsize = bson_iter_int32(&iter_bson);
            std::cout << getReqTime() << cvalsize << "\n";
        }
        else if (strcmp(key, "type") == 0)
        {
            type = bson_iter_utf8(&iter_bson, NULL);
            std::cout << getReqTime() << type << "\n";
        }
        else if (strcmp(key, "array") == 0)
        {

            if (BSON_ITER_HOLDS_ARRAY(&iter_bson))
            {
                bson_iter_array(&iter_bson, &len, &buf);
                bson_init_static(b_arr_doc, buf, len);

                bson_iter_init(&iter_array_document, b_arr_doc);

                while (bson_iter_next(&iter_array_document))
                {
                    if (BSON_ITER_HOLDS_DOCUMENT(&iter_array_document))
                    {
                        bson_iter_document(&iter_array_document, &len, &buf);
                        bson_init_static(b_arr_obj, buf, len);

                        bson_iter_init(&iter_array_object, b_arr_obj);

                        while (bson_iter_next(&iter_array_object))
                        {
                            const char *key = bson_iter_key(&iter_array_object);
                            if (strcmp(key, "cname") == 0)
                            {
                                cname = bson_iter_utf8(&iter_array_object, NULL);
                            }
                            else if (strcmp(key, "cval") == 0)
                            {
                                cval = bson_iter_utf8(&iter_array_object, NULL);
                            }
                            else if (strcmp(key, "cvalsize") == 0)
                            {
                                cvalsize = bson_iter_int32(&iter_array_object);
                            }
                            else if (strcmp(key, "type") == 0)
                            {
                                type = bson_iter_utf8(&iter_array_object, NULL);
                            }
                        }
                    }
                }
            }
            bson_destroy(b_arr_doc);
            bson_destroy(b_arr_obj);
        }
    }

    bson_t *b_rsp = bson_new();
    bson_append_int32(b_rsp, "cvalsize", -1, cvalsize);
    bson_append_utf8(b_rsp, "cname", -1, cname.c_str(), -1);
    bson_append_utf8(b_rsp, "cval", -1, cval.c_str(), -1);
    bson_append_utf8(b_rsp, "type", -1, type.c_str(), -1);
    char *str = bson_as_json(b_rsp, NULL);

    res.body = (str);

    bson_free(str);
    bson_destroy(b_rsp);
    bson_destroy(b_bson);
}

int main()
{
    httplib::Server svr;
    svr.Post("/api/test/bson", ss);
    svr.listen("0.0.0.0", 8888);
}
