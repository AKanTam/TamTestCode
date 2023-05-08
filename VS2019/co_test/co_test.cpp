#include <iostream>
#include <co/all.h>
#include<httplib.h>

void cb(const http::Req &req, http::Res &res)
{
    if (req.url() == "/api/hello")
    {
        std::string body = req.body();
        go([body]
           {co::sleep(5000);std::cout<<"[co]"<<co::thread_id() <<"|"<<co::coroutine_id()<<"|" << " : " << body << "\n"; });
        res.set_status(200);
        res.set_body("hello world");
    }
    else
    {
        res.set_status(404);
    }
}

namespace test {
    DEF_test(test_name) {
        std::cout << "Hello World! \n";
    }

    DEF_test(httpServer) {
        try
        {
            http::Server srv;
            srv.on_req(cb);
            srv.start("0.0.0.0", 8000);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what();
        }

    }

    DEF_test(httpClient) {
        while (1) {
            //http::Client("127.0.0.1:8000").post("/api", "hello world!");
            httplib::Client("127.0.0.1:8000").Post("/api/hello", "hello world!","");

            co::sleep(10);
         }
    }
}

int main()
{
    flag::set_value("max_log_file_size", "104857600");

    
    unitest::run_all_tests();


    while (1)
        ;
}
