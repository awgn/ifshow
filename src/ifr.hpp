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

#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>

#include <linux/ethtool.h>
#include <linux/version.h>
#include <linux/sockios.h>
#include <asm/types.h>

#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <memory>
#include <system_error>

#include <string-utils.hpp>
#include <iomanip.hpp>

#include <proc/files.hpp>

#include <macro.h>
#include <iwlib.h>
#include <ifaddrs.h>

namespace ifshow {

    class ifr
    {
    public:
        ifr(std::string name)
        : m_name(std::move(name))
        , m_ifreq_io()
        {
            strncpy(m_ifreq_io.ifr_name, m_name.c_str(), IFNAMSIZ);
        }

        ~ifr()
        {}

        short int
        flags() const
        {
            if (ioctl(ifr::sock_(), SIOCGIFFLAGS, &m_ifreq_io) < 0)
                throw std::system_error(errno, std::generic_category());

            return m_ifreq_io.ifr_flags;
        }

        std::string
        flags_str() const
        {
            const char *if_flags[] = {
                 "UP", 
                 "BROADCAST", 
                 "DEBUG", 
                 "LOOPBACK", 
                 "PTP", 
                 "NOTRLS", 
                 "RUNNING", 
                 "NOARP",
                 "PROMIS", 
                 "ALLMULTI", 
                 "MASTER", 
                 "SLAVE", 
                 "MULTICAST", 
                 "PORTSEL", 
                 "AUTOMEDIA",
                 "DYNAMIC", 
                 "LOWER_UP", 
                 "DORMANT", 
                 "ECHO", 
            };

            short int fl = flags();
            std::stringstream ret;

            for (int i=1; i <= 19; i++)
                if (fl & IF_FLAG_BIT(i))
                    ret << if_flags[i-1] << ' ';

            return ret.str();
        }

        /*
         * the function return an ethtool_drvinfo structure
         */

        std::unique_ptr<ethtool_drvinfo>
        ethtool_info() const
        {
            std::unique_ptr<ethtool_drvinfo> drvinfo(new ethtool_drvinfo);

            uint32_t req = ETHTOOL_GDRVINFO;	/* netdev ethcmd */

            m_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(drvinfo.get());
            memcpy(m_ifreq_io.ifr_data, (char *) &req, sizeof(req));

            if (ioctl(sock_(), SIOCETHTOOL, &m_ifreq_io) == -1) {
                throw std::system_error(errno, std::generic_category());
            }

            return drvinfo;
        }

        std::unique_ptr<ethtool_cmd>
        ethtool_command() const
        {
            std::unique_ptr<ethtool_cmd> ecmd(new ethtool_cmd);

            ecmd->cmd = ETHTOOL_GSET;
            m_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(ecmd.get());

            if (ioctl(sock_(), SIOCETHTOOL, &m_ifreq_io) == -1) {
                throw std::system_error(errno, std::generic_category());
            }

            return ecmd;
        }

        bool
        ethtool_link() const
        {
            struct ethtool_value edata;
            edata.cmd = ETHTOOL_GLINK;

            m_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(&edata);
            if (ioctl(sock_(), SIOCETHTOOL, &m_ifreq_io) == -1) {
                throw std::system_error(errno, std::generic_category());
            }

            return edata.data;
        }

        std::string
        mac() const
        {
            if (ioctl(sock_(), SIOCGIFHWADDR, &m_ifreq_io) == -1) {
                throw std::system_error(errno, std::generic_category());
            }
            struct ether_addr *eth_addr = (struct ether_addr *) & m_ifreq_io.ifr_addr.sa_data;
            return  ether_ntoa(eth_addr);
        }

        int
        mtu() const
        {
            if (ioctl(sock_(), SIOCGIFMTU, &m_ifreq_io) == -1 ) {
                throw std::system_error(errno, std::generic_category());
            }
            return m_ifreq_io.ifr_mtu;
        }


        int
        metric() const
        {
            if (ioctl(sock_(), SIOCGIFMETRIC, &m_ifreq_io) == -1 ) {
                throw std::system_error(errno, std::generic_category());
            }
            return m_ifreq_io.ifr_metric ? m_ifreq_io.ifr_metric : 1;
        }


        auto 
        inet_addr() const -> std::vector<std::tuple<std::string, std::string, int>>
        {
            std::vector<std::tuple<std::string, std::string, int>> ret;

            // get the list of network interfaces
            //
            struct ifaddrs *ifaddr, *ifa;
            if (getifaddrs(&ifaddr) < 0) {
                throw std::system_error(errno, std::generic_category());
            }

            // loop through the list of interfaces
            for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
                if (strcmp(ifa->ifa_name, m_name.c_str()) != 0) {
                    continue;
                }

                if (ifa->ifa_addr->sa_family == AF_INET) 
                {
                    char host[NI_MAXHOST], netmask[NI_MAXHOST];

                    if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {
                        throw std::system_error(errno, std::generic_category());
                    }

                    if (getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), netmask, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {
                        throw std::system_error(errno, std::generic_category());
                    }

                    // convert the mask to binary
                    uint32_t mask_bin = ntohl(reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask)->sin_addr.s_addr);
                    // count the number of consecutive 1's from the leftmost bit position
                    int prefix_len = 0;

                    while (mask_bin) {
                        prefix_len++;
                        mask_bin <<= 1;
                    }

                    ret.emplace_back(host, netmask, prefix_len);
                }
            }
            freeifaddrs(ifaddr);
            return ret;
        }

        auto
        inet6_addr() const -> std::vector<std::tuple<std::string, int, std::string>>
        {
            std::vector<std::tuple<std::string, int, std::string>> ret;

            char addr6p[8][5], devname[20], addr6[40];
            struct in6_addr in_addr6;
            int plen, scope, dad_status, if_idx;

            FILE *f;

            if ( (f=fopen(proc::IFINET6,"r")) == NULL) {
                return {};
            }

            while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
                          addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                          addr6p[4], addr6p[5], addr6p[6], addr6p[7],
                          &if_idx, &plen, &scope, &dad_status, devname) != EOF)
            {
                std::ostringstream ss;

                if ( strcmp(m_name.c_str(),devname) )
                    continue;

                sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                        addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                        addr6p[4], addr6p[5], addr6p[6], addr6p[7]);

                /* pretty host */
                inet_pton(PF_INET6,addr6,&in_addr6);
                inet_ntop(PF_INET6,&in_addr6, addr6, 40);

                switch (scope) {
                case 0:
                    ss << "global";
                    break;
                case IPV6_ADDR_LINKLOCAL:
                    ss << "link";
                    break;
                case IPV6_ADDR_SITELOCAL:
                    ss << "site";
                    break;
                case IPV6_ADDR_COMPATv4:
                    ss << "compat";
                    break;
                case IPV6_ADDR_LOOPBACK:
                    ss << "host";
                    break;
                default:
                    ss << "unknown";
                }
                
                ret.emplace_back(addr6, plen, ss.str());
            }

            fclose(f);
            return ret;
        }

        struct ifmap
        map() const
        {
            if (ioctl(sock_(), SIOCGIFMAP, &m_ifreq_io) == -1 ) {
                throw std::system_error(errno, std::generic_category());
            }
            return m_ifreq_io.ifr_ifru.ifru_map;
        }

        int
        txqueuelen() const
        {
            if (ioctl(sock_(), SIOCGIFTXQLEN, &m_ifreq_io) == -1 ) {
                throw std::system_error(errno, std::generic_category());
            }
            return m_ifreq_io.ifr_qlen;
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
                more::string_token if_name(":");
                ss >> if_name;

                std::string name = more::trim_copy(static_cast<std::string>(if_name));

                if ( name != m_name)
                    continue;

                int compressed, multicast;

                ss >> ret.rx_bytes >> ret.rx_packets >> ret.rx_errs >> ret.rx_drop >> ret.rx_fifo >> ret.rx_frame
                   >> compressed >> multicast;

                ss >> ret.tx_bytes >> ret.tx_packets >> ret.tx_errs >> ret.tx_drop >> ret.tx_fifo >> ret.tx_colls;
                return ret;
            }

            throw std::runtime_error("internal error");
        }


        wireless_info
        wifi_info() const
        {
            wireless_info info;
            memset(&info, 0, sizeof(wireless_info));

            if (iw_get_basic_config(sock_(), m_name.c_str(), &info.b) < 0)
                throw std::runtime_error("no wireless extension");

            struct iwreq wrq;
            if (iw_get_ext(sock_(), m_name.c_str(), SIOCGIWAP, &wrq) >= 0) {
                info.has_ap_addr = 1;
                memcpy(&(info.ap_addr), &(wrq.u.ap_addr), sizeof(sockaddr));
            }

            // get bit-rate
            if (iw_get_ext(sock_(), m_name.c_str(), SIOCGIWRATE, &wrq) >= 0) {
                info.has_bitrate = 1;
                memcpy(&(info.bitrate), &(wrq.u.bitrate), sizeof(iwparam));
            }

            return info;
        }

    private:
        static int
        sock_()
        {
            static int sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock == -1)
                throw std::system_error(errno, std::generic_category());
            return sock;
        }

        std::string m_name;

        mutable struct ifreq m_ifreq_io;

    };

} // namespace ifshow

