#!/bin/bash

cd ./Docker/ && make start
cd ../src/ && make server SERV_PORT=54543 DB_IP=0.0.0.0 DB_PORT=3306