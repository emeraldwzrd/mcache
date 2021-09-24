# MCache Design

-----------

## Build and Run Tests

----------

Requirements: C++17 as this is what the code was written and tested against.

To build and run this please download the repo, navigate to the to folder and run the following:

`g++ -std=c++17 main.cpp -o main ` 

then run from the same directory

`./main`

## Discussion

----------

MCache is a key/value cache library that allows you to store any type of key that is accepted by the standard C++ hash function. These types are included at the bottom of the README. There are four main functions one can use with this library.

insert(key, value) - insert takes a key and value as parameters and stores them in the cache. You cannot add or modify an existing key/value pair. If you wish to do something similar, you can remove the key and reenter the key with a new value. It returns a boolean whether the operation succeeded or not.

insert(key, value, seconds) - insert with seconds takes a key and value as parameters and stores them in the cache and that key/value will be removed after however many seconds provided. You cannot add or modify an existing key/value pair. If you wish to do something similar, you can remove the key and reenter the key with a new value. It returns a boolean whether the operation succeeded or not.

get(key) - retrieves the value for the given key, if the key does not exist it returns a null pointer

remove(key) - removes the key/value pair from the cache if it exists. It returns a boolean whether the operation succeeded or not.

The implementation involved open addressing and separate chaining. The thought process behind this was to provide an efficient way of inserting, retrieving, and removing keys at the expense of reserved additional memory that may or may not ever be used. The reason this was necessary was to achieve consistent performance requirements. 

With open addressing, worst case you might have to search the entire cache to find the entry and there is a possibility that you may insert the same key twice if an open slot is encountered after a collision, but before you reached the desired key. This can occur if another collision happened previously and was inserted before the current key and then removed.

Separate chaining can be used to reduce wasted space, but at the expense of complicated indexing given capacity requirements. 

To address these issues I've combined the two.

The verification of the requirements can be found in the main.cpp file included in this repository. Several threads are created to add and remove key/value pairs. Then the removed key/value pairs are reinserted to fill the entire cache. A Timer class was created to test the retrieval time of all key/values. Percentiles were calculated by sorting all the times then picking the 95th and 99th percent value of the capacity of the cache. The latest test had the following results.

| Test Results: Percentile | Time             |
| ------------------------ | ---------------- |
| 95th percentile          | 944 nanoseconds  |
| 99th percentile          | 1303 nanoseconds |

Finally, all even entries are provided a time to expire and the program waits to attempt to allow them to expire and reports how many are left in the cache after waiting 3 minutes.

## Future Considerations

---------

The timed cache does not account for entries that are removed and readded. If the provided key was previously set to expire after a certain time, removed from the cache, then readded before the initial expiration, it will be removed if it exists upon the initial expiration. 

Also, the timed cache is checked every second, further precision can be added if necessary.

| Types accepted as key by MCache |
| -------------------- |
| bool               |
| char               |
| signed char        |
| unsigned char      |
| char16_t           |
| char32_t           |
| wchar_t            |
| short              |
| unsigned short     |
| int                |
| unsigned int       |
| long               |
| unsigned long      |
| long long          |
| unsigned long long |
| float              |
| double             |
| long double        |
| string |
| wstring |
| u16string |
| u32string |
