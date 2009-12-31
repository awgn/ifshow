/* $Id: token.hh 206 2009-09-01 16:07:47Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_ 

#include <string>
#include <iostream>

namespace more { 

    // used as istream parser...

    template <char S>
    class token_string
    {
    public:
        token_string()
        : _M_value()
        {}

        token_string(const std::string &value)
        : _M_value(value)
        {}

        ~token_string()
        {}

        friend
        std::istream & operator>>(std::istream &in, token_string &rhs)
        {
            rhs._M_value.clear();
            getline(in,rhs._M_value,S);
            return in;
        }

        operator const std::string &() const
        { 
            return _M_value; 
        }

        const std::string &
        get() const 
        {
            return _M_value;
        }

    private:
        std::string _M_value;
    };

} // namespace more 
#endif /* _TOKEN_H_ */
