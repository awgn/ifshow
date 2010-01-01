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

#ifndef _IFR_HH_
#define _IFR_HH_ 

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <asm/types.h>

#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cerrno>

#include <string-utils.hh> // more

#include <proc_files.h>
#include <macro.h>

#include <mii.h>
#include <iwlib.h>

namespace net { 

    class ifr
    { 
    public:
        ifr(const std::string &name)
        : _M_name(name),
          _M_ifreq_io()
          // _M_drvinfo()
        {
            strncpy(_M_ifreq_io.ifr_name, _M_name.c_str(), IFNAMSIZ);
        }

        ~ifr()
        {}

        short int
        flags() const 
        {
            if (ioctl(ifr::_S_sock(), SIOCGIFFLAGS, &_M_ifreq_io) < 0)
                throw std::runtime_error("SIOCGIFFLAGS");

            return _M_ifreq_io.ifr_flags;
        }

        std::string
        flags_str() const
        {
            const char *if_flags[]= {
                "UP", "BROADCAST", "DEBUG", "LOOPBACK", "PTP", "NOTRL", "RUNNING", "NOARP", 
                "PROMIS", "ALLMULTI", "MASTER", "SLAVE", "MULTICAST", "PORTSEL", "AUTOMEDIA" };

            short int fl = flags(); 
            std::stringstream ret;

            for (int i=1;i<16;i++) 
                if ( fl & IF_FLAG_BIT(i) ) 
                    ret << if_flags[i-1] << ' ';

            return ret.str(); 
        }

        /*
         * the function return an ethtool_drvinfo structure 
         */

        std::tr1::shared_ptr<ethtool_drvinfo>
        ethtool_info() const
        {
            std::tr1::shared_ptr<ethtool_drvinfo> drvinfo(new ethtool_drvinfo);

            uint32_t req = ETHTOOL_GDRVINFO;	/* netdev ethcmd */

            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(drvinfo.get());
            strncpy(_M_ifreq_io.ifr_data, (char *) &req, sizeof(req));

            if (ioctl(_S_sock(), SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            } 
           
            return drvinfo; 
        }

        std::tr1::shared_ptr<ethtool_cmd> 
        ethtool_command() const
        {
            std::tr1::shared_ptr<ethtool_cmd> ecmd(new ethtool_cmd);

            ecmd->cmd = ETHTOOL_GSET;
            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(ecmd.get());

            if (ioctl(_S_sock(), SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            } 
        
            return ecmd;   
        }

        bool
        ethtool_link() const 
        {
            struct ethtool_value edata;
            edata.cmd = ETHTOOL_GLINK;

            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(&edata);
            if (ioctl(_S_sock(), SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            }
            
            return edata.data;    
        }


        // get hwaddr
        //

        std::string
        mac() const 
        {
            if (ioctl(_S_sock(), SIOCGIFHWADDR, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFHWADDR");
            }
            struct ether_addr *eth_addr = (struct ether_addr *) & _M_ifreq_io.ifr_addr.sa_data;
            return  ether_ntoa(eth_addr);
        }

        int 
        mtu() const
        {
            if (ioctl(_S_sock(), SIOCGIFMTU, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMTU");
            }
            return _M_ifreq_io.ifr_mtu;
        }

        int 
        metric() const
        {
            if (ioctl(_S_sock(), SIOCGIFMETRIC, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMETRIC");
            }
            return _M_ifreq_io.ifr_metric ? _M_ifreq_io.ifr_metric : 1;
        }


        template <int SIOC>
        std::string
        inet_addr() const
        {
            if (ioctl(_S_sock(), SIOC, &_M_ifreq_io) != -1 ) {
                struct sockaddr_in *p = (struct sockaddr_in *)&_M_ifreq_io.ifr_addr;

                if(_M_ifreq_io.ifr_addr.sa_family == AF_INET) {
                    char dst[16];
                    return inet_ntop(AF_INET, reinterpret_cast<const void *>(&p->sin_addr), dst, sizeof(dst));
                }
            }
            return std::string();
        }

        std::string
        inet6_addr() const
        {
            char addr6p[8][5], devname[20], addr6[40];
            struct in6_addr in_addr6;
            int plen, scope, dad_status, if_idx;

            FILE *f;
            if ( (f=fopen(proc::IFINET6,"r")) == NULL)   
                throw std::runtime_error("PROC_IFNET6");
            
            while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                          addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                          addr6p[4], addr6p[5], addr6p[6], addr6p[7],
                          &if_idx, &plen, &scope, &dad_status, devname) != EOF) 
            {
                std::ostringstream ss;

                if ( strcmp(_M_name.c_str(),devname) )
                    continue;

                sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                        addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                        addr6p[4], addr6p[5], addr6p[6], addr6p[7]);

                /* pretty host */
                inet_pton(PF_INET6,addr6,&in_addr6);
                inet_ntop(PF_INET6,&in_addr6, addr6, 40);

                ss << "inet6 addr: " << addr6 << "/" << plen << " Scope:";
                switch (scope) {
                case 0:
                    ss << "Global";
                    break;
                case IPV6_ADDR_LINKLOCAL:
                    ss << "Link";
                    break;
                case IPV6_ADDR_SITELOCAL:
                    ss << "Site";
                    break;
                case IPV6_ADDR_COMPATv4:
                    ss << "Compat";
                    break;
                case IPV6_ADDR_LOOPBACK:
                    ss << "Host";
                    break;
                default:
                    ss << "Unknown";
                }

                fclose(f);
                return ss.str();
            }

            fclose(f);
            return std::string();
        }

        struct ifmap 
        map() const
        {
            if (ioctl(_S_sock(), SIOCGIFMAP, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMAP");
            }
            return _M_ifreq_io.ifr_ifru.ifru_map; 
        }

        int 
        txqueuelen() const
        {
            if (ioctl(_S_sock(), SIOCGIFTXQLEN, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCSIFTXQLEN");
            }
            return _M_ifreq_io.ifr_qlen; 
        }

        struct stats {
            unsigned int    rx_bytes;
            unsigned int    rx_packets;
            unsigned int    rx_errs;
            unsigned int    rx_drop;
            unsigned int    rx_fifo;
            unsigned int    rx_frame;

            unsigned int    tx_bytes;
            unsigned int    tx_packets;
            unsigned int    tx_errs;
            unsigned int    tx_drop;
            unsigned int    tx_fifo;
            unsigned int    tx_colls;
        };

        stats 
        get_stats() const
        {
            std::ifstream proc_net_dev(proc::NET_DEV);

            std::string line;

            /* skip 2 lines */
            getline(proc_net_dev,line);
            getline(proc_net_dev,line);

            stats ret;
            while (getline(proc_net_dev,line)) { 
                std::istringstream ss(line);
                more::token_string<':'> if_name;
                ss >> if_name;

                std::string name = more::trim(static_cast<std::string>(if_name));

                if ( name != _M_name)
                    continue;

                int compressed, multicast;

                ss >> ret.rx_bytes >> ret.rx_packets >> ret.rx_errs >> ret.rx_drop >> ret.rx_fifo >> ret.rx_frame 
                   >> compressed >> multicast; 

                ss >> ret.tx_bytes >> ret.tx_packets >> ret.tx_errs >> ret.tx_drop >> ret.tx_fifo >> ret.tx_colls;
                
                // sscanf(ss.str().c_str(), "%*u %u %u %u %u %u %*u %*u %*u %u %u %u %u %u", 
                //        &ret.rx_packets,
                //        &ret.rx_errs,
                //        &ret.rx_drop,
                //        &ret.rx_fifo,
                //        &ret.rx_frame,
                //        &ret.tx_packets,
                //        &ret.tx_errs,
                //        &ret.tx_drop,
                //        &ret.tx_fifo,
                //        &ret.tx_colls);
                
                return ret;
            }

            throw std::runtime_error("internal error");
            return stats();
        }

        std::string
        mii() const
        {
            return do_one_xcvr(_S_sock(), _M_name.c_str(),0);
        }

        wireless_info 
        wifi_info() const
        {
            wireless_info info;
            memset(&info, 0, sizeof(wireless_info));

            if (iw_get_basic_config(_S_sock(), _M_name.c_str(), &info.b) < 0)
                throw std::runtime_error("no wireless extension");

            struct iwreq wrq;
            if (iw_get_ext(_S_sock(), _M_name.c_str(), SIOCGIWAP, &wrq) >= 0) {
                info.has_ap_addr = 1;
                memcpy(&(info.ap_addr), &(wrq.u.ap_addr), sizeof(sockaddr));
            }

            // get bit-rate
            if (iw_get_ext(_S_sock(), _M_name.c_str(), SIOCGIWRATE, &wrq) >= 0) {
                info.has_bitrate = 1;
                memcpy(&(info.bitrate), &(wrq.u.bitrate), sizeof(iwparam));
            }

            return info;
        }

    private:

        static int 
        _S_sock()
        {
            static int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock == -1)
                throw std::runtime_error("socket");
            return sock;
        }

        std::string _M_name;

        mutable struct ifreq _M_ifreq_io;

        // ethtool_drvinfo _M_drvinfo;
    };

} // namespace more

#endif /* _IFR_HH_ */
