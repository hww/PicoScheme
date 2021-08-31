#include "streamop.h"

namespace pscm {

OSTREAM& operator<<(OSTREAM& os, Cons& cons)
{
    os << "(" << car(cons) << " " << cdr(cons) << " " << mrk(cons) << ")";
    return os;
}

/**
 * Output stream operator for Cons cell lists.
 */
static OSTREAM& operator<<(OSTREAM& os, Cons* cons)
{
    Cell iter{ cons };

    os << '(' << car(iter);
    iter = cdr(iter);

    for (Cell slow{ iter }; is_pair(iter); iter = cdr(iter), slow = cdr(slow)) {
        os << ' ' << car(iter);

        if (!is_pair(iter = cdr(iter)) || slow == iter) {
            if (slow == iter)
                return os << " ...)"; // circular list detected

            break;
        }
        os << ' ' << car(iter);
    }
    if (is_nil(iter))
        os << ')'; // list end
    else
        os << " . " << iter << ')'; // dotted pair end

    return os;
}

//! Output stream operator for Symbols.
static OSTREAM& operator<<(OSTREAM& os, const Symbol& sym)
{
    const String& name = sym.value();

    if (name.find_first_of(' ') != String::npos)
        return os << '|' << name << '|';
    else
        return os << name;
}

static OSTREAM& operator<<(OSTREAM& os, const DisplayManip<StringPtr>& manip)
{
    const StringPtr::element_type& str = *manip.value;

    for (auto cp = str.begin(), end = str.end(); cp != end; ++cp)
        if (*cp == '\\' && cp + 1 < end)
            switch (*(++cp)) {
            case 'a':
                os << '\a';
                break;
            case 'b':
                os << '\b';
                break;
            case 'n':
                os << '\n';
                break;
            case 'r':
                os << '\r';
                break;
            case 't':
                os << '\t';
                break;
            default:
                os << *cp;
            }
        else
            os << *cp;
    return os;
}

static OSTREAM& operator<<(OSTREAM& os, const VectorPtr& vptr)
{
    if (vptr->size()) {
        os << "#(" << vptr->front();
        for (auto ip = vptr->begin() + 1, ie = vptr->end(); ip != ie; ++ip)
            os << ' ' << *ip;
        return os << ')';
    } else
        return os << "#()";
}

OSTREAM& operator<<(OSTREAM& os, Intern opcode)
{
    switch (opcode) {
    case Intern::_or:
        return os << "or";
    case Intern::_and:
        return os << "and";
    case Intern::_if:
        return os << "if";
    case Intern::_cond:
        return os << "cond";
    case Intern::_else:
        return os << "else";
    case Intern::_arrow:
        return os << "=>";
    case Intern::_when:
        return os << "when";
    case Intern::_unless:
        return os << "unless";
    case Intern::_define:
        return os << "define";
    case Intern::_setb:
        return os << "set!";
    case Intern::_begin:
        return os << "begin";
    case Intern::_lambda:
        return os << "lambda";
    case Intern::_macro:
        return os << "define-macro";
    case Intern::_apply:
        return os << "apply";
    case Intern::_quote:
        return os << "quote";
    case Intern::_quasiquote:
        return os << "quasiquote";
    case Intern::_unquote:
        return os << "unquote";
    case Intern::_unquotesplice:
        return os << "unquote-splicing";
    default:
        return os << "#<primop>";
    }
}

static OSTREAM& operator<<(OSTREAM& os, const Procedure& proc)
{
    return proc.is_macro() ? os << "#<macro>" : os << "#<clojure>";
}

/**
 * Output stream operator for Cell type arguments.
 */
OSTREAM& operator<<(OSTREAM& os, const Cell& cell)
{
    // clang-format off
    overloads stream{
        [&os](None)                   -> OSTREAM& { return os << "#<none>"; },
        [&os](Nil)                    -> OSTREAM& { return os << "()"; },
        [&os](Bool arg)               -> OSTREAM& { return os << (arg ? "#t" : "#f"); },
        [&os](Char arg)               -> OSTREAM& { return arg != static_cast<Char>(EOF) ?
                                                             (os << "#\\" << arg) : (os << "#\\eof"); },
        [&os](const StringPtr& arg)   -> OSTREAM& { return os << '"' << *arg << '"';},
        [&os](const RegexPtr&)        -> OSTREAM& { return os << "#<regex>"; },
        [&os](const MapPtr&)          -> OSTREAM& { return os << "#<dict>"; },
        [&os](const SymenvPtr& arg)   -> OSTREAM& { return os << "#<symenv " << arg.get() << '>'; },
        [&os](const FunctionPtr& arg) -> OSTREAM& { return os << "#<function " << arg->name() << '>'; },
        [&os](const PortPtr&)         -> OSTREAM& { return os << "#<port>"; },
        [&os](const ClockPtr& arg)    -> OSTREAM& { return os << "#<clock " << *arg << ">"; },
        [&os](auto& arg)              -> OSTREAM& { return os << arg; }
    }; // clang-format on

    return std::visit(std::move(stream), static_cast<const Cell::base_type&>(cell));
}

/**
 * Overloaded output stream operator for Cell types as scheme (display <expr>) function
 * representation.
 */
OSTREAM& operator<<(OSTREAM& os, DisplayManip<Cell> manip)
{
    // clang-format off
    overloads stream{
        [](None)                    { },
        [&os](Char arg)             { os << arg; },
        [&os](const StringPtr& arg) { os << display(arg);},

        // For all other types call normal cell-stream overloaded operator:
        [&os, &manip](auto&)        { os << manip.value; }
    }; // clang-format on

    std::visit(std::move(stream), static_cast<const Cell::base_type&>(manip.value));
    return os;
}

} // namespace pscm
