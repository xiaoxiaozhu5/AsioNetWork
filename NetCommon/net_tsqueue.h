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

		LockType cv_lock(cv_mtx_);
		cv_.notify_one();
	}

	void push_front(const T& item)
	{
		LockType lock(mtx_);
		queue_.emplace_front(std::move(item));

		LockType cv_lock(cv_mtx_);
		cv_.notify_one();
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

	void wait()
	{
		while(empty())
		{
			LockType lock(cv_mtx_);
			cv_.wait(lock);
		}
	}

protected:
	mutable std::mutex mtx_;
	std::deque<T> queue_;
	std::mutex cv_mtx_;
	std::condition_variable cv_;
};
