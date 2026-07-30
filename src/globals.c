/********************************************************************************
 * @file        globals.c
 * @brief       Global limine variables and methods.
 * 
 * @author      Elliot Laborieux
 * @copyright   Copyright (c) 2026 Ellicode
 ********************************************************************************/

#include "limine/limine.h"

// VARIABLES

struct limine_hhdm_response                 *g_lim_hhdm;
struct limine_memmap_response               *g_lim_memmap;
struct limine_executable_address_response   *g_lim_kaddr;
struct limine_module_response               *g_lim_modules;

// METHODS 

void* (*m_pmm_alloc_p)(void);
void (*m_pmm_free_p)(void *);
