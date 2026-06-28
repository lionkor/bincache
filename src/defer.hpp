#pragma once

template <typename F>
struct DeferredAction {
    F action;
    DeferredAction(F action) : action(action) {}
    DeferredAction(const DeferredAction&) = delete;
    DeferredAction& operator=(const DeferredAction&) = delete;
    ~DeferredAction() { action(); }
};
