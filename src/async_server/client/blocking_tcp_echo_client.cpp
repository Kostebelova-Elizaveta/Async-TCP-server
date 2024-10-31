#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <string>
#include <cstring> 
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
namespace po=boost::program_options;

enum { max_length = 1024 };

std::mutex mtx;

std::string generateRandomString() {
    int len = 1 + rand() % 50;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
};

int connectOneThread(std::string host_, int port_, int count, int id) {
    try {
        boost::asio::io_context io_context;
    
        tcp::socket s(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(s, resolver.resolve(host_, std::to_string(port_)));
        
        for (int i=0; i<count; i++) {
            std::string generedMsg = generateRandomString();
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Genered and sent message for thread " << id <<  " is: " << generedMsg << std::endl;
            
            size_t request_length = generedMsg.size(); 
            char request[request_length + 1]; 
            strcpy(request, generedMsg.c_str()); 
            boost::asio::write(s, boost::asio::buffer(request, request_length));
        
            char reply[max_length];
            size_t reply_length = boost::asio::read(s,
                boost::asio::buffer(reply, request_length));
            std::cout << "Reply for thread " << id <<  " is: ";
            std::cout.write(reply, reply_length);
            std::cout << "\n";
        }
      }
      catch (std::exception& e) {
        std::cerr << "Thread " << id << " exception: " << e.what() << "\n";
        return 1;
      }
      return 0;
}

int main(int argc, char* argv[]) {

    po::options_description desc("How to use");
    int port_, loopNum, threadsNum;
    std::string host_;
    std::vector<std::thread> threads;
    desc.add_options()
    ("host,h", po::value<std::string>(&host_)->required(), "Enter host")
    ("port,p", po::value<int>(&port_)->required(), "Enter port")
    ("loop,l", po::value<int>(&loopNum)->required(), "Enter loopNum")
    ("thread,t", po::value<int>(&threadsNum)->required(), "Enter threadsNum")
    ;
    
    po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).run();
    po::store(parsed, vm);
    po::notify(vm);
    
    if ((host_.size() <= 0) || (port_ <= 999) || (loopNum <= 0) || (threadsNum <= 0)) throw std::runtime_error("Wrong args!");
    
    for (int i = 1; i <= threadsNum; i++) {
        threads.push_back(std::thread(connectOneThread, host_, port_, loopNum, i));
    }
    
    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    } 
    
  }
  catch(std::exception& ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    std::cout << desc << std::endl;
  }
    
  return 0;
}
