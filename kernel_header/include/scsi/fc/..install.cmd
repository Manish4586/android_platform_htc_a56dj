cmd_/home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc/.install := perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include/scsi/fc /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc arm fc_els.h fc_fs.h fc_gs.h fc_ns.h; perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/include/scsi/fc /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc arm ; perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/include/generated/scsi/fc /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc/$$F; done; touch /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/scsi/fc/.install
