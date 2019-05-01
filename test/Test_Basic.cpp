#include "Test_Helpers.hpp"

TEST(BasicUtilities, remove_codeblock1)
{
    using T  = std::string;
    using IT = T::const_iterator;

    /* Test - Remove leading tab
     * Should remove a leading tab and leave the space.
     */
    T codestring1("\t code block");
    IT itr = codestring1.begin();
    IT end  = codestring1.end();

    EXPECT_TRUE(mdc::remove_codeblock(itr, end));

    T codestring1_result(itr, codestring1.cend());

    EXPECT_EQ(codestring1_result, " code block");

    /* Test - Remove leading space (up to 3) and tab
     * Should remove a leading tab and the space.
     */
    T codestring2(" \tcode block\t\t    \t");
    itr = codestring2.begin();
    end  = codestring2.end();

    EXPECT_TRUE(mdc::remove_codeblock(itr, end));

    T codestring2_result(itr, codestring2.cend());

    EXPECT_EQ(codestring2_result, "code block\t\t    \t");

    /* Test - Remove leading space (up to 3)
     * Should remove leading spaces but return false.
     */
    T codestring3("   this is your average string, nothing special");
    itr = codestring3.begin();
    end  = codestring3.end();

    EXPECT_FALSE(mdc::remove_codeblock(itr, end));

    T codestring3_result(itr, codestring3.cend());

    EXPECT_EQ(codestring3_result, "this is your average string, nothing special");
}

TEST(BasicMarkersPre, get_markers)
{
    using T  = std::string;
    using IT = T::const_iterator;

    /* This string is preformatted */
    T str1("\ttesting string");
    auto markers = mdc::get_markers(str1);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str1.begin() + 1, mdc::Mark::Preformatted}));

    /* This string is preformatted */
    T str2(" \ttesting string");
    markers = mdc::get_markers(str2);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str2.begin() + 2, mdc::Mark::Preformatted}));

    /* This string is preformatted */
    T str3("   \ttesting string");
    markers = mdc::get_markers(str3);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str3.begin() + 4, mdc::Mark::Preformatted}));
}

TEST(BasicMarkersBegin, get_markers)
{
    using T  = std::string;
    using IT = T::const_iterator;

    /* This string is preformatted */
    T str1("testing string");
    auto markers = mdc::get_markers(str1);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str1.begin(), mdc::Mark::Start}));

    /* This string is preformatted */
    T str2("  testing string");
    markers = mdc::get_markers(str2);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str2.begin() + 2, mdc::Mark::Start}));

    /* This string is preformatted */
    T str3("   testing string");
    markers = mdc::get_markers(str3);

    /* We should have one mark that is at 1 character in (to skip the tab) and it should indicate
     * that this line is preformatted.
     */
    EXPECT_EQ(markers.size(), 1);
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {str3.begin() + 3, mdc::Mark::Start}));
}

TEST(BasicMarkersHeaders, get_markers)
{
    using T  = std::string;
    using IT = T::const_iterator;

    T hs("###");
    auto markers = mdc::get_markers(hs);

    // The Header3 mark should be an iterator to 'after' the marker characters
    // In this case, hs.end()
    EXPECT_TRUE(IsEqual<T>(markers.at(0), {hs.end(), mdc::Mark::Header3}));
}

TEST(BasicUtilities, is_bold_and_italics)
{
    using T  = std::string;
    using IT = T::const_iterator;

    T str("test***test");
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin() + 4, str.end()), 3);
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin() + 5, str.end()), 2);
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin() + 6, str.end()), 1);
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin() + 7, str.end()), 0);

    str = "a";
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin(), str.end()), 0);

    str = "*";
    EXPECT_EQ(mdc::is_bold_and_italics(str.begin(), str.end()), 1);
}
