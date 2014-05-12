#include <windows.h>

class LPDBM {
	private:
		LARGE_INTEGER _dbsize;
		HANDLE _file;
		HANDLE _mapping;
		LPVOID _view;
		char* _viewpos;
		char* _pos;
		SIZE_T _StartPos;
		SIZE_T _Size;
		SIZE_T _Key;
		SIZE_T _Leftover;
		SIZE_T *_Reference;
		SIZE_T _Length;
		SIZE_T _Counter;
		
		void Open();
		void Close();
	public:
		LPDBM(LPCTSTR lpFileName);
		~LPDBM();
		
		SIZE_T GetDBSize();
		void ExtendDBSize(SIZE_T size);
		
		SIZE_T GetMasterKey();
		void SetMasterKey(SIZE_T key);
		
		SIZE_T Add(const VOID *data, SIZE_T length, SIZE_T maxsplits);
		void Remove(SIZE_T key);
		void Read(SIZE_T key, VOID *data, SIZE_T offset, SIZE_T *length);
		void Write(SIZE_T key, const VOID *data, SIZE_T offset, SIZE_T length, SIZE_T maxsplits);
};