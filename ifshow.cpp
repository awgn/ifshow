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

#include <colorful.hpp>      // more
#include <iomanip.hpp>       // more
#include <string-utils.hpp>  // more

#include <proc/net_wireless.hpp>
#include <proc/interrupt.hpp>
#include <proc/net_dev.hpp>

#include <ifr.hpp>
#include <net/if.h>

extern "C" {
#include <pci/pci.h>
}

extern char *__progname;
static const char * version = "2.0";

using namespace ifshow;


struct options
{
    std::vector<std::string>    if_list;
    std::vector<std::string>    driver;
    bool                        verbose;
    bool                        all;
};


typedef more::colorful< more::ecma::bold >          bold;
typedef more::colorful< more::ecma::reset >         reset;
typedef more::colorful< more::ecma::fg::red >       red;
typedef more::colorful< more::ecma::fg::blue>       blue;
typedef more::colorful< more::ecma::fg::yellow>     yellow; 
typedef more::colorful< more::ecma::fg::green>      green; 
typedef more::colorful< more::ecma::fg::cyan>       cyan; 
typedef more::colorful< more::ecma::fg::magenta>    magenta; 
typedef more::colorful< more::ecma::fg::light_grey> grey; 


template <typename CharT, typename Traits, typename Fun>
void pretty_print(std::basic_ostream<CharT, Traits> &out, size_t sp, Fun fun)
{
    out << more::spaces(sp);
    try
    {
        fun();
    }
    catch(std::exception &e)
    {
        out << "info: " << e.what() << " ";
    }
}


template <typename CharT, typename Traits, typename Fun>
void pretty_printLn(std::basic_ostream<CharT, Traits> &out, size_t sp, Fun fun)
{
    pretty_print(out,sp,fun); out << std::endl;
}


int
show_interfaces(const options &opts)
{
    auto ifs = proc::get_if_list();

    auto longest = std::max_element(ifs.begin(), ifs.end(), [](const std::string &lhs, const std::string &rhs) {
                                        return lhs.length() < rhs.length();
                                        }
                                    );

    size_t indent = longest->length() + 2;

    struct pci_access *pacc = pci_alloc();

    // initialize pci library...

    char name_path[] = "/usr/share/misc/pci.ids";

    pci_init(pacc);
    pci_scan_bus(pacc);
    pci_set_name_list_path(pacc, name_path, 0);

    // create a pci filter...
    struct pci_filter filter;
    pci_filter_init(pacc, &filter);

    // buffers for pci functions...
    //
    char pci_namebuf[1024], pci_classbuf[128];

    int devnum = 0;

    for(auto & name : proc::get_if_list())
    {
        try
        {
            // build the interface by name
            //
            ifshow::ifr iif(name);

            // in case the list is given, skip the interface if not included
            //
            if (!opts.if_list.empty() &&
                find(opts.if_list.begin(), opts.if_list.end(), name) == opts.if_list.end())
                continue;

            std::cout << reset();

            // display the interface when it's UP or -a is passed at command line
            //
            if (!opts.all && (iif.flags() & IFF_UP) == 0 && find(opts.if_list.begin(), opts.if_list.end(), name) == opts.if_list.end())
                continue;

            // get ether info...
            //
            //

            std::unique_ptr<ethtool_drvinfo> info;
            try
            {
                info = iif.ethtool_info();
            }
            catch(...)
            {
            }

            // driver filter...
            //

            if (!opts.driver.empty())
            {
                if (!info || std::all_of(std::begin(opts.driver), std::end(opts.driver), [&](const std::string &drv) -> bool
                                         {
                                            return strstr(info->driver, drv.c_str()) == nullptr;
                                         }))
                    continue;
            }

            if (devnum++) {
                std::cout << std::endl;
            }

            pretty_print(std::cout, 0, [&] {

                // display the interface name
                //
                std::cout << std::left << cyan() << std::setw(indent-1) << name << reset() << ' ' << std::flush;

                auto ecmd = iif.ethtool_command();
                if (iif.ethtool_link())
                std::cout << bold();

                // display Link-Speed (if supported)
                //
                if (ecmd)
                {
                    uint32_t speed = ethtool_cmd_speed(ecmd.get());

                    std::cout << "link " << (iif.ethtool_link() ? "yes " : "no ");

                    if (speed != 0 && speed != (uint16_t)(-1) && speed != (uint32_t)(-1))
                        std::cout << "speed " << speed << "Mb/s ";

                    // display half/full duplex...
                    std::cout << "duplex:";
                    switch(ecmd->duplex)
                    {
                    case DUPLEX_HALF:
                        std::cout << "half "; break;
                    case DUPLEX_FULL:
                        std::cout << "full "; break;
                    default:
                        std::cout << "unknown "; break;
                    }

                    // display port...
                    std::cout << "port:";
                    switch (ecmd->port) {
                    case PORT_TP:
                        std::cout << "twisted-pair "; break;
                    case PORT_AUI:
                        std::cout << "AUI "; break;
                    case PORT_BNC:
                        std::cout << "BNC "; break;
                    case PORT_MII:
                        std::cout << "MII "; break;
                    case PORT_FIBRE:
                        std::cout << "FIBRE "; break;
                    case PORT_DA:
                        std::cout << "direct-attach "; break;
                    case PORT_NONE:
                        std::cout << "none "; break;
                    case PORT_OTHER:
                        std::cout << "other "; break;
                    default:
                        std::cout << "unknown "; break;
                    }
                }
            });

            pretty_printLn(std::cout, 0, [&]
            {
                // display HWaddr
                //
                std::cout << "link " << yellow() << iif.mac() << reset();
            });

            std::cout << reset();

            // display wireless config if avaiable
            //

            pretty_printLn(std::cout, indent, [&]
            {
                char buffer[128];

                wireless_info winfo = iif.wifi_info();

                std::cout << winfo.b.name << " ESSID:" << winfo.b.essid << " mode:" <<
                             iw_operation_mode[winfo.b.mode] << " frequency:" << winfo.b.freq << std::endl << more::spaces(indent);

                if (winfo.has_bitrate)
                {
                    iw_print_bitrate(buffer,sizeof(buffer), winfo.bitrate.value);
                    std::cout << "bit-rate:" << buffer << ' ';
                }

                if (winfo.has_ap_addr)
                {
                    std::cout << "access point:" << iw_sawap_ntop(&winfo.ap_addr, buffer);
                }

            });

            // display wireless info, if available
            //
            auto wi = proc::get_wireless(name);
            if ( std::get<0>(wi) != 0.0 ||
                 std::get<1>(wi) != 0.0 ||
                 std::get<2>(wi) != 0.0 ||
                 std::get<3>(wi) != 0.0 )
            {

                pretty_printLn(std::cout, indent, [&]
                {
                    std::cout << "wifi status:" << std::get<0>(wi) <<
                               " link:" << std::get<1>(wi) << " level:" << std::get<2>(wi) <<
                               " noise:" << std::get<3>(wi);
                });
            }

            // display flags, mtu and metric
            //
            pretty_printLn(std::cout, indent, [&]
            {
                std::cout << bold() << iif.flags_str() << reset() << "MTU " << iif.mtu() << " metric " << iif.metric();
            });

            // display inet addr if set
            //
            for(auto const &[addr, netmask, prefix]  : iif.inet_addr())
            {
                pretty_printLn(std::cout, indent, [&]
                {
                    std::cout << "inet " << magenta() << addr << reset() << "/" << prefix;

                });
            }

            // display inet6 addr if set
            //
            for (auto &[addr6, plen, attr] : iif.inet6_addr())
            {
                pretty_printLn(std::cout, indent, [&] { std::cout << "inet6 " << blue() << addr6 << reset() << "/" << plen << " " << attr; });
            }

            if (opts.verbose)
            {
                pretty_printLn(std::cout, indent, [&]
                {
                    // display map info
                    //
                    ifmap m = iif.map();

                    std::cout << "if_index:" << if_nametoindex(name.c_str())
                               << std::hex << " base_addr:0x" << m.base_addr;

                    if (m.mem_start)
                        std::cout << " memory:0x" << m.mem_start << "-0x" << m.mem_end;

                    std::cout << std::dec << " irq:" << static_cast<int>(m.irq);

                    // display irq events per cpu
                    //
                    auto cpuint = proc::get_interrupt_counter(static_cast<int>(m.irq));
                    int n = 0;
                    for(auto ci : cpuint) {
                        if (ci > 0) {
                            std::cout << " cpu" << n++ << ":" << ci;
                        }
                    }

                    std::cout << " dma:" << static_cast<int>(m.dma)
                    << " port:"<< static_cast<int>(m.port) << std::dec;
                });

                pretty_printLn(std::cout, indent, [&]
                {
                    ifshow::ifr::stats s = iif.get_stats();
                    // display stats
                    //
                    std::cout << "Rx bytes:" << s.rx_bytes << " packets:" << s.rx_packets <<
                                 " errors:" << s.rx_errs << " dropped:" << s.rx_drop <<
                                 " overruns:" << s.rx_fifo << " frame:" << s.rx_frame << std::endl;

                    std::cout << more::spaces(indent) << "Tx bytes:" << s.tx_bytes << " packets:" << s.tx_packets <<
                               " errors:" << s.tx_errs << " dropped:" << s.tx_drop <<
                               " overruns:" << s.tx_fifo << std::endl;

                    std::cout << more::spaces(indent) << "colls:" << s.tx_colls << " txqueuelen:" << iif.txqueuelen();
                });

                // ... display drvinfo if available
                //

                if (info)
                {
                    pretty_printLn(std::cout, indent, [&]
                    {
                        char bus_info[33] = {0};
                       
                        strncpy(bus_info, info->bus_info, sizeof(bus_info)-1);

                        // set filter (and display additional pci info, if available)...
                        //
                        if (!pci_filter_parse_slot(&filter, bus_info))
                        {
                            struct pci_dev *dev = pacc->devices;
                            for(;dev; dev=dev->next)
                            {
                                pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS); /* Fill in header info we need */
                                if ( pci_filter_match(&filter, dev) )
                                   break;
                            }

                            if (dev) { // got it!!!

                                   char *pci_name, *pci_class;
                                   pci_class = pci_lookup_name(pacc, pci_classbuf, sizeof(pci_classbuf),
                                                               PCI_LOOKUP_CLASS, dev->device_class);
                                   pci_name = pci_lookup_name(pacc, pci_namebuf, sizeof(pci_namebuf),
                                                              PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);

                                   std::cout << grey() << std::hex << pci_class << ": " << pci_name << reset() << std::dec << std::endl
                                   << more::spaces(indent)  << "vendor_id:" << dev->vendor_id
                                   << " device_id:" << dev->device_id << " device_class:" << dev->device_class;
                            }
                        }
                    });
                }
            }

            if (info)
            {
                pretty_printLn(std::cout, indent, [&]
                {
                    // display ethertool_drivinfo...
                    //
                    std::cout << "driver:" << cyan() << info->driver << reset() << " version:" << info->version;
                    if (strlen(info->fw_version))
                        std::cout << " firmware:" << info->fw_version;
                    if (strlen(info->bus_info))
                        std::cout << " bus:" << red() << info->bus_info << reset();
                });
            }
        }
        catch(...)
        {

        }
    }

    pci_cleanup(pacc);
    std::cout << reset();
    return 0;
}


static
const char usage_str[] = "\
Usage:%s [options]\n\
  -a, --all            display all interfaces\n\
  -d, --driver NAME    filter by driver\n\
  -v, --verbose        \n\
  -V, --version        display the version and exit\n\
  -h, --help           print this help\n";


static const struct option long_options[] = {
    {"driver",   required_argument, NULL, 'd'},
    {"verbose",  no_argument, NULL, 'v'},
    {"version",  no_argument, NULL, 'V'},
    {"all",      no_argument, NULL, 'a'},
    {"help",     no_argument, NULL, 'h'},
    { NULL,      0          , NULL,  0 }};


int
main(int argc, char *argv[])
{
    options opts = { {} , {} , false, false };

    int i;
    while ((i = getopt_long(argc, argv, "hVvad:", long_options, 0)) != EOF)
        switch (i) {
        case 'h':
            printf(usage_str, __progname);
            exit(0);
        case 'V':
            fprintf(stderr, "%s %s\n", __progname ,version);
            exit(0);
        case 'a':
            opts.all=true;
            break;
        case 'v':
            opts.verbose=true;
            break;
        case 'd':
            opts.driver.push_back(optarg);
            break;
        case '?':
            throw std::runtime_error("unknown option");
        }

    argc -= optind;
    argv += optind;

    while( argc > 0 ) {
        opts.if_list.push_back(std::string(argv[0]));
        argc--;
        argv++;
    }

    return show_interfaces(opts);
}

