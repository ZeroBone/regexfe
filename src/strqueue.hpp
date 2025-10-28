#pragma once
#include <string>

class StringQueue {
    std::string data;
    size_t head = 0;

public:
    explicit StringQueue(std::string data) : data(std::move(data)) {}

    void push(const char c) {
        data.push_back(c);
    }

    [[nodiscard]] char front() const {
        return data[head];
    }

    [[nodiscard]] char back() const {
        return data.back();
    }

    void pop(const size_t k = 1) {

        if (k == 0) {
            return;
        }

        if (head + k <= data.size()) {
            head += k;
        }

        if (head > 1024 && head * 2 > data.size()) {
            data.erase(0, head);
            head = 0;
        }
    }

    [[nodiscard]] size_t size() const {
        return data.size() - head;
    }

    [[nodiscard]] bool empty() const { return head >= data.size(); }

    [[nodiscard]] std::string_view view() const {
        return std::string_view(data).substr(head);
    }
};
