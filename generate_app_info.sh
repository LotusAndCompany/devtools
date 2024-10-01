# $1: Application version
# $2: OS name
# $3: OS version
# $4: Compiler name
# $5: Compiler version

# Sample:
#
#   namespace DevTools {
#   constexpr const char* APP_VERSION = "x.y.z[-suffix]";
#   constexpr const char* GIT_REVISION = "0000000000000000000000000000000000000000";
#   constexpr const char* OS_NAME = "OS";
#   constexpr const char* OS_VERSION = "x.y.z";
#   constexpr const char* COMPILER_NAME = "compiler";
#   constexpr const char* COMPILER_VERSION = "x.y.z";
#   }

target_file="app_info.autogen.cpp"

echo "namespace DevTools {" > $target_file

echo "constexpr const char* APP_VERSION = \"$1\";" >> $target_file
echo "constexpr const char* GIT_REVISION = \"`git rev-parse HEAD`\";" >> $target_file
echo "constexpr const char* OS_NAME = \"$2\";" >> $target_file
echo "constexpr const char* OS_VERSION = \"$3\";" >> $target_file
echo "constexpr const char* COMPILER_NAME = \"$4\";" >> $target_file
echo "constexpr const char* COMPILER_VERSION = \"$5\";" >> $target_file

echo "}" >> $target_file
