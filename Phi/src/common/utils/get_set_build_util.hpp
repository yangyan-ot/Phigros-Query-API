/*
 * @File	  : get_set_build_util.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/15 18:39
 * @Introduce : Get,Set属性工具
*/

#pragma once

#ifndef GET_SET_BUILD_UTIL_HPP
#define GET_SET_BUILD_UTIL_HPP  


//PropertyBuilderByTypeName 用于生成类的成员变量
//并生成set和get方法
//variable_type为变量类型,可以是指针类型,也可以是非指针类型,例如int,int*等
//type_shortname为变量类型的缩写,例如bool缩写为b,int缩写为i,double缩写为d等
//method_name为方法名称
//access_permission为变量的访问权限(public, protected, private)
#define PropertyBuilder_ReadWrite(variable_type, type_shortname, method_name, access_permission)\  
access_permission:\  
    variable_type m_##type_shortname##method_name;\  
public:\  
    inline variable_type get##method_name(void)\  
    {\  
        return m_##type_shortname##method_name;\  
    }\  
    inline void set##method_name(variable_type v)\  
    {\  
        m_##type_shortname##method_name = v;\  
    }\  

#define PropertyBuilder_ReadOnly(variable_type, type_shortname, method_name, access_permission)\  
access_permission:\  
    variable_type m_##type_shortname##method_name;\  
public:\  
    inline variable_type get##method_name(void) const\  
    {\  
        return m_##type_shortname##method_name;\  
    }\  

#define PropertyBuilder_WriteOnly(variable_type, type_shortname, method_name, access_permission)\  
access_permission:\  
    variable_type m_##type_shortname##method_name;\  
public:\  
    inline void set##method_name(variable_type v)\  
    {\  
        m_##type_shortname##method_name = v;\  
    }\  

#endif