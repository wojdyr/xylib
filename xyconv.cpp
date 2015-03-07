// Convert file supported by xylib to ascii format
// Licence: Lesser GNU Public License 2.1 (LGPL)

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <string.h>

#include "xylib/xylib.h"
#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h> // GetShortPathName
#endif

using namespace std;

void print_usage()
{
    cout <<
"Usage:\n"
"\txyconv [-t FILETYPE] [-x OPTION] INPUT_FILE OUTPUT_FILE\n"
"\txyconv [-t FILETYPE] [-x OPTION] -m DIR INPUT_FILE1 ...\n"
"\txyconv -i FILETYPE\n"
"\txyconv -g INPUT_FILE ...\n"
"\txyconv [-l|-v|-h]\n"
"  Converts INPUT_FILE to ascii OUTPUT_FILE\n"
"  -t     specify filetype of input file\n"
"  -x     specify option for filetype (can be used more than once)\n"
"  -m DIR convert one or multiple files; output files are written in DIR,\n"
"         with the same basename and extension .xy\n"
"  -l     list all supported file types\n"
"  -v     output version information and exit\n"
"  -h     show this help message and exit\n"
"  -i     show information about filetype\n"
"  -s     do not output metadata\n"
"  -g     guess filetype of file \n"
"  To write the results to standard output use `-' as OUTPUT_FILE\n";
}

// Print version of the library. This program is too small to have own version.
void print_version()
{
    cout << XYLIB_VERSION / 10000 << "."
         << XYLIB_VERSION / 100 % 100 << "."
         << XYLIB_VERSION % 100 << endl;
}

void list_supported_formats()
{
    const xylibFormat* format = NULL;
    for (int i = 0; (format = xylib_get_format(i)) != NULL; ++i)
        cout << setw(20) << left << format->name << ": "
             << format->desc << endl;
}

#ifdef _WIN32
string short_path(const char* path)
{
    string short_path;
    short_path.resize(GetShortPathNameA(path, NULL, 0));
    if (short_path.empty()) // failed, try path anyway
        short_path = path;
    else
        GetShortPathNameA(path, &short_path[0], short_path.size());
    return short_path;
}
#endif

int print_guessed_filetype(int n, char** paths)
{
    bool ok = true;
    for (int i = 0; i < n; ++i) {
        const char* path = paths[i];
        if (n > 1)
            cout << path << ": ";
        try {
#ifdef _WIN32
            ifstream is(short_path(path).c_str());
#else
            ifstream is(path);
#endif
            if (!is) {
                cout << "Error: can't open input file: " << path << endl;
                ok = false;
                continue;
            }
            string details;
            xylib::FormatInfo const* fi = xylib::guess_filetype(path, is,
                                                                &details);
            if (fi && strcmp(fi->name, "text") != 0) {
                cout << fi->name << ": " << fi->desc;
                if (!details.empty())
                    cout << " (" << details << ")";
            }
            else
                // "text" is a fallback, it can be anything
                cout << fi->name << ": unknown or text";
            cout << endl;
        } catch (runtime_error const& e) {
            cout << "Error: " << e.what() << endl;
            ok = false;
        }
    }
    return ok ? 0 : -1;
}

void print_filetype_info(string const& filetype)
{
        xylibFormat const* fi = xylib_get_format_by_name(filetype.c_str());
        if (fi) {
            cout << "Name: " << fi->name << endl;
            cout << "Description: " << fi->desc << endl;
            bool has_exts = (strlen(fi->exts) != 0);
            cout << "Possible extensions: "
                 << (has_exts ? fi->exts : "(not specified)") << endl;
            cout << "Other flags: "
                << (fi->binary ? "binary-file" : "text-file") << " "
                << (fi->multiblock ? "multi-block" : "single-block") << endl;
        }
        else
            cout << "Unknown file format. "
                    "(To guess format of a file use option -g)." << endl;
}

void export_metadata(FILE *f, xylib::MetaData const& meta)
{
    for (size_t i = 0; i != meta.size(); ++i) {
        const string& key = meta.get_key(i);
        const string& value = meta.get(key);
        string::size_type pos = 0;
        for (;;) {
            string::size_type new_pos = value.find('\n', pos);
            string vline = value.substr(pos, new_pos-pos);
            fprintf(f, "# %s: %s\n", key.c_str(), vline.c_str());
            if (new_pos == string::npos)
                break;
            pos = new_pos + 1;
        }
    }
}

void export_plain_text(xylib::DataSet const *d, string const &fname,
                       bool with_metadata)
{
    FILE *f;
    if (fname == "-")
        f = stdout;
    else {
        f = fopen(fname.c_str(), "w");
        if (!f)
            throw xylib::RunTimeError("can't create file: " + fname);
    }

    // output the file-level meta-info
    fprintf(f, "# exported by xylib from a %s file\n", d->fi->name);
    if (with_metadata && d->meta.size() != 0) {
        export_metadata(f, d->meta);
        fprintf(f, "\n");
    }

    int nb = d->get_block_count();
    //printf("%d block(s).\n", nb);
    for (int i = 0; i < nb; ++i) {
        const xylib::Block *block = d->get_block(i);
        if (nb > 1 || !block->get_name().empty())
            fprintf(f, "\n### block #%d %s\n", i, block->get_name().c_str());
        if (with_metadata)
            export_metadata(f, block->meta);

        int ncol = block->get_column_count();
        fprintf(f, "# ");
        // column 0 is pseudo-column with point indices, we skip it
        for (int k = 1; k <= ncol; ++k) {
            string const& name = block->get_column(k).get_name();
            if (k > 1)
                fprintf(f, "\t");
            if (name.empty())
                fprintf(f, "column_%d", k);
            else
                fprintf(f, "%s", name.c_str());
        }
        fprintf(f, "\n");

        int nrow = block->get_point_count();

        for (int j = 0; j < nrow; ++j) {
            for (int k = 1; k <= ncol; ++k) {
                if (k > 1)
                    fprintf(f, "\t");
                fprintf(f, "%.6f", block->get_column(k).get_value(j));
            }
            fprintf(f, "\n");
        }
    }
    if (fname != "-")
        fclose(f);
}


int convert_file(string const& input, string const& output,
                 string const& filetype, string const& options,
                 bool with_metadata)
{
    try {
#ifdef _WIN32
        string input_s = short_path(input.c_str());
#else
        const string& input_s = input;
#endif
        xylib::DataSet *d = xylib::load_file(input_s, filetype, options);
        // validate options
        for (const char *p = options.c_str(); *p != '\0'; ) {
            while (isspace(*p))
                ++p;
            const char* end = p;
            while (*end != '\0' && !isspace(*end))
                ++end;
            string opt(p, end);
            if (!d->is_valid_option(opt))
                printf("WARNING: Invalid option %s for format %s.\n",
                        opt.c_str(), d->fi->name);
            p = end;
        }

        export_plain_text(d, output, with_metadata);
        delete d;
    } catch (runtime_error const& e) {
        cerr << "Error. " << e.what() << endl;
        return -1;
    }
    return 0;
}

string get_basename(const string& path)
{
#ifdef _WIN32
    size_t last_sep = path.find_last_of("\\/");
#else
    size_t last_sep = path.rfind('/');
#endif
    size_t start = last_sep == string::npos ? 0 : last_sep + 1;
    size_t end = path.rfind('.');
    if (end != string::npos && end > start)
        return path.substr(start, end-start);
    else
        return path.substr(start);
}

int main(int argc, char **argv)
{
    // options -l -h -i -g -v are not combined with other options

    if (argc == 2 && strcmp(argv[1], "-l") == 0) {
        list_supported_formats();
        return 0;
    }
    else if (argc == 2 &&
            (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage();
        return 0;
    }
    else if (argc == 2 &&
            (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        print_version();
        return 0;
    }
    else if (argc == 3 && strcmp(argv[1], "-i") == 0) {
        print_filetype_info(argv[2]);
        return 0;
    }
    else if (argc >= 3 && strcmp(argv[1], "-g") == 0)
        return print_guessed_filetype(argc - 2, argv + 2);
    else if (argc < 3) {
        print_usage();
        return -1;
    }

    string filetype;
    string options;
    string option_m;
    bool option_s = false;
    int n = 1;
    while (n < argc - 1) {
        if (strcmp(argv[n], "-m") == 0) {
            option_m = argv[n+1];
            if (!xylib::is_directory(option_m)) {
                cerr << "Directory does not exist: " << option_m << endl;
                return 1;
            }
            n += 2;
        }
        else if (strcmp(argv[n], "-s") == 0) {
            option_s = true;
            ++n;
        }
        else if (strcmp(argv[n], "-t") == 0 && n+1 < argc - 1) {
            filetype = argv[n+1];
            n += 2;
        }
        else if (strcmp(argv[n], "-x") == 0 && n+1 < argc - 1) {
            options += string(" ") + argv[n+1];
            n += 2;
        }
        else
            break;
    }
    if (option_m.empty() && n != argc - 2) {
        print_usage();
        return -1;
    }
    if (!option_m.empty()) {
        for ( ; n < argc; ++n) {
            string path = option_m + "/" + get_basename(argv[n]) + ".xy";
            cout << "converting " << argv[n] << " to " << path << endl;
            convert_file(argv[n], path, filetype, options, !option_s);
        }
        return 0;
    }
    else
        return convert_file(argv[argc-2], argv[argc-1], filetype, options,
                            !option_s);
}

