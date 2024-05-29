#pragma once

template <class T>
class Singleton
{
public:
    static T& instance()
    {
        static T t;
        return t;
    }
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

protected:
    Singleton() = default;
};