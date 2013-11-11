// This client send TCP packet to server. Packet consist from header and body(serialized protofile)

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "InfoPacket.pb.h"
#include "packedmessage.h"

using boost::asio::ip::tcp;

enum { max_length = 1024 };

 void encode_all_header2(data_buffer& buf, unsigned infoSize, long long binarySize, unsigned versionSize) {
      buf.empty();
      // infoSize part of header
      buf.push_back((infoSize >> 24)& 0xFF);
      buf.push_back((infoSize >> 16)& 0xFF);
      buf.push_back((infoSize >> 8)& 0xFF);
      buf.push_back((infoSize & 0xFF));
      // binarySize part of header
      buf.push_back((binarySize >> 56)& 0xFF);
      buf.push_back((binarySize >> 48)& 0xFF);
      buf.push_back((binarySize >> 40)& 0xFF);
      buf.push_back((binarySize >> 32)& 0xFF);
      buf.push_back((binarySize >> 24)& 0xFF);
      buf.push_back((binarySize >> 16)& 0xFF);
      buf.push_back((binarySize >> 8)& 0xFF);
      buf.push_back((binarySize & 0xFF));
      // versionSize part of header
      buf.push_back((versionSize >> 8)& 0xFF);
      buf.push_back((versionSize & 0xFF));
    
    }


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

    // Create and fill our protofile
    int protoType, protoId, protoVersion;
    long long sizeOfBlob;
    std::string protoData;
    std::cout << "Fill protofile"<<std::endl;
    std::cout << "Enter type of proto"<<std::endl;
    std::cin>>protoType;
    std::cout << "Enter id of proto"<<std::endl;
    std::cin>>protoId;
    std::cout << "Enter data of proto"<<std::endl;
    std::cin>>protoData;
    std::cout << "Enter size of BLOB"<<std::endl;
    std::cin>>sizeOfBlob;
    std::cout << "Enter version of protocol"<<std::endl;
    std::cin>>protoVersion;
    
    Rdmp::InfoPacket* infoPckt = new Rdmp::InfoPacket();
    infoPckt->set_id(protoId);
    infoPckt->set_type(protoType);
    infoPckt->set_data(protoData);
    int msg_size = infoPckt->ByteSize();
    std::cout<<"Size of proto array = "<<msg_size<<std::endl;
    /*
    std::vector<int8_t> buffer;
    // Put info about size of proto into the header ( first 4 bytes)
    buffer.push_back((msg_size >> 24)& 0xF);
    buffer.push_back((msg_size >> 16)& 0xF);
    buffer.push_back((msg_size >> 8)& 0xF);
    buffer.push_back((msg_size & 0xFF));
    
      
    buffer.resize(ALL_HEADER_SIZE-1); // Некрасивый ход
    buffer.push_back( 0x05); // Добавим версию - 5
    std::cout << "Try to send Proto: "<<std::endl;
    */
    /*boost::asio::write(s, boost::asio::buffer(buffer,ALL_HEADER_SIZE));
    for(int i=0;i<buffer.size();i++){
	std::cout<<"buffer - "<<i<<" = "<< static_cast<unsigned>(buffer[i])<<std::endl;
      }
      */
    
    data_buffer buf2;
    encode_all_header2(buf2,msg_size,sizeOfBlob,protoVersion);
    buf2.resize(ALL_HEADER_SIZE + msg_size);
    if(infoPckt->SerializeToArray(&buf2[ALL_HEADER_SIZE], msg_size))
      std::cout << "SerializeToArray OK"<<std::endl;
    else
      std::cout << "SerializeToArray NOT OK"<<std::endl;
    std::cout << "Try to send Proto2: "<<std::endl;
    boost::asio::write(s, boost::asio::buffer(buf2,ALL_HEADER_SIZE + msg_size));
    //boost::asio::write(s, boost::asio::buffer(buf2,ALL_HEADER_SIZE));
    for(int i=0;i<buf2.size();i++){
	std::cout<<"buf2 - "<<i<<" = "<< static_cast<unsigned>(buf2[i])<<std::endl;
      }
    
    // Second call of writing - working too :)
    //boost::asio::write(s, boost::asio::buffer(buf2,ALL_HEADER_SIZE + msg_size));
    /* Reply - not used now 
    char reply[max_length];
    size_t reply_length = boost::asio::read(s, boost::asio::buffer(reply, request_length));
    std::cout << "Reply length = "<<reply_length<<std::endl;
    std::cout << "Reply is: ";
    std::cout.write(reply, reply_length);
    std::cout << "\n";
    */
    
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
