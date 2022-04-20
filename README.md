# qoiparser
A QOI Image format parser / converter written in C++

## Build

```
mkdir build
cd build
cmake ..
make
```

## Usage

```
Encode: 

./qoiparser -i "{input-file-path}.png" -o "{output-file-path}.qoi"

Decode: 

./qoiparser -i "{input-file-path}.qoi" -o "{output-file-path}.png"

```
