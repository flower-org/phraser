# Features:

- Folder view functions - move up, move down in the list (Folders and Phrases)
- mb Make `typing_delay_ms` configurable in KeyBlock (block flatbuf structure change)?
- Wipe/randomize all the way up to the bank boundary on DB Restore/DB Create, even with less blocks in DB.
- "Settings" section
  - Phrase Templates
    - Safe deletes (also index phrase templates for phrases)
  - Word Templates
    - new icon "W"
  - Symbol Sets (Aa)
- Copy DB from bank to bank
- Update MasterPassword (not sure how safe that would be to do in place, but adding an option to copy the updated DB to another bank is definitely a good idea.)
- In situations when B0 == B1, we copy the same block twice, which can be improved.

# Fundamental:

### 1. Make USB keyboard work in BIOS.

mb.: see my fork of usb-metamorph
https://github.com/codekrolik2/usb-metamorph/blob/master/USBSerPassThruLine/README.md


# Afterthoughts:

### 1. Block header structure

There is an inefficiency in our `BlockCache` implementation (`registerBlockInBlockCache(...)`) that stems from the fact that historically we placed a few fields that are core to Block DB inside the application-specific FlatBuf structure.
Namely, those are `BlockVersion` and `Tombstoned` status.

Since those fields are not accessible directly from the block header, like BlockType is for example, we can't run `2. Update DB structures` before we decode FlatBuf, because we need to know `Version` and `Tombstoned` status to do that.

For that reason we implemented the core DB Update flow as follows:
  1. We update Domain-specific caches first, because FlatBuf deserialization is a part of that;
  2. We obtain `Version` and `Tombstoned` status from the deserialized FlatBuf, and that allows us to perform core DB RAM structure/indexes update.

One consequence of such order of actions is that we have to explicitly check that the block which we update our caches with
  is the last known version, and we have to perform this check in each individual method for each block type as a part of `1. Update caches`.
If we had `Version` and `Tombstone` status in raw block header, we would be able to run `2. Update DB structures` initially,
  and as a part of that obtain enough information to determine whether the block we're processing is outdated or not.
With that knowledge, it's possible to decide whether we should or shouldn't run `1. Update caches` for a given block more generally for any block type, and get rid of BlockType-specific checks in those methods.

The inaccuracy of current placement of those fields is briefly mentioned in `APDX_A. Phraser Block Storage notes.pdf` (see PhraserManager repo).
Apart from making the base Block DB structure less reusable, it causes practical inconveniences as well, like the one described above.

However, straightening this out is not likely to happen in Phraser, since the potential impact / improvement for this particular (non-commercial) project is hardly worth the effort required.

With that said, if BlockDB technology is to be reused in any other projects, it's strongly recommended to make this structural change before everything else. If there are any afterthoughts in Phraser project, this is the most important of them all.

### 2. UI

It wasn't one of the primary goals of this project to implement a well-designed and robust UI component library.
While current UI does look good (at least to me) and is pretty fast, there was never an emphasis (especially given other priorities) on producing a solid implementation, let alone a complete UI engine. On the contrary, the goal was to put something together ASAP to avoid blocking the development of other features.
That's why curently the UI code is very spaghetti-like and repetitive.

If this project will find its continuation, it's worth remembering that it contains a technical debt in the form of serious UI refactoring.  
No specific design ideas yet, but one thought is that it would probably make sense to switch to C++ to define UI flows and components as classes.