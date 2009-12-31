#ifndef _PROC_NET_DEV_H_
#define _PROC_NET_DEV_H_ 

#include <proc_files.h>

#include <fstream>
#include <sstream>
#include <string>
#include <list>

#include <token.hh>         // more
#include <string-utils.hh>  // more

namespace proc 
{
    extern std::list<std::string> get_if_list();
}

#endif /* _PROC_NET_DEV_H_ */
