#include <iostream>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, int id)
        : socket_(std::move(socket)), id_(id)
    {
    }

    void start()
    {
        std::cout << "connection established: " << id_ << std::endl;
        do_read();
    }
    ~session() {
        std::cout << "connection lost: " << id_ << std::endl;
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    do_write(length);
                }
            });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    do_read();
                }
            });
    }

    tcp::socket socket_;
    int id_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context), next_id_(1)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        auto id = next_id_;
        acceptor_.async_accept(socket_,
            [this, id](boost::system::error_code ec) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket_), id)->start();
                    next_id_++;
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    int next_id_;
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

