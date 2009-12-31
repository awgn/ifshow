#include <proc_net_dev.h>

namespace proc {

    std::list<std::string>
    get_if_list()
    {
        std::ifstream proc_net_dev(proc::NET_DEV);
        std::list<std::string> ret;

        std::string line;

        /* skip 2 lines */
        getline(proc_net_dev,line);
        getline(proc_net_dev,line);

        while (getline(proc_net_dev,line)) { 
            std::istringstream ss(line);
            more::token_string<':'> if_name;
            ss >> if_name;
            ret.push_back(trim(if_name));
        }
        return ret;
    }
}

#ifdef TEST_PROC
#include <algorithm>
#include <iterator>
#include <iostream>

int
main(int argc, char *argv[])
{
    std::list<std::string> ret = proc::get_if_list();
    std::copy(ret.begin(), ret.end(), std::ostream_iterator<std::string>(std::cout, " "));
    return 0;
}

#endif
