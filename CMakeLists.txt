cmake_minimum_required(VERSION 3.15)
project(Pipeline)

add_subdirectory("Lab 1")
add_subdirectory("Lab 2")
add_subdirectory("Lab 3")
add_subdirectory("Lab 4")

# Входной файл, передаваемый пользователем
set(INPUT_FILE "test.c" CACHE STRING "Input C file")
get_filename_component(INPUT_NAME ${INPUT_FILE} NAME_WE)

# Промежуточные файлы
set(STEP1_OUTPUT "preprocessed_${INPUT_NAME}.c")
set(STEP2_OUTPUT "lexAnal_${INPUT_NAME}.txt")
set(STEP3_OUTPUT "syntaxAnal_${INPUT_NAME}.json")

set(STEP4_OUTPUT_1 "semAnal_symbTable_${INPUT_NAME}.txt")
set(STEP4_OUTPUT_2 "semAnal_triads_${INPUT_NAME}.txt")

# Препроцессор
add_custom_command(
    OUTPUT ${STEP1_OUTPUT}
    COMMAND preprocessor ${INPUT_FILE}
    DEPENDS preprocessor ${INPUT_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(step1 DEPENDS ${STEP1_OUTPUT})

# Лексический анализ
add_custom_command(
    OUTPUT ${STEP2_OUTPUT}
    COMMAND lexAnal ${STEP1_OUTPUT}
    DEPENDS lexAnal ${STEP1_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(step2 DEPENDS ${STEP2_OUTPUT})
add_dependencies(step2 step1)

# Синтаксический анализ
add_custom_command(
    OUTPUT ${STEP3_OUTPUT}
    COMMAND syntaxAnal ${STEP2_OUTPUT}
    DEPENDS syntaxAnal ${STEP2_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(step3 DEPENDS ${STEP3_OUTPUT})
add_dependencies(step3 step2)

# Семантический анализ
add_custom_command(
    OUTPUT ${STEP4_OUTPUT_1} ${STEP4_OUTPUT_2}
    COMMAND semAnal ${STEP3_OUTPUT}
    DEPENDS semAnal ${STEP3_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Step 4: semantic analysis (symbols + triads)"
    VERBATIM
)

add_custom_target(step4
    DEPENDS ${STEP4_OUTPUT_1} ${STEP4_OUTPUT_2}
)

add_dependencies(step4 step3)

# Пайплайн
add_custom_target(run_pipeline ALL
    DEPENDS step4
)