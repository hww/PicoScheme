/*********************************************************************************/ /**
 * @file port.hpp
 *
 * Implementation of the three Scheme IO-ports. The standard, file and string
 * ports are thin wrappers around the c++ std::iostream, std::fstream and
 * std::string_stream classes.
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef PORT_HPP
#define PORT_HPP

#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <variant>

#include "platform.hpp"
#include "utils.hpp"

namespace pscm {

struct Cell;
enum class Intern;

//! Enable locale globally and set all standard io-ports accordingly.
//! @param name Name of the locale.
void enable_locale(const char* name = "en_US.UTF-8");

//! Stream manipulator type, to change the default stream output for
//! value to a scheme (display <expr>) output.
template <typename T>
struct DisplayManip {
    const T& value;
};

template <typename T>
DisplayManip<T> display(const T& val) { return { val }; }

//! Output stream operator for scheme (display <expr>) output.
OSTREAM& operator<<(OSTREAM& os, DisplayManip<Cell> cell);

//! Default output stream operator for scheme (write <expr>) output.
OSTREAM& operator<<(OSTREAM& os, const Cell& cell);

//! Output stream operator to write essential opcodes
//! with their descriptive scheme symbol name.
OSTREAM& operator<<(OSTREAM& os, Intern opcode);

/**
 * Scheme io-port fascade to represent either an std::iostream,
 * std::fstream or std::stringstream port.
 */
template <typename Char>
class Port {
public:
    using stream_type = std::basic_iostream<Char, std::char_traits<Char>>;
    using openmode = std::ios_base::openmode;

    virtual bool isStandardPort() const { return false; }
    virtual bool isFilePort() const { return false; }
    virtual bool isStringPort() const { return false; }

    void clear() { return m_stream.clear(); }
    void flush() { m_stream.flush(); }

    bool eof() const { return m_stream.eof(); }
    bool fail() const { return m_stream.fail(); }
    bool good() const { return m_stream.good(); }
    bool bad() const { return m_stream.bad(); }

    virtual void close()
    {
        m_stream.flush().clear();
        m_stream.setstate(stream_type::eofbit);
    }

    operator stream_type&() { return m_stream; }
    stream_type& stream() const { return m_stream; }

    bool isInput() const { return mode & stream_type::in; }
    bool isOutput() const { return mode & stream_type::out; }
    bool isBinary() const { return mode & stream_type::binary; }

protected:
    Port(stream_type& stream, openmode mode)
        : m_stream{ stream }
        , mode{ mode }
    {
        stream.exceptions(stream_type::badbit);
    }

    virtual ~Port()
    {
        std::ios_base::sync_with_stdio(true);
    }

private:
    stream_type& m_stream;
    openmode mode;
};

template <typename Char>
class StandardPort : virtual public std::basic_iostream<Char, std::char_traits<Char>>,
                     virtual public Port<Char> {
public:
    using stream_type = std::basic_iostream<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;
    using stream_type::flush;

    explicit StandardPort(openmode mode = stream_type::out)
        : stream_type{ CIN.rdbuf() }
        , Port<Char>{ *this, mode }
    {
        pscm::enable_locale();

        if (mode & stream_type::out) {
            stream_type::set_rdbuf(COUT.rdbuf());
            stream_type::copyfmt(COUT);
            stream_type::clear(COUT.rdstate());
        }
        if (mode & stream_type::in) {
            stream_type::set_rdbuf(CIN.rdbuf());
            stream_type::copyfmt(CIN);
            stream_type::clear(CIN.rdstate());
        }
    }
    bool isStandardPort() const final { return true; }
};

template <typename Char>
class StringPort : virtual public std::basic_stringstream<Char, std::char_traits<Char>>,
                   virtual public Port<Char> {
public:
    using stream_type = std::basic_stringstream<Char, std::char_traits<Char>>;
    using string_type = std::basic_string<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;
    using stream_type::str;

    explicit StringPort(openmode mode)
        : stream_type{ mode }
        , Port<Char>{ *this, mode }
    {
    }
    explicit StringPort(const string_type& str, openmode mode)
        : stream_type{ str, mode }
        , Port<Char>{ mode }
    {
    }
    bool isStringPort() const final { return true; }
};

template <typename Char>
class FilePort : virtual public std::basic_fstream<Char, std::char_traits<Char>>,
                 virtual public Port<Char> {
public:
    using stream_type = std::basic_fstream<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;
    using stream_type::eof;

    explicit FilePort(const std::basic_string<Char>& filename, openmode mode)
        : stream_type{ string_convert<char>(filename), mode }
        , Port<Char>{ *this, mode }
    {
    }
    void close() final
    {
        if (stream_type::is_open()) {
            stream_type::flush().clear();
            stream_type::close();
        }
    }
    bool isFilePort() const final { return true; }
};

struct input_port_exception : public std::ios_base::failure {

    template <typename PORT>
    input_port_exception(PORT&& port)
        : std::ios_base::failure{ "input port exception" }
    {
        if (!port.isInput())
            reason = "not an input port";
        else if (port.fail())
            reason = "reading from input port failed";
        else if (port.eof())
            reason = "end of file reached";
        else if (port.bad())
            reason = "bad input port state";
        else
            reason = "unknown input port error";
        port.clear();
    }
    const char* what() const noexcept override { return reason.c_str(); }

private:
    std::string reason;
};

struct output_port_exception : public std::ios_base::failure {
    template <typename PORT>
    output_port_exception(PORT&& port)
        : std::ios_base::failure{ "output port exception" }
    {
        if (!port.isOutput())
            reason = "not an output port";
        else if (port.fail())
            reason = "writing to output port failed";
        else if (port.eof())
            reason = "end of file reached";
        else if (port.bad())
            reason = "bad output port state";
        else
            reason = "unknown output port error";
        port.clear();
    }
    const char* what() const noexcept override { return reason.c_str(); }

private:
    std::string reason;
};

} // namespace pscm
#endif // PORT_HPP
