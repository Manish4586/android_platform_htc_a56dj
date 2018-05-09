cmd_usr/initramfs_data.o := /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/gcc-wrapper.py /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-gcc -Wp,-MD,usr/.initramfs_data.o.d  -nostdinc -isystem /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/../lib/gcc/arm-eabi/4.8/include -I/home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/arch/arm/include -Iarch/arm/include/generated -Iinclude  -I/home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include -include /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include/linux/kconfig.h -D__KERNEL__ -mlittle-endian   -I/home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/arch/arm/mach-msm/include -D__ASSEMBLY__ -mabi=aapcs-linux -mno-thumb-interwork -funwind-tables -D__LINUX_ARM_ARCH__=7 -march=armv7-a -include asm/unified.h -msoft-float -gdwarf-2 -DINITRAMFS_IMAGE="usr/initramfs_data.cpio"   -c -o usr/initramfs_data.o /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/usr/initramfs_data.S

source_usr/initramfs_data.o := /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/usr/initramfs_data.S

deps_usr/initramfs_data.o := \
    $(wildcard include/config/64bit.h) \
  /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/arch/arm/include/asm/unified.h \
    $(wildcard include/config/arm/asm/unified.h) \
    $(wildcard include/config/thumb2/kernel.h) \
  /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include/linux/stringify.h \
  /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include/asm-generic/vmlinux.lds.h \
    $(wildcard include/config/hotplug.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/event/tracing.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/syscalls.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/constructors.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/pm/trace.h) \
    $(wildcard include/config/blk/dev/initrd.h) \

usr/initramfs_data.o: $(deps_usr/initramfs_data.o)

$(deps_usr/initramfs_data.o):
