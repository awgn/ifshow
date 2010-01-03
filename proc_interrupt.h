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

#ifndef _PROC_INTERRUPT_H_
#define _PROC_INTERRUPT_H_ 

#include <lexical_cast.hh>  // more
#include <string-utils.hh>  // more

#include <proc_files.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace proc 
{
    extern std::vector<int> get_interrupt_counter(int irq);
}

#endif /* _PROC_INTERRUPT_H_ */
