cmake_minimum_required(VERSION 3.15)
project(Pipeline)

add_subdirectory("Lab 1")
add_subdirectory("Lab 2")
add_subdirectory("Lab 3")
add_subdirectory("Lab 4")

# Пользователь задаёт входной файл
set(INPUT_FILE "test.c" CACHE STRING "Input C file")

# Извлекаем имя без расширения
get_filename_component(INPUT_NAME ${INPUT_FILE} NAME_WE)

# Генерируем имена автоматически
set(STEP1_OUTPUT "preprocessed_${INPUT_NAME}.c")
set(STEP2_OUTPUT "lexAnal_${INPUT_NAME}.txt")
set(STEP3_OUTPUT "syntaxAnal_${INPUT_NAME}.json")
set(FINAL_OUTPUT "semAnal_${INPUT_NAME}.txt")

# Препроцессинг (Lab 1)
add_custom_command(
    OUTPUT ${STEP1_OUTPUT}
    COMMAND $<TARGET_FILE:preprocessor> ${INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS preprocessor ${INPUT_FILE}
    COMMENT "Module 1: preprocessing ${INPUT_FILE}"
    VERBATIM
)

add_custom_target(step1
    DEPENDS ${STEP1_OUTPUT}
)

# Лексический анализ (Lab 2)
add_custom_command(
    OUTPUT ${STEP2_OUTPUT}
    COMMAND $<TARGET_FILE:lexAnal> ${STEP1_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS lexAnal ${STEP1_OUTPUT}
    COMMENT "Module 2: lexical analysis"
    VERBATIM
)

add_custom_target(step2
    DEPENDS ${STEP2_OUTPUT}
)

add_dependencies(step2 step1)

# Синтаксический анализ (Lab 3)
add_custom_command(
    OUTPUT ${STEP3_OUTPUT}
    COMMAND $<TARGET_FILE:syntaxAnal> ${STEP2_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS syntaxAnal ${STEP2_OUTPUT}
    COMMENT "Module 3: syntax analysis"
    VERBATIM
)

add_custom_target(step3
    DEPENDS ${STEP3_OUTPUT}
)

add_dependencies(step3 step2)

# Семантический анализ (Lab 4)
add_custom_command(
    OUTPUT ${FINAL_OUTPUT}
    COMMAND $<TARGET_FILE:semAnal> ${STEP3_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS semAnal ${STEP3_OUTPUT}
    COMMENT "Module 4: semantic analysis"
    VERBATIM
)

add_custom_target(step4
    DEPENDS ${FINAL_OUTPUT}
)

add_dependencies(step4 step3)

# Запуск всего пайплайна
add_custom_target(run_pipeline ALL
    DEPENDS step4
    COMMENT "Running full pipeline"
)