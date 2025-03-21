// Methods that can't be generated all at the same time automatically in Schema_generated.h, 
// that allow us to use multiple types as `root_type`.


#ifndef ROOT_TYPE_FINISHING_METHODS_H_
#define ROOT_TYPE_FINISHING_METHODS_H_

#include "flatbuffers/flatbuffers.h"
#include "Schema_generated.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace phraser {

// === Word === 
// TODO: (test only, remove eventually)

inline const phraser::Word *GetWord(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::Word>(buf);
}

inline const phraser::Word *GetSizePrefixedWord(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::Word>(buf);
}

inline Word *GetMutableWord(void *buf) {
  return ::flatbuffers::GetMutableRoot<Word>(buf);
}

inline phraser::Word *GetMutableSizePrefixedWord(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::Word>(buf);
}

inline bool VerifyWordBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::Word>(nullptr);
}

inline bool VerifySizePrefixedWordBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::Word>(nullptr);
}

inline void FinishWordBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::Word> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedWordBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::Word> root) {
  fbb.FinishSizePrefixed(root);
}

// === PhraseBlock ===

inline const phraser::PhraseBlock *GetPhraseBlock(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::PhraseBlock>(buf);
}

inline const phraser::PhraseBlock *GetSizePrefixedPhraseBlock(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::PhraseBlock>(buf);
}

inline PhraseBlock *GetMutablePhraseBlock(void *buf) {
  return ::flatbuffers::GetMutableRoot<PhraseBlock>(buf);
}

inline phraser::PhraseBlock *GetMutableSizePrefixedPhraseBlock(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::PhraseBlock>(buf);
}

inline bool VerifyPhraseBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::PhraseBlock>(nullptr);
}

inline bool VerifySizePrefixedPhraseBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::PhraseBlock>(nullptr);
}

inline void FinishPhraseBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::PhraseBlock> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPhraseBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::PhraseBlock> root) {
  fbb.FinishSizePrefixed(root);
}

// === SymbolSetsBlock ===

inline const phraser::SymbolSetsBlock *GetSymbolSetsBlock(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::SymbolSetsBlock>(buf);
}

inline const phraser::SymbolSetsBlock *GetSizePrefixedSymbolSetsBlock(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::SymbolSetsBlock>(buf);
}

inline SymbolSetsBlock *GetMutableSymbolSetsBlock(void *buf) {
  return ::flatbuffers::GetMutableRoot<SymbolSetsBlock>(buf);
}

inline phraser::SymbolSetsBlock *GetMutableSizePrefixedSymbolSetsBlock(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::SymbolSetsBlock>(buf);
}

inline bool VerifySymbolSetsBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::SymbolSetsBlock>(nullptr);
}

inline bool VerifySizePrefixedSymbolSetsBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::SymbolSetsBlock>(nullptr);
}

inline void FinishSymbolSetsBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::SymbolSetsBlock> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSymbolSetsBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::SymbolSetsBlock> root) {
  fbb.FinishSizePrefixed(root);
}

// === PhraseTemplatesBlock ===

inline const phraser::PhraseTemplatesBlock *GetPhraseTemplatesBlock(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::PhraseTemplatesBlock>(buf);
}

inline const phraser::PhraseTemplatesBlock *GetSizePrefixedPhraseTemplatesBlock(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::PhraseTemplatesBlock>(buf);
}

inline PhraseTemplatesBlock *GetMutablePhraseTemplatesBlock(void *buf) {
  return ::flatbuffers::GetMutableRoot<PhraseTemplatesBlock>(buf);
}

inline phraser::PhraseTemplatesBlock *GetMutableSizePrefixedPhraseTemplatesBlock(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::PhraseTemplatesBlock>(buf);
}

inline bool VerifyPhraseTemplatesBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::PhraseTemplatesBlock>(nullptr);
}

inline bool VerifySizePrefixedPhraseTemplatesBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::PhraseTemplatesBlock>(nullptr);
}

inline void FinishPhraseTemplatesBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::PhraseTemplatesBlock> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPhraseTemplatesBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::PhraseTemplatesBlock> root) {
  fbb.FinishSizePrefixed(root);
}

// === KeyBlock ===

inline const phraser::KeyBlock *GetKeyBlock(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::KeyBlock>(buf);
}

inline const phraser::KeyBlock *GetSizePrefixedKeyBlock(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::KeyBlock>(buf);
}

inline KeyBlock *GetMutableKeyBlock(void *buf) {
  return ::flatbuffers::GetMutableRoot<KeyBlock>(buf);
}

inline phraser::KeyBlock *GetMutableSizePrefixedKeyBlock(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::KeyBlock>(buf);
}

inline bool VerifyKeyBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::KeyBlock>(nullptr);
}

inline bool VerifySizePrefixedKeyBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::KeyBlock>(nullptr);
}

inline void FinishKeyBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::KeyBlock> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedKeyBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::KeyBlock> root) {
  fbb.FinishSizePrefixed(root);
}

// === FoldersBlock ===

inline const phraser::FoldersBlock *GetFoldersBlock(const void *buf) {
  return ::flatbuffers::GetRoot<phraser::FoldersBlock>(buf);
}

inline const phraser::FoldersBlock *GetSizePrefixedFoldersBlock(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<phraser::FoldersBlock>(buf);
}

inline FoldersBlock *GetMutableFoldersBlock(void *buf) {
  return ::flatbuffers::GetMutableRoot<FoldersBlock>(buf);
}

inline phraser::FoldersBlock *GetMutableSizePrefixedFoldersBlock(void *buf) {
  return ::flatbuffers::GetMutableSizePrefixedRoot<phraser::FoldersBlock>(buf);
}

inline bool VerifyFoldersBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<phraser::FoldersBlock>(nullptr);
}

inline bool VerifySizePrefixedFoldersBlockBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<phraser::FoldersBlock>(nullptr);
}

inline void FinishFoldersBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::FoldersBlock> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedFoldersBlockBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<phraser::FoldersBlock> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace phraser

#endif  // ROOT_TYPE_FINISHING_METHODS_H_
