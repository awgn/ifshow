#include <proc_net_wireless.h>
#include <iomanip.hh>

namespace proc { 

    std::tr1::tuple<double, double, double, double>
    get_wireless(const std::string &wlan)
    {
        std::ifstream proc_net_wireless(proc::NET_WIRELESS);
        
        /* skip 2 lines */
        proc_net_wireless >> more::ignore_line >> more::ignore_line;

        more::token_string<':'> if_name;
        while (proc_net_wireless >> if_name) {

            std::string name = more::trim(if_name);
            if (name != wlan) {
                proc_net_wireless >> more::ignore_line;
               continue; 
            }

            double status, link, level, noise;
            proc_net_wireless >> status >> link >> level >> noise;

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
