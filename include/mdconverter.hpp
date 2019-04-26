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
    Header1 = 1,
    Header2 = 2,
    Header3 = 3,
    Header4 = 4,
    Header5 = 5,
    Header6 = 6,
    List,
    Italics,
    Bold,
    Preformatted,
    Start,
};

class State : public std::stack<Mark>
{
    using base = std::stack<Mark>;

public:
    using base::base;
};

template <typename T>
struct Marker
{
    using iterator = typename T::const_iterator;

    iterator itr_;
    Mark mark_;
};

template <typename T>
struct MDHandler
{
    static constexpr T start(Mark mark)
    {
        if (static_cast<unsigned short>(mark) < 7 && static_cast<unsigned short>(mark) > 0)
        {
            return header_start(static_cast<unsigned short>(mark));
        }
        switch (mark)
        {
            case Mark::Preformatted: return "<pre>";
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
            case Mark::Preformatted: return "</pre>";
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
auto remove_codeblock(T& itr) -> bool
{
    unsigned short length = 0;
    while (*itr == '\t' || *itr == ' ')
    {
        if (*itr == '\t')
            length += tabwidth;
        else
            length += 1;
        itr++;

        // If we found the start of a codeblock, return true
        if (length >= tabwidth) return true;
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
auto get_markers(const T& md_text) -> std::vector<Marker<T>>
{
    unsigned short header = 0;
    std::vector<Marker<T>> markers;

    auto itr = md_text.begin();

    // Is the line a header?
    while (*itr == '#')
    {
        header++;
        itr++;
    }

    // If it's a proper header
    if (header && std::isspace(*itr) && header < 7)
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
            markers.push_back({itr, Mark::Preformatted});
            return markers;
        }

        // TODO - Handle lists here.
    }
    else
    {
        // We can actually insert a normal start
        markers.push_back({md_text.begin(), Mark::Start});
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
