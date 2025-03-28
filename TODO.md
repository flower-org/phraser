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