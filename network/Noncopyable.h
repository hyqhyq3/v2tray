#pragma once

class Noncopyable
{
public:
	Noncopyable() {}
	Noncopyable(const Noncopyable&) = delete;
};