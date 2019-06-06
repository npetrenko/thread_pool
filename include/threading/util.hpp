#pragma once

template <class T>
inline T SetBit(T val, int pos) {
    T mask = static_cast<T>(1);
    mask <<= pos;
    val |= mask;
    return val;
}

template <class T>
inline T UnsetBit(T val, int pos) {
    T mask = static_cast<T>(1);
    mask <<= pos;
    mask = ~mask;
    return val & mask;
}

template <class T>
inline bool TestBit(T val, int pos) {
    T mask = static_cast<T>(1);
    mask <<= pos;
    return mask & val;
}
