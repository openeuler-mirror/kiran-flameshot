# D-Bus 契约静态对照：dbus/org.dharkael.Flameshot.xml vs FlameshotDBusAdapter 槽签名。
#
# 用法：
#   cmake -DKFS_XML=<接口 XML> -DKFS_ADAPTER_HEADER=<adapter 头> -P dbus-contract.cmake
#
# qt5_add_dbus_interface 生成的代理只保证「CLI 调用 ↔ XML」一致；XML 与真实实现
# （FlameshotDBusAdapter）之间的漂移只能在这里拦下来。两侧必须给出同一组
# Q_NOREPLY 方法，且同名、同序、同类型。

cmake_policy(SET CMP0057 NEW) # if(... IN_LIST ...)

if(NOT DEFINED KFS_XML OR NOT DEFINED KFS_ADAPTER_HEADER)
    message(FATAL_ERROR "KFS_XML and KFS_ADAPTER_HEADER are required")
endif()
foreach(_file "${KFS_XML}" "${KFS_ADAPTER_HEADER}")
    if(NOT EXISTS "${_file}")
        message(FATAL_ERROR "missing file: ${_file}")
    endif()
endforeach()

# D-Bus 类型签名 -> C++ 形参类型（本接口 in 参数用到的全部类型）。
function(kfs_cpp_type _signature)
    if(_signature STREQUAL "s")
        set(_type QString)
    elseif(_signature STREQUAL "b")
        set(_type bool)
    elseif(_signature STREQUAL "i")
        set(_type int)
    elseif(_signature STREQUAL "u")
        set(_type uint)
    elseif(_signature STREQUAL "ay")
        set(_type QByteArray)
    else()
        message(FATAL_ERROR "unmapped D-Bus type: ${_signature}")
    endif()
    set(KFS_CPP_TYPE "${_type}" PARENT_SCOPE)
endfunction()

# 在 _signature 上追加一个形参类型；macro 有意共享调用方作用域。
macro(kfs_append_signature _type)
    if(_signature STREQUAL "")
        set(_signature "${_type}")
    else()
        set(_signature "${_signature} ${_type}")
    endif()
endmacro()

# ---- 接口 XML --------------------------------------------------------------
file(READ "${KFS_XML}" _xml)
# 注释内容不含 "--"，因此该模式不会跨越相邻的注释块。
string(REGEX REPLACE "<!--([^-]|-[^-])*-->" "" _xml "${_xml}")
# 以 </method> 为界切块：每块至多包含一个 <method ...>。
string(REPLACE "</method>" ";" _chunks "${_xml}")

set(_xml_pairs "")
foreach(_chunk IN LISTS _chunks)
    if(_chunk MATCHES "<method name=\"([A-Za-z0-9_]+)\">")
        set(_name "${CMAKE_MATCH_1}")
        string(REGEX MATCHALL
               "<arg name=\"[A-Za-z0-9_]+\" type=\"[a-z]+\" direction=\"in\"/>"
               _args "${_chunk}")
        set(_signature "")
        foreach(_arg IN LISTS _args)
            string(REGEX MATCH "type=\"([a-z]+)\"" _ignored "${_arg}")
            kfs_cpp_type("${CMAKE_MATCH_1}")
            kfs_append_signature("${KFS_CPP_TYPE}")
        endforeach()
        list(APPEND _xml_pairs "${_name}=${_signature}")
    endif()
endforeach()

# ---- FlameshotDBusAdapter --------------------------------------------------
file(READ "${KFS_ADAPTER_HEADER}" _header)
string(REGEX MATCHALL "Q_NOREPLY[ \t]+void[ \t]+[A-Za-z0-9_]+[ \t]*\\([^)]*\\)"
       _declarations "${_header}")

set(_cpp_pairs "")
foreach(_declaration IN LISTS _declarations)
    string(REGEX MATCH "void[ \t]+([A-Za-z0-9_]+)[ \t]*\\(([^)]*)\\)"
           _ignored "${_declaration}")
    set(_name "${CMAKE_MATCH_1}")
    set(_parameters "${CMAKE_MATCH_2}")
    string(REPLACE "," ";" _parameter_list "${_parameters}")
    set(_signature "")
    foreach(_parameter IN LISTS _parameter_list)
        string(STRIP "${_parameter}" _parameter)
        string(REPLACE "const" "" _parameter "${_parameter}")
        string(REPLACE "&" " " _parameter "${_parameter}")
        string(REPLACE "*" " " _parameter "${_parameter}")
        string(STRIP "${_parameter}" _parameter)
        string(REGEX MATCHALL "[A-Za-z0-9_]+" _words "${_parameter}")
        list(GET _words 0 _type)
        kfs_append_signature("${_type}")
    endforeach()
    list(APPEND _cpp_pairs "${_name}=${_signature}")
endforeach()

# ---- 对照 ------------------------------------------------------------------
list(LENGTH _xml_pairs _xml_count)
list(LENGTH _cpp_pairs _cpp_count)
if(_xml_count LESS 8)
    message(FATAL_ERROR "接口 XML 解析出的方法数异常: ${_xml_count}，正则可能已失效")
endif()
if(_cpp_count LESS 8)
    message(FATAL_ERROR "adapter 头解析出的方法数异常: ${_cpp_count}，正则可能已失效")
endif()

list(SORT _xml_pairs)
list(SORT _cpp_pairs)

if(NOT _xml_pairs STREQUAL _cpp_pairs)
    message(STATUS "接口 XML 端: ${_xml_pairs}")
    message(STATUS "adapter 端:  ${_cpp_pairs}")
    foreach(_pair IN LISTS _xml_pairs)
        if(NOT _pair IN_LIST _cpp_pairs)
            message(FATAL_ERROR "接口方法在 FlameshotDBusAdapter 中缺失或签名不一致: ${_pair}")
        endif()
    endforeach()
    foreach(_pair IN LISTS _cpp_pairs)
        if(NOT _pair IN_LIST _xml_pairs)
            message(FATAL_ERROR "FlameshotDBusAdapter 的方法未在接口 XML 中声明: ${_pair}")
        endif()
    endforeach()
    message(FATAL_ERROR "D-Bus 契约不一致")
endif()

message(STATUS "D-Bus 契约一致：${_xml_count} 个方法（XML 与 FlameshotDBusAdapter）")
