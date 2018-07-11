#!/bin/bash

docker rmi ffmpeg-builder-music_player -f
docker build -t ffmpeg-builder-music_player .
