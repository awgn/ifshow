use anyhow::Result;

#[cfg(target_os = "macos")]
use crate::macos;

pub fn get_if_list() -> Result<Vec<String>> {
    let addrs = nix::ifaddrs::getifaddrs()?;
    let mut names = std::collections::HashSet::new();
    for ifa in addrs {
        names.insert(ifa.interface_name);
    }
    let mut ret: Vec<String> = names.into_iter().collect();
    ret.sort();
    Ok(ret)
}



#[derive(Default, Debug)]
pub struct Stats {
    pub rx_bytes: u64,
    pub rx_packets: u64,
    
    pub tx_bytes: u64,
    pub tx_packets: u64,
}

#[cfg(target_os = "linux")]
pub fn get_stats(ifname: &str) -> Result<Stats> {
    use std::fs::File;
    use std::io::{BufRead, BufReader};
    use std::path::Path;

    let path = Path::new("/proc/net/dev");
    if !path.exists() {
         return Ok(Stats::default());
    }
    let file = File::open(path)?;
    let reader = BufReader::new(file);

    for (i, line) in reader.lines().enumerate() {
        if i < 2 { continue; }
        let line = line?;
        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.is_empty() { continue; }

        let name = parts[0].trim_end_matches(':');
        if name == ifname {
            if parts.len() < 11 { break; } // Safety
            
            let p = |idx: usize| parts[idx].parse::<u64>().unwrap_or(0);
            
            return Ok(Stats {
                rx_bytes: p(1),
                rx_packets: p(2),
                tx_bytes: p(9),
                tx_packets: p(10),
            });
        }
    }
    Ok(Stats::default())
}

#[cfg(target_os = "macos")]
pub fn get_stats(ifname: &str) -> Result<Stats> {
    Ok(macos::get_stats(ifname).unwrap_or_default())
}

#[cfg(not(any(target_os = "linux", target_os = "macos")))]
pub fn get_stats(_ifname: &str) -> Result<Stats> {
    Ok(Stats::default())
}

pub fn get_inet6_addr(ifname: &str) -> Result<Vec<(String, u32, String)>> {
    let addrs = nix::ifaddrs::getifaddrs()?;
    let mut ret = Vec::new();
    for ifa in addrs {
            if ifa.interface_name == ifname {
                if let Some(address) = ifa.address {
                    if let Some(sockaddr) = address.as_sockaddr_in6() {
                        let ip = sockaddr.ip();
                        // Calculate prefix len from netmask if available
                        let mut prefix = 0;
                        if let Some(netmask) = ifa.netmask.as_ref().and_then(|a| a.as_sockaddr_in6()) {
                            let mask_ip = netmask.ip();
                            prefix = u128::from(mask_ip).count_ones();
                        }
                        // Simplified scope detection
                        let scope = if ip.is_loopback() {
                            "host"
                        } else if ip.is_unicast_link_local() {
                            "link"
                        } else if ip.octets()[0] == 0xfe && (ip.octets()[1] & 0xc0) == 0xc0 { 
                            "site" 
                        } else if ip.is_multicast() {
                            "multicast"
                        } else {
                            "global"
                        };
                        
                        ret.push((ip.to_string(), prefix, scope.to_string()));
                    }
                }
            }
    }
    Ok(ret)
}

