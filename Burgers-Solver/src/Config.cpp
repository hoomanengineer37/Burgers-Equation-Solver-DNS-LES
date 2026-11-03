// Config.cpp
//
// Copyright (c) 2017-2026 Mohammad Hashemi. MIT License (see LICENSE.md).

#include "burgers/Config.hpp"

#include "burgers/Format.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace burgers {

namespace {

std::string trim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// Parses the whole string as a number; "1.5abc" or "" is an error.
double to_double(const std::string& value, const std::string& where)
{
    std::size_t used = 0;
    double result = 0.0;
    try {
        result = std::stod(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) {
        throw std::invalid_argument("Expected a number but got '" + value + "' " + where);
    }
    return result;
}

long to_integer(const std::string& value, const std::string& where)
{
    std::size_t used = 0;
    long result = 0;
    try {
        result = std::stol(value, &used);
    } catch (const std::exception&) {
        used = 0;
    }
    if (used == 0 || used != value.size()) {
        throw std::invalid_argument("Expected an integer but got '" + value + "' " + where);
    }
    return result;
}

Model to_model(const std::string& value, const std::string& where)
{
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return std::tolower(c); });
    if (v == "dns") return Model::DNS;
    if (v == "les") return Model::LES;
    throw std::invalid_argument("Model must be 'dns' or 'les', got '" + value + "' " + where);
}

bool to_bool(const std::string& value, const std::string& where)
{
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return std::tolower(c); });
    if (v == "true" || v == "yes" || v == "1" || v == "on")  return true;
    if (v == "false" || v == "no" || v == "0" || v == "off") return false;
    throw std::invalid_argument("Expected true or false but got '" + value + "' " + where);
}

// Options that may be given on the command line without a value (--quiet).
bool is_flag(const std::string& name) { return name == "quiet"; }

// Sets one setting by name. `where` describes the source, for error messages.
void set_value(SimulationConfig& c, std::string key, const std::string& value,
               const std::string& where)
{
    std::replace(key.begin(), key.end(), '-', '_');   // --n-points == n_points

    if      (key == "nu")           c.nu           = to_double(value, where);
    else if (key == "t_final")      c.t_final      = to_double(value, where);
    else if (key == "Cs")           c.Cs           = to_double(value, where);
    else if (key == "model")        c.model        = to_model(value, where);
    else if (key == "input_file")   c.input_file   = value;
    else if (key == "output_dir")   c.output_dir   = value;
    else if (key == "filter_width") c.filter_width = static_cast<int>(to_integer(value, where));
    else if (key == "n_outputs")    c.n_outputs    = static_cast<int>(to_integer(value, where));
    else if (key == "series_every") c.series_every = static_cast<int>(to_integer(value, where));
    else if (key == "n_progress")   c.n_progress   = static_cast<int>(to_integer(value, where));
    else if (key == "quiet")        c.quiet        = to_bool(value, where);
    else if (key == "n_points") {
        const long n = to_integer(value, where);
        if (n < 0) throw std::invalid_argument("n_points must be positive " + where);
        c.n_points = static_cast<std::size_t>(n);
    }
    else throw std::invalid_argument("Unknown option '" + key + "' " + where);
}

} // namespace

const char* model_name(Model m) { return m == Model::DNS ? "dns" : "les"; }

void SimulationConfig::validate() const
{
    if (!(nu > 0.0))       throw std::invalid_argument("nu must be > 0");
    if (!(t_final > 0.0))  throw std::invalid_argument("t_final must be > 0");
    if (input_file.empty() && n_points < 3)
                           throw std::invalid_argument("n_points must be >= 3");
    if (!(Cs >= 0.0))      throw std::invalid_argument("Cs must be >= 0");
    if (filter_width < 2 || filter_width % 2 != 0)
                           throw std::invalid_argument("filter_width must be an even number >= 2");
    if (n_outputs < 1)     throw std::invalid_argument("n_outputs must be >= 1");
    if (output_dir.empty()) throw std::invalid_argument("output_dir must not be empty");
    if (series_every < 1)  throw std::invalid_argument("series_every must be >= 1");
    if (n_progress < 1)    throw std::invalid_argument("n_progress must be >= 1");
}

void SimulationConfig::write(std::ostream& os) const
{
    os << "# Burgers solver run settings (re-run with: burgers --config <this file>)\n"
       << "nu           = " << format_double(nu) << '\n'
       << "t_final      = " << format_double(t_final) << '\n'
       << "n_points     = " << n_points << '\n'
       << "model        = " << model_name(model) << '\n'
       << "Cs           = " << format_double(Cs) << '\n'
       << "filter_width = " << filter_width << '\n'
       << "n_outputs    = " << n_outputs << '\n'
       << "output_dir   = " << output_dir << '\n'
       << "series_every = " << series_every << '\n'
       << "n_progress   = " << n_progress << '\n'
       << "quiet        = " << (quiet ? "true" : "false") << '\n';
    if (!input_file.empty()) {
        os << "input_file   = " << input_file << '\n';
    }
}

void read_config_file(const std::string& path, SimulationConfig& config)
{
    std::ifstream file(path);
    if (!file) {
        throw std::invalid_argument("Cannot open config file: " + path);
    }

    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        ++line_number;
        const auto comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        line = trim(line);
        if (line.empty()) continue;

        const std::string where = "in " + path + " (line " + std::to_string(line_number) + ")";
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            throw std::invalid_argument("Expected 'key = value' " + where);
        }
        set_value(config, trim(line.substr(0, eq)), trim(line.substr(eq + 1)), where);
    }
}

ParseResult parse_command_line(int argc, char** argv, SimulationConfig& config)
{
    // Split argv into (name, value) pairs; accepts "--name value" and "--name=value".
    std::vector<std::pair<std::string, std::string>> options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") return ParseResult::HelpShown;
        if (arg.rfind("--", 0) != 0) {
            throw std::invalid_argument("Unexpected argument '" + arg + "' (options start with --)");
        }
        arg.erase(0, 2);
        const auto eq = arg.find('=');
        const bool next_is_option = (i + 1 >= argc) || std::string(argv[i + 1]).rfind("--", 0) == 0;
        if (eq != std::string::npos) {
            options.emplace_back(arg.substr(0, eq), arg.substr(eq + 1));
        } else if (is_flag(arg) && next_is_option) {
            options.emplace_back(arg, "true");
        } else if (i + 1 < argc) {
            options.emplace_back(arg, argv[++i]);
        } else {
            throw std::invalid_argument("Option --" + arg + " needs a value");
        }
    }

    // The config file is applied first, wherever --config appears, so that
    // the other flags always override it.
    for (const auto& [name, value] : options) {
        if (name == "config") read_config_file(value, config);
    }
    for (const auto& [name, value] : options) {
        if (name != "config") set_value(config, name, value, "on the command line");
    }
    return ParseResult::Run;
}

void print_usage(std::ostream& os, const char* program_name)
{
    const SimulationConfig d;   // defaults
    os << "Usage: " << program_name << " [--config FILE] [options]\n\n"
       << "Solves the 1D viscous Burgers equation  du/dt + u du/dx = nu d2u/dx2  on [0, 1]\n"
       << "by DNS or LES. Writes CSV snapshots, time_series.csv and run_config.cfg.\n\n"
       << "Options (command-line values override the config file):\n"
       << "  --config FILE        read settings from FILE (key = value lines)\n"
       << "  --nu VALUE           kinematic viscosity, Re = 1/nu    [" << d.nu << "]\n"
       << "  --t-final VALUE      final time                        [" << d.t_final << "]\n"
       << "  --n-points N         number of grid points             [" << d.n_points << "]\n"
       << "  --model dns|les      direct simulation or Smagorinsky LES [dns]\n"
       << "  --Cs VALUE           Smagorinsky constant (LES)        [" << d.Cs << "]\n"
       << "  --filter-width N     box-filter width in cells (DNS)   [" << d.filter_width << "]\n"
       << "  --input-file FILE    initial profile u(x,0); default is sin(pi x)\n"
       << "  --n-outputs N        number of output snapshots        [" << d.n_outputs << "]\n"
       << "  --output-dir DIR     where results are written         [" << d.output_dir << "]\n"
       << "  --series-every N     time-series row every N steps     [" << d.series_every << "]\n"
       << "  --n-progress N       progress lines printed per run    [" << d.n_progress << "]\n"
       << "  --quiet              print only errors and the final summary\n"
       << "  -h, --help           show this message\n\n"
       << "Example:\n"
       << "  " << program_name << " --config cases/les.cfg --nu 0.005\n";
}

} // namespace burgers
