#pragma once


#include <mutex>
#include <optional>
#include <utility>

template<typename T>
class SnapshotChannel {
public:

	//worldthread
	void publish(T snapshot) {
		std::scoped_lock lock(m_mutex);

		m_pending = std::move(snapshot);

	}


	//renderer
	std::optional<T> acquire() {
		std::scoped_lock lock(m_mutex);

		if (!m_pending.has_value()) return std::nullopt;

		auto result = std::move(m_pending);

		m_pending.reset();

		return result;
	}


private:
	std::mutex m_mutex;

	std::optional<T> m_pending;

};


template<typename T>
class PtrSnapshotChannel {
public:
	void publish(std::unique_ptr<T> snapshot)
	{
		std::scoped_lock lock(m_mutex);
		m_pending = std::move(snapshot);
	}

	std::unique_ptr<T> acquire()
	{
		std::scoped_lock lock(m_mutex);
		return std::move(m_pending);
	}

private:
	std::mutex m_mutex;
	std::unique_ptr<T> m_pending;
};