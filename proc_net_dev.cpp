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

#include <proc_net_dev.h>
#include <iomanip.hh>
#include <string-utils.hh>

namespace proc {

    std::list<std::string>
    get_if_list()
    {
        std::ifstream proc_net_dev(proc::NET_DEV);
        std::list<std::string> ret;

        // skip the first 2 lines 
        //    
        proc_net_dev >> more::ignore_line >> more::ignore_line;

        more::string_token if_name(":");
        while (proc_net_dev >> if_name)
        {
            ret.push_back(more::trim_copy(if_name.str()));
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
