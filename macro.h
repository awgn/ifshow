#ifndef _MACRO_H_
#define _MACRO_H_ 

#define IF_FLAG_BIT(i) (1<<(i-1))

#define IPV6_ADDR_ANY           0x0000U
#define IPV6_ADDR_UNICAST       0x0001U
#define IPV6_ADDR_MULTICAST     0x0002U
#define IPV6_ADDR_ANYCAST       0x0004U
#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U
#define IPV6_ADDR_COMPATv4      0x0080U
#define IPV6_ADDR_SCOPE_MASK    0x00f0U
#define IPV6_ADDR_MAPPED        0x1000U
#define IPV6_ADDR_RESERVED      0x2000U         /* reserved address space */

#endif /* _MACRO_H_ */
