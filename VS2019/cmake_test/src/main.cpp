#include <iostream>
#include <co/all.h>

co::WaitGroup waitgroup;

void cb(const http::Req &req, http::Res &res)
{
    if (req.is_method_get())
    {
        if (req.url() == "/hello")
        {
            res.set_status(200);
            res.set_body("hello world");
        }
        else
        {
            res.set_status(404);
        }
    }
    else
    {
        res.set_status(405); // method not allowed
    }
}

int main()
{
    co::go([]
           { http::Server().on_req(cb).start("0.0.0.0", 8888); });

    co::go([]
           {
               http::Client c("127.0.0.1:8888");
               while (true)
               {
                co::go([&c]{
                   try
                   {
                       c.get("/hello");
                       if (c.status() == 200)
                       {
                           std::cout << "接口请求成功，结果为:" << c.body() << std::endl;
                       }
                   }
                   catch (const std::exception &e)
                   {
                       std::cerr << e.what() << '\n';
                   }
                   catch(...){
                    std::cout <<"捕获到未知异常\n";
                   }
                });
                   co::sleep(1);
               } });

    waitgroup.add();
    waitgroup.wait();
}