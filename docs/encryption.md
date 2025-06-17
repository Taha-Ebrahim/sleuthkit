# Encryption and Encrypted File Systems with The Sleuth Kit

* We supposedly have encryption support for APFS and NTFS volume level encyrption.
* We also supposeduly have support for FAT32 with EFS file level encryption. 

The APFS encryption pipeline is relatively confusing. The standard usage includes first calling `mmls` on the disk image, however, I cannot get this to work on encrypted filesystems. 

Without calling `mmls` we have to guess the `pstat` file offset for where the APFS volume begins. This is relatively straightforward and can be done with a script, however, `pstat` as it is currently does not seem to have support for encrypted filesystems. Even if we guess the correct offset there would be no way to use it. There is no way to get the APSB super number that is required for APFS with encrypted filesystems. 

After we get the encrypted filesystem super block number, the offset and the encyrpted key we can then use `fls` which has support for a password. 

As it stands currently there does not seem to be any way to use an encrypted APFS volume with the Sleuth Kit, at least in the way that it is generally used. 

