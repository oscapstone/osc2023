#ifndef OSCOS_ALIGN_H
#define OSCOS_ALIGN_H

/// \brief Returns \p X aligned to multiples of \p A.
///
/// This macro is usually used to align a pointer. When doing so, cast the
/// pointer to `uintptr_t` and cast the result back to the desired pointer type.
///
/// \param X The value to be aligned. Must be of an arithmetic type.
/// \param A The alignment.
#define ALIGN(X, A) (((X) + (A)-1) / (A) * (A))

#endif
