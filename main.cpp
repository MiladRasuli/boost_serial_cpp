/** BOOST ASIO SERIAL EXAMPLE

  boost::asio::serial_port
  boost::asio::serial_port::async_read_some

  **/

  /* compile with
    g++ -o serial serial.cpp -lboost_system -lboost_thread
    */

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <memory>

using rcv_callback_type = void(__stdcall*)(char*, uint32_t);
using err_callback_type = void(__stdcall*)(const char*);
class Serial {

private:
    char read_msg_[512];
    boost::asio::io_service io_;
    boost::asio::serial_port ser;
    std::thread thread_{};
    std::string com{};
    uint32_t baud{ 9600 };
    rcv_callback_type rcv_function = [](char*, uint32_t) -> void {};
    err_callback_type err_function = [](const char*) -> void {};
    void handler(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (errno) {

            err_function(error.message().data());
            return;
        }
        read_some();
        read_msg_[bytes_transferred] = 0;
        rcv_function(read_msg_, bytes_transferred);
    }
    void read_some()
    {
        ser.async_read_some(boost::asio::buffer(read_msg_, 512),
            boost::bind(&Serial::handler,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
public:
    ~Serial()
    {
        if (ser.is_open())
            ser.close();
        exit(0);
    }

    Serial(const std::string dev_name, uint32_t baud_rate) :
        io_{}, ser(io_), com{ dev_name }, baud{ baud_rate }
    {
        /*
              port.set_option( boost::asio::serial_port_base::parity() );	// default none
              port.set_option( boost::asio::serial_port_base::character_size( 8 ) );
              port.set_option( boost::asio::serial_port_base::stop_bits() );	// default one
        */
        read_some();

        // run the IO service as a separate thread, so the main thread can do others
        std::thread t([this] {io_.run(); });
        t.detach();
    }
    auto start() -> int32_t {
        try
        {
            ser.open(com);
            ser.set_option(boost::asio::serial_port_base::baud_rate(baud));
        }
        catch (const std::exception&err)
        {
            
            err_function(err.what());
            return 0;
        }
        return 1;
    }
    auto stop() -> int32_t {
        try
        {
            rcv_function = [](auto, auto)->void {};
            err_function = [](auto)->void {};
            ser.cancel();
            ser.close();
        }
        catch (const std::exception&err )
        {
            err_function(err.what());
            return 0;
        }
        return 1;
    }
    void write(std::string msg)
    {
        ser.write_some(boost::asio::buffer(msg, msg.length()));
    }
    auto on_receive(rcv_callback_type&& cb) ->void {
        rcv_function = std::move(cb);
    }
    auto on_error(err_callback_type&& cb) ->void {
        err_function = std::move(cb);
    }
};

/* serial <devicename> */
auto rcv_callback(char* msg, uint32_t len) ->void {
    std::cout << msg << "\t" << len << "\n";
}
/* serial <devicename> */
auto err_callback(const char* msg) ->void {
    std::cout << msg << "\n";
}

int main()
{
    //ser->write("hello serial!!");

    bool is_open{ false };
    //std::unique_ptr<Serial>ser{nullptr};
    //std::unique_ptr<boost::asio::io_service> io_{new };
    //boost::asio::io_service io_{};
    Serial ser{ "COM10",9600 };
    // ser.reset(new Serial(&io_, "COM10", 9600));
    std::string command{};
    do {
        std::cin >> command;

        if (command == "start") {
            std::cout << "start.\n";

            ser.on_receive(rcv_callback);
            ser.on_error(err_callback);
            ser.start();
            is_open = true;
        }
        else if (command == "stop") {
            if (!is_open) continue;
            std::cout << "stop.\n";
            //ser.on_receive([](auto,auto)->void {});
            //ser.on_error([](auto)->void {});
            ser.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
 
            is_open = false;
        }

        if (is_open == true)
        {
            ser.write(command);
        }

    } while (command != "close");

    return 0;
}