/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __ARM_KVM_H__
#define __ARM_KVM_H__
#define KVM_SPSR_EL1 0
#define KVM_SPSR_SVC KVM_SPSR_EL1
#define KVM_SPSR_ABT 1
#define KVM_SPSR_UND 2
#define KVM_SPSR_IRQ 3
#define KVM_SPSR_FIQ 4
#define KVM_NR_SPSR 5
#ifndef __ASSEMBLY__
#include <asm/types.h>
#include <asm/ptrace.h>
#define __KVM_HAVE_GUEST_DEBUG
#define __KVM_HAVE_IRQ_LINE
#define KVM_REG_SIZE(id)   (1U << (((id) & KVM_REG_SIZE_MASK) >> KVM_REG_SIZE_SHIFT))
struct kvm_regs {
 struct user_pt_regs regs;
 __u64 sp_el1;
 __u64 elr_el1;
 __u64 spsr[KVM_NR_SPSR];
 struct user_fpsimd_state fp_regs;
};
#define KVM_ARM_TARGET_AEM_V8 0
#define KVM_ARM_TARGET_FOUNDATION_V8 1
#define KVM_ARM_TARGET_CORTEX_A57 2
#define KVM_ARM_TARGET_XGENE_POTENZA 3
#define KVM_ARM_NUM_TARGETS 4
#define KVM_ARM_DEVICE_TYPE_SHIFT 0
#define KVM_ARM_DEVICE_TYPE_MASK (0xffff << KVM_ARM_DEVICE_TYPE_SHIFT)
#define KVM_ARM_DEVICE_ID_SHIFT 16
#define KVM_ARM_DEVICE_ID_MASK (0xffff << KVM_ARM_DEVICE_ID_SHIFT)
#define KVM_ARM_DEVICE_VGIC_V2 0
#define KVM_VGIC_V2_ADDR_TYPE_DIST 0
#define KVM_VGIC_V2_ADDR_TYPE_CPU 1
#define KVM_VGIC_V2_DIST_SIZE 0x1000
#define KVM_VGIC_V2_CPU_SIZE 0x2000
#define KVM_ARM_VCPU_POWER_OFF 0
#define KVM_ARM_VCPU_EL1_32BIT 1
struct kvm_vcpu_init {
 __u32 target;
 __u32 features[7];
};
struct kvm_sregs {
};
struct kvm_fpu {
};
struct kvm_guest_debug_arch {
};
struct kvm_debug_exit_arch {
};
struct kvm_sync_regs {
};
struct kvm_arch_memory_slot {
};
#define KVM_REG_ARM_COPROC_MASK 0x000000000FFF0000
#define KVM_REG_ARM_COPROC_SHIFT 16
#define KVM_REG_ARM_CORE (0x0010 << KVM_REG_ARM_COPROC_SHIFT)
#define KVM_REG_ARM_CORE_REG(name) (offsetof(struct kvm_regs, name) / sizeof(__u32))
#define KVM_REG_ARM_DEMUX (0x0011 << KVM_REG_ARM_COPROC_SHIFT)
#define KVM_REG_ARM_DEMUX_ID_MASK 0x000000000000FF00
#define KVM_REG_ARM_DEMUX_ID_SHIFT 8
#define KVM_REG_ARM_DEMUX_ID_CCSIDR (0x00 << KVM_REG_ARM_DEMUX_ID_SHIFT)
#define KVM_REG_ARM_DEMUX_VAL_MASK 0x00000000000000FF
#define KVM_REG_ARM_DEMUX_VAL_SHIFT 0
#define KVM_REG_ARM64_SYSREG (0x0013 << KVM_REG_ARM_COPROC_SHIFT)
#define KVM_REG_ARM64_SYSREG_OP0_MASK 0x000000000000c000
#define KVM_REG_ARM64_SYSREG_OP0_SHIFT 14
#define KVM_REG_ARM64_SYSREG_OP1_MASK 0x0000000000003800
#define KVM_REG_ARM64_SYSREG_OP1_SHIFT 11
#define KVM_REG_ARM64_SYSREG_CRN_MASK 0x0000000000000780
#define KVM_REG_ARM64_SYSREG_CRN_SHIFT 7
#define KVM_REG_ARM64_SYSREG_CRM_MASK 0x0000000000000078
#define KVM_REG_ARM64_SYSREG_CRM_SHIFT 3
#define KVM_REG_ARM64_SYSREG_OP2_MASK 0x0000000000000007
#define KVM_REG_ARM64_SYSREG_OP2_SHIFT 0
#define ARM64_SYS_REG_SHIFT_MASK(x,n)   (((x) << KVM_REG_ARM64_SYSREG_ ## n ## _SHIFT) &   KVM_REG_ARM64_SYSREG_ ## n ## _MASK)
#define __ARM64_SYS_REG(op0,op1,crn,crm,op2)   (KVM_REG_ARM64 | KVM_REG_ARM64_SYSREG |   ARM64_SYS_REG_SHIFT_MASK(op0, OP0) |   ARM64_SYS_REG_SHIFT_MASK(op1, OP1) |   ARM64_SYS_REG_SHIFT_MASK(crn, CRN) |   ARM64_SYS_REG_SHIFT_MASK(crm, CRM) |   ARM64_SYS_REG_SHIFT_MASK(op2, OP2))
#define ARM64_SYS_REG(...) (__ARM64_SYS_REG(__VA_ARGS__) | KVM_REG_SIZE_U64)
#define KVM_REG_ARM_TIMER_CTL ARM64_SYS_REG(3, 3, 14, 3, 1)
#define KVM_REG_ARM_TIMER_CNT ARM64_SYS_REG(3, 3, 14, 3, 2)
#define KVM_REG_ARM_TIMER_CVAL ARM64_SYS_REG(3, 3, 14, 0, 2)
#define KVM_DEV_ARM_VGIC_GRP_ADDR 0
#define KVM_DEV_ARM_VGIC_GRP_DIST_REGS 1
#define KVM_DEV_ARM_VGIC_GRP_CPU_REGS 2
#define KVM_DEV_ARM_VGIC_CPUID_SHIFT 32
#define KVM_DEV_ARM_VGIC_CPUID_MASK (0xffULL << KVM_DEV_ARM_VGIC_CPUID_SHIFT)
#define KVM_DEV_ARM_VGIC_OFFSET_SHIFT 0
#define KVM_DEV_ARM_VGIC_OFFSET_MASK (0xffffffffULL << KVM_DEV_ARM_VGIC_OFFSET_SHIFT)
#define KVM_ARM_IRQ_TYPE_SHIFT 24
#define KVM_ARM_IRQ_TYPE_MASK 0xff
#define KVM_ARM_IRQ_VCPU_SHIFT 16
#define KVM_ARM_IRQ_VCPU_MASK 0xff
#define KVM_ARM_IRQ_NUM_SHIFT 0
#define KVM_ARM_IRQ_NUM_MASK 0xffff
#define KVM_ARM_IRQ_TYPE_CPU 0
#define KVM_ARM_IRQ_TYPE_SPI 1
#define KVM_ARM_IRQ_TYPE_PPI 2
#define KVM_ARM_IRQ_CPU_IRQ 0
#define KVM_ARM_IRQ_CPU_FIQ 1
#define KVM_ARM_IRQ_GIC_MAX 127
#define KVM_PSCI_FN_BASE 0x95c1ba5e
#define KVM_PSCI_FN(n) (KVM_PSCI_FN_BASE + (n))
#define KVM_PSCI_FN_CPU_SUSPEND KVM_PSCI_FN(0)
#define KVM_PSCI_FN_CPU_OFF KVM_PSCI_FN(1)
#define KVM_PSCI_FN_CPU_ON KVM_PSCI_FN(2)
#define KVM_PSCI_FN_MIGRATE KVM_PSCI_FN(3)
#define KVM_PSCI_RET_SUCCESS 0
#define KVM_PSCI_RET_NI ((unsigned long)-1)
#define KVM_PSCI_RET_INVAL ((unsigned long)-2)
#define KVM_PSCI_RET_DENIED ((unsigned long)-3)
#endif
#endif
