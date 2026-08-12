#pragma once

#include <string>
#include <variant>
#include <optional>
#include <utility>
#include <stdexcept>

namespace bijoy::error {

// -----------------------------------------------------------------------------
// ErrorCode / Error payload
// -----------------------------------------------------------------------------
struct Error {
    int code{0};
    std::string message;
    std::string details;

    Error() = default;
    Error(int c, std::string msg, std::string det = "")
        : code(c), message(std::move(msg)), details(std::move(det)) {}
};

// -----------------------------------------------------------------------------
// Result<T, E>: Type-safe monadic error container
// Forces explicitly checking for errors via [[nodiscard]]
// -----------------------------------------------------------------------------
template <typename T, typename E = Error>
class [[nodiscard]] Result {
public:
    Result(const T& value) : m_data(value) {}
    Result(T&& value) : m_data(std::move(value)) {}
    Result(const E& error) : m_data(error) {}
    Result(E&& error) : m_data(std::move(error)) {}

    [[nodiscard]] bool IsOk() const noexcept {
        return std::holds_alternative<T>(m_data);
    }

    [[nodiscard]] bool IsError() const noexcept {
        return std::holds_alternative<E>(m_data);
    }

    explicit operator bool() const noexcept {
        return IsOk();
    }

    const T& Value() const& {
        if (IsError()) {
            throw std::runtime_error("Attempted to access value of failed Result: " + GetError().message);
        }
        return std::get<T>(m_data);
    }

    T& Value() & {
        if (IsError()) {
            throw std::runtime_error("Attempted to access value of failed Result: " + GetError().message);
        }
        return std::get<T>(m_data);
    }

    T&& Value() && {
        if (IsError()) {
            throw std::runtime_error("Attempted to access value of failed Result: " + GetError().message);
        }
        return std::get<T>(std::move(m_data));
    }

    const E& GetError() const {
        if (IsOk()) {
            throw std::runtime_error("Attempted to access error of successful Result");
        }
        return std::get<E>(m_data);
    }

    T ValueOr(T fallback) const& {
        return IsOk() ? std::get<T>(m_data) : std::move(fallback);
    }

private:
    std::variant<T, E> m_data;
};

// -----------------------------------------------------------------------------
// Void specialization for Result (Status indicator)
// -----------------------------------------------------------------------------
template <typename E>
class [[nodiscard]] Result<void, E> {
public:
    Result() : m_error(std::nullopt) {}
    Result(const E& error) : m_error(error) {}
    Result(E&& error) : m_error(std::move(error)) {}

    [[nodiscard]] bool IsOk() const noexcept {
        return !m_error.has_value();
    }

    [[nodiscard]] bool IsError() const noexcept {
        return m_error.has_value();
    }

    explicit operator bool() const noexcept {
        return IsOk();
    }

    const E& GetError() const {
        if (IsOk()) {
            throw std::runtime_error("Attempted to access error of successful Result");
        }
        return m_error.value();
    }

private:
    std::optional<E> m_error;
};

using Status = Result<void, Error>;

} // namespace bijoy::error
