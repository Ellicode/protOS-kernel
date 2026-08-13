#include "graphics/console.h"
#include "utils/utils.h"
#include <stdint.h>

#include "pci.h"

char *class_codes[] = {
    [PCI_CC_UNCLASSIFIED]                       = "Unclassified",
    [PCI_CC_MASS_STORAGE_CONTROLLER]            = "Mass Storage Controller",
    [PCI_CC_NETWORK_CONTROLLER]                 = "Network Controller",
    [PCI_CC_DISPLAY_CONTROLLER]                 = "Display Controller",
    [PCI_CC_MULTIMEDIA_CONTROLLER]              = "Multimedia Controller",
    [PCI_CC_MEMORY_CONTROLLER]                  = "Memory Controller",
    [PCI_CC_BRIDGE]                             = "Bridge",
    [PCI_CC_SIMPLE_COMMUNICATION_CONTROLLER]    = "Simple Communication Controller",
    [PCI_CC_BASE_SYSTEM_PERIPHERAL]             = "Base System Peripheral",
    [PCI_CC_INPUT_DEVICE_CONTROLLER]            = "Input Device Controller",
    [PCI_CC_DOCKING_STATION]                    = "Docking Station",
    [PCI_CC_PROCESSOR]                          = "Processor",
    [PCI_CC_SERIAL_BUS_CONTROLLER]              = "Serial Bus Controller",
    [PCI_CC_WIRELESS_CONTROLLER]                = "Wireless Controller",
    [PCI_CC_INTELLIGENT_CONTROLLER]             = "Intelligent Controller",
    [PCI_CC_SATTELITE_COMMUNICATION_CONTROLLER] = "Sattelite Communication Controller",
    [PCI_CC_SIGNAL_PROCESSING_CONTROLLER]       = "Signal Processing Controller",
    [PCI_CC_PROCESSING_ACCELERATOR]             = "Processing Accelerator",
    [PCI_CC_NON_ESSENTIAL_INSTRUMENTATION]      = "Non-Essential Instrumentation",
    [PCI_CC_CO_PROCESSOR]                       = "Co-Processor" // who tf uses dual-cpu setups anways
};

uint8_t pci_config_read_byte(
    uint8_t bus,
    uint8_t slot,
    uint8_t func,
    uint8_t offset
) {
    uint32_t address =
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC) |
        0x80000000;

    outl(0xCF8, address);
    return (uint8_t)(
        (inl(0xCFC) >> ((offset & 3) * 8)) & 0xFF
    );
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address =
        ((uint32_t)bus  << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)func << 8)  |
        (offset & 0xFC) |
        0x80000000;

    outl(0xCF8, address);
    return (uint16_t)(
        (inl(0xCFC) >> ((offset & 3) * 8)) & 0xFFFF
    );
}

uint16_t pci_check_vendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    uint8_t class_code;
    if ((vendor = pci_config_read_word(bus, slot, 0, 0)) != 0xFFFF) {
       device = pci_config_read_word(bus, slot, 0, 2);
       class_code = pci_config_read_byte(bus, slot, 0, 0x0B);
       print_f("found PCI device %d:%d.%d %x:%x %x %s\n", bus, slot, 0, vendor, device, class_code, class_codes[class_code]);
    } 
    return (vendor);
}

void pci_scan_devices() {
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            pci_check_vendor(bus, slot);
        }
    }
}
