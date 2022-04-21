# qoiparser
A [QOI image format](https://qoiformat.org/) parser / converter written in C++ based on the [QOI file format specification]( https://qoiformat.org/qoi-specification.pdf)

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
