1. List control
    - scrolling text
    - use to implement a menu
        - Unseal, Download DB, Upload DB, Unseal (show password)

2. Interactions over Serial port
    - Download DB and Upload DB
    - Practical test - Upload, then Download
        - compare files

2. OnScreenKeyboard enhancements
    - password entry mode
    - use to Unseal / Unseal (show passworD)

3. Test PBKDF2

---

Cleanup:

1. Get rid of StructBuf
    - superceded by FlatBuf
    - 

2. Removing StructBuf will cause Registry to stop compiling
    - Registry needs to be ported to FlatBuf
    - Or removed altogether, investigate which is preferable

3. PhraserUtils - name too generic? move icons/symbols to SpecialSymbolUtil?

---

Fundamental:

1. Make USB keyboard work in BIOS.