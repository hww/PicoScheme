# Your project's CMakeFile shold have this:
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

idf_component_register(SRCS
  "src/cell.cpp"
  "src/clock.cpp"
  "src/gc.cpp"
  "src/number.cpp"
  "src/parser.cpp"
  "src/port.cpp"
  "src/primop.cpp"
  "src/procedure.cpp"
  "src/scheme.cpp"
  INCLUDE_DIRS "src")
