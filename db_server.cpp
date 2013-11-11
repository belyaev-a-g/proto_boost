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
//#include <boost/cstdint.hpp>
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
    //typedef boost::shared_ptr<stringdb::Request> RequestPointer;
    //typedef boost::shared_ptr<stringdb::Response> ResponsePointer;

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
    std::vector<int8_t> m_readbuf;
    //vector<uint8_t> m_readbuf;
    //PackedMessage<stringdb::Request> m_packed_request;
    PackedMessage<Rdmp::InfoPacket> m_packed_request;

    DbConnection(boost::asio::io_service& io_service, StringDatabase& db)
        : m_socket(io_service), m_db_ref(db),
        m_packed_request(boost::shared_ptr<Rdmp::InfoPacket>(new Rdmp::InfoPacket()))
    {
    }
    
    void handle_read_header(const boost::system::error_code& error)
    {
        DEBUG && (std::cerr << "handle read " << error.message() << '\n');
        if (!error) {
            DEBUG && (std::cerr << "Got header!\n");
            DEBUG && (std::cerr << show_hex(m_readbuf) << std::endl);
            //unsigned msg_len = m_packed_request.decode_header(m_readbuf);
            //DEBUG && (cerr << msg_len << " bytes\n");
            //start_read_body(msg_len);
            unsigned int proto_len = m_packed_request.print_all_header(m_readbuf);
	    std::cout<<"Length of Proto = "<<proto_len<<std::endl;
            start_read_proto_body(proto_len);
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
    
    void handle_read_proto(const boost::system::error_code& error)
    {
        DEBUG && (std::cerr << "handle read proto " << error << '\n');
        if (!error) {
            DEBUG && (std::cerr << "Got body!\n");
            DEBUG && (std::cerr << show_hex(m_readbuf) << std::endl);
	    // TODO - realize it in another protofile
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
	    
	    
            //handle_request(); 
            start_read_header();
        }
        else
	    std::cout<<"ERROR before Parse Proto "<<std::endl;
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


