#pragma once
#include "buffer.h"
#include <memory>
#include <list>
#include <string>

namespace art
{	
	enum data_type{_Is_Data = 0,_Is_Value,_Is_CStr,_Is_StdString};

	template<typename T>
	struct translater
	{
		const int _type = _Is_Value;
		static art::buffer trans(T t){
				art::buffer bf;
				bf.data = (char*)&t;
				bf.len = sizeof(T);
				return bf;
			}
	};

	template<>
	struct translater<const char*>{
		const int _type = _Is_CStr;

		static art::buffer trans(const char* t){
			art::buffer bf;
			bf.data = (char*)t;
			bf.len = strlen(t);
			return bf;
		}
	};

	template<>
	struct translater<std::string>{
		const int _type = _Is_StdString;

		static art::buffer trans(std::string t){
			art::buffer bf;
			bf.data = (char*)t.c_str();
			bf.len = t.length();
			return bf;
		}
	};


	//////////////////////////////////////////////////////////////////////////
	//protocol buffer stream
	class pstream  {

	public:
		enum _EncodeType{ _BigEndian,_LittleEndian };
		enum { _DefautBufferSize = 256 ,_MinBufferSize = 64};

	public:
		  
		typedef std::shared_ptr<art::buffer> buffer;
		typedef art::buffer::size_t			 size_t;
		typedef std::list<buffer>			 buffer_l;

	public:
		pstream();
		pstream(_EncodeType type, size_t buffersize = _DefautBufferSize);
		template<typename _A,typename _F>
		pstream(_EncodeType type, size_t buffersize = _DefautBufferSize,
			_A ab = alloc_default(),_F = free_default() );
		
		virtual ~pstream();

	public:
		size_t		length();
		
		template<typename T>
		void		write(const T& v);

		template<typename T>
		void		read(T& t);
		buffer		read_bytes(size_t size);
		void		write_bytes(char* data, size_t size);
		buffer		get_data_buffer();
		
		template<typename T>
		void test(T t)
		{
			//translater<T>::trans(t);
		}

		

	private:
		void		alloc_one_buffer();
		void		check_and_realloc(const size_t& sz);
		void		free_bind(char* v);
		size_t		surplus_wt_size();
		size_t		surplus_rd_size();

		template<typename T>
		void		write2(T t);

		template<typename T>
		void		read2(T& t);

		


	public:
		template<typename T>
		pstream&  operator << (const T& t);

		template<typename T>
		pstream& operator >> (T& t);
		

	
	protected:
		buffer_l		_Bufferlist;
		size_t			_BufferSize;
		size_t			_Wter;
		size_t			_Rder;
		alloc_base*		_Alloc;
		free_base*		_Free;
		_EncodeType		_EcType;
		bool			_Encode;;
		art::buffer		_OutBuffer;
	};

	template<typename T>
	art::pstream& art::pstream::operator <<(const T& t)
	{
		write(t);
		return *this;
	}

	template<typename T>
	pstream& art::pstream::operator >> (T& t)
	{
		//push(t);
		read(t);
		return *this;
	}


	template<typename T>
	void art::pstream::read2(T& t)
	{
		int rsz = sizeof(T);

		memcpy(&t, _Data + _Rder, rsz);

		_Rder += sizeof(t);
	}


	template<typename T>
	void art::pstream::write2(T t){
		write_bytes((char*)&t, sizeof(T));
	}

// 	template<>
// 	void art::pstream::write2(char* str)
// 	{
// 		write_bytes((char*)str, strlen(str));
// 	}



	template<typename _T>
	void reverse(_T& t){
		char* temp = (char*)&t;
		char ch;
		int s = sizeof(t);
		int hfs = s >> 1;
		for (int i = 0; i < hfs; ++i){
			ch = temp[i];
			temp[i] = temp[s - i - 1];
			temp[s - i - 1] = ch;
		}
	}


	template<typename T>
	void art::pstream::write(const T& v){
		check_and_realloc(sizeof(T));

		T t = v;
		if (_EcType == _BigEndian){
			reverse(t);
		}
		write2(t);
	}
	

	template<typename T>
	void art::pstream::read(T& t){
		read2(t);
		if (_EcType == _BigEndian){
			reverse(t);
		}
	}


	template<typename _A, typename _F>
	art::pstream::pstream(_EncodeType type, size_t buffersize /*= _DefautBufferSize*/, _A ab /*= alloc_default()*/, _F /*= free_default() */)
		:_EcType(type)
		, _BufferSize(buffersize)
		, _Wter(0)
		, _Rder(0)
		, _Encode(true)
	{
		if (buffersize < _MinBufferSize) buffersize = _MinBufferSize;

		auto ab = alloc_proxy<_A>();
		auto fb = free_proxy<_F>();

		_Alloc = ab;
		_Free = fb;

		alloc_one_buffer();
	}

	typedef pstream ipstream;
	
	class opstream : public pstream
	{
	public:
		opstream(){}
		~opstream(){}

		opstream(char* data, art::buffer::size_t size);
		opstream(art::buffer bf);

	};

}





