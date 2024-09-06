

<div align="center">
  <h1 style="text-align:center;">ft_ping</h1>
</div>
<h3 align="center">
  <i>
      Recoding the ping command.
  </i>
</h3>
<p align="center">
    <img src="https://github.com/user-attachments/assets/024a9cfa-33cc-4e8f-932a-fc94f2913469" width="300">
</p>


Ping is a tool used for network testing.

The ping tool primarily uses two types of ICMP datagrams: ICMP ECHO_REQUEST and ICMP ECHO_REPLY. These are used to check the reachability of a network device and measure the time it takes for data to travel to and from the target device. 

ICMP (Internet Control Message Protocol) is used primarily for sending message errors and operational information.

Virtually all modern routers are equipped to understand and process ICMP ECHO_REQUEST datagrams.

      Host A                              Host B
    192.168.1.1                        192.168.1.2
         |                                    |
         |          ICMP Echo Request         |
         |   ------------------------------>  |
         |                                    |
         |          ICMP Echo Reply           |
         |   <------------------------------  |
         |                                    |

ICMP is located in the Internet layer of the TCP/IP model, which is responsible for host-to-host packet delivery. It operates directly on top of the Internet Protocol (IP)

         +-----------------------------------------+
         |           Application Layer             |
         +-----------------------------------------+
         |           Transport Layer               |
         |   (TCP, UDP)                            |
         +-----------------------------------------+
         |           Internet Layer                |
         |   (IP, ICMP, ARP, RARP)                 |
         +-----------------------------------------+
         |           Link Layer                    |
         |   (Ethernet, WiFi)                      |
         +-----------------------------------------+
         |           Physical Layer                |
         |   (Cables, Radio Signals)               |
         +-----------------------------------------+



The ICMP ECHO_REQUEST is identified by the following details:
- Type: 8
- Code: 0
- Description: 
    - Message send by originating host to the target host.
- Fields:
    - ID
    - Sequence Number: Increments with each request and it's used to check the order of replies and identify lost packets.
    - Data Payload: Bytes of padding to make packet useful for measurement.

The ICMP ECHO_REPLY is identified by the following details:
- Type: 0
- Code: 0
- Description:
    - This is the message sent by the target back to the originating host in response to an ECHO_REQUEST.
    - This indicates the target is reachable and provides a measurement for network latency.
- Fields:
    - ID and Sequence Number: These match those of the corresponding ECHO_REQUEST to help sender identify which is which.
    - Data Payload: Same sent by ECHO_REQUEST

Usage:
    ./ft_ping [option...] host

The subject of this project requires the implementation of the -v and -? options.

- Option -? provides usage instructions.
- Option -v provides verbose output

For more information about usage of ping from inetutils-2.0 check out https://www.gnu.org/software/inetutils/manual/inetutils.pdf page 10.


## How to Run ft_ping

To compile and run `ft_ping`, follow the instructions below:

### 1. Install Make and C11

In order to run this project, you will need to have `Make` and a compiler that supports the C11 standard installed on your system. Here are the steps to install both.

### Install Make

#### On Ubuntu/Debian-based systems:

You can install `Make` using the `apt` package manager:

```bash
sudo apt update
sudo apt install build-essential
```

This will install `Make` and other essential development tools.

#### On macOS:

You can install `Make` using `Homebrew`:

1. If you don't have Homebrew installed, install it first:

    ```bash
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    ```

2. Once Homebrew is installed, run:

    ```bash
    brew install make
    ```

#### On Windows:

On Windows, you can install `Make` via [Cygwin](https://www.cygwin.com/) or the [MinGW-w64](http://mingw-w64.org/doku.php) toolchain.

### Install a C11-Compatible Compiler

#### On Ubuntu/Debian-based systems:

Install GCC, which supports C11:

```bash
sudo apt update
sudo apt install gcc
```

#### On macOS:

Xcode comes with `clang`, which supports C11. You can install Xcode Command Line Tools with:

```bash
xcode-select --install
```

Alternatively, you can use Homebrew to install GCC:

```bash
brew install gcc
```

#### On Windows:

You can install GCC as part of the MinGW toolchain:

1. Download the MinGW-w64 installer from [here](http://mingw-w64.org/doku.php/download).
2. Follow the installation instructions to set up GCC on your system.
### 2. Compile the Project
Use the `make` command to compile the project:

```bash
make
```

### 3. Run the Program

After the project is compiled, you can run the `ft_ping` executable with the following command:

```bash
./ft_ping [options...] host
```

- Replace `[options...]` with any desired options (e.g., `-v`, `-t`, etc.).
- Replace `host` with the target host you want to ping.

### Example

To ping `example.com`:

```bash
./ft_ping example.com
```

To ping `example.com` with a specific option:

```bash
./ft_ping -v example.com
```

Ensure you have the necessary permissions to run the ping command on your system (you may need `sudo`).
