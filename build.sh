#! /bin/sh
PWD=`pwd`
LIBDOWNLOAD=$PWD/libdownload

if [ -z "$COIN_DEPS" ]; then
	printf "No COIN_DEPS detected! "
	printf "Setup COIN_DEPS before build: export COIN_DEPS=`pwd`/depslib \\n"
fi

if [ ! -d "$LIBDOWNLOAD" ];then
	mkdir $LIBDOWNLOAD
fi

if [ ! -d "$COIN_DEPS" ];then
	mkdir $COIN_DEPS
fi

cd $COIN_DEPS

if [ ! -d ./boost ];then
	mkdir boost
fi

###install boost
cd $LIBDOWNLOAD

if [ ! -f ./boost_1_66_0.tar.gz ];then
	wget https://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz
	if [ ! -f ./boost_1_66_0.tar.gz ];then
		echo "Error cannot download boost_1_66_0.tar.gz" >&2
		exit 1
	fi
fi

if [ ! -d ./boost_1_66_0 ];then
	tar xf boost_1_66_0.tar.gz
	if [ ! -d boost_1_66_0 ];then
		echo "Error cannot tar xf boost_1_66_0.tar.gz"
		exit 1
	fi
fi

cd boost_1_66_0
GCCJAM=`find ./ -name gcc.jam`
sed -i "s/if \$(link) = shared/if \$(link) = shared \|\| \$(link) = static/g" $GCCJAM
./bootstrap.sh
./b2 --prefix=$COIN_DEPS/boost --build-dir=boost.build link=static runtime-link=static variant=release install
