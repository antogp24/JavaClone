cmake_minimum_required(VERSION 3.15)
project(JavaClone)
set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
message("Using ${CMAKE_BUILD_TYPE} mode")

set(SOURCES
        src/Arena.cpp
        src/Arena.h
        src/AstPrinter.h
        src/Call_Info.h
        src/Color.h
        src/Environment.cpp
        src/Environment.h
        src/Error.cpp
        src/Error.h
        src/Expr.cpp
        src/Expr.h
        src/FolderReader.cpp
        src/FolderReader.h
        src/Interpreter.cpp
        src/Interpreter.h
        src/JavaCallable.h
        src/JavaClass.cpp
        src/JavaClass.h
        src/JavaFunction.cpp
        src/JavaFunction.h
        src/JavaInstance.cpp
        src/JavaInstance.h
        src/JavaNativeFunction.h
        src/JavaObject.cpp
        src/JavaObject.h
        src/Lexer.cpp
        src/Lexer.h
        src/Main.cpp
        src/Parser.cpp
        src/Parser.h
        src/Stmt.cpp
        src/Stmt.h
        src/Token.cpp
        src/Token.h
        src/TokenType.h
        src/Visibility.cpp
        src/Visibility.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

#target_link_libraries(${PROJECT_NAME} PRIVATE "-lstdc++exp")
