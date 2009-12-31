#ifndef _PROC_NET_WIRELESS_H_
#define _PROC_NET_WIRELESS_H_ 

#include <proc_files.h>

#include <tr1/tuple>
#include <fstream>
#include <sstream>
#include <string>
#include <list>

#include <token.hh>         // more
#include <string-utils.hh>  // more

namespace proc 
{
    extern std::tr1::tuple<double, double, double, double> 
    get_wireless(const std::string &);
}


#endif /* _PROC_NET_WIRELESS_H_ */
