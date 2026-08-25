#pragma once

#include <string>
#include <memory>

namespace FreeEffect {

class Command {
public:
    virtual ~Command() = default;
    
    virtual void execute() = 0;
    virtual void undo() = 0;
    
    virtual std::string getDescription() const = 0;
};

using CommandPtr = std::shared_ptr<Command>;

} // namespace FreeEffect
