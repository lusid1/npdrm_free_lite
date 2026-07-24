/*
 *  Copyright (C) 2014-2018 qwikrazor87
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"
#include "sctrl.h"
#include "np9660_patch.h"

PSP_MODULE_INFO("npdrm_free_loader", PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(0);

static STMOD_HANDLER previous = NULL;

int module_start_handler(SceModule2 *module)
{
	int ret = previous ? previous(module) : 0;

	return ret;
}

int thread_start(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
	previous = sctrlHENSetStartModuleHandler(module_start_handler);

	SceUID blockid = sceKernelAllocPartitionMemory(1, "npdrm_free_module", PSP_SMEM_Low, size_np9660_patch, NULL);
	void *modbuf = sceKernelGetBlockHeadAddr(blockid);

	if (blockid >= 0) {
		memcpy(modbuf, np9660_patch, size_np9660_patch);
		sctrlHENLoadModuleOnReboot("/kd/iofilemgr_dnas.prx", modbuf, size_np9660_patch, BOOTLOAD_UMDEMU | BOOTLOAD_POPS);
	}

	return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
	SceUID thid = sceKernelCreateThread("npdrm_free_loader", thread_start, 0x22, 0x2000, 0, NULL);

	if (thid >= 0)
		sceKernelStartThread(thid, args, argp);

	return 0;
}

int module_stop(SceSize args __attribute__((unused)), void *argp __attribute__((unused)))
{
	return 0;
}
