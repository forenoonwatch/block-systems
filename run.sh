#!/bin/bash
make -j engine && touch client/src/main.cpp && make -j client_only && make -j run_client
