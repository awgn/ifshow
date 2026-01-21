use std::io;
use std::mem;
use std::os::fd::{AsRawFd, OwnedFd};
use nix::sys::socket::{socket, AddressFamily, SockFlag, SockType};
use libc::{c_char, c_int, c_ulong, c_void};

#[cfg(target_os = "macos")]
use crate::macos;

// IOCTL Constants
#[cfg(target_os = "linux")]
pub const SIOCGIFHWADDR: c_ulong = 0x8927;
#[cfg(target_os = "linux")]
pub const SIOCGIFMTU: c_ulong = 0x8921;
#[cfg(target_os = "linux")]
pub const SIOCGIFMETRIC: c_ulong = 0x891d;
#[cfg(target_os = "linux")]
pub const SIOCGIFMAP: c_ulong = 0x8970;
#[cfg(target_os = "linux")]
pub const SIOCGIFTXQLEN: c_ulong = 0x8942;
#[cfg(target_os = "linux")]
pub const SIOCETHTOOL: c_ulong = 0x8946;

#[cfg(target_os = "macos")]
pub const SIOCGIFMTU: c_ulong = 0xc0206933;
#[cfg(target_os = "macos")]
pub const SIOCGIFMETRIC: c_ulong = 0xc020691d;

#[cfg(target_os = "macos")]
pub const SIOCETHTOOL: c_ulong = 0;

#[cfg(not(any(target_os = "linux", target_os = "macos")))]
pub const SIOCGIFMTU: c_ulong = 0;
#[cfg(not(any(target_os = "linux", target_os = "macos")))]
pub const SIOCGIFMETRIC: c_ulong = 0;

#[cfg(not(any(target_os = "linux", target_os = "macos")))]
pub const SIOCETHTOOL: c_ulong = 0;

// Ethtool Constants
#[cfg(target_os = "linux")]
pub const ETHTOOL_GDRVINFO: u32 = 0x00000003;
pub const ETHTOOL_GLINK: u32 = 0x0000000a;

// If flags
pub const IFF_UP: i16 = 0x1;

// Structs

#[repr(C)]
#[derive(Clone, Copy)]
pub struct IfMap {
    pub mem_start: c_ulong,
    pub mem_end: c_ulong,
    pub base_addr: u16,
    pub irq: u8,
    pub dma: u8,
    pub port: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub union IfrData {
    pub ifru_addr: libc::sockaddr,
    pub ifru_dstaddr: libc::sockaddr,
    pub ifru_broadaddr: libc::sockaddr,
    pub ifru_netmask: libc::sockaddr,
    pub ifru_hwaddr: libc::sockaddr,
    pub ifru_flags: i16,
    pub ifru_ivalue: c_int,
    pub ifru_mtu: c_int,
    pub ifru_map: IfMap,
    pub ifru_slave: [c_char; 16],
    pub ifru_newname: [c_char; 16],
    pub ifru_data: *mut c_void,
}

#[repr(C)]
pub struct IfReq {
    pub ifr_name: [c_char; 16],
    pub ifr_ifru: IfrData,
}

impl IfReq {
    pub fn new(name: &str) -> Self {
        let mut req: IfReq = unsafe { mem::zeroed() };
        let bytes = name.as_bytes();
        let len = std::cmp::min(bytes.len(), 15);
        for (i, &byte) in bytes.iter().enumerate().take(len) {
            req.ifr_name[i] = byte as c_char;
        }
        req.ifr_name[len] = 0;
        req
    }
}

#[repr(C)]
#[derive(Debug, Default)]
pub struct EthtoolDrvInfo {
    pub cmd: u32,
    pub driver: [c_char; 32],
    pub version: [c_char; 32],
    pub fw_version: [c_char; 32],
    pub bus_info: [c_char; 32],
    pub erom_version: [c_char; 32],
    pub reserved2: [c_char; 12],
    pub n_priv_flags: u32,
    pub n_stats: u32,
    pub testinfo_len: u32,
    pub eedump_len: u32,
    pub regdump_len: u32,
}



#[repr(C)]
#[derive(Debug, Default)]
pub struct EthtoolValue {
    pub cmd: u32,
    pub data: u32,
}

// IOCTL Functions

nix::ioctl_write_ptr_bad!(ioctl_ethtool, SIOCETHTOOL, IfReq);
#[cfg(target_os = "linux")]
nix::ioctl_read_bad!(ioctl_get_hwaddr, SIOCGIFHWADDR, IfReq);
nix::ioctl_read_bad!(ioctl_get_mtu, SIOCGIFMTU, IfReq);
nix::ioctl_read_bad!(ioctl_get_metric, SIOCGIFMETRIC, IfReq);



pub struct Interface {
    name: String,
    sock: OwnedFd,
}

impl Interface {
    pub fn new(name: &str) -> io::Result<Self> {
        // Create a dummy socket for ioctls
        let sock = socket(AddressFamily::Inet, SockType::Datagram, SockFlag::empty(), None)
            .map_err(io::Error::other)?;
            
        Ok(Self {
            name: name.to_string(),
            sock,
        })
    }

    pub fn flags(&self) -> io::Result<i16> {
        let addrs = nix::ifaddrs::getifaddrs()?;
        for ifa in addrs {
            if ifa.interface_name == self.name {
                return Ok(ifa.flags.bits() as i16);
            }
        }
        Err(io::Error::new(io::ErrorKind::NotFound, "Interface not found"))
    }

    pub fn is_up(&self) -> bool {
        self.flags().map(|f| f & IFF_UP != 0).unwrap_or(false)
    }

    pub fn is_running(&self) -> bool {
        // IFF_RUNNING is 0x40
        self.flags().map(|f| f & 0x40 != 0).unwrap_or(false)
    }

    pub fn flags_str(&self) -> String {
        let flags = match self.flags() {
            Ok(f) => f,
            Err(_) => return String::new(),
        };

        let mut ret = Vec::new();
        // Standard flags
        if flags & 0x1 != 0 { ret.push("UP"); }
        if flags & 0x2 != 0 { ret.push("BROADCAST"); }
        if flags & 0x4 != 0 { ret.push("DEBUG"); }
        if flags & 0x8 != 0 { ret.push("LOOPBACK"); }
        if flags & 0x10 != 0 { ret.push("PTP"); }
        if flags & 0x20 != 0 { ret.push("NOTRAILERS"); }
        if flags & 0x40 != 0 { ret.push("RUNNING"); }
        if flags & 0x80 != 0 { ret.push("NOARP"); }
        if flags & 0x100 != 0 { ret.push("PROMISC"); }
        if flags & 0x200 != 0 { ret.push("ALLMULTI"); }
        if flags & 0x400 != 0 { ret.push("MASTER"); }
        if flags & 0x800 != 0 { ret.push("SLAVE"); }
        if flags & 0x1000 != 0 { ret.push("MULTICAST"); }
        if flags & 0x2000 != 0 { ret.push("PORTSEL"); }
        if flags & 0x4000 != 0 { ret.push("AUTOMEDIA"); }
        if flags & (0x8000u16 as i16) != 0 { ret.push("DYNAMIC"); }

        ret.join(" ")
    }

    pub fn ethtool_drvinfo(&self) -> io::Result<EthtoolDrvInfo> {
        #[cfg(target_os = "linux")]
        {
            let mut info: EthtoolDrvInfo = Default::default();
            info.cmd = ETHTOOL_GDRVINFO;
            
            let mut req = IfReq::new(&self.name);
            req.ifr_ifru.ifru_data = &mut info as *mut _ as *mut c_void;

            unsafe { ioctl_ethtool(self.sock.as_raw_fd(), &mut req) }.map_err(|e| io::Error::from_raw_os_error(e as i32))?;
            Ok(info)
        }
        
        #[cfg(target_os = "macos")]
        {
            if let Some((driver, version, bus)) = macos::get_driver_info(&self.name) {
                let mut info: EthtoolDrvInfo = Default::default();
                // Copy strings to info.driver, info.version, info.bus_info
                let copy_str = |dest: &mut [c_char], src: &str| {
                    let bytes = src.as_bytes();
                    for (i, b) in bytes.iter().enumerate() {
                        if i >= dest.len() - 1 { break; }
                        dest[i] = *b as c_char;
                    }
                    // Null terminate if space permits or if truncated
                    let last_idx = std::cmp::min(bytes.len(), dest.len() - 1);
                    dest[last_idx] = 0;
                };
                
                copy_str(&mut info.driver, &driver);
                copy_str(&mut info.version, &version);
                copy_str(&mut info.bus_info, &bus);
                
                Ok(info)
            } else {
                Err(io::Error::new(io::ErrorKind::NotFound, "Driver info not found"))
            }
        }

        #[cfg(not(any(target_os = "linux", target_os = "macos")))]
        {
             Err(io::Error::new(io::ErrorKind::Unsupported, "Not supported on this OS"))
        }
    }



    pub fn ethtool_link(&self) -> io::Result<bool> {
        let mut val = EthtoolValue {
            cmd: ETHTOOL_GLINK,
            ..Default::default()
        };

        let mut req = IfReq::new(&self.name);
        req.ifr_ifru.ifru_data = &mut val as *mut _ as *mut c_void;

        unsafe { ioctl_ethtool(self.sock.as_raw_fd(), &req) }.map_err(|e| io::Error::from_raw_os_error(e as i32))?;
        Ok(val.data != 0)
    }

    #[cfg(target_os = "linux")]
    pub fn mac(&self) -> io::Result<String> {
        let mut req = IfReq::new(&self.name);
        unsafe { ioctl_get_hwaddr(self.sock.as_raw_fd(), &mut req) }.map_err(|e| io::Error::from_raw_os_error(e as i32))?;
        let addr = unsafe { req.ifr_ifru.ifru_hwaddr.sa_data };
        // sa_data is [i8; 14]. MAC is first 6.
        Ok(format!("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
            addr[0] as u8, addr[1] as u8, addr[2] as u8, 
            addr[3] as u8, addr[4] as u8, addr[5] as u8))
    }

    #[cfg(not(target_os = "linux"))]
    pub fn mac(&self) -> io::Result<String> {
        let addrs = nix::ifaddrs::getifaddrs()?;
        for ifa in addrs {
            if ifa.interface_name == self.name {
                if let Some(address) = ifa.address {
                    if let Some(link) = address.as_link_addr() {
                         if let Some(addr) = link.addr() {
                             let s = addr.iter().map(|b| format!("{:02x}", b)).collect::<Vec<_>>().join(":");
                             return Ok(s);
                         }
                    }
                }
            }
        }
        Ok("".to_string())
    }

    pub fn mtu(&self) -> io::Result<i32> {
        let mut req = IfReq::new(&self.name);
        unsafe { ioctl_get_mtu(self.sock.as_raw_fd(), &mut req) }.map_err(|e| io::Error::from_raw_os_error(e as i32))?;
        unsafe { Ok(req.ifr_ifru.ifru_mtu) }
    }
    
    pub fn metric(&self) -> io::Result<i32> {
        let mut req = IfReq::new(&self.name);
        match unsafe { ioctl_get_metric(self.sock.as_raw_fd(), &mut req) } {
            Ok(_) => unsafe { Ok(if req.ifr_ifru.ifru_ivalue == 0 { 1 } else { req.ifr_ifru.ifru_ivalue }) },
            Err(e) => Err(io::Error::from_raw_os_error(e as i32)),
        }
    }
    
    // Inet addrs (using nix::ifaddrs is easier here, as C++ uses getifaddrs)
    pub fn inet_addrs(&self) -> Vec<(String, String, i32)> {
        let mut ret = Vec::new();
        if let Ok(addrs) = nix::ifaddrs::getifaddrs() {
            for ifa in addrs {
                if ifa.interface_name == self.name {
                    if let Some(address) = ifa.address {
                         if let Some(sockaddr) = address.as_sockaddr_in() {
                             let ip_u32 = sockaddr.ip();
                             let ip = std::net::Ipv4Addr::from(ip_u32);
                             
                             let mask_opt = ifa.netmask.as_ref().and_then(|a| a.as_sockaddr_in().map(|s| s.ip()));
                             
                             if let Some(mask_u32) = mask_opt {
                                 let mask_ip = std::net::Ipv4Addr::from(mask_u32);
                                 // Mask is u32.
                                 let prefix = u32::from(mask_ip).count_ones() as i32;
                                 
                                 ret.push((ip.to_string(), mask_ip.to_string(), prefix));
                             } else {
                                 ret.push((ip.to_string(), "".to_string(), 0));
                             }
                         }
                    }
                }
            }
        }
        ret
    }
}