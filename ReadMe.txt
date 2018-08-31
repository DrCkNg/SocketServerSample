Socket Server Sample
====================

Socket Server Sample is a simple Socket Server application which listens to a dedicated Port for a string with CR(13) LF(10) as string terminator. It replies the client data (with an optional acknowledge appendix, Trigger Command Ack) with the original data and an increasing counter number. 

Settings:
---------
Customization settings to be located in the running folder config.ini file's [Server] section.
Port:  Server listening port. 
       Default(10038)
Time Out Per Serving(Sec):  Server cut off receiving data after transmission started longer than this duration without terminator found. 
                            Default(5)
Trigger Command Ack: Any text to append to server reply, for this sample, after counter number.
                     Default(_ACK)

Operation:
---------
Start Listening:  Server start listening to the client connection.

Stop Listening:   Server stop listening to the client connection.


Copyright (c) [2018] [DrCkNg@mail.com]  for JKBS project.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
