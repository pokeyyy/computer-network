Checkpoint 0 Writeup
====================

My name: 何旭

My ID: 502024330015

## Structure and design

#### Section 3 webget.cc
Since I am not very familiar with network programming, during the implementation of the get_URL function in webget.cc, I referred to the example code at [TCPSocket](https://cs144.github.io/doc/lab0/class_t_c_p_socket.html), which helps me a lot to understand the general framework of socket programming.

* First, create a TCPSocket, which will be used to communicate with the target website.Then, establish a connection between the TCPSocket and the target address, allowing data to be read and written through that socket.

* Write the operation statement for retrieving a web page implemented in the second section.

* Continuously read data using a loop and print all the output from the server until the socket reaches “EOF”.

* In the end, close the connection.

#### Section 4 byte_stream
1. Setting variables in the ByteStream class
* I use the string type as the data structure to implement the byte stream. 
* Besides, I use two uint64_t type variables and one bool type variable to represent the number of bytes written, the number of bytes read, and the stream's closure status, respectively. 

2. Implementation of functions in the Writer class
* For the close() function, directly modify the boolean variable to indicate that the Writer process is now closed.
* For the is_closed() function, directly return the member variable which describes the stream's closure status.
* For the available_capacity() function, directly return the difference between the capacity of this stream and the number of bytes in the stream.
* For the bytes_pushed(),directly return the member variable which describes the number of bytes written.
* For the push() function, take the minimum of the size of data and the result returned by available_capacity(), and push this size of bytes into the stream, updating the number of bytes written.

3. Implementation of functions in the Reader class
* For the is_finished() function, directly return the bool result that whether the stream has closed and fully popped.
* For the bytes_popped() function,directly return the member variable which describes the number of bytes read.
* For the peek() function, directly return byte stream whose type is string.
* For the pop() function, if the length to be removed is greater than the data length in the buffer, an error should be reported; otherwise, remove len length of data from the buffer. 
* For the bytes_buffered() function, directly return the difference between the number of bytes written and the number of bytes read. 

## Implement challenges
* During the implementation of the byte_stream, the biggest challenge I encountered was the choice of data type for the buffer. Initially, I used ```queue<char>``` as its data type, but in the peek() function, the returned char type could not match with the ```string_view``` type. After researching, I found that using a ```string``` type variable would be more suitable.
* In the check0.pdf, it says that ```"When the reader has read to the end of the stream, it will reach “EOF” (end of file) and nomore bytes can be read."``` I tried to insert 'EOF' at the end of the stream, but it didn't work. So I just delete it, and it can work well. I still have question about it.

## Experimental results and performance
#### 2.1
![2.1](\resourses\2.1.png "2.1")

#### 2.2
![2.2_1](\resourses\2.2_1.png "2.2_1")
![2.2_2](\resourses\2.2_2.png "2.2_2")

#### 2.3
![2.3_1](\resourses\2.3_1.png "2.3_1")
![2.3_2](\resourses\2.3_2.png "2.3_2")

#### 3
![3](\resourses\3.png "3")

#### 4
![4](\resourses\4.png "4")
