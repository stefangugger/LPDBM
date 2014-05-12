#include "LPDBM.h"

LPDBM::LPDBM(LPCTSTR lpFileName) {
	_file = CreateFile(
		lpFileName,
		GENERIC_ALL,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	GetDBSize();

	if (_dbsize.QuadPart == 0) {
		ExtendDBSize(24);
		Open();
		*((SIZE_T *)(_viewpos)) = 0;
		*((SIZE_T *)(_viewpos + 8)) = 0;
		*((SIZE_T *)(_viewpos + 16)) = 40;
	}
	else {
		Open();
	}
}

LPDBM::~LPDBM() {
	Close();
	CloseHandle(_file);
}

void LPDBM::Open() {
	GetDBSize();
	_mapping = CreateFileMapping(
		_file,
		NULL,
		PAGE_EXECUTE_READWRITE,
		_dbsize.HighPart,
		_dbsize.LowPart,
		NULL
		);
		
	_view = MapViewOfFile(
		_mapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
		);
	_viewpos = (char*)_view;
}

void LPDBM::Close() {
	UnmapViewOfFile(_view);
	CloseHandle(_mapping);
}

SIZE_T LPDBM::GetDBSize() {
	GetFileSizeEx(
		_file,
		&_dbsize
		);
	
	return _dbsize.QuadPart;
}

void LPDBM::ExtendDBSize(SIZE_T size) {
	bool reopen = _mapping != NULL;
	
	if (reopen) {
		Close();
	}

	_dbsize.QuadPart = size;

	SetFilePointerEx(
		_file,
		_dbsize,
		NULL,
		FILE_END
		);
		
	SetEndOfFile(
		_file
		);
	
	if (reopen) {
		Open();
	}
}

SIZE_T LPDBM::GetMasterKey() {
	return *((SIZE_T *)(_viewpos));
}

void LPDBM::SetMasterKey(SIZE_T key) {
	*((SIZE_T *)(_viewpos)) = key;
}

SIZE_T LPDBM::Add(const VOID *data, SIZE_T length, SIZE_T maxsplits) {
	if (length == 0) {
		return 0;
	}
	_Reference = NULL;
	_StartPos = *((SIZE_T *)(_viewpos+8));
	for (_Counter = 0; _Counter < maxsplits; _Counter++) {
		if (_StartPos == 0 || length == 0) break;
		if (_Reference != NULL) *_Reference = _StartPos;
		else _Key = _StartPos;
		_pos = _viewpos+_StartPos;
		_Size = *((SIZE_T *)_pos);
		if (_Size > length) {
			_Leftover = _Size-length;
			if (_Leftover < 17) _Leftover = 17;
			_Size = length-_Leftover;
			_pos += 8+_Size;
			*((SIZE_T *)_pos) = 16+_Size;
			_pos += 8;
			*((SIZE_T *)_pos) = _Leftover-16;
			_pos = _viewpos+_StartPos;
		}		
		_pos += 8;
		CopyMemory((PVOID)_pos,data,_Size);
		data = (char *)data+_Size;
		length -= _Size;
		_pos += _Size;
		_StartPos = *((SIZE_T *)_pos);
		*((SIZE_T *)_pos) = 0;
		_Reference = (SIZE_T *)_pos;
	}
	*((SIZE_T *)(_viewpos)) = _StartPos;
	if (length > 0) {
		_StartPos = *((SIZE_T*)(_viewpos+16));
		if (_Reference != NULL) *_Reference = _StartPos;
		else _Key = _StartPos;
		_Leftover = GetDBSize()-_StartPos;
		if (_Leftover < length+16) {
			_Leftover = length+16-_Leftover;
			ExtendDBSize(_Leftover);
		}
		_pos = _viewpos+_StartPos;
		*((SIZE_T *)_pos) = length;
		_pos += 8;
		CopyMemory((PVOID)_pos,data,length);
		_pos += length;
		*((SIZE_T *)_pos) = 0;
		*((SIZE_T*)(_viewpos+16)) = _StartPos+16+length;
	}
	return _Key;
}

void LPDBM::Remove(SIZE_T key) {
	_StartPos = *((SIZE_T *)(_viewpos + 8));
	while (key != 0) {
		_pos = _viewpos + key;
		_Size = *((SIZE_T *)_pos);
		_pos += 8 + _Size;
		_Key = *((SIZE_T *)_pos);
		*((SIZE_T *)_pos) = _StartPos;
		if (_Key == 0) *((SIZE_T *)(_viewpos + 8)) = key;
		_StartPos = key;
		key = _Key;
	}
}

void LPDBM::Read(SIZE_T key, VOID *data, SIZE_T offset, SIZE_T *length) {
	_Length = 0;
	while (key != 0 && _Length != *length) {
		_pos = _viewpos+key;
		_Size = *((SIZE_T *)_pos);
		if (_Size <= offset) {
			offset -= _Size;
			_pos += 8+_Size;
			key = *((SIZE_T *)_pos);
		} else {
			_pos += 8;
			_Leftover = _Size-offset;
			if (_Length+_Leftover > *length) _Leftover = *length-_Length;			
			CopyMemory(data,(PVOID)(_pos+offset),_Leftover);
			data = (char *)data+_Leftover;
			_Length += _Leftover;
			_pos += _Size;
			key = *((SIZE_T *)_pos);
			offset = 0;
		}
	}
	*length = _Length;
}

void LPDBM::Write(SIZE_T key, const VOID *data, SIZE_T offset, SIZE_T length, SIZE_T maxsplits) {
	_Length = 0;
	while (key != 0 && _Length != length) {
		_pos = _viewpos + key;
		_Size = *((SIZE_T *)_pos);
		if (_Size <= offset) {
			offset -= _Size;
			_pos += 8 + _Size;
			key = *((SIZE_T *)_pos);
		}
		else {
			_pos += 8;
			_Leftover = _Size - offset;
			if (_Length + _Leftover > length) _Leftover = length - _Length;
			CopyMemory((PVOID)(_pos + offset), data, _Leftover);
			data = (char *)data + _Leftover;
			_Length += _Leftover;
			_pos += _Size;
			key = *((SIZE_T *)_pos);
			offset = 0;
		}
	}
	if (_Length < length) {
		Add(data,length-_Length,maxsplits);
	}
}