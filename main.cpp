#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <array>
#define MAX_RCV_SIZE 1000

int main()
{
	//std::array<char, MAX_RCV_SIZE> rcv_buf;
	std::string message{"Hello from serial!!\r\n"};
	std::string com{"COM10"};	
	auto bud{ boost::asio::serial_port_base::baud_rate{ 9600 } };

	boost::asio::io_service io_{};
	boost::asio::serial_port ser{io_,com};
	ser.set_option(bud);
	//the settings of the serial set on 
	// stop bit 1, parity none, baud rate 9600, data 8bit , COM10
	std::thread{ [&] {io_.run(); } }.detach();
	
	ser.write_some(boost::asio::buffer(message.data(), message.length()));

	//std::cin.get();
	ser.close();
	return EXIT_SUCCESS;
}

