
# Fundamental:

### 1. Make USB keyboard work in BIOS.

mb.: see my fork of usb-metamorph
https://github.com/codekrolik2/usb-metamorph/blob/master/USBSerPassThruLine/README.md


# Afterthoughts:

### 1. Block header structure

  There is an inefficiency in our Block Cache implementation that stems from the fact that historically we placed a few fields
    that are core to Block DB inside the FlatBuf structure.
  Namely, those are `BlockVersion` and `Tombstoned` flag.
  Since those values are not accessible directly from the block header, like BlockType is for example, we can't run `2. Update DB structures` before we decode FlatBuf, because we need to know Version and Tombstoned status for that.
  For that reason the way we implemented this core flow is as follows:
   1. We update Domain-specific caches first, because a part of that is FlatBuf deserialization;
   2. We obtain `Version` and `Tombstoned` status from the deserialized FlatBuf, and that allows us to perform DB structure update.
  
  One consequence of said order of actions is that we have to explicitly check that the block which we update our caches with
    is the last known version, and we have to perform this check in each individual method for each block type as a part of `1. Update caches`
  If we had `Version` and `Tombstone` status in raw block header, we would be able to run `2. Update DB structures` first,
    and as a par tof that obtain enough information to determine whether the block we're processing is outdated or not.
  With that knowledge, it's possible to decide whether we should or shouldn't run `1. Update caches` for any block type, and get rid of 
    BlockType-specific checks in those methods.
  
  The inconvenience of current placement of those fields is briefly mentioned in `APDX_A. Phraser Block Storage notes.pdf` (see PhraserManager repo).
  Apart from making the base Block DB structure less reusable, it causes practical inconveniences as well, like the one describe above.
  
  However, straightening this out is not likely to happen in Phraser, since the impact on this particular project is hardly worth the effort required.
  With that said, if BlockDB technology is to be reused in any other projects, it's highly recommended to make this structural improvement before 
    everything else. If there are any afterthoughts in Phraser project, this is the most important of them all.
