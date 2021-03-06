
#include "svgview.h"
#include "wx/wx.h"
#include "wx/dir.h"
#include <wx/mstream.h>
#include <wxSVG/svg.h>


#ifdef USE_LIBAV
#include <wxSVG/mediadec_ffmpeg.h>
#endif



#include <algorithm>


//////////////////////////////////////////////////////////////////////////////
////////////////////////////////  MainFrame //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
enum
{
	wxID_FIT = 1,
	wxID_HITTEST,

	MENU_NAV_PREV_ID,
	MENU_NAV_NEXT_ID,
	MENU_OPEN_DIR_ID,
	TOOLBAR_INFO_ICON_ID
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_OPEN, MainFrame::OnOpenFile)
EVT_MENU(MENU_OPEN_DIR_ID, MainFrame::OnOpenDir)
EVT_MENU(wxID_SAVE, MainFrame::OnSave)

EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(MENU_NAV_PREV_ID, MainFrame::OnPrev)
EVT_MENU(MENU_NAV_NEXT_ID, MainFrame::OnNext)

END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MySVGCanvas, wxSVGCtrl)
EVT_LEFT_UP(MySVGCanvas::OnMouseLeftUp)
END_EVENT_TABLE()


MainFrame::MainFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style) :
wxFrame(parent, wxID_ANY, title, pos, size, style) {



	// Make a menubar
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_OPEN, _T("&Open file ...\tCtrl-O"));
	fileMenu->Append(MENU_OPEN_DIR_ID, _T("Open dir...\tCtrl+Alt-O"));
	fileMenu->Append(wxID_SAVE, _T("&Save as...\tCtrl-S"));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _T("&Exit\tAlt-X"));
	
	wxMenu *navMenu = new wxMenu;
	navMenu->Append(MENU_NAV_PREV_ID, _T("Previus"));
	navMenu->Append(MENU_NAV_NEXT_ID, _T("Next"));


	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _T("&File"));
	menuBar->Append(navMenu, _T("Navigation"));
	SetMenuBar(menuBar);


	LoadToolbar();

	Connect(wxEVT_CHAR_HOOK, wxKeyEventHandler(MainFrame::OnKeyDown));

#ifndef __WXMSW__
	SetIcon(wxICON_FROM_MEMORY(wxsvg));
#else
	SetIcon(wxICON(wxsvg));
#endif

	m_svgCtrl = new MySVGCanvas(this);

	m_svgCtrl->SetFitToFrame(true);
	
	if (wxTheApp->argc > 1){
		auto commandLine = wxTheApp->argv[1];
		if (wxDirExists(commandLine)){
			LoadDir(commandLine);
		}
		else if (wxFileExists(commandLine)){
			LoadDirWithFile(commandLine);
		}
		else{
			m_isInDir = false;
			// just silently ignore it
		}
	}
	Center();
	DoCreateStatusBar();


	SetMinSize(GetSize());

	Show(true);
}

void MainFrame::OnOpenFile(wxCommandEvent& event)
{
	wxString filename = wxFileSelector(_T("Choose a file to open"),
		_T(""), _T(""), _T(""), _T("SVG files (*.svg)|*.svg|All files (*.*)|*.*"));
	if (!filename.empty()){

		//TODO: Check error here
		LoadDirWithFile(filename);
		
	}
}


void MainFrame::OnOpenDir(wxCommandEvent& event)
{
	const wxString& dir = wxDirSelector("Choose a folder");
	if (!dir.empty())
	{
		LoadDir(dir);
	}
	
}
void MainFrame::OnSave(wxCommandEvent& event)
{
	wxString filename = wxFileSelector(_T("Choose a file to save"),
		_T(""), _T(""), _T(""), _T("SVG files (*.svg)|*.svg|All files (*.*)|*.*"), wxFD_SAVE);
	if (!filename.empty())
		m_svgCtrl->GetSVG()->Save(filename);
}

//void MainFrame::Hittest(wxCommandEvent& event)
//{
//	m_svgCtrl->SetShowHitPopup(event.IsChecked());
//}


void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

MySVGCanvas::MySVGCanvas(wxWindow* parent) :
wxSVGCtrl(parent), m_ShowHitPopup(false)
{
}

void MySVGCanvas::SetShowHitPopup(bool show)
{
	m_ShowHitPopup = show;
}

void MySVGCanvas::OnMouseLeftUp(wxMouseEvent & event)
{
	if (m_ShowHitPopup)
	{
		wxSVGDocument* my_doc = GetSVG();
		wxSVGSVGElement* my_root = my_doc->GetRootElement();
		wxSVGRect rect(event.m_x, event.m_y, 1, 1);
		wxNodeList clicked = my_root->GetIntersectionList(rect, *my_root);
		wxString message;
		message.Printf(_T("Click : %d,%d                  \n"), event.m_x, event.m_y);
		for (unsigned int i = 0; i < clicked.GetCount(); i++)
		{
			wxString obj_desc;
			wxSVGElement* obj = clicked.Item(i);
			obj_desc.Printf(_T("Name : %s, Id : %s\n"), obj->GetName().c_str(), obj->GetId().c_str());
			message.Append(obj_desc);
		}
		wxMessageBox(message, _T("Hit Test (objects bounding box)"));
	}
}


void MainFrame::DoCreateStatusBar(){

	wxStatusBar *statbarOld = GetStatusBar();
	if (statbarOld)
	{
		SetStatusBar(NULL);
		delete statbarOld;
	}

	wxStatusBar *statbarNew = NULL;
	/*switch (kind)
	{
	case StatBar_Default:*/
	statbarNew = new wxStatusBar(this, wxID_ANY, wxSTB_SIZEGRIP | wxSTB_ELLIPSIZE_START, "wxStatusBar");
	const int fieldsWidth[] = { -2, 150 };

	statbarNew->SetFieldsCount(2, fieldsWidth);

	//break;

	//case StatBar_Custom:
	//	statbarNew = new MyStatusBar(this, style);
	//	break;

	//default:
	//	wxFAIL_MSG(wxT("unknown status bar kind"));
	//}

	SetStatusBar(statbarNew);
	//ApplyPaneStyle();
	PositionStatusBar();

}



void MainFrame::LoadDirWithFile(const wxString& filename){


	m_workingDir = wxPathOnly(filename);


	m_isInDir = true;
	auto new_files = wxArrayString();
		//get firs svg file in the dir	
	int numFiles = wxDir::GetAllFiles(m_workingDir, &new_files, "*.svg", wxDirFlags::wxDIR_FILES);

	if (numFiles){
		//discard old files and use new ones
		m_files = new_files;

		//find file index
		auto pos = std::find_if(m_files.begin(), m_files.end(), [&filename](const wxString & file){return file == filename; });
		assert(pos != m_files.end()); //the file mus be there
		ShowFile(pos - m_files.begin());
	}
}
void MainFrame::LoadDir(const wxString& dirname){

	m_workingDir = dirname;
	m_isInDir = true;
	
	auto new_files = wxArrayString();
	//get firs svg file in the dir	
	int numFiles = wxDir::GetAllFiles(m_workingDir, &new_files, "*.svg", wxDirFlags::wxDIR_FILES);

	if (numFiles){
		m_files = new_files;
		//load file
		ShowFile(0);
	}
}

void MainFrame::ShowFile(int index){
	assert(index < m_files.size());

	auto filename = m_files[index];

	//check file exist
	if (wxFileExists(filename)){
		m_svgCtrl->Load(filename);
		m_current_file = index;

		SetStatusText(wxString::Format(_T("%03d/%03d"), index + 1, m_files.size()), 1);
		SetStatusText(filename);

	}
	else{
		//file would be removed or modified
		//reload dir with first
		LoadDir(m_workingDir);

	}

}


//TODO: add settings to o cyclic
void MainFrame::ShowNext(){

	if (m_current_file + 1 < m_files.size())
		ShowFile(m_current_file + 1);
}

void MainFrame::ShowPrev(){

	if (m_current_file > 0)
		ShowFile(m_current_file - 1);
}


void MainFrame::OnNext(wxCommandEvent& event){
	ShowNext();

}
void MainFrame::OnPrev(wxCommandEvent& event){
	ShowPrev();

}


//handle keystrokes for navigation

void MainFrame::OnKeyDown(wxKeyEvent & event){

	auto key_code = event.GetKeyCode();
	//wxMessageBox(_T("Hello"), _T("hello"));
	switch (key_code)
	{

	case wxKeyCode::WXK_RIGHT:
	case wxKeyCode::WXK_DOWN:{
		ShowNext();
	}
							 break;
	case wxKeyCode::WXK_LEFT:
	case wxKeyCode::WXK_UP:{
		ShowPrev();
	}
	default:{
		event.Skip(); // propagate up
		break;
	}
	}

}


void MainFrame::LoadToolbar(){

	auto toolbar = this->CreateToolBar(wxTB_FLAT);
	auto infoIcon = wxImage("resources/info@512px.png");
	infoIcon = infoIcon.Scale(16, 16);
	
	toolbar->AddTool(TOOLBAR_INFO_ICON_ID, wxString(_T("Info")),wxBitmap(infoIcon));
	toolbar->AddStretchableSpace(); 
	toolbar->Realize(); // make tools appear
}