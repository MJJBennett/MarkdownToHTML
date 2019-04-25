// Testing include
#include "gtest/gtest.h"

// File to test
#include "mdconverter.hpp"

TEST(BasicUtilities, remove_codeblock1)
{
    using T = std::string;
    using IT = T::const_iterator;

    /* Test - Remove leading tab
     * Should remove a leading tab and leave the space.
     */
    T codestring1("\t code block");
    IT itr = codestring1.begin();

    EXPECT_TRUE(mdc::remove_codeblock(itr));

    T codestring1_result(itr, codestring1.cend());

    EXPECT_EQ(codestring1_result, " code block");

    /* Test - Remove leading space (up to 3) and tab
     * Should remove a leading tab and the space.
     */
    T codestring2(" \tcode block\t\t    \t");
    itr = codestring2.begin();

    EXPECT_TRUE(mdc::remove_codeblock(itr));

    T codestring2_result(itr, codestring2.cend());

    EXPECT_EQ(codestring2_result, "code block\t\t    \t");

    /* Test - Remove leading space (up to 3)
     * Should remove leading spaces but return false.
     */
    T codestring3("   this is your average string, nothing special");
    itr = codestring3.begin();

    EXPECT_FALSE(mdc::remove_codeblock(itr));

    T codestring3_result(itr, codestring3.cend());

    EXPECT_EQ(codestring3_result, "this is your average string, nothing special");
}
