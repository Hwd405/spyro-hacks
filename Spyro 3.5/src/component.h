#pragma once
#include <vector>
#include <exception>
#include <iostream>
#include <memory>
#include <type_traits>

/**
* Readers and Components - credit to Altro50 for most of this
*/

class Reader {
private:
    const unsigned char* start;
    const unsigned char* pos;
    const unsigned char* max;

public:
    Reader(const unsigned char* buf, int size) {
        this->start = buf;
        this->pos = buf;
        this->max = buf + size;
    }

    Reader(Reader& oldReader) {
        this->start = oldReader.getPos();
        this->pos = oldReader.getPos();
        this->max = oldReader.getPos() + oldReader.sizeLeft();
    }

    const unsigned char* getPos() { return pos; }

    unsigned int sizeLeft() { return (int)(max - pos); }

    template <typename T> T peek() {
        if (sizeLeft() < sizeof(T)) {
            printf("ERROR - Reader encountered buffer overflow\n");
            throw std::exception("Reader encountered buffer overflow");
        }

        if constexpr (std::is_standard_layout<T>()) {
            // Regular value
            return *(T*)pos;
        }
        else {
            T value = new T(this->clone());
            return value;
        }
    }

    template <typename T> T read() {
        if (sizeLeft() < sizeof(T)) {
            printf("ERROR - Reader encountered buffer overflow\n");
            throw std::exception("Reader encountered buffer overflow");
        }

        if constexpr (std::is_standard_layout<T>()) {
            // Regular value
            T value = *(T*)pos;
            pos += sizeof(T);
            return value;
        }
        else {
            T value = new T(this);
            return value;
        }
    }

    std::unique_ptr<Reader> clone() { return std::make_unique<Reader>(*this); }

    std::unique_ptr<Reader> slice(unsigned int begin, unsigned int size) {
        return std::make_unique<Reader>(this->start + begin, size);
    }

    /**
     * Is deallocated manually
     */
    unsigned char* sliceBuffer(unsigned int begin, unsigned int size) {
        auto buffer = new unsigned char[size]();
        memcpy(buffer, this->start + begin, size);
        return buffer;
    }

    Reader* sliceRaw(unsigned int begin, unsigned int size) {
        return new Reader(this->start + begin, size);
    }

    std::unique_ptr<Reader> slice(unsigned int size) {
        this->increasePos(size);
        return std::make_unique<Reader>(this->pos - size, size);
    }

    void increasePos(unsigned int increment) {

        if (sizeLeft() < increment) {
            printf("ERROR - Reader encountered buffer overflow\n");
            throw std::exception("Reader encountered buffer overflow");
        }

        pos += increment;
    }
};

template <class T> class Component {
public:
    int size; // Does not include the size value itself, unlike the real thing
    T* data;

    Component() {};

    Component(void* data, int size) : data(data), size(size) {};

    Component(Reader* r) {
        this->size = r->read<int>() - 4;
        if constexpr (std::is_standard_layout<T>()) {
            this->data = (T*)r->getPos();
            r->increasePos(this->size);
        }
        else {
            this->data = new T(r->slice(this->size));
        }
    }

    virtual ~Component() {
        if constexpr (!std::is_standard_layout<T>()) {
            // Delete if was instantiated
            delete data;
        }
    }
};

template <class T> struct Block {
public:
    std::vector<T> entries;

    Block(std::unique_ptr<Reader> r) {
        // Number of mobys for the moby block
        auto count = r->read<int>();

        entries = std::vector<T>();

        for (int i = 0; i < count; i++) {
            entries.push_back(r->read<T>());
        }
    }
};