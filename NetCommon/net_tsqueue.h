#pragma once

template <typename T>
class tsqueue
{
public:
	using LockType = std::unique_lock<std::mutex>;
	tsqueue() = default;
	tsqueue(const tsqueue<T>&) = delete;
	virtual ~tsqueue() {clear();}

	const T& front()
	{
		LockType lock(mtx_);
		return  queue_.front();
	}

	const T& back()
	{
		LockType lock(mtx_);
		return queue_.back();
	}

	void push_back(const T& item)
	{
		LockType lock(mtx_);
		queue_.emplace_back(std::move(item));
	}

	void push_front(const T& item)
	{
		LockType lock(mtx_);
		queue_.emplace_front(std::move(item));
	}

	bool empty() const
	{
		LockType lock(mtx_);
		return queue_.empty();
	}

	size_t count()
	{
		LockType lock(mtx_);
		return queue_.size();
	}

	void clear()
	{
		LockType lock(mtx_);
		queue_.clear();
	}

	T pop_front()
	{
		LockType lock(mtx_);
		auto item = std::move(queue_.front());
		queue_.pop_front();
		return item;
	}

	T pop_back()
	{
		LockType lock(mtx_);
		auto item = std::move(queue_.back());
		queue_.pop_back();
		return item;
	}

protected:
	std::mutex mtx_;
	std::deque<T> queue_;
};
