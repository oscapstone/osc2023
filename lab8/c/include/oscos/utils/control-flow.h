#ifndef OSCOS_UTILS_CONTROL_FLOW_H
#define OSCOS_UTILS_CONTROL_FLOW_H

/// \brief Used to tell an operation whether it should exit early or go on as
///        usual.
///
/// This is modelled after Rust's `std::ops::ControlFlow`, but this doesn't
/// carry values.
typedef enum { CF_CONTINUE, CF_BREAK } control_flow_t;

#endif
