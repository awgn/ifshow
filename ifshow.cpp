#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>

#include <algorithm>
#include <iterator>
#include <iomanip>

#include <getopt.h>

#include <colorful.hh>  // more
#include <token.hh>     // more
#include <ifr.h>        // more
#include <string-utils.hh>

#include <proc_net_wireless.h>
#include <proc_interrupt.h>
#include <proc_net_dev.h>


extern char *__progname;
static const char * version = "1.0";

typedef more::colorful< TYPELIST(more::ecma::bold) > BOLD;
typedef more::colorful< TYPELIST(more::ecma::reset) > RESET;


void
show_interfaces(bool all, bool verbose, const std::list<std::string> &iflist = std::list<std::string>())
{
    std::list<std::string> ifs = proc::get_if_list();
    std::list<std::string>::const_iterator it = ifs.begin();

    size_t maxlen = 0;
    for(; it != ifs.end(); ++it)
        maxlen = std::max(maxlen, it->size());

    maxlen++;
    it = ifs.begin();

    for(int n=0; it != ifs.end(); ++it) {
        std::cout << RESET();

        try 
        { 
            net::ifr iif(*it);

            if (iflist.size() &&
                find(iflist.begin(), iflist.end(), *it) == iflist.end())
                continue;

            if (!all && (iif.flags() & IFF_UP) == 0 && find(iflist.begin(), iflist.end(), *it) == iflist.end())
                continue;

            // leave an empty line
            if (n)
                std::cout << std::endl;

            // mii test
            std::string mii_test = iif.mii();
            if (mii_test.find("ok") != std::string::npos)
                std::cout << BOLD();

            // display the interface name
            std::cout << std::left << std::setw(maxlen) << *it << ' ';    
            n++;

            std::cout << "MII:" << mii_test << RESET() << std::endl;

            // display wireless info
            std::tr1::tuple<double, double, double, double> wi = proc::get_wireless(*it);
            if ( std::tr1::get<0>(wi) != 0.0 ||
                 std::tr1::get<1>(wi) != 0.0 ||
                 std::tr1::get<2>(wi) != 0.0 ||
                 std::tr1::get<3>(wi) != 0.0 )
            {
                std::cout << std::string(maxlen+1,' ') << "wifi_status:" << std::tr1::get<0>(wi) <<
                            " link:" << std::tr1::get<1>(wi) << " level:" << std::tr1::get<2>(wi) << 
                            " noise:" << std::tr1::get<3>(wi) << std::endl; 
            }

            // display HWaddr
            std::cout << std::string(maxlen+1,' ') << "HWaddr " << iif.mac() << std::endl;

            // display flags, mtu and metric
            std::cout << std::string(maxlen+1,' ') << iif.flags_str() 
                      << "MTU:" << iif.mtu() << " Metric:" << iif.metric() << std::endl; 

            // display inet addr if set
            std::string inet_addr = iif.inet_addr<SIOCGIFADDR>();
            if ( inet_addr.size() ) {
                std::cout << std::string(maxlen+1,' ') << "inet addr:" << inet_addr  
                << " Bcast:" << iif.inet_addr<SIOCGIFBRDADDR>() 
                << " Mask:" << iif.inet_addr<SIOCGIFNETMASK>(); 

                std::cout << std::endl;
            }

            // display inet6 addr if set
            std::string inet6_addr = iif.inet6_addr();
            if (inet6_addr.size())
                std::cout << std::string(maxlen+1,' ') << inet6_addr << std::endl;

            // display map info
            ifmap m = iif.map();
            std::cout << std::hex << std::string(maxlen+1,' ') << "base_addr:0x" << m.base_addr;
            if (m.mem_start)
                std::cout << " memory:0x" << m.mem_start << "-0x" << m.mem_end;

            std::cout << std::dec << " irq:" << static_cast<int>(m.irq);

            // display irq events per cpu
            std::vector<int> cpuint = proc::get_interrupt_counter(static_cast<int>(m.irq));
            for(unsigned int n = 0; n != cpuint.size(); ++n)
                std::cout << " cpu" << n << "=" << cpuint[n];

            std::cout << " dma:" << static_cast<int>(m.dma) 
                      << " port:"<< static_cast<int>(m.port) << std::dec << std::endl;

            // display stats
            net::ifr::stats s = iif.get_stats();
            std::cout << std::string(maxlen+1,' ') << "Rx bytes:" << s.rx_bytes << " packets:" << s.rx_packets << 
                         " errors:" << s.rx_errs << " dropped:" << s.rx_drop << 
                         " overruns: " << s.rx_fifo << " frame:" << s.rx_frame << std::endl;
 
            std::cout << std::string(maxlen+1,' ') << "Tx bytes:" << s.tx_bytes << " packets:" << s.tx_packets << 
                         " errors:" << s.tx_errs << " dropped:" << s.tx_drop << 
                         " overruns: " << s.tx_fifo << " colls:" << s.tx_colls << std::endl;
                                                 
            // ... display drvinfo if available
            std::pair<bool, const ethtool_drvinfo *> info = iif.ether_info();
            if (info.first)  {
                std::cout << std::string(maxlen+1,' ') << "ether_driver:" << info.second->driver << " version:" << info.second->version;  
                if (strlen(info.second->fw_version))
                    std::cout << " firmware:" << info.second->fw_version; 
                if (strlen(info.second->bus_info))
                std::cout << " bus:" << info.second->bus_info;
                std::cout << std::endl;
            }

        }
        catch(std::exception &e)
        {
            if (verbose)
                std::cout << *it << ": " << e.what() << std::endl;
            continue;
        }
    }
        
    std::cout << RESET();
}


static 
const char usage_str[] = "\
Usage:%s [options]\n\
-a, --all            display all interfaces                   \n\
-v, --verbose        increase verbosity\n\
-V, --version        display the version and exit.            \n\
-h, --help           print this help.                         \n";


static const struct option long_options[] = {
    {"verbose",  no_argument, NULL, 'v'},
    {"version",  no_argument, NULL, 'V'},
    {"all",      no_argument, NULL, 'S'},
    {"help",     no_argument, NULL, 'h'},
    { NULL,      0          , NULL,  0 }};


int
main(int argc, char *argv[])
{

    int i;
    bool all = false;
    bool verb = false;

    while ((i = getopt_long(argc, argv, "hVva", long_options, 0)) != EOF)
        switch (i) {
        case 'h':
            printf(usage_str, __progname);
            exit(0);
        case 'v':
            verb = true;
            break;
        case 'V':
            fprintf(stderr, "%s %s\n", __progname ,version);
            exit(0);
        case 'a':
            all=true;
            break;
        case '?':
            throw std::runtime_error("unknown option"); 
        }

    argc -= optind;
    argv += optind;

    std::list<std::string> ifs;

    while( argc > 0 ) {
        ifs.push_back(std::string(argv[0]));

        argc--;
        argv++;
    }

    // std::copy(ifs.begin(), ifs.end(), std::ostream_iterator<std::string>(std::cout, " "));

    show_interfaces(all, verb, ifs);

    return 0;
}

