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
//#include <boost/thread.hpp>
#include <thread>

class Serial {

private:
    char read_msg_[512];
    boost::asio::io_service io_;
    boost::asio::serial_port ser;
    std::string com{};
    uint32_t baud{9600};
    void handler(const boost::system::error_code& error, size_t bytes_transferred)
    {
        read_msg_[bytes_transferred] = 0;
        std::cout << bytes_transferred << " bytes: " << read_msg_ << std::endl;

        read_some();
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
        if(ser.is_open())
            ser.close();
    }

    Serial(const std::string dev_name,uint32_t baud_rate) :
        io_(), ser(io_), com{ dev_name }, baud{baud_rate}
    {
        /*
              port.set_option( boost::asio::serial_port_base::parity() );	// default none
              port.set_option( boost::asio::serial_port_base::character_size( 8 ) );
              port.set_option( boost::asio::serial_port_base::stop_bits() );	// default one
        */
        read_some();

        // run the IO service as a separate thread, so the main thread can do others
        std::thread t( boost::bind(&boost::asio::io_service::run, &io_) );
        t.detach();
    }
    auto start() -> int32_t {
        try
        {
            ser.open(com);
            ser.set_option( boost::asio::serial_port_base::baud_rate( baud ) );
        }
        catch (const std::exception&)
        {
            return 0;
        }
        return 1;
    }
    auto stop() -> int32_t {
        try
        {
            ser.cancel();
        }
        catch (const std::exception&)
        {
            return 0;
        }
        return 1;
    }
    void write(std::string msg)
    {
        ser.write_some(boost::asio::buffer(msg, msg.length()));
    }
};

/* serial <devicename> */

int main(/*int argc, char* argv[]*/)
{
    Serial s("COM10",9600);
    s.start();
    s.write("hello serial!!");
    // wait some
    Sleep(10);
    std::cin.get();
    s.stop();
    Sleep(10);
    s.start();
    s.write("Hello serial again!!\r\n");
    std::cin.get();
    s.stop();
    Sleep(10);
    s.start();
    s.write("Hello serial again3!!\r\n");
    std::cin.get();
    std::cin.get();

    s.stop();
    return 0;

}