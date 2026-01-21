use anyhow::Result;
use std::collections::HashMap;

fn get_vendor_name(vendor_id: u16) -> Option<&'static str> {
    match vendor_id {
        0x8086 => Some("Intel Corporation"),
        0x10ec => Some("Realtek Semiconductor Co., Ltd."),
        0x14e4 => Some("Broadcom Inc."),
        0x1969 => Some("Qualcomm Atheros"),
        0x168c => Some("Qualcomm Atheros"),
        0x10de => Some("NVIDIA Corporation"),
        0x1022 => Some("Advanced Micro Devices, Inc. [AMD]"),
        0x1106 => Some("VIA Technologies, Inc."),
        0x11ab => Some("Marvell Technology Group Ltd."),
        0x13f0 => Some("Sundance Technology Inc. / IC Plus Corp"),
        0x1737 => Some("Linksys"),
        0x1814 => Some("Ralink corp."),
        0x1b21 => Some("ASMedia Technology Inc."),
        0x8139 => Some("Realtek"),
        0x10b7 => Some("3Com Corporation"),
        0x1186 => Some("D-Link System Inc"),
        0x1fc9 => Some("Tehuti Networks Ltd."),
        0x1d6a => Some("Aquantia Corp."),
        0x1425 => Some("Chelsio Communications Inc"),
        0x15b3 => Some("Mellanox Technologies"),
        _ => None,
    }
}

fn get_common_device_name(vendor_id: u16, device_id: u16) -> Option<&'static str> {
    match (vendor_id, device_id) {
        (0x8086, 0x100e) => Some("82540EM Gigabit Ethernet Controller"),
        (0x8086, 0x100f) => Some("82545EM Gigabit Ethernet Controller"),
        (0x8086, 0x10d3) => Some("82574L Gigabit Network Connection"),
        (0x8086, 0x1533) => Some("I210 Gigabit Network Connection"),
        (0x8086, 0x1539) => Some("I211 Gigabit Network Connection"),
        (0x8086, 0x15b7) => Some("Ethernet Connection"),
        (0x8086, 0x15b8) => Some("Ethernet Connection"),
        (0x10ec, 0x8168) => Some("RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller"),
        (0x10ec, 0x8169) => Some("RTL8169 PCI Gigabit Ethernet Controller"),
        (0x10ec, 0x8136) => Some("RTL810xE PCI Express Fast Ethernet controller"),
        (0x14e4, 0x1677) => Some("NetXtreme BCM5751 Gigabit Ethernet"),
        (0x14e4, 0x165f) => Some("NetXtreme BCM5720 Gigabit Ethernet"),
        (0x14e4, 0x4331) => Some("BCM4331 802.11a/b/g/n"),
        _ => None,
    }
}

#[derive(Debug, Clone, Default)]
pub struct PciDeviceInfo {
    pub vendor_id: u16,
    pub device_id: u16,
    pub vendor_name: Option<String>,
    pub device_name: Option<String>,
    pub subsystem_vendor: Option<u16>,
    pub subsystem_device: Option<u16>,
    pub class: Option<u8>,
    pub subclass: Option<u8>,
    pub revision: Option<u8>,
    pub bus: Option<u8>,
    pub device: Option<u8>,
    pub function: Option<u8>,
    pub driver: Option<String>,
    pub numa_node: Option<i32>,
    pub irq: Option<u32>,
}

impl PciDeviceInfo {
    pub fn format_class(&self) -> String {
        if let (Some(class), Some(subclass)) = (self.class, self.subclass) {
            match (class, subclass) {
                (0x02, 0x00) => "Ethernet controller".to_string(),
                (0x02, 0x80) => "Network controller".to_string(),
                (0x0d, 0x11) => "802.1a controller".to_string(),
                (0x0d, 0x20) => "802.11b controller".to_string(),
                (0x0d, 0x80) => "Wireless controller".to_string(),
                _ => format!("Class {:02x}:{:02x}", class, subclass),
            }
        } else {
            "Unknown".to_string()
        }
    }

    pub fn pci_address(&self) -> Option<String> {
        if let (Some(bus), Some(dev), Some(func)) = (self.bus, self.device, self.function) {
            Some(format!("{:02x}:{:02x}.{}", bus, dev, func))
        } else {
            None
        }
    }
}

#[cfg(feature = "pci-info")]
pub fn get_pci_devices() -> Result<HashMap<String, PciDeviceInfo>> {
    use pci_info::PciInfo;
    
    let mut devices = HashMap::new();
    
    match PciInfo::enumerate_pci() {
        Ok(pci_devices) => {
            for dev_result in pci_devices {
                let dev = match dev_result {
                    Ok(d) => d,
                    Err(_) => continue,
                };
                
                let class = match dev.device_class() {
                    Ok(c) => u8::from(c),
                    Err(_) => continue,
                };
                let subclass = match dev.device_subclass() {
                    Ok(sc) => u8::from(sc),
                    Err(_) => continue,
                };
                
                if class != 0x02 && class != 0x0d {
                    continue;
                }

                let location = dev.location();
                let (bus, device, function) = match location {
                    Ok(loc) => (Some(loc.bus()), Some(loc.device()), Some(loc.function())),
                    Err(_) => (None, None, None),
                };

                let mut info = PciDeviceInfo {
                    vendor_id: dev.vendor_id(),
                    device_id: dev.device_id(),
                    class: Some(class),
                    subclass: Some(subclass),
                    revision: dev.revision().ok(),
                    bus,
                    device,
                    function,
                    ..Default::default()
                };

                if let Some(vendor) = get_vendor_name(info.vendor_id) {
                    info.vendor_name = Some(vendor.to_string());
                }
                
                if let Some(device) = get_common_device_name(info.vendor_id, info.device_id) {
                    info.device_name = Some(device.to_string());
                }

                info.subsystem_vendor = dev.subsystem_vendor_id().ok().flatten();
                info.subsystem_device = dev.subsystem_device_id().ok().flatten();

                if let (Some(b), Some(d), Some(f)) = (bus, device, function) {
                    let key = format!("{:02x}:{:02x}.{}", b, d, f);
                    devices.insert(key, info);
                }
            }
        }
        Err(e) => {
            eprintln!("Warning: Could not enumerate PCI devices: {}", e);
        }
    }
    
    Ok(devices)
}

#[cfg(not(feature = "pci-info"))]
pub fn get_pci_devices() -> Result<HashMap<String, PciDeviceInfo>> {
    Ok(HashMap::new())
}

pub fn find_pci_info_for_interface(
    interface_name: &str,
    bus_info: &str,
    pci_devices: &HashMap<String, PciDeviceInfo>,
) -> Option<PciDeviceInfo> {
    if bus_info.is_empty() {
        return None;
    }

    let clean_bus = bus_info.trim_start_matches("pci@");
    
    let pci_addr = if let Some(addr) = parse_pci_address(clean_bus) {
        addr
    } else if let Some(addr) = extract_pci_from_sysfs(interface_name) {
        addr
    } else {
        return None;
    };

    pci_devices.get(&pci_addr).cloned()
}

fn parse_pci_address(bus_info: &str) -> Option<String> {
    let parts: Vec<&str> = bus_info.split(':').collect();
    
    if parts.len() >= 2 {
        let last_part = parts[parts.len() - 1];
        let second_last = parts[parts.len() - 2];
        
        if last_part.contains('.') {
            let dev_func: Vec<&str> = last_part.split('.').collect();
            if dev_func.len() == 2 {
                if let Ok(bus) = u8::from_str_radix(second_last, 16) {
                    if let Ok(dev) = u8::from_str_radix(dev_func[0], 16) {
                        if let Ok(func) = u8::from_str_radix(dev_func[1], 16) {
                            return Some(format!("{:02x}:{:02x}.{}", bus, dev, func));
                        }
                    }
                }
            }
        }
    }
    
    None
}

#[cfg(target_os = "linux")]
fn extract_pci_from_sysfs(interface_name: &str) -> Option<String> {
    use std::fs;
    use std::path::PathBuf;
    
    let sysfs_path = PathBuf::from(format!("/sys/class/net/{}/device/uevent", interface_name));
    
    if let Ok(content) = fs::read_to_string(&sysfs_path) {
        for line in content.lines() {
            if line.starts_with("PCI_SLOT_NAME=") {
                let addr = line.trim_start_matches("PCI_SLOT_NAME=");
                return parse_pci_address(addr);
            }
        }
    }
    
    let device_link = PathBuf::from(format!("/sys/class/net/{}/device", interface_name));
    if let Ok(target) = fs::read_link(&device_link) {
        if let Some(filename) = target.file_name() {
            if let Some(addr_str) = filename.to_str() {
                return parse_pci_address(addr_str);
            }
        }
    }
    
    None
}

#[cfg(not(target_os = "linux"))]
fn extract_pci_from_sysfs(_interface_name: &str) -> Option<String> {
    None
}