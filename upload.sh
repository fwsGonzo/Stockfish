#!/bin/bash
BIN=kvmprograms

upload_program() {
	base=`basename $1`
	tar -cJf - $1 | curl -H "Host: filebin.varnish-software.com" --data-binary "@-" -X POST https://filebin.varnish-software.com/$BIN/$base.tar.xz
}

upload_program src/stockfish
