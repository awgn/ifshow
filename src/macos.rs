use std::process::Command;
use serde::Deserialize;
use std::collections::HashMap;
use once_cell::sync::Lazy;
use crate::proc::Stats;

#[derive(Deserialize, Debug, Clone)]
pub struct NetworkInterface {
    #[serde(rename = "_name")]
    pub _name: String,
    pub interface: Option<String>,
    pub hardware: Option<String>, // "Ethernet", "AirPort"
    #[serde(rename = "type")]
    pub type_: Option<String>,
}

#[derive(Deserialize, Debug)]
pub struct SystemProfilerOutput {
    #[serde(rename = "SPNetworkDataType")]
    pub network_data: Vec<NetworkInterface>,
}

pub static SYSTEM_PROFILER_DATA: Lazy<Option<HashMap<String, NetworkInterface>>> = Lazy::new(|| {
    let output = Command::new("system_profiler")
        .arg("SPNetworkDataType")
        .arg("-json")
        .output()
        .ok()?;
    
    if !output.status.success() {
        return None;
    }

    let parsed: SystemProfilerOutput = serde_json::from_slice(&output.stdout).ok()?;
    let mut map = HashMap::new();
    for iface in parsed.network_data {
        if let Some(name) = &iface.interface {
            map.insert(name.clone(), iface);
        }
    }
    Some(map)
});

pub fn get_driver_info(if_name: &str) -> Option<(String, String, String)> {
    let data = SYSTEM_PROFILER_DATA.as_ref()?;
    let iface = data.get(if_name)?;
    
    let driver = iface.hardware.clone().or(iface.type_.clone()).unwrap_or_else(|| "Unknown".to_string());
    // System profiler doesn't give driver version easily.
    let version = "N/A".to_string(); 
    // Bus info is also not directly linked in SPNetworkDataType
    let bus = "N/A".to_string(); 

    Some((driver, version, bus))
}

pub fn get_stats(if_name: &str) -> Option<Stats> {
    let mut ifap: *mut libc::ifaddrs = std::ptr::null_mut();
    if unsafe { libc::getifaddrs(&mut ifap) } != 0 {
        return None;
    }

    let mut current = ifap;
    let mut stats = None;

    while !current.is_null() {
        let name_ptr = unsafe { (*current).ifa_name };
        let name = unsafe { std::ffi::CStr::from_ptr(name_ptr) }.to_string_lossy();
        
        if name == if_name {
            let addr = unsafe { (*current).ifa_addr };
            if !addr.is_null() && unsafe { (*addr).sa_family } == libc::AF_LINK as u8 {
                let data = unsafe { (*current).ifa_data } as *mut libc::if_data;
                if !data.is_null() {
                    let d = unsafe { *data };
                    stats = Some(Stats {
                        rx_bytes: d.ifi_ibytes as u64,
                        rx_packets: d.ifi_ipackets as u64,
                        tx_bytes: d.ifi_obytes as u64,
                        tx_packets: d.ifi_opackets as u64,
                    });
                    break;
                }
            }
        }
        current = unsafe { (*current).ifa_next };
    }

    unsafe { libc::freeifaddrs(ifap) };
    stats
}
