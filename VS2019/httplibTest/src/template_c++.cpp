#include <iostream>
#include <co/all.h>
#include <httplib.h>
#include <thread>

httplib::Client* client;

int main()
{
    client = new httplib::Client("127.0.0.1",6666);
    client->set_connection_timeout(0, 20);
    client->set_keep_alive(true);

    while (1) {
        co::go([] {
            try
            {
                if (auto resPost = client->Post("/api/fep/data", "hello", "application/json"))
                {
                    std::cout << resPost->status << "Hello World!\n";
                }
                else {
                    auto err = resPost.error();
                    std::cout << time(0) << "  ]  " << std::this_thread::get_id() << ":  HTTP error: " << httplib::to_string(err) << std::endl;
                }
            }
            catch (const std::exception&e)
            {
                std::cout << e.what() << "\n";
            }
            catch (...) {
                std::cout << "exception&\n";
            }

                
            });
            Sleep(10);
    }
}

