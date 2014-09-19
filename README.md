Wandboard
=========

Wandboard (Android) Resources.

Android 4.4.2 considerations
============================

The official Wandboard KitKat image has three major flaws:

1. The partition table is highly unusual, placing the third
   primary partition, i.e. `'data'`, **behind** the extended
   partition. That is just broken by design.

2. The official *Google apps* are missing (which isn't the
   fault of the Wandboard folks, btw.)

3. If you managed to get the *Google apps* running, you could
   not install **paid** apps from the *PlayStore*, since some
   mandatory kernel options are missing.

I tried to fix these issues as follows:

1. I changed the `'data'` partition to be the last *logical*
   partition, thus making it easy to size (i.e. you can take
   the remaining free space of the SD card for it, after
   creating all the other partitions.)
   To reflect the change in partition numbering, I compiled
   a new `uramdisk.img` file that does the mounting correctly.

2. I incorporated the official *Google apps* and *SuperSU (root)*
   into my `'system'` archive.

3. I compiled a new `uImage` with the appropriate options.
   Also, I replaced the *Wifi* modules in the `'system'` image
   to match my kernel. Thus, if you just want to use this
   `uImage`, to keep workable *Wifi*, you **need to copy**
   `./Android-4.4.2/kernel/brcmfmac.ko` and 
   `./Android-4.4.2/kernel/brcmutil.ko` to the `./bin/wifi/`
   folder of your system partition.

Android 4.4.2 Setup
===================

First of all, download the official Wandboard KitKat image and
write it to your card:

```bash
  cd /tmp
  wget http://wandboard.org/images/downloads/android-4.4.2-wandboard-20140815.zip
  unzip android-4.4.2-wandboard-20140815.zip
  sudo dd if=android-4.4.2-wandboard-20140815.img of=/dev/sdX bs=1M
```

This will take a while.

Now, repartition the Micro SD card and create appropriate filesystems
according to the following table. **Make sure to leave the first
7 Megabytes of the card as they are since they contain the
bootloader.**

I recommend using GNU parted for the re-partitioning task.

```
Number  Start   End     Size    Type      File system  Flags  Label
 1      7746kB  77.5MB  69.7MB  primary   fat32        lba    imx6
 2      77.5MB  178MB   101MB   primary   ext4                rootfs
 3      178MB   15.8GB  15.6GB  extended
 5      179MB   3138MB  2959MB  logical   ext4                system
 6      3139MB  3678MB  539MB   logical   ext4                cache
 7      3679MB  3692MB  12.6MB  logical   ext4                vendor
 8      3693MB  3706MB  12.6MB  logical   ext4                misc
 9      3707MB  15.8GB  12.1GB  logical   ext4                data
```
Partition number 9 can be freely sized depending on the size
of the card.

Partition number 1 is FAT32:

```bash
  sudo mkfs.vfat -F 32 -n imx6 /dev/sdX1
```

All other partitions are ext4:

```bash
  sudo mkfs.ext4 -L <Label> /dev/sdXY
```
Mount partition number 1 to a temporary location and populate
it with the contents of the ./Android-4.4.2/imx6/ directory:

```bash
  sudo mkdir -p /mnt/tmp
  sudo mount /dev/sdX1 /mnt/tmp
  sudo rsync -a --progress ./Android-4.4.2/imx6/ /mnt/tmp/
  sudo umount /mnt/tmp
```

Download the system archive (rooted and with gapps) to a temporary
location:

```bash
  cd /tmp
  wget http://www.tk-webart.de/projekte/wandboard/2014-09-18-wandboard-android-4.4.2-system_rooted+gapps.tar.xz
```

Mount partition number 5 to a temporary location and populate
it with the contents of the previously downloaded archive:

```bash
  sudo mount /dev/sdX5 /mnt/tmp
  cd /mnt/tmp
  sudo tar xpf /tmp/2014-09-18-wandboard-android-4.4.2-system_rooted+gapps.tar.xz
  cd
  sudo umount /mnt/tmp
  sudo sync
```

Eject the Micro SD card and insert it into the Wandboard's
CPU module slot.

Boot up and enjoy Android 4.4. 'KitKat'!
