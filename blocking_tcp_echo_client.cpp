
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "InfoPacket.pb.h"
#include "packedmessage.h"

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), argv[1], argv[2]);
    tcp::resolver::iterator iterator = resolver.resolve(query);

    tcp::socket s(io_service);
    boost::asio::connect(s, iterator);

    using namespace std; // For strlen.
    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = strlen(request);
    boost::asio::write(s, boost::asio::buffer(request, request_length));
/*
    char reply[max_length];
    size_t reply_length = boost::asio::read(s,
        boost::asio::buffer(reply, request_length));
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
  */  
    std::cout<<"Введите длину передаваемого сообщения"<<std::endl;
    int test_length;
    std::cin>>test_length;
    char * big_data = new char[test_length];
    memset(big_data,'q',test_length);
    //size_t sended_length = boost::asio::write(s, boost::asio::buffer(big_data, test_length)); 
    //std::cout << "sended_length = "<<sended_length<<std::endl;
    
    
    std::cout<<"Попробуем передать сообщение с разбивкой по 60.000 знаков"<<std::endl;
    int counti = 0;
    for(int tmpLength = 0 ; tmpLength + 1024 <test_length; counti++) {
        //size_t sended_length = boost::asio::write(s, boost::asio::buffer(big_data + tmpLength, 60000)); 
      boost::system::error_code ec;
        size_t sended_length = boost::asio::write(s, boost::asio::buffer(big_data + tmpLength, 1024),boost::asio::transfer_all(),ec); 
	std::cout << "sended_length = "<<1024<<" counti = "<<counti<<"  tmpLength = "<<tmpLength<<std::endl;
	tmpLength+=1024;
    }
    std::cout<<"Закончили передавать"<<std::endl;
    
    test_length = boost::asio::read(s, boost::asio::buffer(big_data, test_length));
    std::cout << "Length = "<<test_length<<std::endl;
    std::cout << "Ответ is: ";
    //std::cout.write(big_data, test_length);
    std::cout << "\n";
    std::cout << "Length = "<<test_length<<std::endl;
  }
  catch (std::exception& e)
  {
    std::cout<<"Exception - ufff"<<std::endl;
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
