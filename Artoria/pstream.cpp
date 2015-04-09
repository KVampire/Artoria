#include "pstream.h"
#include <assert.h>
#include <functional>


art::pstream::pstream() : pstream(_LittleEndian){
	alloc_one_buffer();
}

art::pstream::pstream(_EncodeType type, size_t buffersize) : _BufferSize(buffersize)
,_EcType(type)
,_Wter(0)
, _Rder(0)
, _Encode(true)
{
	if (buffersize < _MinBufferSize) _BufferSize = _MinBufferSize;

	_Alloc = new alloc_proxy<alloc_default>();
	_Free = new free_proxy<free_default>(free_default());

	alloc_one_buffer();
}


art::pstream::~pstream(){
	delete _Alloc;
	delete _Free;
}


void art::pstream::check_and_realloc(const size_t& sz){
	if (_Wter % _BufferSize + sz > _BufferSize){
		alloc_one_buffer();
	}
}

void art::pstream::free_bind(char* v){
	_Free->free(v);
}


void art::pstream::alloc_one_buffer(){
	
	auto buffer = create_buffer(_BufferSize, 
		std::bind(&alloc_base::malloc,_Alloc,std::placeholders::_1),
		std::bind(&free_base::free,_Free, std::placeholders::_1));
	
	zero_buffer(buffer);
	_Bufferlist.push_back(buffer);
}

size_t art::pstream::surplus_wt_size(){
	return _BufferSize - _Wter % _BufferSize;
}

size_t art::pstream::surplus_rd_size(){
	return _BufferSize - _Rder % _BufferSize;
}

size_t art::pstream::length(){
	return _Wter;
}

art::pstream::buffer art::pstream::get_data_buffer(){
	
	auto b = create_buffer(_Wter,
		std::bind(&alloc_base::malloc, _Alloc, std::placeholders::_1),
		std::bind(&free_base::free, _Free, std::placeholders::_1));

	size_t idx = 0;
	size_t sz = 0;
	for (auto iter = _Bufferlist.begin(); iter != _Bufferlist.end() && idx < _Wter; ++iter){
		sz = _Wter - idx > _BufferSize ? _BufferSize : _Wter - idx;
		memcpy(b.get() + idx, iter->get(), sz);
		idx += sz;
	}

	return 0;
}

art::pstream::buffer art::pstream::read_bytes(size_t size)
{
	assert((_Rder + size < _OutBuffer.len));

	auto b = create_buffer(size,
		std::bind(&alloc_base::malloc, _Alloc, std::placeholders::_1),
		std::bind(&free_base::free, _Free, std::placeholders::_1));


	memcpy(b->data, _OutBuffer.data + _Rder, size);

	_Rder += size;

	return b;
}

void art::pstream::write_bytes(char* datai, size_t size)
{
	int rsz = size;
	int sz = rsz;
	char* data = datai;
	int sursize = 0;
	auto wrtbuff = _Bufferlist.back();
	while (rsz > 0){
		sursize = surplus_wt_size();
		if (rsz > sursize){
			memcpy(wrtbuff->data + _Wter % _BufferSize, data + sz - rsz, sursize);
			rsz -= sursize;
			_Wter += (sz - rsz);
		}
		else{
			memcpy(wrtbuff->data + _Wter % _BufferSize, data + sz - rsz, rsz);
			_Wter += rsz;
			rsz = 0;
		}
	}
}


art::opstream::opstream(art::buffer bf){
	_OutBuffer = bf;
}
