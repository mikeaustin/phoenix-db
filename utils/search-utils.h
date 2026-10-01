#ifndef SEARCH_UTILS
#define SEARCH_UTILS

#include <optional>

template <typename T>
std::optional<size_t> lowerBound(const T *array, size_t count, T value) {
    int left = 0, right = count - 1;

    while (left < right) {
        int mid = left + (right - left) / 2; 

        if (array[mid] >= value) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    if (array[left] == value) {
      return left;
    }

    return std::nullopt;
}

#endif
