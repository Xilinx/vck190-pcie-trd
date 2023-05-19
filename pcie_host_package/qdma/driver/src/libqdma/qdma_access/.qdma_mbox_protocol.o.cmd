cmd_/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o := gcc -Wp,-MD,/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/.qdma_mbox_protocol.o.d  -nostdinc -isystem /usr/lib/gcc/x86_64-linux-gnu/5/include  -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -Werror-implicit-function-declaration -Wno-format-security -std=gnu89 -fno-PIE -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -funit-at-a-time -DCONFIG_X86_X32_ABI -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -DCONFIG_AS_FXSAVEQ=1 -DCONFIG_AS_SSSE3=1 -DCONFIG_AS_CRC32=1 -DCONFIG_AS_AVX=1 -DCONFIG_AS_AVX2=1 -DCONFIG_AS_AVX512=1 -DCONFIG_AS_SHA1_NI=1 -DCONFIG_AS_SHA256_NI=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -O2 --param=allow-store-data-races=0 -DCC_HAVE_ASM_GOTO -Wframe-larger-than=1024 -fstack-protector-strong -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-var-tracking-assignments -pg -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wno-pointer-sign -Wno-array-bounds -Wno-maybe-uninitialized -fno-strict-overflow -fno-merge-all-constants -fmerge-constants -fno-stack-check -fconserve-stack -Werror=implicit-int -Werror=strict-prototypes -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -D__READ_ONCE_DEFINED__ -DLINUX -D__KERNEL__ -DMODULE -O2 -pipe -Wall -Werror -DGITSP -DGIT -DKERNEL_HAS_KCONFIG_H -DKERNEL_HAS_EXPORT_H -DDEBUGFS -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma -DUBUNTU_VERSION_CODE -DUBUNTU_VERSION_CODE -D__QDMA_VF__ -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/../include -I/lib/modules/4.15.0-128-generic/build/../include -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/../libqdma/qdma_access/qdma_soft_access -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/../libqdma/qdma_access/eqdma_soft_access -I/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/../libqdma/qdma_access/qdma_s80_hard_access -I.  -DMODULE  -DKBUILD_BASENAME='"qdma_mbox_protocol"'  -DKBUILD_MODNAME='"qdma_vf"' -c -o /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.c

source_/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o := /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.c

deps_/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o := \
  include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/gcov/kernel.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.h \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_platform_env.h \
  include/linux/types.h \
    $(wildcard include/config/have/uid16.h) \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/arch/dma/addr/t/64bit.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  include/uapi/linux/types.h \
  arch/x86/include/uapi/asm/types.h \
  include/uapi/asm-generic/types.h \
  include/asm-generic/int-ll64.h \
  include/uapi/asm-generic/int-ll64.h \
  arch/x86/include/uapi/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/uapi/asm-generic/bitsperlong.h \
  include/uapi/linux/posix_types.h \
  include/linux/stddef.h \
  include/uapi/linux/stddef.h \
  arch/x86/include/asm/posix_types.h \
    $(wildcard include/config/x86/32.h) \
  arch/x86/include/uapi/asm/posix_types_64.h \
  include/uapi/asm-generic/posix_types.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/atomic/sleep.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/arch/has/refcount.h) \
    $(wildcard include/config/lock/down/kernel.h) \
    $(wildcard include/config/panic/timeout.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /usr/lib/gcc/x86_64-linux-gnu/5/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/stringify.h \
  include/linux/export.h \
    $(wildcard include/config/have/underscore/symbol/prefix.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/module/rel/crcs.h) \
    $(wildcard include/config/trim/unused/ksyms.h) \
    $(wildcard include/config/unused/symbols.h) \
  arch/x86/include/asm/linkage.h \
    $(wildcard include/config/x86/64.h) \
    $(wildcard include/config/x86/alignment/16.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/kasan.h) \
  arch/x86/include/asm/barrier.h \
    $(wildcard include/config/x86/ppro/fence.h) \
  arch/x86/include/asm/alternative.h \
    $(wildcard include/config/smp.h) \
  arch/x86/include/asm/asm.h \
  arch/x86/include/asm/nops.h \
    $(wildcard include/config/mk7.h) \
    $(wildcard include/config/x86/p6/nop.h) \
  include/asm-generic/barrier.h \
  include/linux/kasan-checks.h \
  include/linux/bitops.h \
  include/linux/bits.h \
  arch/x86/include/asm/bitops.h \
    $(wildcard include/config/x86/cmov.h) \
  arch/x86/include/asm/rmwcc.h \
  include/asm-generic/bitops/find.h \
    $(wildcard include/config/generic/find/first/bit.h) \
  include/asm-generic/bitops/sched.h \
  arch/x86/include/asm/arch_hweight.h \
  arch/x86/include/asm/cpufeatures.h \
  arch/x86/include/asm/required-features.h \
    $(wildcard include/config/x86/minimum/cpu/family.h) \
    $(wildcard include/config/math/emulation.h) \
    $(wildcard include/config/x86/pae.h) \
    $(wildcard include/config/x86/cmpxchg64.h) \
    $(wildcard include/config/x86/use/3dnow.h) \
    $(wildcard include/config/matom.h) \
    $(wildcard include/config/x86/5level.h) \
    $(wildcard include/config/paravirt.h) \
  arch/x86/include/asm/disabled-features.h \
    $(wildcard include/config/x86/intel/mpx.h) \
    $(wildcard include/config/x86/intel/umip.h) \
    $(wildcard include/config/x86/intel/memory/protection/keys.h) \
    $(wildcard include/config/page/table/isolation.h) \
  include/asm-generic/bitops/const_hweight.h \
  include/asm-generic/bitops/le.h \
  arch/x86/include/uapi/asm/byteorder.h \
  include/linux/byteorder/little_endian.h \
    $(wildcard include/config/cpu/big/endian.h) \
  include/uapi/linux/byteorder/little_endian.h \
  include/linux/swab.h \
  include/uapi/linux/swab.h \
  arch/x86/include/uapi/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/ext2-atomic-setbit.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/typecheck.h \
  include/linux/printk.h \
    $(wildcard include/config/message/loglevel/default.h) \
    $(wildcard include/config/console/loglevel/default.h) \
    $(wildcard include/config/early/printk.h) \
    $(wildcard include/config/printk/nmi.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/kmsg/ids.h) \
    $(wildcard include/config/dynamic/debug.h) \
  include/linux/init.h \
    $(wildcard include/config/strict/kernel/rwx.h) \
    $(wildcard include/config/strict/module/rwx.h) \
  include/linux/kern_levels.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/uapi/linux/kernel.h \
  include/uapi/linux/sysinfo.h \
  arch/x86/include/asm/cache.h \
    $(wildcard include/config/x86/l1/cache/shift.h) \
    $(wildcard include/config/x86/internode/cache/shift.h) \
    $(wildcard include/config/x86/vsmp.h) \
  include/linux/dynamic_debug.h \
    $(wildcard include/config/jump/label.h) \
  include/linux/jump_label.h \
  arch/x86/include/asm/jump_label.h \
  include/linux/build_bug.h \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_access_common.h \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_access_export.h \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_access_errors.h \
    $(wildcard include/config/bar.h) \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_resource_mgmt.h \
  /scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_platform.h \

/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o: $(deps_/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o)

$(deps_/scratch/vmk180_trd_platform2_2020.2/pcie_host_package/qdma/driver/src/libqdma/qdma_access/qdma_mbox_protocol.o):
