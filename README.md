# Table in shared memory

## Background
I have another project on github named tool-immemorydb, which have over complicated function and non-user-friendly interface. And it's only provide with C++.
Now I create this new project, it will be simple, and hopefully have user-friendly interface. And I want proview it with different languages.
The basic function is create a table in Shared Memory (POSIX standard), and provide best access time. It can used as a cache between different processes.
The table can provide a access lock for data safe.
The table can be a hash table or sorted table for quick search.

## What is Shared Memory Object?
An object that represents memory that can be mapped concurrently into the address space of more than one process.
For details about shared memory please refer to the Base Definitions volume of IEEE Std 1003.1-2001, [Section 3.340, Shared Memory Object](http://pubs.opengroup.org/onlinepubs/009695299/basedefs/xbd_chap03.html#tag_03_340)

## Author
Created by [Scott Zhou](http://www.scottzhou.me)

## License
This project is licensed under the [MIT license](http://opensource.org/licenses/MIT).
