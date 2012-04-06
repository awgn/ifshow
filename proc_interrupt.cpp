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

#include <proc_interrupt.hpp>

namespace proc {

    std::vector<int>
    get_interrupt_counter(int irq)
    {
        std::ifstream proc_interrupt(proc::INTERRUPT);
        std::vector<int> ret;

        // skip the first line...
        //

        proc_interrupt >> more::ignore_line;

        more::string_token intstr(":");

        while( proc_interrupt >> intstr )
        {
            int intnum;

            try 
            {
                intnum = more::lexical_cast<int>(static_cast<std::string>(intstr));
            }
            catch(...)
            {
                proc_interrupt >> more::ignore_line;
                continue;
            }

            if (intnum != irq) {
                proc_interrupt >> more::ignore_line;
                continue;
            }

            int value;
            while( proc_interrupt >> value ) 
                 ret.push_back(value);
           
            break; 
        }
        
        return ret;
    }

} // namespace proc

#ifdef TEST_PROC

#include <algorithm>
#include <iterator>
#include <iostream>

int
main(int argc, char *argv[])
{
    std::vector<int> v = get_interrupt_counter(23);
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));

    return 0;
}

#endif
