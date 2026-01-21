use std::process::Command;
use serde::Deserialize;
use std::collections::HashMap;
use once_cell::sync::Lazy;
use crate::proc::Stats;
use crate::pci_utils::PciDeviceInfo;

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
    
    // Try to get PCI address from ioreg
    let bus = get_pci_address_from_ioreg(if_name).unwrap_or_else(|| "N/A".to_string());

    Some((driver, version, bus))
}

/// Extract PCI address and additional info from ioreg
fn get_pci_address_from_ioreg(if_name: &str) -> Option<String> {
    let output = Command::new("ioreg")
        .arg("-r")
        .arg("-l")
        .arg("-n")
        .arg(if_name)
        .output()
        .ok()?;
    
    if !output.status.success() {
        return None;
    }
    
    let content = String::from_utf8_lossy(&output.stdout);
    
    // Look for IOPCIDevice parent or location
    for line in content.lines() {
        if line.contains("pcidebug") || line.contains("IOPCIDevice") {
            // Try to extract something like "PCI0@0/RP03@1C,2/PXSX@0"
            if let Some(start) = line.find("\"") {
                if let Some(end) = line[start+1..].find("\"") {
                    return Some(line[start+1..start+1+end].to_string());
                }
            }
        }
        
        // Alternative: look for "location" property
        if line.contains("\"location\"") {
            if let Some(eq_pos) = line.find('=') {
                let value = line[eq_pos+1..].trim();
                if let Some(stripped) = value.strip_prefix('"') {
                    if let Some(end) = stripped.find('"') {
                        return Some(stripped[..end].to_string());
                    }
                }
            }
        }
    }
    
    None
}

/// Get detailed PCI info from ioreg for network interfaces
pub fn get_pci_info_from_ioreg(if_name: &str) -> Option<PciDeviceInfo> {
    let output = Command::new("ioreg")
        .arg("-r")
        .arg("-l")
        .arg("-n")
        .arg(if_name)
        .output()
        .ok()?;
    
    if !output.status.success() {
        return None;
    }
    
    let content = String::from_utf8_lossy(&output.stdout);
    let mut pci_info = PciDeviceInfo::default();
    let mut found_any = false;
    
    for line in content.lines() {
        let trimmed = line.trim();
        
        // Vendor ID
        if trimmed.contains("\"vendor-id\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.vendor_id = value as u16;
                found_any = true;
            }
        }
        
        // Device ID
        if trimmed.contains("\"device-id\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.device_id = value as u16;
                found_any = true;
            }
        }
        
        // Subsystem Vendor ID
        if trimmed.contains("\"subsystem-vendor-id\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.subsystem_vendor = Some(value as u16);
                found_any = true;
            }
        }
        
        // Subsystem ID
        if trimmed.contains("\"subsystem-id\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.subsystem_device = Some(value as u16);
                found_any = true;
            }
        }
        
        // Revision
        if trimmed.contains("\"revision-id\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.revision = Some(value as u8);
                found_any = true;
            }
        }
        
        // Class code
        if trimmed.contains("\"class-code\"") {
            if let Some(value) = extract_hex_value(trimmed) {
                pci_info.class = Some(((value >> 16) & 0xFF) as u8);
                pci_info.subclass = Some(((value >> 8) & 0xFF) as u8);
                found_any = true;
            }
        }
        
        // IOName (driver)
        if trimmed.contains("\"IOName\"") {
            if let Some(name) = extract_string_value(trimmed) {
                pci_info.driver = Some(name);
                found_any = true;
            }
        }
    }
    
    if found_any {
        Some(pci_info)
    } else {
        None
    }
}

fn extract_hex_value(line: &str) -> Option<u32> {
    // Format: "key" = <hex_value>
    if let Some(eq_pos) = line.find('=') {
        let value_part = line[eq_pos+1..].trim();
        if value_part.starts_with('<') && value_part.contains('>') {
            let hex_str = value_part.trim_start_matches('<').trim_end_matches('>').trim();
            // Handle both formats: "0x1234" and just "1234"
            let clean_hex = hex_str.trim_start_matches("0x");
            return u32::from_str_radix(clean_hex, 16).ok();
        }
    }
    None
}

fn extract_string_value(line: &str) -> Option<String> {
    // Format: "key" = "value"
    if let Some(eq_pos) = line.find('=') {
        let value_part = line[eq_pos+1..].trim();
        if let Some(stripped) = value_part.strip_prefix('"') {
            if let Some(end) = stripped.find('"') {
                return Some(stripped[..end].to_string());
            }
        }
    }
    None
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
