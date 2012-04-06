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


#include <iomanip.hpp>

#include <proc_net_wireless.hpp>

namespace proc { 

    std::tr1::tuple<double, double, double, double>
    get_wireless(const std::string &wlan)
    {
        std::ifstream proc_net_wireless(proc::NET_WIRELESS);
        
        /* skip 2 lines */
        proc_net_wireless >> more::ignore_line >> more::ignore_line;

        more::string_token if_name(":");
        while (proc_net_wireless >> if_name) {

            std::string name = more::trim_copy(if_name.str());
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
