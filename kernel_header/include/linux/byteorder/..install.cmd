cmd_/home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder/.install := perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/include/linux/byteorder /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder arm big_endian.h little_endian.h; perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/include/linux/byteorder /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder arm ; perl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/kernel/scripts/headers_install.pl /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/include/generated/linux/byteorder /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder arm ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder/$$F; done; touch /home/aa/Project/A56DJDUGL_HTC_WWE_M601_CRC_Sense80GP_Stable_503111/out/target/product/htc_a56djdugl/obj/KERNEL_OBJ/usr/include/linux/byteorder/.install