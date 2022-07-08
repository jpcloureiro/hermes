/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_BIGINT_H
#define HERMES_SUPPORT_BIGINT_H

#include "hermes/Support/Compiler.h"

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MathExtras.h"

#include <optional>
#include <vector>

namespace hermes {
namespace bigint {

using BigIntDigitType = uint64_t;

static constexpr size_t BigIntDigitSizeInBytes = sizeof(BigIntDigitType);
static constexpr size_t BigIntDigitSizeInBits = BigIntDigitSizeInBytes * 8;

/// Arbitrary upper limit on number of Digits a bigint may have.
static constexpr size_t BigIntMaxSizeInDigits = 0x400; // 1k digits == 8k bytes

/// Helper function that should be called before allocating a Digits array on
/// the stack.
inline constexpr bool tooManyDigits(unsigned numDigits) {
  return BigIntMaxSizeInDigits < numDigits;
}

/// \return number of BigInt digits to represent \p v bits.
inline size_t numDigitsForSizeInBits(uint32_t v) {
  return static_cast<size_t>(llvh::alignTo(v, BigIntDigitSizeInBits)) /
      BigIntDigitSizeInBits;
}

/// \return number of BigInt digits to represent \p v bytes.
inline size_t numDigitsForSizeInBytes(uint32_t v) {
  return static_cast<size_t>(llvh::alignTo(v, BigIntDigitSizeInBytes)) /
      BigIntDigitSizeInBytes;
}

/// Returns another view of \p src where high order bytes that are just used
/// for sign extension are dropped. Returns an empty ArrayRef if all bytes in \p
/// src are zero.
llvh::ArrayRef<uint8_t> dropExtraSignBits(llvh::ArrayRef<uint8_t> src);

/// \return the byte value that represents the sign extension of \p byte.
/// I.e., returns 0 if \p byte is 0b0xxxxxxx, and ~0, if \p byte is
/// 0x1xxxxxxx.
template <typename T>
static constexpr T getSignExtValue(uint8_t byte) {
  // We rely on the unsigned (i.e., "logical") shift right to convert the sign
  // bit to [0, 1], then do 0 - [0, 1] to get 0ull or ~0ull as the sign
  // extension value.
  uint64_t signExtValue = 0ull - (byte >> 7);

  // But still possibly truncate the value as requested by the caller.
  return static_cast<T>(signExtValue);
}

/// MutableBigIntRef is used to represent bigint payloads that are mutated
/// by any of the API functions below. Note how numDigits is a reference to
/// numDigits - the API may modify that in order to canonicalize the payloads.
struct MutableBigIntRef {
  BigIntDigitType *digits;
  uint32_t &numDigits;
};

/// The "catch-all" enum type with the possible return type from the bigint
/// APIs. It contains all possible errors that any bigint API function could
/// return (e.g., division by zero), even for functions that never return them.
enum class OperationStatus : uint32_t {
  RETURNED,
  DEST_TOO_SMALL,
} HERMES_ATTRIBUTE_WARN_UNUSED_RESULT_TYPE;

/// Initializes \p dst with \p data, sign-extending the bits as needed.
OperationStatus initWithBytes(
    MutableBigIntRef dst,
    llvh::ArrayRef<uint8_t> data);

} // namespace bigint
} // namespace hermes

#endif // HERMES_SUPPORT_BIGINT_H