// This client send TCP packet to server. Packet consist from header and body(serialized protofile)

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <boost/asio.hpp>
#include "InfoPacket.pb.h"
#include "packedmessage.h"


enum { max_length = 1024 };

     




std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
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
    
    
    std::string string_with_photo = get_file_contents("blob_send.jpeg");
    sizeOfBlob = string_with_photo.size();
    
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
   /* 
    for(int i=0;i<buf2.size();i++){
	std::cout<<"buf2 - "<<i<<" = "<< static_cast<unsigned>(buf2[i])<<std::endl;
      }
     */ 
   /*
     if(sizeOfBlob) {
       std::cout<<"Send BLOB with size = "<<sizeOfBlob<<std::endl;
       char * blobData = new char[sizeOfBlob];
       memset(blobData,'q',sizeOfBlob);
       boost::system::error_code ec;
       size_t sendedLength = boost::asio::write(s, boost::asio::buffer(blobData, sizeOfBlob), boost::asio::transfer_all(), ec);
       std::cout<<"BLOB data sended =  "<<sendedLength<<std::endl;
     }
     */
    // New BLOB sending 
    if(sizeOfBlob) {
       std::cout<<"Send BLOB with size = "<<sizeOfBlob<<std::endl;
       //char * blobData = string_with_photo.c_str(); 
       //memset(blobData,'q',sizeOfBlob);
       boost::system::error_code ec;
       size_t sendedLength = boost::asio::write(s, boost::asio::buffer(string_with_photo.c_str(), sizeOfBlob), boost::asio::transfer_all(), ec);
       std::cout<<"BLOB data sended =  "<<sendedLength<<std::endl;
     } 
     
    // Second call of writing - working too :)
    //boost::asio::write(s, boost::asio::buffer(buf2,ALL_HEADER_SIZE + msg_size));
    
    
    //Time to read answer from server - our proto
    /* Reply - not used now */
    std::vector<uint8_t> m_readbuf;
    PacketHeaderInfo receivedPcktHeaderInfo;
    std::cout<<"We wait answer now"<<std::endl;
    char reply[max_length];
    m_readbuf.resize(ALL_HEADER_SIZE);
    size_t reply_length = boost::asio::read(s, boost::asio::buffer(m_readbuf, ALL_HEADER_SIZE));
    std::cout << "Reply length = "<<reply_length<<std::endl;
    PackedMessage<Rdmp::InfoPacket> receivedPckt(boost::shared_ptr<Rdmp::InfoPacket>(new Rdmp::InfoPacket()));
    receivedPckt.getPacketHeaderInfo(m_readbuf, receivedPcktHeaderInfo);
    // Read body of proto
    {
      std::vector<uint8_t> m_readproto;
       m_readproto.resize(receivedPcktHeaderInfo.protoSize);
       size_t reply_lengthi2 = boost::asio::read(s, boost::asio::buffer(m_readproto, receivedPcktHeaderInfo.protoSize));
       std::cout << "Reply proto = "<<reply_lengthi2<<std::endl;
       std::cout << "m_readproto.size  = "<<m_readproto.size()<<std::endl;
       std::cout<<"Parse Proto "<<std::endl;
	    // PARSE m_readbuf - to Proto
	    Rdmp::InfoPacket* infoPckt2 = new Rdmp::InfoPacket();
	    if(infoPckt2->ParseFromArray(&m_readproto[0], m_readproto.size())) {
	      std::cout<<"Parse OK"<<std::endl;
	      std::cout<<"TYPE = "<<infoPckt2->type()<<std::endl;
	      std::cout<<"ID = "<<infoPckt2->id()<<std::endl;
	    }
	    else
	      std::cout<<"Parse NOT OK"<<std::endl;
	    
    }
    
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
