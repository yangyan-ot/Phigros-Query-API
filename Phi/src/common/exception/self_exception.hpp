/*
 * @File	  : self_exception.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/05 21:15
 * @Introduce : 文件异常类
*/

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef SELF_EXCEPTION_HPP
#define SELF_EXCEPTION_HPP  

namespace self {
    class FileException : public std::exception {
    private:
        const char* msg{ "File Exception" };
    public:
        FileException(const char* msg) {
            this->msg = msg;
        }

        const char* getMessage() {
            return msg;
        }

        virtual const char* what() const throw() {
            return msg;
        }
    };

    class TimeoutException : public std::runtime_error {
    private:
        const char* msg{};
    public:
        TimeoutException(const char* msg = "Timeout Exception") : std::runtime_error(msg) {
            this->msg = msg;
        };

        const char* getMessage() {
            return msg;
        }

        virtual const char* what() const throw() {
            return msg;
        }
    };

    class HTTPException : public std::runtime_error {
    private:
        std::string msg{};
        uint16_t status{};
        uint16_t code{ 500 };
    public:
        HTTPException(std::string_view msg = "Severe HTTP Error", uint16_t code = 500, uint16_t status = 0) : std::runtime_error(msg.data()) {
            this->msg = msg;
            this->code = code;
            this->status = status;
        };

        HTTPException& operator=(const HTTPException& rhs) {
            this->code = rhs.code;
            this->msg = rhs.msg;
            this->status = rhs.status;
            return *this;
        }

        const std::string& getMessage() const {
            return this->msg;
        }

        const uint16_t getCode() const {
            return this->code;
        }

        const uint16_t getStatus() const {
            return this->status;
        }

        virtual const char* what() const throw() {
            return this->msg.data();
        }
    };
};

#endif