# Encryption and Encrypted File Systems with The Sleuth Kit

* TSK supports APFS and NTFS volume level encryption.

## APFS Encryption Pipeline: 
* TSK provides some support for encrypted APFS file systems, however as it currently stands there are a number of limitations that make it difficult to use. 

* As it was demonstrated when it was released, `mmls` must be called on the disk image in order to find the offset with which to call `pstat`. 

* `pstat` is able to give us the APSB super block number which is required for calling tools like `fls`, `istat` and `icat`. 

* As it currently stands `pstat` does not have support for encrypted filesystems, this means that we are unable to get the APSB super block number without decrypting the disk image outside of TSK and calling `pstat` on the decrypted disk image. 

### Notes: 
* This is one specific use-case that is based on the demonstration in [APFS Support and Demo](https://www.youtube.com/watch?v=k1XPillJ7aw&t=424s). 
* This is based on an APFS disk image that was created and encrypted using macOS Disk Utility.

## Issues Requiring Resolution:
* There is limited support for getting the APSB super block number that is required for calling TSK tools with APFS disk images.





