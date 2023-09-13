# inMemory-DB

## Contributors

<a href="https://github.com/ZeyadAttia5/inMemory-DB/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=ZeyadAttia5/inMemory-DB" />
</a>


## Description

This is a simple in memory database that supports the following commands:

- set name value – Set the variable name to the value value. Neither variable names nor values will contain spaces.
- get name – Print out the value of the variable name.
- del name – Unset the variable name, making it just like that variable was never set.
- aset list_name value key – Add the element value to the sorted set called list_name. If list_name does not exist, create it. Elements in a list are ordered.
- aget list_name key – Print out the value of the element at index key in sorted set list_name.
- adel list_name key – Unset the element at index key in sorted set list_name, making it just like that element was never set.
- adel list_name – Unset the entire sorted set list_name, making it just like that sorted set was never set.
- expire name time_in_seconds – Set a timeout on variable name, such that it will expire and be deleted in time_in_seconds seconds.
- ttl name – Print out the remaining time to live (in seconds) of the variable name.

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

## Example

```bash
./client SET name zeyad -> sets the variable name to the value zeyad
./client GET name -> prints out the value of the variable name which is "zeyad"
./client DEL name -> unsets the variable name
./client GET name -> prints out NULL since the variable name is not set
```







