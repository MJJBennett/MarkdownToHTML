// Testing include
#include "gtest/gtest.h"

// File to test
#include "mdconverter.hpp"

template<typename T>
::testing::AssertionResult IsEqual(mdc::Marker<T> lhs, mdc::Marker<T> rhs)
{
    // If they're equal, no need to be fancy
    if (lhs == rhs) return ::testing::AssertionSuccess();
    // If they aren't equal, print additional information
    return ::testing::AssertionFailure() << "\n\tLeft Mark: " << mdc::util::markToString(lhs.mark_) << "\n\tRight Mark: " << mdc::util::markToString(rhs.mark_);
}
