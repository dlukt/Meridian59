#include "json_utils.h"
#include <iomanip>
#include <sstream>

std::string JsonEscape(const std::string& input)
{
    std::ostringstream ss;
    for (char c : input)
    {
        switch (c)
        {
            case '\"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f')
                {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                }
                else
                {
                    ss << c;
                }
        }
    }
    return ss.str();
}
