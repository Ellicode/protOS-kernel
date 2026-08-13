#ifndef PCI_H
#define PCI_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    PCI_CC_UNCLASSIFIED                         = 0x0,
    PCI_CC_MASS_STORAGE_CONTROLLER              = 0x1,
    PCI_CC_NETWORK_CONTROLLER                   = 0x2,
    PCI_CC_DISPLAY_CONTROLLER                   = 0x3,
    PCI_CC_MULTIMEDIA_CONTROLLER                = 0x4,
    PCI_CC_MEMORY_CONTROLLER                    = 0x5,
    PCI_CC_BRIDGE                               = 0x6,
    PCI_CC_SIMPLE_COMMUNICATION_CONTROLLER      = 0x7,
    PCI_CC_BASE_SYSTEM_PERIPHERAL               = 0x8,
    PCI_CC_INPUT_DEVICE_CONTROLLER              = 0x9,
    PCI_CC_DOCKING_STATION                      = 0xA,
    PCI_CC_PROCESSOR                            = 0xB,
    PCI_CC_SERIAL_BUS_CONTROLLER                = 0xC,
    PCI_CC_WIRELESS_CONTROLLER                  = 0xD,
    PCI_CC_INTELLIGENT_CONTROLLER               = 0xE,
    PCI_CC_SATTELITE_COMMUNICATION_CONTROLLER   = 0xF,
    PCI_CC_ENCRYPTION_CONTROLLER                = 0x10,
    PCI_CC_SIGNAL_PROCESSING_CONTROLLER         = 0x11,
    PCI_CC_PROCESSING_ACCELERATOR               = 0x12,
    PCI_CC_NON_ESSENTIAL_INSTRUMENTATION        = 0x13,
    PCI_CC_CO_PROCESSOR                         = 0x40
} pci_cc_t;

void pci_scan_devices();

#endif // PCI_H
