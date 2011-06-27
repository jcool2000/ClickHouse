#ifndef DBMS_COMMON_WRITEBUFFER_H
#define DBMS_COMMON_WRITEBUFFER_H

#include <algorithm>
#include <cstring>

#include <DB/IO/BufferBase.h>


namespace DB
{

/** Простой абстрактный класс для буферизованной записи данных (последовательности char) куда-нибудь.
  * В отличие от std::ostream, предоставляет доступ к внутреннему буферу,
  *  а также позволяет вручную управлять позицией внутри буфера.
  *
  * Наследники должны реализовать метод nextImpl().
  */
class WriteBuffer : public BufferBase
{
public:
    WriteBuffer(Position ptr, size_t size) : BufferBase(ptr, size, 0) {}
    void set(Position ptr, size_t size) { BufferBase::set(ptr, size, 0); }

	/** записать данные, находящиеся в буфере (от начала буфера до текущей позиции);
	  * переместить позицию в начало; кинуть исключение, если что-то не так
	  */
	inline void next()
	{
		bytes += offset();
		nextImpl();
		pos = working_buffer.begin();
	}

	/** желательно в наследниках поместить в деструктор вызов nextImpl(),
	  * чтобы последние данные записались
	  */
	virtual ~WriteBuffer() {}


	inline void nextIfAtEnd()
	{
		if (pos == working_buffer.end())
			next();
	}

	
	void write(const char * from, size_t n)
	{
		size_t bytes_copied = 0;

		while (bytes_copied < n)
		{
			nextIfAtEnd();
			size_t bytes_to_copy = std::min(static_cast<size_t>(working_buffer.end() - pos), n - bytes_copied);
			std::memcpy(pos, from + bytes_copied, bytes_to_copy);
			pos += bytes_to_copy;
			bytes_copied += bytes_to_copy;
		}
	}


	inline void write(char x)
	{
		nextIfAtEnd();
		*pos = x;
		++pos;
	}
	
private:
	/** Записать данные, находящиеся в буфере (от начала буфера до текущей позиции).
	  * Кинуть исключение, если что-то не так.
	  */
	virtual void nextImpl() = 0;
};


}

#endif
