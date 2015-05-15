SOURCE="$1/qxmpp-0.7.6/src"
TARGET="$2/../qxmpp-0.7.6/include/qxmpp"
mkdir -p $TARGET
find $SOURCE -name "*.h" -exec cp -f {} $TARGET ";"
