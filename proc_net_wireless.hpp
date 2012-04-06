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

#ifndef _PROC_NET_WIRELESS_H_
#define _PROC_NET_WIRELESS_H_ 

#include <tuple>
#include <fstream>
#include <sstream>
#include <string>
#include <list>

#include <string-utils.hpp>  // more
#include <proc_files.hpp>

namespace proc 
{
    extern std::tuple<double, double, double, double> 
    get_wireless(const std::string &);
}


#endif /* _PROC_NET_WIRELESS_H_ */
