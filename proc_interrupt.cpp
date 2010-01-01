#include <proc_interrupt.h>
#include <iomanip.hh>

namespace proc {

    std::vector<int>
    get_interrupt_counter(int irq)
    {
        std::ifstream proc_interrupt(proc::INTERRUPT);
        std::vector<int> ret;

        // skip the first line...
        //

        proc_interrupt >> more::ignore_line;

        more::token_string<':'> intstr;

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
