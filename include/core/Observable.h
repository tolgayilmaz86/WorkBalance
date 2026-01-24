#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace WorkBalance::Core {

/// @brief Observable wrapper that notifies observers when value changes
/// @tparam T The type of value being observed (must support operator!=)
///
/// Example usage:
/// @code
/// Observable<int> count{0};
/// count.observe([](int old_val, int new_val) {
///     std::cout << "Changed from " << old_val << " to " << new_val << '\n';
/// });
/// count.set(5);  // Triggers observer
/// count.set(5);  // No trigger (same value)
/// @endcode
template <typename T>
class Observable {
  public:
    /// @brief Observer callback signature: receives old and new values
    using Observer = std::function<void(const T&, const T&)>;

    /// @brief Construct with an initial value
    /// @param initial The initial value (default constructed if not provided)
    explicit Observable(T initial = {}) : m_value(std::move(initial)) {
    }

    /// @brief Get the current value
    /// @return Const reference to the current value
    [[nodiscard]] const T& get() const noexcept {
        return m_value;
    }

    /// @brief Implicit conversion to the underlying type
    [[nodiscard]] operator const T&() const noexcept {
        return m_value;
    }

    /// @brief Set a new value, notifying observers if changed
    /// @param new_value The new value to set
    void set(T new_value) {
        if (m_value != new_value) {
            T old_value = std::exchange(m_value, std::move(new_value));
            notifyObservers(old_value, m_value);
        }
    }

    /// @brief Set a new value without checking for changes
    /// @param new_value The new value to set
    /// @note This always notifies observers, even if the value hasn't changed
    void forceSet(T new_value) {
        T old_value = std::exchange(m_value, std::move(new_value));
        notifyObservers(old_value, m_value);
    }

    /// @brief Add an observer that will be called when the value changes
    /// @param observer Callback function receiving (old_value, new_value)
    void observe(Observer observer) {
        m_observers.push_back(std::move(observer));
    }

    /// @brief Remove all observers
    void clearObservers() noexcept {
        m_observers.clear();
    }

    /// @brief Check if there are any observers
    [[nodiscard]] bool hasObservers() const noexcept {
        return !m_observers.empty();
    }

    /// @brief Get the number of observers
    [[nodiscard]] std::size_t observerCount() const noexcept {
        return m_observers.size();
    }

    /// @brief Modify the value in-place using a function
    /// @param modifier Function that takes T& and modifies it
    /// @return true if the value changed (and observers were notified)
    template <typename F>
    bool modify(F&& modifier) {
        T old_value = m_value;
        modifier(m_value);
        if (old_value != m_value) {
            notifyObservers(old_value, m_value);
            return true;
        }
        return false;
    }

  private:
    void notifyObservers(const T& old_value, const T& new_value) {
        for (const auto& observer : m_observers) {
            if (observer) {
                observer(old_value, new_value);
            }
        }
    }

    T m_value;
    std::vector<Observer> m_observers;
};

/// @brief Computed observable that derives its value from other observables
/// @tparam T The computed value type
///
/// Example usage:
/// @code
/// Observable<int> width{10};
/// Observable<int> height{20};
/// ComputedObservable<int> area([&]() { return width.get() * height.get(); });
/// // Note: computed value must be manually updated when dependencies change
/// @endcode
template <typename T>
class ComputedObservable {
  public:
    using ComputeFunc = std::function<T()>;
    using Observer = std::function<void(const T&, const T&)>;

    explicit ComputedObservable(ComputeFunc compute) : m_compute(std::move(compute)), m_cached_value(m_compute()) {
    }

    /// @brief Get the cached computed value
    [[nodiscard]] const T& get() const noexcept {
        return m_cached_value;
    }

    /// @brief Implicit conversion
    [[nodiscard]] operator const T&() const noexcept {
        return m_cached_value;
    }

    /// @brief Recompute the value and notify observers if changed
    void update() {
        T new_value = m_compute();
        if (m_cached_value != new_value) {
            T old_value = std::exchange(m_cached_value, std::move(new_value));
            for (const auto& observer : m_observers) {
                if (observer) {
                    observer(old_value, m_cached_value);
                }
            }
        }
    }

    /// @brief Add an observer
    void observe(Observer observer) {
        m_observers.push_back(std::move(observer));
    }

  private:
    ComputeFunc m_compute;
    T m_cached_value;
    std::vector<Observer> m_observers;
};

} // namespace WorkBalance::Core
