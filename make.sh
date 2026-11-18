echo Building gnius-live
cd gnius-live
g++ -o gnius-live -I . -I headers \
    src/gnius-live.cpp \
    src/GniusServer.cpp \
    -lpthread || exit
strip gnius-live
echo Done
