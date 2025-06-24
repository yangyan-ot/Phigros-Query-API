/*
 * @File	  : prevent_inject.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/07 22:42
 * @Introduce : ���SQLע��
*/

#pragma once

#include <string_view>

#ifndef PREVENT_INJECT_HPP
#define PREVENT_INJECT_HPP 

namespace self {
    // ���SQL
    bool CheckSQL(std::string_view sql)
    {
        const std::string key[9] = { "%","/","union","|","&","^" ,"#","/*","*/" };
        for (int i{ 0 }; i < 9; i++)
        {
            if (sql.find(key[i]) != std::string::npos)
            {
                return false;
            }
        }
        return true;
    }

    // ������
    bool CheckParameter(std::string_view Parameter)
    {
        const std::string key[14] { "and","*","="," ","%0a","%","/","union","|","&","^" ,"#","/*","*/" };
        for (int i{ 0 }; i < 14; i++)
        {
            if (Parameter.find(key[i]) != std::string::npos)
            {
                return false;
            }
        }
        return true;
    }

    // ������(�ַ���ģʽ)
    template <std::size_t SIZE = 12>
    bool CheckParameterStr(std::string_view Parameter, const std::array<std::string, SIZE> key = std::array<std::string, SIZE>({ "*", "=", " ","%0a","%","/","|","&","^" ,"#","/*","*/" })) {
        for (int i{ 0 }; i < key.size(); i++)
        {
            if (Parameter.find(key.at(i)) != std::string::npos)
            {
                return false;
            }
        }
        return true;
    }
}

#endif
