/*
 *  Copyright (c) 2004-2010 Bonelli Nicola <bonelli@antifork.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

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

#include <colorful.hh>      // more
#include <token.hh>         // more
#include <iomanip.hh>       // more
#include <string-utils.hh>  // more

#include <proc_net_wireless.h>
#include <proc_interrupt.h>
#include <proc_net_dev.h>
#include <ifr.h>        

extern char *__progname;
static const char * version = "1.0";

typedef more::colorful< TYPELIST(more::ecma::bold) > BOLD;
typedef more::colorful< TYPELIST(more::ecma::reset) > RESET;


struct comp_length : std::binary_function<bool, const std::string &, const std::string &>
{
    template <typename T>
    bool operator()(const T &lhs, const T &rhs) const
    { return lhs.length() < rhs.length(); }
};

void
show_interfaces(bool all, bool verbose, const std::list<std::string> &iflist = std::list<std::string>())
{
    std::list<std::string> ifs = proc::get_if_list();
    std::list<std::string>::const_iterator longest = std::max_element(ifs.begin(), ifs.end(), comp_length());

    size_t indent = longest->length() + 2;

    std::list<std::string>::const_iterator it = ifs.begin();
    std::list<std::string>::const_iterator it_end = ifs.end();

    for(int n=0; it != it_end; ++it) {

        std::cout << RESET();

        try 
        { 
            // build the interface by name
            //

            net::ifr iif(*it);

            // in case the list is given, skip the interface if not included
            //

            if (iflist.size() &&
                find(iflist.begin(), iflist.end(), *it) == iflist.end())
                continue;

            // display the interface when it's UP or -a is passed at command line
            //

            if (!all && (iif.flags() & IFF_UP) == 0 && find(iflist.begin(), iflist.end(), *it) == iflist.end())
                continue;

            // leave an empty line
            //
            if (n)
                std::cout << std::endl;

            // mii test
            //

            std::string mii_test = iif.mii();
            if (mii_test.find("ok") != std::string::npos) 
                std::cout << BOLD();

            // display the interface name
            //

            std::cout << std::left << std::setw(indent-1) << *it << ' ';    
            n++;

            // display HWaddr
            //

            std::cout << "HWaddr " << iif.mac() << std::endl;

            if ( mii_test.find("not supported") == std::string::npos || verbose)
                std::cout << more::spaces(indent) << "MII:" << mii_test << RESET() << std::endl;

            // display wireless config if avaiable
            //

            try {
                char buffer[128];

                wireless_info winfo = iif.wifi_info();
                std::cout << more::spaces(indent) << winfo.b.name << " ESSID:" << winfo.b.essid << " Mode:" << 
                    iw_operation_mode[winfo.b.mode] << " Frequency:" << winfo.b.freq << std::endl; 

                // print bit-rate 
                int s = 0;
                if (winfo.has_bitrate) {
                    if (!s++)
                        std::cout << more::spaces(indent);
                    iw_print_bitrate(buffer,sizeof(buffer), winfo.bitrate.value);
                    std::cout << "Bit-Rate:" << buffer << ' ';
                }

                if (winfo.has_ap_addr) {
                    if (!s++)
                        std::cout << more::spaces(indent);
                    std::cout << "Access Point: " << iw_sawap_ntop(&winfo.ap_addr, buffer);
                }
 
                if (s > 0)
                   std::cout << std::endl; 
            }
            catch(...) {
                if (verbose)
                    std::cout << more::spaces(indent) << "no wireless extensions." << std::endl;
            }
            // display wireless info, if available
            //

            std::tr1::tuple<double, double, double, double> wi = proc::get_wireless(*it);
            if ( std::tr1::get<0>(wi) != 0.0 ||
                 std::tr1::get<1>(wi) != 0.0 ||
                 std::tr1::get<2>(wi) != 0.0 ||
                 std::tr1::get<3>(wi) != 0.0 )
            {
                std::cout << more::spaces(indent) << "wifi_status:" << std::tr1::get<0>(wi) <<
                            " link:" << std::tr1::get<1>(wi) << " level:" << std::tr1::get<2>(wi) << 
                            " noise:" << std::tr1::get<3>(wi) << std::endl; 
            }

            // display flags, mtu and metric
            //

            std::cout << more::spaces(indent) << iif.flags_str() 
                      << "MTU:" << iif.mtu() << " Metric:" << iif.metric() << std::endl; 

            // display inet addr if set
            //

            std::string inet_addr = iif.inet_addr<SIOCGIFADDR>();
            if ( inet_addr.size() ) {
                std::cout << more::spaces(indent) << "inet addr:" << inet_addr  
                << " Bcast:" << iif.inet_addr<SIOCGIFBRDADDR>() 
                << " Mask:" << iif.inet_addr<SIOCGIFNETMASK>(); 

                std::cout << std::endl;
            }

            // display inet6 addr if set
            //

            std::string inet6_addr = iif.inet6_addr();
            if (inet6_addr.size())
                std::cout << more::spaces(indent) << inet6_addr << std::endl;

            // display map info
            //

            ifmap m = iif.map();
            std::cout << std::hex << more::spaces(indent) << "base_addr:0x" << m.base_addr;
            if (m.mem_start)
                std::cout << " memory:0x" << m.mem_start << "-0x" << m.mem_end;

            std::cout << std::dec << " irq:" << static_cast<int>(m.irq);

            // display irq events per cpu
            //

            std::vector<int> cpuint = proc::get_interrupt_counter(static_cast<int>(m.irq));
            for(unsigned int n = 0; n != cpuint.size(); ++n)
                std::cout << " cpu" << n << "=" << cpuint[n];

            std::cout << " dma:" << static_cast<int>(m.dma) 
                      << " port:"<< static_cast<int>(m.port) << std::dec << std::endl;

            // display stats
            //

            net::ifr::stats s = iif.get_stats();
            std::cout << more::spaces(indent) << "Rx bytes:" << s.rx_bytes << " packets:" << s.rx_packets << 
                         " errors:" << s.rx_errs << " dropped:" << s.rx_drop << 
                         " overruns: " << s.rx_fifo << " frame:" << s.rx_frame << std::endl;
 
            std::cout << more::spaces(indent) << "Tx bytes:" << s.tx_bytes << " packets:" << s.tx_packets << 
                         " errors:" << s.tx_errs << " dropped:" << s.tx_drop << 
                         " overruns: " << s.tx_fifo << " colls:" << s.tx_colls << " txqueuelen:" << iif.txqueuelen() << std::endl;
                                                 
            // ... display drvinfo if available
            //

            std::pair<bool, const ethtool_drvinfo *> info = iif.ether_info();
            if (info.first)  {
                std::cout << more::spaces(indent) << "ether_driver:" << info.second->driver << " version:" << info.second->version;  
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

