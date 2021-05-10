#pragma once
#include "net_common.h"

#include <random>

class validation
{
public:
	validation()
	{
		std::random_device device;
		std::mt19937 generator(device());

		validation_init_ = generator();
		validation_out_ = hash_it(validation_init_);
	}

	~validation()
	{
		
	}

	uint64_t get_init() const 
	{
		return validation_init_;
	}

	void set_other(uint64_t other)
	{
		validation_in_ = other;
	}

	bool validate()
	{
		if(validation_in_ == validation_out_)
			return true;
		return false;	
	}

	uint64_t hash_it(uint64_t ori)
	{
		uint64_t seed = 0x432389de343e2345;
		return ori ^ seed;
	}

private:
	
	uint64_t validation_init_;
	uint64_t validation_in_;
	uint64_t validation_out_;
};
