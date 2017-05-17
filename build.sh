if [ ! -d "boost" ]; then
  ./prerequisites.sh
fi

( cd AccelerometerGraph && ./build.sh )
