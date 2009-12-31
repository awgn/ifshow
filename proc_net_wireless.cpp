#include <proc_net_wireless.h>

namespace proc { 

    std::tr1::tuple<double, double, double, double>
    get_wireless(const std::string &wlan)
    {
        std::ifstream proc_net_dev(proc::NET_WIRELESS);
        std::string line;

        /* skip 2 lines */
        getline(proc_net_dev,line);
        getline(proc_net_dev,line);

        while (getline(proc_net_dev,line)) { 
            std::istringstream ss(line);
            more::token_string<':'> if_name;
            ss >> if_name;
            
            std::string name = more::trim(if_name);
            if (name != wlan)
               continue; 

            double status, link, level, noise;
            ss >> status >> link >> level >> noise;

            return std::tr1::make_tuple(status,link,level,noise);
        }
        
        return std::tr1::make_tuple(0.0,0.0,0.0,0.0);
    }

} // namespace proc

#ifdef TEST_PROC
int
main(int argc, char *argv[])
{
    std::tr1::tuple<double, double, double, double> ret = proc::get_wireless("wlan0");

    std::cout << std::tr1::get<0>(ret) << ' ' << 
                 std::tr1::get<1>(ret) << ' ' <<
                 std::tr1::get<2>(ret) << ' ' <<
                 std::tr1::get<3>(ret) << std::endl;
    return 0;
}
#endif
