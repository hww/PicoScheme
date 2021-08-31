#ifndef STREAMOP_H_
#define STREAMOP_H_

#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <variant>

#include "types.hpp"
#include "utils.hpp"
#include "cell.hpp"

namespace pscm {

enum class Intern;

//! Output stream operator for scheme (display <expr>) output.
OSTREAM& operator<<(OSTREAM& os, DisplayManip<Cell> cell);

//! Default output stream operator for scheme (write <expr>) output.
OSTREAM& operator<<(OSTREAM& os, const Cell& cell);

//! Output stream operator to write essential opcodes
//! with their descriptive scheme symbol name.
OSTREAM& operator<<(OSTREAM& os, Intern opcode);


OSTREAM& operator<<(OSTREAM& os, const Cons& cons);

}

#endif // STREAMOP_H_
