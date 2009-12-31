#ifndef _PROC_INTERRUPT_H_
#define _PROC_INTERRUPT_H_ 

#include <token.hh>
#include <lexical_cast.hh>
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
