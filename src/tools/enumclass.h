#include <array>
#include <string>

template <class T> class EnumClass
{
  public:
    size_t count() const;
    const std::string &string() const;
    const T value(const std::string &stringValue) const;
};
