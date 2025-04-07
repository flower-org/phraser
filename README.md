# WIP! First release coming soon.
# Phraser - USB Hardware Password Manager

![phraser.jpg](phraser.jpg)


Phraser is a compact and secure USB hardware password manager designed to keep your passwords safe and easily accessible. Built on the Thumby platform, which is based on the RP2040 microcontroller, Phraser combines portability with robust security features.

## Features

- **Secure Storage**: Stores your passwords securely with AES-256 encryption.
- **Compact Design**: Keychain form factor for portability (props to [Thumby](https://thumby.us/) team).
- **Emulates USB Keyboard**: Helps you to fill in pass- and other -words by emulating a USB keyboard.
- **Open Source**: Fully open-source firmware, Apache 2.
- **Flash-friendly DB design**: Facilitates uniform flash sector wear and bit rot protection ([more info](https://github.com/flower-org/PhraserManager/blob/main/1.%20Phraser%20DB%20-%20Optimizing%20Flash%20Wear%20and%20Bit%20Rot.md)).
- **Up to 3 Password Banks**: Up to 3 independent password databases, each with it's own password.

## Phraser Manager

[Phraser Manager](https://github.com/flower-org/PhraserManager) is a desktop application designed to work seamlessly with your Phraser device and Phraser DB backups. It provides a user-friendly cross-platform interface for managing your password databases with the following features:

- **Backup Password Database**: Create backups of your password database to ensure your data is safe and recoverable.
- **Restore Password Database**: Easily restore your password database from a backup file.
- **Modify Password Entries**: Update existing password entries or metadata in DB backup files from the desktop application.
- **DB Maintenance**: Compact, resize, change AES key, change password etc.

## Prerequisites

- [Thumby](https://thumby.us/) device
- Micro-USB cable for connection  

## Acknowledgements
- Special thanks to [Kirill Emelyanov](https://github.com/emelyanovkr) for the PlatformIO setup assistance.

## Credits
- Raspberry Pi Foundation for RP2040 microcontroller
  - https://www.raspberrypi.com/products/rp2040/
- TinyCircuits for amazing Thumby system and SDK
  - https://thumby.us/
  - https://github.com/TinyCircuits/TinyCircuits-Thumby-Lib/
  - https://github.com/TinyCircuits/TinyCircuits-GraphicsBuffer-Lib
- Platform IO for Raspberry Pi RP2040 platform
  - https://github.com/platformio/platform-raspberrypi
- Max Gerhardt for maintaining a fork of the above platform
  - https://github.com/maxgerhardt/platform-raspberrypi
- Earle F. Philhower, III for Arduino-Pico
  - https://github.com/earlephilhower/arduino-pico
- Google, Inc. for FlatBuffers technology
  - https://github.com/google/flatbuffers
- Divide Labs for FlatCC - FlatBuffers in C
  - https://github.com/dvidelabs/flatcc
- kokke for blazingly fast AES implementation
  - https://github.com/kokke/tiny-AES-c
- Rob Tillaart for Adler32 checksum implementation
  - https://github.com/RobTillaart/Adler
- PolarSSL and Dhiru Kholia for PBKDF2 and SHA256
  - https://github.com/kholia/PKCS5_PBKDF2
- Hyeon Kim for Tiny Red-Black Tree (modified version is used)
  - https://github.com/simnalamburt/tiny-rbtree
- marekweb for Arraylist and Hashtable in C (modified version is used)
  - https://github.com/marekweb/datastructs-c

## Notable project milestones

- Apr 06, 2025: First real life use for actual passwords.
  - Migrated passwords for several accounts to Phraser.


- Apr 07, 2025: DB Backup lifecycle management IRL
  - In ideal world, these operations should be performed on an offline machine:
  - Create DB backup file using Phraser Manager;
  - Encrypt the backup file with Cryptor (RSA);
  - Date the encrypted file;
  - Store the encrypted file reliably - multiple copies, local, cloud.
  - DB is encrypted, why encrypt it again with RSA?
    - Yes, the DB is always encrypted, but if somebody gets a hold of a backup file, they theoretically might still bruteforce its master pass (even though it's very hard given PBKDF2, double-layered encryption with Key Block and NO metadata leaks).
    - To reduce that risk to absolute minimum, I additionally encrypt backup files with RSA, which is not considered bruteforceable.
    - Even if the RSA private key gets compromised, one would still have to decrypt the DB itself.
    - To make master pass bruteforcing even harder, use non-standard number of PBKDF2 iterations (UP+A+B on Unseal).
    - Also, changing your passwords regularly will reduce the usefullness of outdated backups.
