#!/usr/bin/env sh

usage() { echo "Usage: $0 -p ios [-c]"; echo "  -p -> platform, ios or android (required)"; echo "  -c -> cpp11 support"; 1>&2; exit 1; }

projectPath=$(pwd)/./
compile_cpp_11="0"

while getopts "p:c" o; do
    case "${o}" in
        p)
            p=${OPTARG}
            if [ "$p" != "ios" ]; then
               usage
            fi
            ;;
        c)
            c="cpp11"
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

if [ -z "${p}" ]; then
    usage
fi

if [ "$c" == "cpp11" ]; then
   compile_cpp_11="1"
fi


rm -rf build.ios
mkdir build.ios
pushd .
cd build.ios
cmake -DCMAKE_TOOLCHAIN_FILE=../../cmake/toolchain/ios/iOS.cmake -DCOMPILE_CPP_11=$compile_cpp_11 -GXcode ..
(cd $projectPath/build.ios && xcodebuild -target "platform-sdk" -arch "i386" -sdk "iphonesimulator")
resultcode=$?
popd

echo
if [ $resultcode = 0 ] ; then
  echo "COMPILE XCODE PROJECT SUCCEEDED"
else
  echo "COMPILE XCODE PROJECT FAILED"
fi
echo

exit $resultcode
