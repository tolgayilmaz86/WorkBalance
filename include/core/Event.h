#pragma once

#include <functional>
#include <map>
#include <cstddef>

namespace WorkBalance::Core {

/// @brief A simple publish/subscribe event system for decoupled communication
/// @tparam Args The types of arguments passed to event handlers
///
/// Example usage:
/// @code
/// Event<int> onValueChanged;
/// auto id = onValueChanged.subscribe([](int value) {
///     std::cout << "Value: " << value << '\n';
/// });
/// onValueChanged.emit(42);  // Prints "Value: 42"
/// onValueChanged.unsubscribe(id);
/// @endcode
template <typename... Args>
class Event {
  public:
    using Handler = std::function<void(Args...)>;
    using HandlerId = std::size_t;

    /// @brief Subscribe a handler to this event
    /// @param handler The function to call when the event is emitted
    /// @return A unique identifier that can be used to unsubscribe
    [[nodiscard]] HandlerId subscribe(Handler handler) {
        const auto id = m_next_id++;
        m_handlers[id] = std::move(handler);
        return id;
    }

    /// @brief Unsubscribe a handler from this event
    /// @param id The identifier returned from subscribe()
    void unsubscribe(HandlerId id) {
        m_handlers.erase(id);
    }

    /// @brief Emit the event, calling all subscribed handlers
    /// @param args The arguments to pass to the handlers
    void emit(Args... args) const {
        for (const auto& [id, handler] : m_handlers) {
            if (handler) {
                handler(args...);
            }
        }
    }

    /// @brief Check if there are any subscribers
    /// @return true if at least one handler is subscribed
    [[nodiscard]] bool hasSubscribers() const noexcept {
        return !m_handlers.empty();
    }

    /// @brief Get the number of subscribers
    /// @return The count of subscribed handlers
    [[nodiscard]] std::size_t subscriberCount() const noexcept {
        return m_handlers.size();
    }

    /// @brief Remove all subscribers
    void clear() noexcept {
        m_handlers.clear();
    }

  private:
    std::map<HandlerId, Handler> m_handlers;
    HandlerId m_next_id = 0;
};

/// @brief Convenience type aliases for common event signatures
using VoidEvent = Event<>;
using IntEvent = Event<int>;
using BoolEvent = Event<bool>;

/// @brief RAII guard for automatic event unsubscription
///
/// Example usage:
/// @code
/// {
///     EventGuard guard(event, [](int x) { handle(x); });
///     // handler is active within this scope
/// }
/// // handler is automatically unsubscribed
/// @endcode
template <typename... Args>
class EventGuard {
  public:
    EventGuard(Event<Args...>& event, typename Event<Args...>::Handler handler)
        : m_event(&event), m_id(event.subscribe(std::move(handler))) {
    }

    ~EventGuard() {
        if (m_event) {
            m_event->unsubscribe(m_id);
        }
    }

    // Non-copyable
    EventGuard(const EventGuard&) = delete;
    EventGuard& operator=(const EventGuard&) = delete;

    // Movable
    EventGuard(EventGuard&& other) noexcept : m_event(std::exchange(other.m_event, nullptr)), m_id(other.m_id) {
    }

    EventGuard& operator=(EventGuard&& other) noexcept {
        if (this != &other) {
            if (m_event) {
                m_event->unsubscribe(m_id);
            }
            m_event = std::exchange(other.m_event, nullptr);
            m_id = other.m_id;
        }
        return *this;
    }

    /// @brief Get the handler ID
    [[nodiscard]] typename Event<Args...>::HandlerId id() const noexcept {
        return m_id;
    }

  private:
    Event<Args...>* m_event;
    typename Event<Args...>::HandlerId m_id;
};

} // namespace WorkBalance::Core
