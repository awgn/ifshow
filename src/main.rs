use clap::Parser;
use colored::*;
use anyhow::Result;

mod ifr;
mod proc;
#[cfg(target_os = "macos")]
mod macos;

#[derive(Parser)]
#[command(name = "ifshow", about = "Show network interface information")]
struct Cli {
    /// Display all interfaces (even if down)
    #[arg(short, long)]
    all: bool,

    /// Filter by driver
    #[arg(short, long)]
    driver: Vec<String>,

    /// Verbose output
    #[arg(short, long)]
    verbose: bool,

    /// Interface list
    #[arg(trailing_var_arg = true)]
    interfaces: Vec<String>,
}

// use pci::{Pci, PciHeader}; // Unknown API for now

fn main() -> Result<()> {
    let cli = Cli::parse();

    let if_list_proc = proc::get_if_list()?;

    // Sort uniquely
    let mut all_interfaces = if_list_proc.clone();
    all_interfaces.sort();
    all_interfaces.dedup();

    for name in &all_interfaces {
        // Filter by name
        if !cli.interfaces.is_empty() && !cli.interfaces.contains(name) {
            continue;
        }

        let iif = match ifr::Interface::new(name) {
            Ok(i) => i,
            Err(_) => continue,
        };

        // Filter by flags (UP) unless -a
        let is_up = iif.is_up();
        if !cli.all && !is_up && !cli.interfaces.contains(name) {
             continue;
        }

        // --- Driver Info (Linux) ---
        let drv_info = iif.ethtool_drvinfo().ok();

        // Filter by driver (if filter active)
        if !cli.driver.is_empty() {
             let mut matched = false;
             if let Some(info) = &drv_info {
                 let drv_str = unsafe { std::ffi::CStr::from_ptr(info.driver.as_ptr()) }.to_string_lossy();
                 for d in &cli.driver {
                     if drv_str.contains(d) {
                         matched = true;
                         break;
                     }
                 }
             }
             if !matched { continue; }
        }

        // --- Header (Name + Link Status) ---
        print!("{}", name.bold().green());

        let mut link_detected = iif.ethtool_link().unwrap_or(false);
        #[cfg(not(target_os = "linux"))]
        {
            if !link_detected {
                // On macOS, use running flag as proxy for link
                 link_detected = iif.is_running();
            }
        }

        if link_detected {
            print!(" ({})", "LINK UP".bold().white().on_green());
        } else {
             print!(" ({})", "NO LINK".red());
        }

        println!(); // End of header line

        let indent = "    ";

        // --- Hardware Address (MAC) ---
        if let Ok(mac) = iif.mac() {
            if !mac.is_empty() {
                println!("{}MAC:     {}", indent, mac.yellow());
            }
        }

        // --- IP Addresses ---
        // IPv4
        for (addr, _mask, prefix) in iif.inet_addrs() {
            println!("{}IPv4:    {}/{}", indent, addr.cyan(), prefix);
        }
        // IPv6
        if let Ok(addrs) = proc::get_inet6_addr(name) {
             for (addr, plen, _scope) in addrs {
                  println!("{}IPv6:    {}/{}", indent, addr.blue(), plen);
             }
        }

        // --- Driver / Bus Info ---
        if let Some(info) = &drv_info {
             let drv_str = unsafe { std::ffi::CStr::from_ptr(info.driver.as_ptr()) }.to_string_lossy();
             let ver_str = unsafe { std::ffi::CStr::from_ptr(info.version.as_ptr()) }.to_string_lossy();
             let bus_str = unsafe { std::ffi::CStr::from_ptr(info.bus_info.as_ptr()) }.to_string_lossy();

             println!("{}Driver:  {} (v: {})", indent, drv_str.white().bold(), ver_str);
             if !bus_str.is_empty() {
                  println!("{}Bus:     {}", indent, bus_str);
             }

             // Check generic pci crate if requested? (omitted per plan, using ethtool info is sufficient)
        }

        // --- Status / Flags ---
        let flags_str = iif.flags_str();
        if !flags_str.is_empty() {
             println!("{}Flags:   {}", indent, flags_str.dimmed());
        }

        // --- MTU / Metric ---
        let mtu = iif.mtu().unwrap_or(0);
        let metric = iif.metric().unwrap_or(0);
        println!("{}MTU:     {} (Metric: {})", indent, mtu, metric);

        // --- Stats (Linux only currently) ---
        if let Ok(stats) = proc::get_stats(name) {
             if stats.rx_bytes > 0 || stats.tx_bytes > 0 {
                  println!("{}Stats:   RX: {} bytes ({} pkts), TX: {} bytes ({} pkts)",
                      indent,
                      stats.rx_bytes, stats.rx_packets,
                      stats.tx_bytes, stats.tx_packets
                  );
             }
        }

        println!(); // Separator
    }

    Ok(())
}
