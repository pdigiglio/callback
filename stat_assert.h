#pragma once

/**
 * \brief Replicate the behavior of `static_assert`. The idea is to have a class
 * template that is only complete in the `true` case (see below).
 */
template <bool> //
struct stat_assert;

/**
 * \brief Specialize the `true` case to be completed.
 */
template <> //
struct stat_assert<true> {
  /**
   * \brief Ctor.
   * \param reason The reason for the failure
   */
  explicit stat_assert(char const *reason = 0) { (void)reason; }
};
