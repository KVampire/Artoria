#pragma once
#include <memory>

namespace art
{
	struct  buffer
	{
		typedef  std::size_t size_t;
		char*			data;
		size_t			len;
	};

	class alloc_base{

		friend class pstream;
	public:
		alloc_base(){}
	protected:
		virtual char* malloc(std::size_t size){ return 0; };
	};

	class free_base{
		friend class pstream;
	public:
		free_base(){}
		virtual void free(char*){};
	};

	template <typename T>
	class alloc_proxy : public alloc_base
	{
		friend class pstream;
	protected:
		T	_Alloc;

		virtual char* malloc(std::size_t size){
			return _Alloc(size);
		}

	};

	template <typename T>
	class free_proxy : public free_base
	{
	public:
		free_proxy(T t) :_Free(t){}
	protected:
		T _Free;

		virtual void free(char* data){
			_Free(data);
		}
	};


	class alloc_default{
	public:
		char* operator() (std::size_t sz){
			return new char[sz];
		}
	};

	class free_default{
	public:
		free_default(){}
	public:
		void operator() (char* data){
			delete[] data;
		}
	};

	class buffer_deletor
	{
	public:
		void operator() (buffer* v){
			fb->free(v->data); 
			delete v;
			delete fb;
		}
		free_base*	fb;
	};


	template <typename _A,typename _F>
	std::shared_ptr<buffer> create_buffer(buffer::size_t size, _A a,_F f){
		buffer* bf = new buffer;
		bf->data = a(size);
		buffer_deletor  bd;
		
		bd.fb = new free_proxy<_F>(f);
		//bd.f = f;
		bf->len = size;
		return std::shared_ptr<buffer>(bf, bd);
	}

	inline
	std::shared_ptr<buffer> create_buffer(buffer::size_t size){
		alloc_default a;
		buffer* bf = new buffer;
		bf->data = a(size);
		buffer_deletor bd;
		bd.fb = new free_proxy<free_default>(free_default());
		
		bf->len = size;
		return std::shared_ptr<buffer>(bf,bd);
	}

	inline void zero_buffer(std::shared_ptr<buffer> bf){
		memset(bf->data, 0, bf->len);
	}

}
