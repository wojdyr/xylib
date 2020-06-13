// xyconvert - GUI to xylib
// Licence: Lesser GNU Public License 2.1 (LGPL)

#include <wx/wx.h>
#include <wx/aboutdlg.h>
#include <wx/cmdline.h>
#include <wx/filefn.h>
#include <wx/ffile.h>
#include <wx/filepicker.h>
#include <wx/settings.h>
#include <wx/infobar.h>
#include "xyconvert16.xpm"
#include "xyconvert48.xpm"
#include "xybrowser.h"

using namespace std;

class App : public wxApp
{
public:
    bool OnInit();
    void OnAbout(wxCommandEvent&);
    void OnConvert(wxCommandEvent&);
    void OnClose(wxCommandEvent&) { GetTopWindow()->Close(); }
    void OnDirCheckBox(wxCommandEvent&);
    void OnFolderChanged(wxFileCtrlEvent& event);
private:
    wxCheckBox *dir_cb, *overwrite, *header;
    wxDirPickerCtrl *dirpicker;
    XyFileBrowser *browser;
    wxTextCtrl *ext_tc;
    wxInfoBar *info_bar;
};

DECLARE_APP(App)
IMPLEMENT_APP(App)


static const wxCmdLineEntryDesc cmdLineDesc[] = {
    { wxCMD_LINE_SWITCH, "V", "version",
          "output version information and exit", wxCMD_LINE_VAL_NONE, 0 },
    { wxCMD_LINE_PARAM,  0, 0, "default-path", wxCMD_LINE_VAL_STRING,
                                                wxCMD_LINE_PARAM_OPTIONAL},
    { wxCMD_LINE_NONE, 0, 0, 0,  wxCMD_LINE_VAL_NONE, 0 }
};


bool App::OnInit()
{
    // to make life simpler, use the same version number as xylib
    wxString version = xylib_get_version();

    // reading numbers won't work with decimal points different than '.'
    setlocale(LC_NUMERIC, "C");

    SetAppName("xyConvert");
    wxCmdLineParser cmdLineParser(cmdLineDesc, argc, argv);
    if (cmdLineParser.Parse(false) != 0) {
        cmdLineParser.Usage();
        return false;
    }
    if (cmdLineParser.Found(wxT("V"))) {
        wxMessageOutput::Get()->Printf("xyConvert, powered by xylib "
                                       + version + "\n");
        return false;
    }
    //wxInitAllImageHandlers();
#ifdef __WXOSX__
    // wxInfoBar uses close_png in wxRendererMac::DrawTitleBarBitmap()
    wxImage::AddHandler(new wxPNGHandler);
#endif

    wxFrame *frame = new wxFrame(NULL, wxID_ANY, "xyConvert");

#ifdef __WXOSX__
    wxMenu* dummy_menu = new wxMenu;
    dummy_menu->Append(wxID_ABOUT, "&About xyConvert");
    dummy_menu->Append(wxID_EXIT, "");
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(dummy_menu, " ");
    frame->SetMenuBar(menuBar);
    Connect(wxID_ABOUT, wxEVT_MENU, wxCommandEventHandler(App::OnAbout));
#endif

#ifdef __WXMSW__
    // use icon resource, the name is assigned in xyconvert.rc
    frame->SetIcon(wxIcon("a_xyconvert"));
#else
    wxIconBundle ib;
    ib.AddIcon(wxIcon(xyconvert48_xpm));
    ib.AddIcon(wxIcon(xyconvert16_xpm));
    frame->SetIcons(ib);
#endif

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    info_bar = new wxInfoBar(frame);
    sizer->Add(info_bar, wxSizerFlags().Expand());
    browser = new XyFileBrowser(frame);
    sizer->Add(browser, wxSizerFlags(1).Expand());

    wxStaticBoxSizer *outsizer = new wxStaticBoxSizer(wxVERTICAL, frame,
                                                      "text output");
    wxBoxSizer *hsizer = new wxBoxSizer(wxHORIZONTAL);
    dir_cb = new wxCheckBox(frame, wxID_ANY, "directory:");
    hsizer->Add(dir_cb, wxSizerFlags().Centre().Border());
    dirpicker = new wxDirPickerCtrl(frame, wxID_ANY);
    hsizer->Add(dirpicker, wxSizerFlags(1));
    hsizer->AddSpacer(10);
    hsizer->Add(new wxStaticText(frame, wxID_ANY, "extension:"),
                  wxSizerFlags().Centre().Border());
    ext_tc = new wxTextCtrl(frame, wxID_ANY, "xy");
    ext_tc->SetMinSize(wxSize(50, -1));
    hsizer->Add(ext_tc, wxSizerFlags().Centre());
    hsizer->AddSpacer(10);
    overwrite = new wxCheckBox(frame, wxID_ANY, "allow overwrite");
    hsizer->Add(overwrite, wxSizerFlags().Centre());
    outsizer->Add(hsizer, wxSizerFlags().Expand());
    header = new wxCheckBox(frame, wxID_ANY, "add header");
    outsizer->Add(header, wxSizerFlags().Border());
    sizer->Add(outsizer, wxSizerFlags().Expand().Border());

    wxBoxSizer *btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton *about = new wxButton(frame, wxID_ABOUT);
    wxButton *convert = new wxButton(frame, wxID_CONVERT);
    wxButton *close = new wxButton(frame, wxID_EXIT);
    btn_sizer->Add(about, wxSizerFlags().Border());
    btn_sizer->AddStretchSpacer();
    btn_sizer->Add(convert, wxSizerFlags().Border());
    btn_sizer->Add(close, wxSizerFlags().Border());
    sizer->Add(btn_sizer, wxSizerFlags().Expand().Border());

    if (cmdLineParser.GetParamCount() > 0) {
        wxFileName fn(cmdLineParser.GetParam(0));
        if (fn.FileExists()) {
            browser->filectrl->SetPath(fn.GetFullPath());
            browser->update_file_options();
        }
    }
    wxString ini_directory = browser->filectrl->GetDirectory();
    if (ini_directory.empty())
        dirpicker->SetPath(wxGetCwd());
    else
        dirpicker->SetPath(ini_directory);
    dirpicker->Enable(false);

    frame->SetSizerAndFit(sizer);
//#ifdef __WXGTK__
    int screen_height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    frame->SetSize(-1, screen_height > 760 ? 680 : 550);
//#endif

#ifdef __WXMSW__
    // wxMSW bug workaround
    frame->SetBackgroundColour(browser->GetBackgroundColour());
#endif

    frame->Show();

    Connect(dir_cb->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
            wxCommandEventHandler(App::OnDirCheckBox));
    browser->Connect(browser->filectrl->GetId(), wxEVT_FILECTRL_FOLDERCHANGED,
            wxFileCtrlEventHandler(App::OnFolderChanged), NULL, this);

    Connect(about->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(App::OnAbout));
    Connect(convert->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(App::OnConvert));
    Connect(close->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(App::OnClose));
    return true;
}

void App::OnConvert(wxCommandEvent&)
{
    bool with_header = header->GetValue();
    int block_nr = browser->block_ch->GetSelection();
    int idx_x = browser->x_column->GetValue();
    int idx_y = browser->y_column->GetValue();
    bool has_err = browser->std_dev_b->GetValue();
    int idx_err = browser->s_column->GetValue();
    bool dec_comma = browser->comma_cb->GetValue();

    wxArrayString paths;
    browser->filectrl->GetPaths(paths);
    string options;
    if (dec_comma)
        options = "decimal-comma";

    int conv_counter = 0;
    wxString new_path;
    for (size_t i = 0; i < paths.GetCount(); ++i) {
        wxFileName old_filename(paths[i]);
        wxString fn = old_filename.GetName() + "." + ext_tc->GetValue();
        new_path = dirpicker->GetPath() + wxFILE_SEP_PATH + fn;
        if (!overwrite->GetValue() && wxFileExists(new_path)) {
            int answer = wxMessageBox("File " + fn + " exists.\n"
                                      "Overwrite?",
                                      "Overwrite?",
                                      wxYES|wxNO|wxCANCEL|wxICON_QUESTION);
            if (answer == wxCANCEL)
                break;
            if (answer != wxYES)
                continue;
        }
        try {
            wxBusyCursor wait;
            xylib::DataSet const *ds = xylib::load_file(
                                            (const char*) paths[i].ToUTF8(),
                                            browser->get_filetype(), options);
            xylib::Block const *block = ds->get_block(block_nr);
            xylib::Column const& xcol = block->get_column(idx_x);
            xylib::Column const& ycol = block->get_column(idx_y);
            xylib::Column const* ecol = (has_err ? &block->get_column(idx_err)
                                                 : NULL);
            const int np = block->get_point_count();

            wxFFile f(new_path, "w");
            if (!f.IsOpened()) {
                wxMessageBox("Cannot open file for writing:\n" + new_path,
                             "Error", wxOK|wxICON_ERROR);
                continue;
            }
            if (with_header) {
                f.Write(wxString("# Converted by xyConvert ") +
                        xylib_get_version() + ".\n"
                        "# Original file: " + paths[i] + "\n");
                if (ds->get_block_count() > 1)
                    fprintf(f.fp(), "# (block %d) %s\n",
                            block_nr, block->get_name().c_str());
                if (block->get_column_count() > 2) {
                    string xname = (xcol.get_name().empty() ? string("x")
                                                            : xcol.get_name());
                    string yname = (ycol.get_name().empty() ? string("y")
                                                            : ycol.get_name());
                    fprintf(f.fp(), "#%s\t%s", xname.c_str(), yname.c_str());
                    if (has_err) {
                        string ename = (ecol->get_name().empty() ? string("err")
                                                            : ecol->get_name());
                        fprintf(f.fp(), "\t%s", ename.c_str());
                    }
                    fprintf(f.fp(), "\n");
                }
            }

            for (int j = 0; j < np; ++j) {
                fprintf(f.fp(), "%.9g\t%.9g",
                        xcol.get_value(j), ycol.get_value(j));
                if (has_err)
                    fprintf(f.fp(), "\t%.9g", ecol->get_value(j));
                fprintf(f.fp(), "\n");
            }
            conv_counter++;
        } catch (runtime_error const& e) {
            wxMessageBox(e.what(), "Error", wxOK|wxICON_ERROR);
        }
    }
    if (conv_counter >= 1) {
        wxString str = "[" + wxDateTime::Now().FormatISOTime() + "]   ";
        if (conv_counter == 1)
            str += "file converted: " + new_path;
        else
            str += wxString::Format("%d files converted", conv_counter);

        info_bar->ShowMessage(str, wxICON_INFORMATION);
    }
}

void App::OnAbout(wxCommandEvent&)
{
    wxAboutDialogInfo adi;
    adi.SetVersion(xylib_get_version());
    wxString desc = "A simple converter based on the xylib library.\n"
                    "It can convert files supported by xylib\n"
                    "into two- or three-column text format.\n"
                    "This tool is designed to convert multiple files\n"
                    "so the output filename is not explicit.";
    adi.SetDescription(desc);
    adi.SetWebSite("http://xylib.sf.net/");
    adi.SetCopyright("(C) 2008-2015 Marcin Wojdyr <wojdyr@gmail.com>");
    wxAboutBox(adi);
}

void App::OnDirCheckBox(wxCommandEvent& event)
{
    bool checked = event.IsChecked();
    dirpicker->Enable(checked);
    if (!checked)
        dirpicker->SetPath(browser->filectrl->GetDirectory());
}

void App::OnFolderChanged(wxFileCtrlEvent& event)
{
    if (!dir_cb->GetValue())
        dirpicker->SetPath(event.GetDirectory());
}

