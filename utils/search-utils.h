#include <optional>

template <typename T>
std::optional<size_t> lowerBound(T *array, size_t size, T value) {
    int left = 0, right = size / sizeof(T) - 1;

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

template <typename T, typename U>
T *lowerBound2(T *array, size_t size, U value) {
    int left = 0, right = size / sizeof(T) - 1;

    while (left < right) {
        int mid = left + (right - left) / 2; 

        if (*reinterpret_cast<U *>(&array[mid]) >= value) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }

    if (*reinterpret_cast<U *>(&array[left]) == value) {
      return &array[left];
    }

    return nullptr;
}
