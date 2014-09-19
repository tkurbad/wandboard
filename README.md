wandboard
=========

Wandboard (Android) Resources

Android 4.4.2 Setup
===================

Partition the Micro SD card and create appropriate filesystems
according to the following table.

I recommend to use GNU parted.

Number  Start   End     Size    Type      File system  Flags  Label
 1      7746kB  77.5MB  69.7MB  primary   fat32        lba    imx6
 2      77.5MB  178MB   101MB   primary   ext4                rootfs
 3      178MB   15.8GB  15.6GB  extended
 5      179MB   3138MB  2959MB  logical   ext4                system
 6      3139MB  3678MB  539MB   logical   ext4                cache
 7      3679MB  3692MB  12.6MB  logical   ext4                vendor
 8      3693MB  3706MB  12.6MB  logical   ext4                misc
 9      3707MB  15.8GB  12.1GB  logical   ext4                data

Partition number 9 can be freely sized depending on the size
of the card.

Partition number 1 is FAT32:

  mkfs.vfat -F 32 -n imx6 /dev/sdX1

All other partitions are ext4:

  mkfs.ext4 -L <Label> /dev/sdXY

Mount partition number 1 to a temporary location and populate
it with the contents of the ./Android-4.4.2/imx6/ directory:

  mkdir -p /mnt/tmp
  mount /dev/sdX1 /mnt/tmp
  rsync -a --progress ./Android-4.4.2/imx6/ /mnt/tmp/
  umount /mnt/tmp

Download the system archive (rooted and with gapps) to a temporary
location:

  cd /tmp
  wget http://www.tk-webart.de/projekte/wandboard/2014-09-18-wandboard-android-4.4.2-system_rooted+gapps.tar.xz

Mount partition number 5 to a temporary location and populate
it with the contents of the previously downloaded archive:

  mount /dev/sdX5 /mnt/tmp
  cd /mnt/tmp
  tar xpf /tmp/2014-09-18-wandboard-android-4.4.2-system_rooted+gapps.tar.xz
  cd
  umount /mnt/tmp
  sync

Eject the Micro SD card and insert it into the Wandboard's
CPU module slot.

Boot up and enjoy Android 4.4. 'KitKat'!
