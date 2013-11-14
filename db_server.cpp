//
// db_server.cpp: DbServer implementation
//
#include "db_server.h"
#include "packedmessage.h"
#include "InfoPacket.pb.h"
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#define DEBUG true


typedef std::map<std::string, std::string> StringDatabase;



// Database connection - handles a connection with a single client.
// Create only through the DbConnection::create factory.
//
class DbConnection : public boost::enable_shared_from_this<DbConnection>
{
public:
    typedef boost::shared_ptr<DbConnection> Pointer;

    static Pointer create(boost::asio::io_service& io_service, StringDatabase& db)
    {
        return Pointer(new DbConnection(io_service, db));
    }

    boost::asio::ip::tcp::socket& get_socket()
    {
        return m_socket;
    }

    void start()
    {
        start_read_header();
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    StringDatabase& m_db_ref;
    std::vector<uint8_t> m_readbuf;
    PackedMessage<Rdmp::InfoPacket> m_packed_request;
    PacketHeaderInfo currentPcktHeaderInfo;

    DbConnection(boost::asio::io_service& io_service, StringDatabase& db)
        : m_socket(io_service), m_db_ref(db),
        m_packed_request(boost::shared_ptr<Rdmp::InfoPacket>(new Rdmp::InfoPacket())), currentPcktHeaderInfo({0,0,0})
    {
    }
    
    void handle_read_header(const boost::system::error_code& error)
    {
        DEBUG && (std::cerr << "handle read_header error =  " << error.message() << '\n');
        if (!error) {
            DEBUG && (std::cerr << "Got header!\n");
            DEBUG && (std::cerr << show_hex(m_readbuf) << std::endl);
            //unsigned msg_len = m_packed_request.decode_header(m_readbuf);
            //DEBUG && (cerr << msg_len << " bytes\n");
            //start_read_body(msg_len);
	    //PacketHeaderInfo currentPcktHeaderInfo{0,0,0};
	    m_packed_request.getPacketHeaderInfo(m_readbuf, currentPcktHeaderInfo);
            start_read_proto_body(currentPcktHeaderInfo.protoSize);
        }
    }

    void handle_read_body(const boost::system::error_code& error)
    {
        DEBUG && (std::cerr << "handle body " << error << '\n');
        if (!error) {
            DEBUG && (std::cerr << "Got body!\n");
            DEBUG && (std::cerr << show_hex(m_readbuf) << std::endl);
	    // TODO - realize it in another protofile
            //handle_request(); 
            start_read_header();
        }
    }
    
     void handle_read_binary(const boost::system::error_code& error, uint32_t idPckt)
    {
      std::cout<<"Binary chunk dropped"<<std::endl;
      send_proto_confirmation( idPckt);
    }
    
  void handle_read_binaries_parts(const boost::system::error_code& error,
      uint32_t bytes_to_receive, uint32_t idPckt , uint32_t bytes_transferred)
  {
    if (!error)
    {
      std::cout<<"handle_read_binaries_parts"<<std::endl;
      std::cout<<"Обработчик сессии - bytes_to_receive = "<<bytes_to_receive<<std::endl;
      
      data_buffer bin_chunk2;
      //FIXME - magic number
      if(bytes_to_receive >= 1024 ) {
	m_socket.async_read_some(boost::asio::buffer(bin_chunk2, 1024),
	 boost::bind(&DbConnection::handle_read_binaries_parts, shared_from_this(),
	 boost::asio::placeholders::error,bytes_to_receive - 1024, idPckt, boost::asio::placeholders::bytes_transferred));
      }
      else {
	if(bytes_to_receive) {
	  m_socket.async_read_some(boost::asio::buffer(bin_chunk2, bytes_to_receive),
	 boost::bind(&DbConnection::handle_read_binaries_parts, shared_from_this(),
	 boost::asio::placeholders::error,0, idPckt, boost::asio::placeholders::bytes_transferred));
	}
      }
      
      if(bytes_to_receive == 0) {
	std::cout<<"Binary chunk dropped"<<std::endl;
	send_proto_confirmation( idPckt);
      }
    }
    else
    {
      std::cerr<<"ERROR!!! handle_read_binaries_parts "<<std::endl;
      std::cout<<"Error = "<<error.message();
      std::cerr<<"ERROR!!! handle_read_binaries_parts "<<std::endl;
      //delete this; // BUGFIX - i don't know need delete it or not!
    }
  }
 
   
  void handle_read_binaries_parts_async(const boost::system::error_code& error,
      uint32_t bytes_to_receive, uint32_t idPckt , uint32_t bytes_transferred)
  {
    if (!error)
    {
      std::cout<<"handle_read_binaries_parts_async"<<std::endl;
      std::cout<<"Обработчик сессии - bytes_to_receive = "<<bytes_to_receive<<std::endl;
      
      data_buffer bin_chunk2(1024);
      //bin_chunk2.resize(1024);
      //FIXME - magic number
      if(bytes_to_receive >= 1024 ) {
	boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&bin_chunk2[0], 1024);
	boost::asio::async_read(m_socket, buf,boost::bind(&DbConnection::handle_read_binaries_parts_async, shared_from_this(),
	 boost::asio::placeholders::error,bytes_to_receive - 1024, idPckt, boost::asio::placeholders::bytes_transferred));
      }
      else {	
	/*
	if(bytes_to_receive) {
	  bin_chunk2.resize(bytes_to_receive);
	boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&bin_chunk2[0], bytes_to_receive);
	boost::asio::async_read(m_socket, buf,boost::bind(&DbConnection::handle_read_binaries_parts_async, shared_from_this(),
	 boost::asio::placeholders::error,0, idPckt, boost::asio::placeholders::bytes_transferred));
	}
	*/
	if(bytes_to_receive) {
	  bin_chunk2.resize(bytes_to_receive);
	boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&bin_chunk2[0], bytes_to_receive);
	boost::asio::async_read(m_socket, buf,boost::bind(&DbConnection::send_proto_confirmation, shared_from_this(),
	idPckt));
	}
      }
     /* 
      if(bytes_to_receive == 0) {
	std::cout<<"Binary chunk dropped"<<std::endl;
	send_proto_confirmation( idPckt);
      }
      */
    }
    else
    {
      std::cerr<<"ERROR!!! handle_read_binaries_parts_async "<<std::endl;
      std::cout<<"Error = "<<error.message();
      std::cerr<<"ERROR!!! handle_read_binaries_parts_async "<<std::endl;
      //delete this; // BUGFIX - i don't know need delete it or not!
    }
  }
 
    
    void handle_read_proto(const boost::system::error_code& error)
    {
        DEBUG && (std::cerr << "handle read proto " << error << '\n');
        if (!error) {
            DEBUG && (std::cerr << "Got body!\n");
            DEBUG && (std::cerr << show_hex(m_readbuf) << std::endl);
	    std::cout<<"Parse Proto "<<std::endl;
	    // PARSE m_readbuf - to Proto
	    Rdmp::InfoPacket* infoPckt2 = new Rdmp::InfoPacket();
	    if(infoPckt2->ParseFromArray(&m_readbuf[ALL_HEADER_SIZE], m_readbuf.size() - ALL_HEADER_SIZE)) {
	      std::cout<<"Parse OK"<<std::endl;
	      std::cout<<"TYPE = "<<infoPckt2->type()<<std::endl;
	      std::cout<<"ID = "<<infoPckt2->id()<<std::endl;
	    }
	    else
	      std::cout<<"Parse NOT OK"<<std::endl;
	    
	    
	    if(currentPcktHeaderInfo.binarySize) {
	      //TODO - Read binary blob by parts ( 1024 ).
	      std::cout<<"Read binary chunk"<<std::endl;
	      /*
	      std::vector<int8_t> bin_chunk;
	      bin_chunk.resize(currentPcktHeaderInfo.binarySize);
	      boost::asio::async_read(m_socket, boost::asio::buffer(bin_chunk),
                boost::bind(&DbConnection::handle_read_binary, shared_from_this(),
                    boost::asio::placeholders::error, infoPckt2->id()));
                    */
	       ///*
	      boost::system::error_code ec2;
	      //handle_read_binaries_parts(ec2, currentPcktHeaderInfo.binarySize ,infoPckt2->id(), 0 );
	      handle_read_binaries_parts_async(ec2, currentPcktHeaderInfo.binarySize ,infoPckt2->id(), 0 );
	      //*/
	      
	    }
	    else {
	      send_proto_confirmation( infoPckt2->id());
	    }
        }
        else
	    std::cerr<<"ERROR before Parse Proto "<<std::endl;
	    std::cout<<"Error = "<<error.message()<<std::endl;
	    std::cerr<<"ERROR before Parse Proto "<<std::endl;
    } 

    // Called when enough data was read into m_readbuf for a complete request
    // message. 
    // Parse the request, execute it and send back a response.
    //
    /*
    void handle_request()
    {
        if (m_packed_request.unpack(m_readbuf)) {
            RequestPointer req = m_packed_request.get_msg();
            ResponsePointer resp = prepare_response(req);
            
            vector<uint8_t> writebuf;
            PackedMessage<stringdb::Response> resp_msg(resp);
            resp_msg.pack(writebuf);
            asio::write(m_socket, asio::buffer(writebuf));
        }
    }
    */

    void send_proto_confirmation(uint32_t idPckt)
    {
      std::cout<<" I answer for pckt with id = "<<idPckt<<std::endl;
      data_buffer confirmBuf;
      PackedMessage<Rdmp::InfoPacket> confirmPckt(boost::shared_ptr<Rdmp::InfoPacket>(new Rdmp::InfoPacket()));
      auto packResult = confirmPckt.packConfirm(confirmBuf,idPckt);
      std::cout<<" NOW wikk be crash "<<std::endl;
      auto result = boost::asio::write(m_socket, boost::asio::buffer(confirmBuf));
      std::cout<<"Result of sending = "<<result<<std::endl;
      start_read_header();
    } 
    
    void start_read_header()
    {
        m_readbuf.resize(ALL_HEADER_SIZE);
        boost::asio::async_read(m_socket, boost::asio::buffer(m_readbuf),
                boost::bind(&DbConnection::handle_read_header, shared_from_this(),
                    boost::asio::placeholders::error));
    }

    void start_read_body(unsigned msg_len)
    {
        // m_readbuf already contains the header in its first HEADER_SIZE
        // bytes. Expand it to fit in the body as well, and start async
        // read into the body.
        //
        m_readbuf.resize(ALL_HEADER_SIZE + msg_len);
        boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&m_readbuf[ALL_HEADER_SIZE], msg_len);
        boost::asio::async_read(m_socket, buf,
                boost::bind(&DbConnection::handle_read_body, shared_from_this(),
                    boost::asio::placeholders::error));
    }

    void start_read_proto_body(unsigned msg_len)
    {
        m_readbuf.resize(ALL_HEADER_SIZE + msg_len);
        boost::asio::mutable_buffers_1 buf = boost::asio::buffer(&m_readbuf[ALL_HEADER_SIZE], msg_len);
        boost::asio::async_read(m_socket, buf,
                boost::bind(&DbConnection::handle_read_proto, shared_from_this(),
                    boost::asio::placeholders::error));
    }

    /*
    ResponsePointer prepare_response(RequestPointer req)
    {
        string value;
        switch (req->type())
        {
            case stringdb::Request::GET_VALUE: 
            {
                StringDatabase::iterator i = m_db_ref.find(req->request_get_value().key());
                value = i == m_db_ref.end() ? "" : i->second;
                break; 
            }
            case stringdb::Request::SET_VALUE:
                value = req->request_set_value().value();
                m_db_ref[req->request_set_value().key()] = value;
                break;
            case stringdb::Request::COUNT_VALUES:
            {
                stringstream sstr;
                sstr << m_db_ref.size();
                value = sstr.str();
                break;
            }
            default:
                assert(0 && "Whoops, bad request!");
                break;
        }
        ResponsePointer resp(new stringdb::Response);
        resp->set_value(value);
        return resp;
    }
*/
};

struct DbServer::DbServerImpl
{
    boost::asio::ip::tcp::acceptor acceptor;
    StringDatabase db;

    DbServerImpl(boost::asio::io_service& io_service, unsigned port)
        : acceptor(io_service,boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port))
    {
        start_accept();
    }

    void start_accept()
    {
        // Create a new connection to handle a client. Passing a reference
        // to db to each connection poses no problem since the server is 
        // single-threaded.
        //
        DbConnection::Pointer new_connection = 
            DbConnection::create(acceptor.get_io_service(), db);

        // Asynchronously wait to accept a new client
        acceptor.async_accept(new_connection->get_socket(),
            boost::bind(&DbServerImpl::handle_accept, this, new_connection,
                boost::asio::placeholders::error));
    }

    void handle_accept(DbConnection::Pointer connection,
            const boost::system::error_code& error)
    {
        // A new client has connected
        //
        if (!error) {
            // Start the connection
            //
            connection->start();

            // Accept another client
            //
            start_accept();
        }
    }
};


DbServer::DbServer(boost::asio::io_service& io_service, unsigned port)
    : d(new DbServerImpl(io_service, port))
{
}


DbServer::~DbServer()
{
}


