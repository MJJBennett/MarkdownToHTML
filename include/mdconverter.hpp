#include <cassert>
#include <iostream>
#include <optional>
#include <stack>
#include <string>
#include <vector>

namespace mdc
{
namespace internal
{
template <typename T>
struct DeduceType
{
    using error_ = typename T::__error_;
};
}  // namespace internal

/* Type definitions */

enum class Mark : unsigned short
{
    // Headers go first to allow easy conversions
    // between (int)Mark -> Header #
    Header1 = 1,
    Header2 = 2,
    Header3 = 3,
    Header4 = 4,
    Header5 = 5,
    Header6 = 6,
    Italics,
    Bold,
    BoldItalics,  // Used to avoid ordering issues
    Preformatted,
    Start,
    List,
};

Mark header_from_int(const unsigned short num)
{
    assert(num > 0 && num < 7);
    return static_cast<Mark>(num);
}

class State : public std::stack<Mark>
{
    /**
     * State -
     * Tracks certain 'stateful' markers.
     */

    using base = std::stack<Mark>;

public:
    using base::base;  // Currently just std::stack wrapper
};

template <typename T>
struct Marker
{
    /**
     * Marker -
     * Tracks the location of a 'special character'
     * within a string.
     */

    using iterator = typename T::const_iterator;
    using type     = T;

    bool operator==(const Marker<T>& other) const noexcept
    {
        return (other.itr_ == itr_ && other.mark_ == mark_);
    }

    iterator itr_;
    Mark mark_;
};

template <typename T>
struct MDHandler
{
    /**
     * MDHandler -
     * Used to generate the proper tags for a given marker.
     */

    static constexpr T start(Mark mark)
    {
        if (static_cast<unsigned short>(mark) < 7 && static_cast<unsigned short>(mark) > 0)
        {
            return header_start(static_cast<unsigned short>(mark));
        }
        switch (mark)
        {
            case Mark::Preformatted: return "<pre><code>";
            case Mark::Start: return "<p>";
            default: return "<undefined>";
        }
    }
    static constexpr T end(Mark mark)
    {
        if (static_cast<unsigned short>(mark) < 7 && static_cast<unsigned short>(mark) > 0)
        {
            return header_end(static_cast<unsigned short>(mark));
        }
        switch (mark)
        {
            case Mark::Preformatted: return "</code></pre>";
            case Mark::Start: return "</p>";
            default: return "</undefined>";
        }
    }

    static constexpr T header_start(unsigned short i)
    {
        T str("<h.>");
        str[2] = '0' + i;
        return str;
    }
    static constexpr T header_end(unsigned short i)
    {
        T str("</h.>");
        str[3] = '0' + i;
        return str;
    }
};

/* Function Definitions */

constexpr static unsigned short tabwidth = 4;
template <typename T>
auto remove_codeblock(T& itr, const T& end, const unsigned int allowed_spaces = 0) -> bool
{
    const unsigned int spaces_to_codeblock = tabwidth + allowed_spaces;
    unsigned short length                  = 0;
    while (itr != end && (*itr == '\t' || *itr == ' '))
    {
        if (*itr == '\t')
            length += tabwidth;
        else
            length += 1;
        itr++;

        // If we found the start of a codeblock, return true
        if (length >= spaces_to_codeblock) return true;
    }
    return false;
}

template <template <typename> class C, typename T, typename H>
auto convert_collection(const C<T>& md_text, const H& handler) -> decltype(C<T>())
{
    C<T> html_text;
    State state;
    for (auto&& s : md_text)
    {
        const auto res = convert(s, handler, state);
        std::cout << "(" << res << ")" << std::endl;
        html_text.emplace_back(res);
    }

    return html_text;
}

template <typename T>
auto next_is(const T& itr, const T& end, char id) -> bool
{
    return itr + 1 != end && *(itr + 1) == id;
}

template <typename T>
auto is_whitespace(const T& itr) -> bool
{
    const auto c = *itr;
    return c == '\t' || std::isspace(c) || c == '\n' || c == '\r';
}

/**
 * Returns:
 *  0 if nothing
 *  1 if *
 *  2 if **
 *  3 if ***
 */
template <typename T>
auto is_bold_and_italics(const T& itr, const T& end) -> int
{
    return ((*itr == '*') ? (1 + (next_is(itr, end, '*') ? (1 + next_is(itr + 1, end, '*')) : 0))
                          : 0);
}

template <typename T>
auto next_is_whitespace(const T& itr, const T& end) -> bool
{
    return itr + 1 != end && is_whitespace(itr + 1);
}

template <typename T>
auto is_list(const T& itr) -> bool
{
    return false;  // Unimplemented
}

template <typename T>
auto remove_header(T& itr, const T& end) -> int
{
    int count = 0;
    while (itr != end && *itr == '#')
    {
        count++;
        itr++;
    }
    return count;
}

/**
 * Returns a vector of Markers, iterators to breaks in the input string
 * where tags need to be inserted.
 * This function works only on a single line - stitch together multiple lines
 * before calling this.
 * Caveat: List metadata must somehow be embedded to support extra indentation.
 */
template <typename T>
auto get_markers(const T& md_text) -> std::vector<Marker<T>>
{
    // Store each mark in this vector.
    std::vector<Marker<T>> markers;

    // Iterate through string character-by-character.
    auto itr       = md_text.begin();
    const auto end = md_text.end();

    // First remove leading whitespace
    if (remove_codeblock(itr, end))
    {
        // TODO - Handle newlines within the text here.
        markers.push_back({itr, Mark::Preformatted});
        return markers;
    }

    if (const int header = remove_header(itr, end); header > 0)
    {
        markers.push_back({itr, header_from_int(header)});
    }

    if (markers.empty())
    {
        markers.push_back({itr, Mark::Start});
    }

    // At this point, we have at least one leading marker (Mark::Start, Mark::List,
    // Mark::Header...) Now we need to enumerate the rest of the markers Necessary to support:
    // - Italics
    // - Bold
    // - Link (TODO - How to deal with these?) - Not as big a deal with stiched lines
    // Note: According to the spec, markdown can be easily be multiline, so we have to just mark
    // the location of characters
    // Nothing else we can do Links can be multiline but do have a guaranteed ]( portion (no
    // newline possible) - appears that link body cannot have spaces
    // Consider:
    /*      [this
     *      *is*
     *      a](https://*this*link)
     */
    // This should parse to:
    // <link @ https://*this*link>this<i>is</i>a</link>
    // May want this function to be stateful to avoid inserting at every [ character?
    // Probably not a big deal.
    // [  -> LinkStart
    // ]( -> LinkJoin
    // ) -> LinkEnd
    // RE: Bold/Italics: Apparently resolution is backwards, e.g.
    /*      ***this
     *      this ***this*** this
     */
    // should leave the first three * alone, not the last three

    while (itr != end)
    {
        // Scout first
        switch (is_bold_and_italics(itr, end))
        {
            case 1: markers.push_back({itr, Mark::Italics}); break;
            case 2:
                markers.push_back({itr, Mark::Bold});
                itr++;
                break;
            case 3:
                markers.push_back({itr, Mark::BoldItalics});
                itr += 2;
                break;
            default: break;
        }

        itr++;
    }

    return markers;
}

int offset(const Mark& m)
{
    switch (m)
    {
        case Mark::Bold: return 2;
        case Mark::Start: return 0;
        default: return 1;
    }
}

template <typename T, typename H>
auto convert(const T& md_text, const H& handler, State& state) -> T
{
    T html_text;

    auto markers = get_markers(md_text);

    auto itr       = markers.begin();
    const auto end = markers.end();

    while (itr != end)
    {
        // Add the marker
        html_text += handler.start((*itr).mark_);
        // Now add the remaining text
        html_text += T((*itr).itr_ + offset((*itr).mark_),
                       (itr + 1 == end ? md_text.end() : (*(itr + 1)).itr_));
        itr++;
    }

    return html_text;
}

template <typename T, typename H>
auto convert(const T& md_text, const H& handler) -> T
{
    State state;
    return convert(md_text, handler, state);
}

}  // namespace mdc

/* Utility Functions */

namespace mdc::util
{
std::string markToString(mdc::Mark mark)
{
    switch (mark)
    {
        case mdc::Mark::Header1: return "Header1";
        case mdc::Mark::Header2: return "Header2";
        case mdc::Mark::Header3: return "Header3";
        case mdc::Mark::Header4: return "Header4";
        case mdc::Mark::Header5: return "Header5";
        case mdc::Mark::Header6: return "Header6";
        case mdc::Mark::List: return "List";
        case mdc::Mark::Italics: return "Italics";
        case mdc::Mark::Bold: return "Bold";
        case mdc::Mark::BoldItalics: return "BoldItalics";
        case mdc::Mark::Preformatted: return "Preformatted";
        case mdc::Mark::Start: return "Start";

        default: return "Undefined";
    }
}
}  // namespace mdc::util

