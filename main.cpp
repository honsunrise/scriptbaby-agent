#include "pongo_connect.h"
#include "python_interpreter.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("compression", po::value<int>(), "set compression level")
            ("key,k", po::value<std::string>(), "user authentication key")
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("compression")) {
        std::cout << "Compression level was set to "
             << vm["compression"].as<int>() << ".\n";
    } else {
        std::cout << "Compression level was not set.\n";
    }

    if (vm.count("key")) {
        std::cout << "User authentication key was set to "
                  << vm["key"].as<std::string>() << ".\n";
    } else {
        std::cout << "User authentication key was not set.\n";
    }
    pongo_protocol &protocol = pongo_protocol::get_instance();
    protocol.set_key(vm["key"].as<std::string>());
    pongo_connect *connect = new pongo_connect(&protocol);
    connect->connect_to_manager();
    connect->main_loop();
    delete connect;
    return 0;
}