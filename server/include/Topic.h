#ifndef TOPIC_H
#define TOPIC_H

#include <vector>
#include <string>
#include <functional>
#include <algorithm>

struct Topic {

	std::string name;

	Topic(const std::string& name) noexcept: name(name) { };

	void subscribe(int fd) {
		this->subscribers.push_back(fd);
	};

	void forEachSubscriber(const std::function<void(int)>& runF) const {
		for (const auto& fd: this->subscribers) runF(fd);
	};

	void unsubscribe(int fd) {
		subscribers.erase(
			std::remove(
				subscribers.begin(),
				subscribers.end(),
				fd),
			subscribers.end());
	};

	~Topic() = default;

private:
	std::vector<int> subscribers;
};

#endif
