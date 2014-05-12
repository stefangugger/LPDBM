LPDBM is implemented with memory mapped files for better performance.

The size of position and size information is 8 byte. That allows to have a database with the theoretic size of 2^64 byte. That also means that the database has 24 byte overhead itself and every data part has 16 byte overhead.
