/*
g++ -std=c++23 -Wall -Wextra -Wpedantic 13.04.cpp -o 13.04.out

./13.04.out --test
./13.04.out --demo
./13.04.out . '[.]cpp$'

./13.04.out . '[.]cpp$' | awk -F' \\| ' '{print $NF}' > program.out
ls -A1 . | grep -E '[.]cpp$' > grep.out
diff -u grep.out program.out
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////////////////

auto make_type(fs::file_status const & status) -> char
{
    if (fs::is_directory(status))
    {
        return 'd';
    }

    if (fs::is_regular_file(status))
    {
        return 'f';
    }

    if (fs::is_symlink(status))
    {
        return 'l';
    }

    return '?';
}

//////////////////////////////////////////////////////////////////////////////////////

auto make_permissions(fs::perms permissions) -> std::string
{
    if (permissions == fs::perms::unknown)
    {
        return "---";
    }

    auto select = [permissions](fs::perms bit, char symbol) -> char
    {
        return (permissions & bit) == fs::perms::none ? '-' : symbol;
    };

    std::string result;

    result += select(fs::perms::owner_read, 'r');
    result += select(fs::perms::owner_write, 'w');
    result += select(fs::perms::owner_exec, 'x');

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

auto is_existing_directory(fs::path const & path) -> bool
{
    std::error_code error;
    auto const status = fs::status(path, error);

    return !error && fs::is_directory(status);
}

//////////////////////////////////////////////////////////////////////////////////////

auto directory_byte_size(fs::path const & path) -> std::uintmax_t
{
    if (!is_existing_directory(path))
    {
        return 0;
    }

    auto const options = fs::directory_options::skip_permission_denied;
    std::error_code error;
    auto iterator = fs::recursive_directory_iterator(path, options, error);
    auto const end = fs::recursive_directory_iterator();
    auto result = std::uintmax_t{ 0 };

    while (!error && iterator != end)
    {
        auto const status = iterator->symlink_status(error);

        if (!error && fs::is_regular_file(status))
        {
            auto const current_size = iterator->file_size(error);

            if (!error)
            {
                result += current_size;
            }
        }

        error.clear();
        iterator.increment(error);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

auto entry_byte_size(fs::directory_entry const & entry) -> std::uintmax_t
{
    std::error_code error;
    auto const status = entry.symlink_status(error);

    if (error)
    {
        return 0;
    }

    if (fs::is_regular_file(status))
    {
        auto const result = entry.file_size(error);

        return error ? 0 : result;
    }

    if (fs::is_directory(status))
    {
        return directory_byte_size(entry.path());
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

auto format_size(std::uintmax_t size) -> std::string
{
    auto const units = std::array{ 'B', 'K', 'M', 'G' };
    auto const bytes_per_unit = std::uintmax_t{ 1024 };
    auto const field_width = 4;
    auto index = std::size_t{ 0 };

    while (index + 1 < units.size() && size >= bytes_per_unit)
    {
        size /= bytes_per_unit;
        ++index;
    }

    std::ostringstream stream;
    stream << std::setw(field_width) << size << " (" << units[index] << ')';

    return stream.str();
}

//////////////////////////////////////////////////////////////////////////////////////

auto size(fs::directory_entry const & entry) -> std::string
{
    return format_size(entry_byte_size(entry));
}

//////////////////////////////////////////////////////////////////////////////////////

auto make_last_write_time(fs::directory_entry const & entry) -> std::string
{
    std::error_code error;
    auto const file_time = entry.last_write_time(error);

    if (error)
    {
        return "unknown";
    }

    auto const system_time = fs::file_time_type::clock::to_sys(file_time);
    auto const seconds = std::chrono::floor<std::chrono::seconds>(system_time);
    auto const time_value = std::chrono::system_clock::to_time_t(seconds);
    auto const * const local_time = std::localtime(&time_value);

    if (local_time == nullptr)
    {
        return "unknown";
    }

    std::ostringstream stream;
    stream << std::put_time(local_time, "%F %T");

    return stream.str();
}

//////////////////////////////////////////////////////////////////////////////////////

class RegexNamePredicate
{
public:
    explicit RegexNamePredicate(std::string pattern)
        : m_pattern(std::move(pattern))
        , m_regex(m_pattern, std::regex::extended)
    {
    }

    auto operator()(fs::directory_entry const & entry) const -> bool
    {
        auto const name = entry.path().filename().string();

        return std::regex_search(name, m_regex);
    }

    auto pattern() const -> std::string const &
    {
        return m_pattern;
    }

private:
    std::string m_pattern;
    std::regex m_regex;
};

//////////////////////////////////////////////////////////////////////////////////////

template < typename Predicate >
auto collect_entries_if(fs::path const & path, Predicate const & predicate)
    -> std::vector<fs::directory_entry>
{
    auto result = std::vector<fs::directory_entry>{};

    if (!is_existing_directory(path))
    {
        return result;
    }

    auto const options = fs::directory_options::skip_permission_denied;
    std::error_code error;
    auto iterator = fs::directory_iterator(path, options, error);
    auto const end = fs::directory_iterator();

    while (!error && iterator != end)
    {
        if (predicate(*iterator))
        {
            result.push_back(*iterator);
        }

        iterator.increment(error);
    }

    std::sort
    (
        result.begin(),
        result.end(),
        [](fs::directory_entry const & lhs, fs::directory_entry const & rhs)
        {
            return lhs.path().filename().string() < rhs.path().filename().string();
        }
    );

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

void show_entry(fs::directory_entry const & entry)
{
    std::error_code error;
    auto const status = entry.symlink_status(error);
    auto const checked_status = error ? fs::file_status{} : status;

    std::cout
        << "show : entry : "
        << make_type(checked_status) << " | "
        << make_permissions(checked_status.permissions()) << " | "
        << size(entry) << " | "
        << make_last_write_time(entry) << " | "
        << entry.path().filename().string() << '\n';
}

//////////////////////////////////////////////////////////////////////////////////////

template < typename Predicate >
void show_if(fs::path const & path, Predicate const & predicate)
{
    for (auto const & entry : collect_entries_if(path, predicate))
    {
        show_entry(entry);
    }
}

//////////////////////////////////////////////////////////////////////////////////////

class TemporaryDirectory
{
public:
    TemporaryDirectory()
        : m_path(make_path())
    {
        fs::create_directories(m_path);
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        fs::remove_all(m_path, error);
    }

    TemporaryDirectory(TemporaryDirectory const &) = delete;

    auto operator=(TemporaryDirectory const &) -> TemporaryDirectory & = delete;

    auto path() const -> fs::path const &
    {
        return m_path;
    }

private:
    static auto make_path() -> fs::path
    {
        auto const base = fs::temp_directory_path();
        auto const ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        auto const name = std::string{ "cpp_regex_dir_" } + std::to_string(ticks);

        return base / name;
    }

    fs::path m_path;
};

//////////////////////////////////////////////////////////////////////////////////////

auto write_text(fs::path const & path, std::string_view text) -> bool
{
    std::ofstream stream(path);
    stream << text;

    return stream.good();
}

//////////////////////////////////////////////////////////////////////////////////////

auto collect_names(fs::path const & path, std::string const & pattern)
    -> std::vector<std::string>
{
    auto const predicate = RegexNamePredicate(pattern);
    auto result = std::vector<std::string>{};

    for (auto const & entry : collect_entries_if(path, predicate))
    {
        result.push_back(entry.path().filename().string());
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

auto expect_names
(
    std::string_view test_name,
    std::vector<std::string> const & actual,
    std::vector<std::string> const & expected
) -> bool
{
    if (actual == expected)
    {
        return true;
    }

    std::cerr << "test failed : " << test_name << '\n';
    std::cerr << "expected    :";

    for (auto const & name : expected)
    {
        std::cerr << ' ' << name;
    }

    std::cerr << "\nactual      :";

    for (auto const & name : actual)
    {
        std::cerr << ' ' << name;
    }

    std::cerr << '\n';

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////

auto fill_sample_directory(fs::path const & path) -> bool
{
    std::error_code error;
    auto result = true;

    result = write_text(path / "alpha.txt", "alpha\n") && result;
    result = write_text(path / "beta.cpp", "beta\n") && result;
    result = write_text(path / "gamma.cpp", "gamma\n") && result;
    result = write_text(path / "notes.md", "notes\n") && result;

    fs::create_directory(path / "src", error);
    result = !error && result;

    result = write_text(path / "src" / "hidden.cpp", "hidden\n") && result;

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

auto run_tests() -> bool
{
    auto const directory = TemporaryDirectory();
    auto const root = directory.path();
    auto result = fill_sample_directory(root);

    result = expect_names
    (
        "cpp suffix",
        collect_names(root, "[.]cpp$"),
        { "beta.cpp", "gamma.cpp" }
    ) && result;

    result = expect_names
    (
        "text suffix",
        collect_names(root, "[.]txt$"),
        { "alpha.txt" }
    ) && result;

    result = expect_names
    (
        "directory name",
        collect_names(root, "^src$"),
        { "src" }
    ) && result;

    result = expect_names
    (
        "no recursive search",
        collect_names(root, "hidden"),
        {}
    ) && result;

    std::cout << (result ? "tests : passed\n" : "tests : failed\n");

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////

void run_demo()
{
    auto const directory = TemporaryDirectory();
    auto const root = directory.path();

    if (!fill_sample_directory(root))
    {
        std::cerr << "demo : failed to create sample directory\n";
        return;
    }

    auto const pattern = std::string{ "[.]cpp$" };
    auto const predicate = RegexNamePredicate(pattern);

    std::cout << "demo : path : " << root.string() << '\n';
    std::cout << "demo : regex : " << pattern << '\n';
    std::cout << "demo : grep  : ls -1 " << root.string() << " | grep -E '" << pattern << "'\n";

    show_if(root, predicate);
}

//////////////////////////////////////////////////////////////////////////////////////

void show_usage(std::string_view program_name)
{
    std::cout
        << "usage : " << program_name << " <path> <regex>\n"
        << "        " << program_name << " --test\n"
        << "        " << program_name << " --demo\n"
        << "example : " << program_name << " . '[.]cpp$'\n"
        << "grep   : ls -1 . | grep -E '[.]cpp$'\n";
}

//////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
    try
    {
        if (argc == 2 && std::string_view{ argv[1] } == "--test")
        {
            return run_tests() ? 0 : 1;
        }

        if (argc == 2 && std::string_view{ argv[1] } == "--demo")
        {
            run_demo();
            return 0;
        }

        if (argc != 3)
        {
            show_usage(argv[0]);
            return 1;
        }

        auto const path = fs::path{ argv[1] };
        auto const predicate = RegexNamePredicate(argv[2]);

        show_if(path, predicate);

        return 0;
    }
    catch (std::regex_error const & error)
    {
        std::cerr << "error : invalid regex : " << error.what() << '\n';
        return 2;
    }
    catch (std::exception const & error)
    {
        std::cerr << "error : " << error.what() << '\n';
        return 3;
    }
}