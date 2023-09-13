# inMemory-DB

## Contributors

<a href="https://github.com/ZeyadAttia5/inMemory-DB/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=ZeyadAttia5/inMemory-DB" />
</a>


## Description

This is a simple in memory database that supports the following commands:

- ```set <key> <value>``` – Set the variable name to the value value. Neither variable names nor values will contain spaces.
- ```get <key<``` – Print out the value of the variable name.
- ```del <key>``` – Unset the variable name, making it just like that variable was never set.
- ```aset <list_name> <value> <key>``` – Add the element value to the sorted set called list_name. If list_name does not exist, create it. Elements in a list are ordered.
- ```aget <list_name> <key>``` – Print out the value of the element at index key in sorted set list_name.
- ```adel <list_name> <key>``` – Unset the element at index key in sorted set list_name, making it just like that element was never set.
- ```adel <list_name>``` – Unset the entire sorted set list_name, making it just like that sorted set was never set.
- ```expire <key> <time_in_seconds>``` – Set a timeout on variable name, such that it will expire and be deleted in time_in_seconds seconds.
- ```ttl <key>``` – Print out the remaining time to live (in seconds) of the variable name.

## Installation

```bash
git clone https://github.com/ZeyadAttia5/inMemory-DB.git
cd inMemory-DB
```

## Usage

To run the server:

```bash
gcc server.cpp -o server -pthread
./server
```

To run the client:

```bash
gcc client.cpp -o client -pthread
./client <command>
```

## Example 1

```bash
./client set name zeyad -> sets the variable name to the value zeyad
./client get name -> prints out the value of the variable name which is "zeyad"
./client del name -> unsets the variable name
./client get name -> prints out NULL since the variable name is not set
```

## Example 2

```bash
./client aset list 1 zeyad -> creates a sorted set called list and adds the value 1 and the key zeyad to it
./client aset list 2 amr -> adds the value 2 and the key amr to the sorted set list
```

## Example 3

```bash
./client set name zeyad -> sets the variable name to the value zeyad
./client expire name 10 -> sets a timeout on variable name, such that it will expire and be deleted in 10 seconds
./client ttl name -> prints out the remaining time to live (in seconds) of the variable name
```







