// This client send TCP packet to server. Packet consist from header and body(serialized protofile)

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include "InfoPacket.pb.h"
#include "packedmessage.h"


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

    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), argv[1], argv[2]);
    boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    boost::asio::ip::tcp::socket s(io_service);
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
    
    data_buffer buf2;
    
    PackedMessage<Rdmp::InfoPacket> m_packed_request(boost::shared_ptr<Rdmp::InfoPacket>(new Rdmp::InfoPacket()));
    m_packed_request.get_msg()->set_id(protoId);
    m_packed_request.get_msg()->set_type(protoType);
    m_packed_request.get_msg()->set_data(protoData);
    int msg_size = m_packed_request.get_msg()->ByteSize();
    m_packed_request.encode_all_header(buf2,msg_size,sizeOfBlob,protoVersion);
    
    std::cout<<"Size of proto array = "<<msg_size<<std::endl;
    buf2.resize(ALL_HEADER_SIZE + msg_size);
    if(m_packed_request.get_msg()->SerializeToArray(&buf2[ALL_HEADER_SIZE], msg_size))
      std::cout << "SerializeToArray OK"<<std::endl;
    else
      std::cout << "SerializeToArray NOT OK"<<std::endl;
    std::cout << "Try to send Proto2: "<<std::endl;
    boost::asio::write(s, boost::asio::buffer(buf2,ALL_HEADER_SIZE + msg_size));
    
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
