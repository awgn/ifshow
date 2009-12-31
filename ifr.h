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

namespace net { 

    class ifr
    { 
    public:
        ifr(const std::string &name)
        : _M_name(name)
        {}

        ~ifr()
        {}

        short int
        flags() const 
        {
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(), IFNAMSIZ);
            if (ioctl(ifr::_S_sock(), SIOCGIFFLAGS, &ifreq_io) < 0)
                throw std::runtime_error("SIOCGIFFLAGS");

            return ifreq_io.ifr_flags;
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

        std::pair<bool, const ethtool_drvinfo *> 
        ether_info()
        {
            struct ifreq ifr;
            uint32_t req = ETHTOOL_GDRVINFO;	/* netdev ethcmd */

            strncpy(ifr.ifr_name, _M_name.c_str(),IF_NAMESIZE);

            ifr.ifr_data = reinterpret_cast<__caddr_t>(& _M_drvinfo);
            strncpy(ifr.ifr_data, (char *) &req, sizeof(req));

            if (ioctl(_S_sock(), SIOCETHTOOL, &ifr) == -1) {
                errno = 0;
                return std::make_pair(false, &_M_drvinfo);
            } 
            
            return std::make_pair(true, &_M_drvinfo); 
        }

        /* SIOCGIFHWADDR */

        std::string
        mac() const 
        {
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(),IFNAMSIZ); 
            if (ioctl(_S_sock(), SIOCGIFHWADDR, &ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFHWADDR");
            }
            struct ether_addr *eth_addr = (struct ether_addr *) & ifreq_io.ifr_addr.sa_data;
            return  ether_ntoa(eth_addr);
        }

        int 
        mtu() const
        {
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(),IFNAMSIZ);
            if (ioctl(_S_sock(), SIOCGIFMTU, &ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMTU");
            }
            return ifreq_io.ifr_mtu;
        }

        int 
        metric() const
        {
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(), IFNAMSIZ);
            if (ioctl(_S_sock(), SIOCGIFMETRIC, &ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMETRIC");
            }
            return ifreq_io.ifr_metric ? ifreq_io.ifr_metric : 1;
        }


        template <int SIOC>
        std::string
        inet_addr() const
        {
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(), IFNAMSIZ);
            if (ioctl(_S_sock(), SIOC, &ifreq_io) != -1 ) {
                struct sockaddr_in *p = (struct sockaddr_in *)&ifreq_io.ifr_addr;

                if(ifreq_io.ifr_addr.sa_family == AF_INET) {
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
            struct ifreq ifreq_io;
            strncpy(ifreq_io.ifr_name, _M_name.c_str(), IFNAMSIZ);
            if (ioctl(_S_sock(), SIOCGIFMAP, &ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMAP");
            }
            return ifreq_io.ifr_ifru.ifru_map; 
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
        ethtool_drvinfo _M_drvinfo;
    };

} // namespace more

#endif /* _IFR_HH_ */
