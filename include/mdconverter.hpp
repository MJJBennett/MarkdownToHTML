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
    unsigned short length = 0;
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
    while (itr != end && *itr == '#') count++;
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
    if (remove_codeblock(itr))
    {
        // TODO - Handle newlines within the text here.
        markers.push_back({itr, Mark::Preformatted});
        return markers;
    }

    if (const int header = remove_header(itr); header > 0)
    {
        markers.push_back({itr, Mark::Header(header)});
    }

    // Is the line a header?
    // Note: Must check for end. It's undefined behaviour to dereference end or past it.
    while (itr != end && *itr == '#')
    {
        header++;
        itr++;
    }

    // If it's a proper header
    if (header && is_whitespace(itr) && header < 7)
    {
        // Advance past the space
        itr++;
        // Now we can use this as our 'start'
        markers.push_back({itr, static_cast<Mark>(header)});
    }
    else if (header == 0)
    {
        // We haven't parsed anything yet.
        // Could still be a list or preformatted code block
        if (remove_codeblock(itr))
        {
            // This is a pre/code block. Do no more formatting.
        }

        if (is_list(itr))
        {
            // Do list logic here
        }
        else
        {
            // If it isn't a list, a preformatted block, or a header
            // We might have removed leading whitespace above - deal with that now
            markers.push_back({itr, Mark::Start});
        }
    }
    else
    {
        // We can actually insert a normal start
        markers.push_back({md_text.begin(), Mark::Start});
    }

    // At this point, we have at least one leading marker (Mark::Start, Mark::List,
    // Mark::Header...) Now we need to enumerate the rest of the markers Necessary to support:
    // - Italics
    // - Bold
    // - Link (TODO - How to deal with these?)
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

template <typename T, typename H>
auto resolve_all(const H& handler, State& state) -> T
{
    T ret;
    while (!state.empty())
    {
        ret += handler.end(state.top());
        state.pop();
    }
    return ret;
}

template <typename T, typename H>
auto convert(const T& md_text, const H& handler, State& state) -> T
{
    T html_text;
    if (is_preformatted(md_text, state.

    auto markers = get_markers(md_text);

    // Before we do all of them, check for a preformatted block
    if (markers.front().mark_ == Mark::Preformatted)
    {
        html_text += resolve_all<T>(handler, state);
        html_text += T(markers.front().itr_, md_text.end());
        return html_text;
    }

    for (auto&& marker : markers)
    {
        // Markers is a list of all markers where we need to start/stop the string
        // We can copy in between
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

