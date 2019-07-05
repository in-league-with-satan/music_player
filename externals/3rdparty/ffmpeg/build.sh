#!/bin/bash

#sudo apt-get -y install autoconf automake libtool build-essential cmake git pkg-config

missing_deps=""

test_cmd() {
  if ! [ -x "$(command -v $1)" ]; then
    missing_deps="$missing_deps $2"
  fi
}

test_cmd autoconf autoconf
test_cmd automake automake
test_cmd libtoolize libtool
test_cmd make build-essential
test_cmd cmake cmake
test_cmd git git
test_cmd pkg-config pkg-config

if [[ ! -z "$missing_deps" ]]; then
  echo "following packages are required:$missing_deps"
  exit 1
fi


str_opt="-march=native -O3"

git_up_to_date="Already up to date."

build_counter=0

PATH_ROOT=`pwd`

PATH_BASE=$PATH_ROOT

PATH_BUILD=$PATH_ROOT/tmp

PATH_ORIG=$PATH
PATH=$PATH_ROOT/lib:$PATH_ROOT/include:$PATH_ROOT/bin:$PATH_ORIG


cpu_count=`nproc`


export PKG_CONFIG_PATH="$PATH_BASE/lib/pkgconfig"



if [ ! -e $PATH_BUILD ]; then
  mkdir $PATH_BUILD
fi


build_nasm() {
  cd $PATH_BUILD

  if [ ! -e nasm ]; then
    build_counter=$((build_counter + 1))

    git clone git://repo.or.cz/nasm.git --branch nasm-2.14.02 --single-branch --depth 1
    cd nasm

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --bindir="$PATH_BASE/bin"
    make everything -j$cpu_count
    make install
  fi
}

build_opus() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e opus ]; then
    git clone --depth 1 https://github.com/xiph/opus.git
    cd opus

  else
    cd opus
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
    make -j$cpu_count
    make install
  fi
}

build_ogg() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e ogg ]; then
    git clone --depth 1 https://github.com/xiph/ogg.git
    cd ogg

  else
    cd ogg
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./autogen.sh
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
    make -j$cpu_count
    make install
  fi
}

build_vorbis() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e vorbis ]; then
    git clone --depth 1 https://github.com/xiph/vorbis.git
    cd vorbis

  else
    cd vorbis
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    ./autogen.sh
    CFLAGS="$str_opt" LDFLAGS="-L$PATH_BASE/lib" CPPFLAGS="-I$PATH_BASE/include" ./configure --prefix="$PATH_BASE" --with-ogg="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
    make -j$cpu_count
    make install
  fi
}

build_aac() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e fdk-aac ]; then
    git clone --depth 1 git://git.code.sf.net/p/opencore-amr/fdk-aac
    cd fdk-aac

  else
    cd fdk-aac
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ]; then
    build_counter=$((build_counter + 1))

    autoreconf -fiv
    CFLAGS="$str_opt" ./configure --prefix="$PATH_BASE" --disable-shared CFLAGS="$str_opt"
    make -j$cpu_count
    make install
  fi
}

build_ff() {
  build_required=1

  cd $PATH_BUILD

  if [ ! -e ffmpeg ]; then
    git clone --depth 1 git://source.ffmpeg.org/ffmpeg
    cd ffmpeg

  else
    cd ffmpeg
    git reset --hard
    git clean -dfx
    git pull | grep "$git_up_to_date" && build_required=0
  fi

  if [ "$build_required" -eq "1" ] || [ "$build_counter" -gt "0" ]; then
    make distclean
    ./configure --prefix="$PATH_BASE" --extra-libs="-lpthread -lstdc++" --extra-cflags="-I$PATH_BASE/include $DECKLINK_INCLUDE $str_opt" --extra-ldflags="-L$PATH_BASE/lib" --bindir="$PATH_BASE/bin" --pkg-config-flags="--static" \
      --disable-everything \
      --disable-libfreetype \
      --disable-crystalhd \
      --disable-vaapi \
      --disable-vdpau \
      --disable-zlib \
      --disable-bzlib \
      --disable-lzma \
      --disable-libxcb \
      --disable-avdevice \
      --disable-swscale \
      --disable-postproc \
      --disable-dxva2 \
      --disable-ffplay \
      --disable-ffprobe \
      --disable-doc \
      --disable-htmlpages \
      --disable-manpages \
      --disable-podpages \
      --disable-txtpages \
      --enable-pic \
      --enable-gpl \
      --enable-nonfree \
      --enable-libopus \
      --enable-libvorbis \
      --enable-libfdk-aac \
      --enable-protocol=file \
      --enable-demuxer=wav \
      --enable-demuxer=flac \
      --enable-demuxer=ape \
      --enable-demuxer=ogg \
      --enable-demuxer=mp3 \
      --enable-demuxer=aac \
      --enable-demuxer=ac3 \
      --enable-demuxer=mov \
      --enable-decoder=pcm_* \
      --enable-decoder=flac \
      --enable-decoder=ape \
      --enable-decoder=opus \
      --enable-decoder=libopus \
      --enable-decoder=vorbis \
      --enable-decoder=libvorbis \
      --enable-decoder=mp3 \
      --enable-decoder=aac \
      --enable-decoder=libfdk_aac \
      --enable-decoder=ac3 \
      --enable-decoder=alac \
      --enable-filter=volume \
      --enable-filter=equalizer \
      --enable-filter=aresample \
      --enable-small

    make -j$cpu_count
    make install
  fi
}


build_nasm
build_opus
build_ogg
build_vorbis
build_aac
build_ff
