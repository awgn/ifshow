#include <proc_net_dev.h>
#include <iomanip.hh>

namespace proc {

    std::list<std::string>
    get_if_list()
    {
        std::ifstream proc_net_dev(proc::NET_DEV);
        std::list<std::string> ret;

        // skip the first 2 lines 
        //    
        proc_net_dev >> more::ignore_line >> more::ignore_line;

        more::token_string<':'> if_name;
        while (proc_net_dev >> if_name)
        {
            ret.push_back(trim(if_name));
            proc_net_dev >> more::ignore_line;
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
