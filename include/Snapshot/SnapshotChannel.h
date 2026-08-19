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
	bool acquire(T& out) {
		std::scoped_lock lock(m_mutex);

		if (!m_pending.has_value()) return false;

		out = std::move(*m_pending);
		m_pending.reset();

		return true;
	}


private:
	std::mutex m_mutex;

	std::optional<T> m_pending;

};