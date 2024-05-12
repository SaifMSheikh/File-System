FROM gcc:latest
WORKDIR /file-system
COPY . .
RUN g++ src/*.c src/*.cpp -o fs -Wno-write-strings
CMD ["./fs", "data.dat"]
