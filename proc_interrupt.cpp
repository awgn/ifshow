#include <proc_interrupt.h>

namespace proc {

    std::vector<int>
    get_interrupt_counter(int irq)
    {
        std::ifstream proc_interrupt(proc::INTERRUPT);
        std::vector<int> ret;

        std::string line;
        getline(proc_interrupt,line);

        while( getline(proc_interrupt,line)) {
            int intnum; more::token_string<':'> intstr;
            std::istringstream ss(line);

            ss >> intstr;

            try 
            {
                intnum = more::lexical_cast<int>(static_cast<std::string>(intstr));
            }
            catch(...)
            {
                continue;
            }

            if (intnum != irq)
                continue;

            int value;
            while( ss >> value )
                ret.push_back(value);
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
