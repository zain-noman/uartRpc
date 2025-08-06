# Motivation
At this point, its become somewhat of a right of passage for embedded systems engineers to make their own protocol over UART to communicate with either some sort of desktop application or some other device. Almost everytime, the resultant protocol turns out to be brittle. There are just lots of edge cases. 

Now I'm well aware of the XKCD comic for creating standards so I'm not really trying to make a new standard. Its just that I couldn't find exactly what i was looking for online and was needed to make it myself so why not publish it. I've listed below what this repository tries to accomplish

1. Have a client-server relationship (only one client and only one server)
2. Be platform agnostic
4. Handle noise/corruption using Checksums/CRCs
5. Implement a framing protocol to detect where one packet ends and another begins
6. Implement "stream" responses


## Client server architecture
One device will have the client role i.e. it can **only** make requests to the server. The other device will have the server role and it will **only** respond to requests.

## Format
All messages will be encoded using the COBS framing structure. A frame will have the following contents prior to COBS encoding
```
| Type  |    Value    |  CRC  |
| 8 bit | 0-251 bytes | 8 bit |
```
The CRC field will be the CRC of the whole packet (excluding the 'CRC' field itself ofcourse). The polynomial 0x31 will be used.

There is no length field inside the packet as the cobs framing itself can be used to decipher the length.

The type field will be set by user but it must obey the following rules 
- Type < 128 : normal command/response
- Type >= 128 : stream command/response
- Type == 0xFF : stop stream command

## Stream Responses
Often times we run into a situation where we want to monitor something, for example we might want to see the ADC readings, and say we would want the device to send adc readings after every second. For such scenarios, the 'Value' field will contain the folllowing
```
| Subtype |  Index in Stream  |     Value    |
|  4 bit  | 12 bit, MSB first |  0-249 bytes |
```

The subtype field allows for sending things like errrors and end of stream. The Index-In-Stream field gives a way to determine if any packets were missed. Index in stream begins with 0 

subtypes greater than or equal to 8 indicate end of stream

## Non Blocking, Callback based api
such api's can be a bit cumbersome to use but are easier to implement in a platform agnostic way.