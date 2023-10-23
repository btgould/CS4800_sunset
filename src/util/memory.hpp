#pragma once

#include <memory>

template <typename T> using Ref = std::shared_ptr<T>;
template <typename T, typename... Args> constexpr Ref<T> CreateRef(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> using ScopedRef = std::unique_ptr<T>;
template <typename T, typename... Args> constexpr ScopedRef<T> CreateScopedRef(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}
