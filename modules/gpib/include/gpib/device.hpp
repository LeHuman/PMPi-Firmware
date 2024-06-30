#pragma once

#include "node.hpp"

namespace GPIB {

class Device : public Node {
private:
    void setState(State state) override;

public:
    int init(void) override;
};

} // namespace GPIB