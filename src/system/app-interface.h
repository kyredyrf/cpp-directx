#pragma once

class IApp
{
public:
    virtual ~IApp() = 0 { }
    virtual std::tuple<int, int> GetScreenSize() const = 0;
    virtual void OnSetup() = 0;
    virtual void OnCleanup() = 0;
    virtual void OnUpdate() = 0;
};
