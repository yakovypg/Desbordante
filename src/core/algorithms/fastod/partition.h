#pragma once

#include <string>
#include <memory>

namespace algos::fastod {

class Partition {
public:
    virtual std::string ToString() const = 0;
    virtual std::shared_ptr<Partition> Copy() const = 0;

    virtual void Product(short attribute) = 0;
    virtual bool Split(short right) = 0;
    virtual bool Swap(short left, short right, bool ascending) = 0;
};

} // namespace algos::fastod