//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2004, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// frmMain.cpp - The main form
//
// Note: Due to the size of frmMain, event handler, browser and statistics 
//       functions are in events.cpp, browser.cpp and statistics.cpp.
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

// wxWindows headers
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/settings.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/toolbar.h>
#include <wx/imaglist.h>

// App headers
#include "misc.h"
#include "menu.h"
#include "frmMain.h"
#include "ctlSQLBox.h"
#include "pgConn.h"
#include "pgDatabase.h"
#include "pgSet.h"
#include "pgServer.h"
#include "pgObject.h"
#include "pgCollection.h"
#include "frmQueryBuilder.h"
#include "frmHelp.h"


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(windowList);

// Icons
#include "images/aggregate.xpm"
#include "images/baddatabase.xpm"
#include "images/check.xpm"
#include "images/closeddatabase.xpm"
#include "images/cast.xpm"
#include "images/conversion.xpm"
#include "images/column.xpm"
#include "images/connect.xpm"
#include "images/create.xpm"
#include "images/database.xpm"
#include "images/domain.xpm"
#include "images/drop.xpm"
#include "images/function.xpm"
#include "images/group.xpm"
#include "images/index.xpm"
#include "images/language.xpm"
#include "images/foreignkey.xpm"
#include "images/namespace.xpm"
#include "images/operator.xpm"
#include "images/operatorclass.xpm"
#include "images/pgAdmin3.xpm"
#include "images/properties.xpm"
#include "images/property.xpm"
#include "images/public.xpm"
#include "images/refresh.xpm"
#include "images/querybuilder.xpm"
#include "images/relationship.xpm"
#include "images/rule.xpm"
#include "images/sequence.xpm"
#include "images/server.xpm"
#include "images/servers.xpm"
#include "images/serverbad.xpm"
#include "images/sql.xpm"
#include "images/statistics.xpm"
#include "images/table.xpm"
#include "images/trigger.xpm"
#include "images/type.xpm"
#include "images/user.xpm"
#include "images/vacuum.xpm"
#include "images/view.xpm"
#include "images/viewdata.xpm"
#include "images/viewfiltereddata.xpm"
#include "images/triggerfunction.xpm"
#include "images/constraints.xpm"
#include "images/primarykey.xpm"
#include "images/unique.xpm"
#include "images/help2.xpm"
#include "images/agent.xpm"
#include "images/job.xpm"
#include "images/jobdisabled.xpm"
#include "images/step.xpm"
#include "images/schedule.xpm"


#if wxDIALOG_UNIT_COMPATIBILITY
#error wxWindows must be compiled with wxDIALOG_UNIT_COMPATIBILITY=0!
#endif

frmMain::frmMain(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
    msgLevel=0;

    wxWindowBase::SetFont(settings->GetSystemFont());

    {
        wxLogInfo(wxT("Using fontmetrics %d/%d, %d Point"), GetCharWidth(), GetCharHeight(), GetFont().GetPointSize());
        wxLogInfo(wxT("Native Description '%s'"), GetFont().GetNativeFontInfoDesc().c_str());
        wxWindowDC dc(this);
        dc.SetFont(GetFont());

        wxCoord w, h, d, e;

        dc.GetTextExtent(wxT("M"), &w, &h, &d, &e);
        wxLogInfo(wxT("Draw size of 'M': w=%d, h=%d, descent %d, external lead %d."), w, h, d, e);

        dc.GetTextExtent(wxT("g"), &w, &h, &d, &e);
        wxLogInfo(wxT("Draw size of 'g': w=%d, h=%d, descent %d, external lead %d."), w, h, d, e);

        dc.GetTextExtent(wxT("Mg"), &w, &h, &d, &e);
        wxLogInfo(wxT("Draw size of 'Mg': w=%d, h=%d, descent %d, external lead %d."), w, h, d, e);
    }

    // Current database
    m_database = NULL;
    denyCollapseItem=wxTreeItemId();

    // Icon
    SetIcon(wxIcon(pgAdmin3_xpm));

    // Build menus
    menuBar = new wxMenuBar();

        // File Menu
    fileMenu = new wxMenu();
    fileMenu->Append(MNU_SAVEDEFINITION, _("&Save definition..."),_("Save the SQL definition of the selected object."));
    fileMenu->AppendSeparator();
    fileMenu->Append(MNU_ADDSERVER, _("&Add Server..."),          _("Add a connection to a server."));
    fileMenu->Append(MNU_PASSWORD, _("C&hange password..."),      _("Change your password."));
    fileMenu->AppendSeparator();
    fileMenu->Append(MNU_OPTIONS, _("&Options..."),               _("Show options dialog."));
#ifdef __WXMAC__
    wxApp::s_macPreferencesMenuItemId = MNU_OPTIONS;
#endif
    fileMenu->AppendSeparator();
    fileMenu->Append(MNU_EXIT, _("E&xit\tAlt-F4"),                _("Quit this program."));
    menuBar->Append(fileMenu, _("&File"));

    // Edit Menu
    editMenu = new wxMenu();
    newMenu=new wxMenu();
    editMenu->Append(MNU_NEWOBJECT, _("New &Object"), newMenu,    _("Create a new object."));
    editMenu->AppendSeparator();
    editMenu->Append(MNU_CREATE, _("&Create"),                    _("Create a new object of the same type as the selected object."));
    editMenu->Append(MNU_DROP, _("&Delete/Drop\tDel"),            _("Delete/Drop the selected object."));
    editMenu->Append(MNU_PROPERTIES, _("&Properties"),    		  _("Display/edit the properties of the selected object."));
    menuBar->Append(editMenu, _("&Edit"));

    // Tools Menu
    toolsMenu = new wxMenu();
    toolsMenu->Append(MNU_CONNECT, _("&Connect..."),              _("Connect to the selected server."));
    toolsMenu->Append(MNU_DISCONNECT, _("Disconnec&t"),           _("Disconnect from the selected server."));
    toolsMenu->AppendSeparator();
    toolsMenu->Append(MNU_SQL, _("&Query tool"),                  _("Execute arbitrary SQL queries."));
    toolsMenu->Append(MNU_QUERYBUILDER, _("Query &builder"),      _("Start the query builder."));
	toolsMenu->Append(MNU_VIEWDATA, _("View &Data"),              _("View the data in the selected object."));
	toolsMenu->Append(MNU_VIEWFILTEREDDATA, _("View F&iltered Data"), _("Apply a filter and view the data in the selected object."));
    toolsMenu->Append(MNU_MAINTENANCE, _("&Maintenance"),         _("Maintain the current database or table."));
    toolsMenu->Append(MNU_INDEXCHECK, _("&FK Index check"),       _("Checks existence of foreign key indexes"));
    toolsMenu->Append(MNU_RELOAD, _("Re&load module"),            _("Reload library module which implements this function."));
    toolsMenu->Append(MNU_STATUS, _("&Server Status"),            _("Displays the current database status."));
    menuBar->Append(toolsMenu, _("&Tools"));

    // View Menu
    viewMenu = new wxMenu();
    viewMenu->Append(MNU_SYSTEMOBJECTS, _("&System objects"),     _("Show or hide system objects."), wxITEM_CHECK);
    viewMenu->AppendSeparator();
    viewMenu->Append(MNU_REFRESH, _("Re&fresh\tF5"),              _("Refresh the selected object."));
    menuBar->Append(viewMenu, _("&Display"));

    // Help Menu
    helpMenu = new wxMenu();
    helpMenu->Append(MNU_CONTENTS, _("&Help..."),                 _("Open the pgAdmin III helpfile."));
    helpMenu->Append(MNU_FAQ, _("pgAdmin III &FAQ"),              _("Frequently asked questions about pgAdmin III."));
    helpMenu->Append(MNU_HELP, _("&SQL Help\tF1"),                _("Display help on SQL commands."));
    helpMenu->Append(MNU_PGSQLHELP, _("&PostgreSQL Help"),        _("Display help on PostgreSQL database system."));
    helpMenu->Append(MNU_TIPOFTHEDAY, _("&Tip of the day..."),    _("Show a tip of the day."));
    helpMenu->AppendSeparator();
    helpMenu->Append(MNU_BUGREPORT, _("&Bugreport"),              _("How to send a bugreport to the pgAdmin Development Team."));
    helpMenu->Append(MNU_ABOUT, _("&About..."),                   _("Show about dialog."));
#ifdef __WXMAC__
    wxApp::s_macAboutMenuItemId = MNU_ABOUT;
#endif 
    menuBar->Append(helpMenu, _("&Help"));

    treeContextMenu = new wxMenu();
    // Tree Context Menu
    newContextMenu = new wxMenu();
    treeContextMenu->Append(MNU_REFRESH, _("Re&fresh\tF5"),		  _("Refresh the selected object."));
    treeContextMenu->Append(MNU_RELOAD, _("Re&load module"),	  _("Reload library module which implements this function."));
    treeContextMenu->Append(MNU_NEWOBJECT, _("New &Object"), newContextMenu, _("Create a new object."));
    treeContextMenu->AppendSeparator();
	treeContextMenu->Append(MNU_VIEWDATA, _("View &Data"),        _("View the data in the selected object."));
	treeContextMenu->Append(MNU_VIEWFILTEREDDATA, _("View F&iltered Data"),  _("Apply a filter and view the data in the selected object."));
    treeContextMenu->Append(MNU_MAINTENANCE, _("&Maintenance"),   _("Maintain the current database or table."));
    treeContextMenu->Append(MNU_INDEXCHECK, _("&FK Index check"), _("Checks existence of foreign key indexes"));
    treeContextMenu->Append(MNU_CONNECT, _("&Connect..."),        _("Connect to the selected server."));
    treeContextMenu->Append(MNU_DISCONNECT, _("&Disconnect"),     _("Disconnect from the selected server."));
    treeContextMenu->AppendSeparator();
    treeContextMenu->Append(MNU_CREATE, _("&Create"),             _("Create a new object of the same type as the selected object."));
    treeContextMenu->Append(MNU_DROP, _("&Delete/Drop"),  	      _("Delete/Drop the selected object."));
    treeContextMenu->Append(MNU_PROPERTIES, _("&Properties"),     _("Display/edit the properties of the selected object."));


    // Add the Menubar and set some options
    SetMenuBar(menuBar);

    editMenu->Enable(MNU_NEWOBJECT, false);
    treeContextMenu->Enable(MNU_NEWOBJECT, false);
    fileMenu->Enable(MNU_PASSWORD, false);
    viewMenu->Check(MNU_SYSTEMOBJECTS, settings->GetShowSystemObjects());

    // Status bar
    statusBar = CreateStatusBar(3);
    int iWidths[3] = {0, -1, 100};
    SetStatusWidths(3, iWidths);
    SetStatusText(wxT(""), 0);
    SetStatusText(_("Ready."), 1);
    SetStatusText(_("0 Secs"), 2);

    wxAcceleratorEntry entries[4];
    entries[0].Set(wxACCEL_NORMAL, WXK_F5, MNU_REFRESH);
    entries[1].Set(wxACCEL_NORMAL, WXK_DELETE, MNU_DELETE);
    entries[2].Set(wxACCEL_NORMAL, WXK_F1, MNU_HELP);
    entries[3].Set(wxACCEL_SHIFT, WXK_F10, MNU_CONTEXTMENU);
    wxAcceleratorTable accel(4, entries);

    SetAcceleratorTable(accel);

    toolBar=CreateToolBar();
    toolBar->SetToolBitmapSize(wxSize(32, 32));

    toolBar->AddTool(MNU_ADDSERVER, _("Add Server"), wxBitmap(connect_xpm), _("Add a connection to a server."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_REFRESH, _("Refresh"), wxBitmap(refresh_xpm), _("Refresh the selected object."), wxITEM_NORMAL);
    toolBar->AddSeparator();
    toolBar->AddTool(MNU_CREATE, _("Create"), wxBitmap(create_xpm), _("Create a new object of the same type as the selected object."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_DROP, _("Drop"), wxBitmap(drop_xpm), _("Drop the currently selected object."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_PROPERTIES, _("Properties"), wxBitmap(properties_xpm), _("Display/edit the properties of the selected object."), wxITEM_NORMAL);
    toolBar->AddSeparator();
    toolBar->AddTool(MNU_SQL, _("Query tool"), wxBitmap(sql_xpm), _("Execute arbitrary SQL queries."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_QUERYBUILDER, _("Query builder"), wxBitmap(querybuilder_xpm), _("Visually build SQL queries."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_VIEWDATA, _("View Data"), wxBitmap(viewdata_xpm), _("View the data in the selected object."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_VIEWFILTEREDDATA, _("View Filtered Data"), wxBitmap(viewfiltereddata_xpm), _("Apply a filter and view the data in the selected object."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_MAINTENANCE, _("Maintenance"), wxBitmap(vacuum_xpm), _("Maintain the current database or table."), wxITEM_NORMAL);
    toolBar->AddSeparator();
    toolBar->AddTool(MNU_HELP, _("SQL Help"), wxBitmap(help2_xpm), _("Display help on SQL commands."));

    // Display the bar and configure buttons. 
    toolBar->Realize();
    SetButtons(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
    toolBar->EnableTool(MNU_STOP, FALSE);
    
    // Setup the vertical splitter & treeview
    vertical = new wxSplitterWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN);
    horizontal = new wxSplitterWindow(vertical, -1, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN);
    browser = new wxTreeCtrl(vertical, CTL_BROWSER, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS | wxSIMPLE_BORDER);
    int splitpos=settings->Read(wxT("frmMain/SplitVertical"), 200);
    if (splitpos < 50)
        splitpos = 50;
    if (splitpos > GetSize().x-50)
        splitpos = GetSize().x-50;
    vertical->SplitVertically(browser, horizontal, splitpos);
    vertical->SetMinimumPaneSize(50);

    // Setup the horizontal splitter for the listview & sql pane
    listViews = new wxNotebook(horizontal, CTL_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_BOTTOM);
    properties = new ctlListView(listViews, CTL_PROPVIEW, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    statistics = new ctlListView(listViews, CTL_STATVIEW, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    dependsOn = new ctlListView(listViews, CTL_DEPVIEW, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    referencedBy = new ctlListView(listViews, CTL_REFVIEW, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    listViews->AddPage(properties, _("Properties"));        // NBP_PROPERTIES
    listViews->AddPage(statistics, _("Statistics"));        // NBP_STATISTICS
    listViews->AddPage(dependsOn, _("Depends on"));         // NBP_DEPENDSON
    listViews->AddPage(referencedBy, _("Referenced by"));   // NBP_REFERENCEDBY
    sqlPane = new ctlSQLBox(horizontal, CTL_SQLPANE, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxSIMPLE_BORDER | wxTE_READONLY | wxTE_RICH2);

    splitpos=settings->Read(wxT("frmMain/SplitHorizontal"), 300);
    if (splitpos < 50)
        splitpos = 50;
    if (splitpos > GetSize().y-50)
        splitpos = GetSize().y-50;
    horizontal->SplitHorizontally(listViews, sqlPane, splitpos);
    horizontal->SetMinimumPaneSize(50);

    //Setup the global imagelist
	images = new wxImageList(16, 16, true, 50);
    images->Add(wxIcon(property_xpm));
    images->Add(wxIcon(statistics_xpm));
    images->Add(wxIcon(servers_xpm));
    images->Add(wxIcon(server_xpm));
    images->Add(wxIcon(serverbad_xpm));
    images->Add(wxIcon(database_xpm));
    images->Add(wxIcon(language_xpm));
    images->Add(wxIcon(namespace_xpm));
    images->Add(wxIcon(aggregate_xpm));
    images->Add(wxIcon(function_xpm));
    images->Add(wxIcon(operator_xpm));
    images->Add(wxIcon(sequence_xpm));
    images->Add(wxIcon(table_xpm));
    images->Add(wxIcon(type_xpm));
    images->Add(wxIcon(view_xpm));
    images->Add(wxIcon(user_xpm));
    images->Add(wxIcon(group_xpm));
    images->Add(wxIcon(baddatabase_xpm));
    images->Add(wxIcon(closeddatabase_xpm));
    images->Add(wxIcon(domain_xpm));
    images->Add(wxIcon(check_xpm));
    images->Add(wxIcon(column_xpm));
    images->Add(wxIcon(relationship_xpm));
    images->Add(wxIcon(index_xpm));
    images->Add(wxIcon(rule_xpm));
    images->Add(wxIcon(trigger_xpm));
    images->Add(wxIcon(foreignkey_xpm));
    images->Add(wxIcon(cast_xpm));
    images->Add(wxIcon(conversion_xpm));
    images->Add(wxIcon(operatorclass_xpm));
    images->Add(wxIcon(triggerfunction_xpm));
    images->Add(wxIcon(constraints_xpm));
    images->Add(wxIcon(primarykey_xpm));
    images->Add(wxIcon(unique_xpm));
    images->Add(wxIcon(public_xpm));

    // agent, job, jobdisabled, step, schedule
    images->Add(wxIcon(agent_xpm));
    images->Add(wxIcon(job_xpm));
    images->Add(wxIcon(jobdisabled_xpm));
    images->Add(wxIcon(step_xpm));
    images->Add(wxIcon(schedule_xpm));

    browser->SetImageList(images);

    // Add the root node
    pgObject *serversObj = new pgServers();
    servers = browser->AddRoot(_("Servers"), PGICON_SERVERS, -1, serversObj);

    properties->SetImageList(images, wxIMAGE_LIST_SMALL);
    // Add the property view columns
    properties->AddColumn(_("Properties"), 500);
    properties->InsertItem(0, _("No properties are available for the current selection"), PGICON_PROPERTY);


    statistics->SetImageList(images, wxIMAGE_LIST_SMALL);
    // Add the statistics view columns & set the colour
    statistics->AddColumn(_("Statistics"), 500);
    statistics->InsertItem(0, _("No statistics are available for the current selection"), PGICON_STATISTICS);

    wxColour background;
    background = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    statistics->SetBackgroundColour(background);
    dependsOn->SetBackgroundColour(background);
    referencedBy->SetBackgroundColour(background);
    sqlPane->SetBackgroundColour(background);

    // Load servers
    RetrieveServers();
    browser->Expand(servers);
}


frmMain::~frmMain()
{
    StoreServers();

    settings->Write(wxT("frmMain"), GetSize(), GetPosition());
    settings->Write(wxT("frmMain/SplitHorizontal"), horizontal->GetSashPosition());
    settings->Write(wxT("frmMain/SplitVertical"), vertical->GetSashPosition());

    // Clear the treeview
    browser->DeleteAllItems();

    delete treeContextMenu;
	delete images;
}


void frmMain::Refresh(pgObject *data)
{
    StartMsg(wxString::Format(_("Refreshing %s..."), data->GetTypeName().c_str()));
    browser->Freeze();

    wxTreeItemId currentItem=data->GetId();
    browser->DeleteChildren(data->GetId());

	// refresh information about the object

    data->SetDirty();
    
    pgObject *newData = data->Refresh(browser, currentItem);

    if (newData != data)
    {
        wxLogInfo(wxT("Deleting ") + data->GetTypeName() + wxT(" ") 
            + data->GetQuotedFullIdentifier() + wxT(" for Refresh"));

        if (newData)
        {
            wxLogInfo(wxT("Replacing with new Node ") + newData->GetTypeName() + wxT(" ") 
                + newData->GetQuotedFullIdentifier() + wxT(" for Refresh"));
            newData->SetId(currentItem);    // not done automatically
            browser->SetItemData(currentItem, newData);
            delete data;
        }
        else
        {
            wxLogInfo(wxT("No object to replace: vanished after refresh."));
            wxTreeItemId delItem=currentItem;
            currentItem=browser->GetItemParent(currentItem);
            browser->SelectItem(currentItem);
            browser->Delete(delItem);
        }
    }
    if (currentItem)
        execSelChange(currentItem, true);
    browser->Thaw();
    EndMsg();
}



void frmMain::OnPageChange(wxNotebookEvent& event)
{
    wxTreeItemId item=browser->GetSelection();
	if (!item)
		return;
    pgObject *data = (pgObject*)browser->GetItemData(item);

    if (!data)
        return;

    switch (event.GetSelection())
    {
        case NBP_STATISTICS:
            data->ShowStatistics(statistics);
            break;
        case NBP_DEPENDSON:
            data->ShowDependsOn(dependsOn);
            break;
        case NBP_REFERENCEDBY:
            data->ShowReferencedBy(referencedBy);
            break;
        default:
            break;
    }
}


ctlListView *frmMain::GetStatistics()
{
    if (listViews->GetSelection() == NBP_STATISTICS)
        return statistics;
    return 0;
}


ctlListView *frmMain::GetDependsOn()
{
    if (listViews->GetSelection() == NBP_DEPENDSON)
        return dependsOn;
    return 0;
}


ctlListView *frmMain::GetReferencedBy()
{
    if (listViews->GetSelection() == NBP_REFERENCEDBY)
        return referencedBy;
    return 0;
}


pgObject *frmMain::GetSelectedObject()
{
    wxTreeItemId item=browser->GetSelection();
    pgObject *data = (pgObject*)browser->GetItemData(item);

    /* When it isn't connected. */
    if (data == NULL)
        return 0;
	
    if (FindFocus() == properties && data->IsCollection())
    {
        return ((pgCollection*)data)->FindChild(browser, properties->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
    }
    else
        return data;
}


void frmMain::RemoveFrame(wxWindow *frame)
{
    frames.DeleteObject(frame);
}

wxTreeItemId frmMain::RestoreEnvironment(pgServer *server)
{
    wxTreeItemId item, lastItem;
    wxString lastDatabase=server->GetLastDatabase();
    if (lastDatabase.IsNull())
        return item;

    wxCookieType cookie;
    pgObject *data;
    item = browser->GetFirstChild(server->GetId(), cookie);
    while (item)
    {
        data = (pgObject *)browser->GetItemData(item);
        if (data->GetType() == PG_DATABASES)
            break;
        // Get the next item
        item = browser->GetNextChild(server->GetId(), cookie);
    }
    if (!item)
        return item;

    // found DATABASES item
    data->ShowTree(this, browser, 0, 0);
    lastItem=item;

    item = browser->GetFirstChild(lastItem, cookie);
    while (item)
    {
        data = (pgObject *)browser->GetItemData(item);
        if (data->GetType() == PG_DATABASE && data->GetName() == lastDatabase)
            break;
        // Get the next item
        item = browser->GetNextChild(lastItem, cookie);
    }
    if (!item)
        return lastItem;

    // found last DATABASE 
    data->ShowTree(this, browser, 0, 0);
    lastItem = item;

    wxString lastSchema=server->GetLastSchema();
    if (lastSchema.IsNull())
        return lastItem;

    item = browser->GetFirstChild(lastItem, cookie);
    while (item)
    {
        data = (pgObject *)browser->GetItemData(item);
        if (data->GetType() == PG_SCHEMAS)
            break;
        // Get the next item
        item = browser->GetNextChild(lastItem, cookie);
    }
    if (!item)
        return lastItem;

    // found SCHEMAS item
    data->ShowTree(this, browser, 0, 0);
    lastItem=item;

    item = browser->GetFirstChild(lastItem, cookie);
    while (item)
    {
        data = (pgObject *)browser->GetItemData(item);
        if (data->GetType() == PG_SCHEMA && data->GetName() == lastSchema)
            break;
        // Get the next item
        item = browser->GetNextChild(lastItem, cookie);
    }

    return (item ? item : lastItem);
}

int frmMain::ReconnectServer(pgServer *server)
{
    // Create a server object and connect it.
    int res = server->Connect(this, TRUE);

    // Check the result, and handle it as appropriate
    wxTreeItemId item;
    switch (res)
    {
        case PGCONN_OK:
            StartMsg(_("Restoring previous environment"));
            wxLogInfo(wxT("pgServer object initialised as required."));
            browser->SetItemImage(server->GetId(), PGICON_SERVER, wxTreeItemIcon_Normal);
            browser->SetItemImage(server->GetId(), PGICON_SERVER, wxTreeItemIcon_Selected);

            server->ShowTreeDetail(browser);
            browser->Freeze();
            item=RestoreEnvironment(server);
            browser->Thaw();

            if (item)
            {
                browser->SelectItem(item);

                wxYield();
                browser->Expand(item);
                browser->EnsureVisible(item);
            }
            EndMsg();
            break;
        case PGCONN_DNSERR:
            /*
            // looks strange to me. Shouldn_t server be removed from the tree as well?
            delete server;
            OnAddServer(wxCommandEvent());
            break;
            */
        case PGCONN_BAD:
            wxLogError(server->GetLastError());
            break;
        default:
            wxLogInfo(wxT("pgServer object didn't initialise because the user aborted."));
            break;
    }
    return res;
}

void frmMain::StoreServers()
{
    wxLogInfo(wxT("Storing listed servers for later..."));

    // Store the currently listed servers for later retrieval.

    // Write the individual servers
    // Iterate through all the child nodes of the Servers node
    wxCookieType cookie;
    wxString key;
    pgObject *data;
    pgServer *server;
    int numServers = 0;

    wxTreeItemId item = browser->GetFirstChild(servers, cookie);
    while (item) {
        data = (pgObject *)browser->GetItemData(item);
        if (data->GetType() == PG_SERVER) {
            // We have a sever, so cast the object and save the settings
            ++numServers;
            server = (pgServer *)data;

            // Hostname
            key.Printf(wxT("Servers/Server%d"), numServers);
            settings->Write(key, server->GetName());

            key.Printf(wxT("Servers/Description%d"), numServers);
            settings->Write(key, server->GetDescription());

            // Port
            key.Printf(wxT("Servers/Port%d"), numServers);
            settings->Write(key, server->GetPort());

            // Trusted
            key.Printf(wxT("Servers/Trusted%d"), numServers);
            settings->Write(key, BoolToStr(server->GetTrusted()));

            // Database
            key.Printf(wxT("Servers/Database%d"), numServers);
            settings->Write(key, server->GetDatabaseName());

            // Username
            key.Printf(wxT("Servers/Username%d"), numServers);
            settings->Write(key, server->GetUsername());

            // last Database
            key.Printf(wxT("Servers/LastDatabase%d"), numServers);
            settings->Write(key, server->GetLastDatabase());

            // last Schema
            key.Printf(wxT("Servers/LastSchema%d"), numServers);
            settings->Write(key, server->GetLastSchema());
            
            key.Printf(wxT("Servers/SSL%d"), numServers);
            settings->Write(key, server->GetSSL());
        }

        // Get the next item
        item = browser->GetNextChild(servers, cookie);
    }

    // Write the server count
    settings->Write(wxT("Servers/Count"), numServers);

    wxLogInfo(wxT("Stored %d servers."), numServers);
}

void frmMain::RetrieveServers()
{
    // Retrieve previously stored servers
    wxLogInfo(wxT("Reloading servers..."));

    long numServers=settings->Read(wxT("Servers/Count"), 0L);

    int loop, port, ssl=0;
    wxString key, servername, description, database, username, lastDatabase, lastSchema, trusted;
    pgServer *server;

    for (loop = 1; loop <= numServers; ++loop) {
        
        // Server
        key.Printf(wxT("Servers/Server%d"), loop);
        settings->Read(key, &servername, wxT(""));

        // Comment
        key.Printf(wxT("Servers/Description%d"), loop);
        settings->Read(key, &description, wxT(""));

        // Trusted
        key.Printf(wxT("Servers/Trusted%d"), loop);
        settings->Read(key, &trusted, wxT(""));

        // Port
        key.Printf(wxT("Servers/Port%d"), loop);
        settings->Read(key, &port, 0);

        // Database
        key.Printf(wxT("Servers/Database%d"), loop);
        settings->Read(key, &database, wxT(""));

        // Username
        key.Printf(wxT("Servers/Username%d"), loop);
        settings->Read(key, &username, wxT(""));

        // last Database
        key.Printf(wxT("Servers/LastDatabase%d"), loop);
        settings->Read(key, &lastDatabase, wxT(""));

        // last Schema
        key.Printf(wxT("Servers/LastSchema%d"), loop);
        settings->Read(key, &lastSchema, wxT(""));

        // SSL mode
#ifdef SSL
        key.Printf(wxT("Servers/SSL%d"), loop);
        settings->Read(key, &ssl, 0);
#endif

        // Add the Server node
        server = new pgServer(servername, description, database, username, port, StrToBool(trusted), ssl);
        server->iSetLastDatabase(lastDatabase);
        server->iSetLastSchema(lastSchema);
        browser->AppendItem(servers, server->GetFullName(), PGICON_SERVERBAD, -1, server);
    }

    // Reset the Servers node text
    wxString label;
    label.Printf(_("Servers (%d)"), browser->GetChildrenCount(servers, FALSE));
    browser->SetItemText(servers, label);
}


void frmMain::SetButtons(bool refresh, bool create, bool drop, bool properties, bool sql, bool viewData, bool maintenance)
{
    toolBar->EnableTool(MNU_REFRESH, refresh);
    toolBar->EnableTool(MNU_CREATE, create);
    toolBar->EnableTool(MNU_DROP, drop);
    toolBar->EnableTool(MNU_PROPERTIES, properties);
    toolBar->EnableTool(MNU_SQL, sql);
    toolBar->EnableTool(MNU_QUERYBUILDER, sql);
    toolBar->EnableTool(MNU_VIEWDATA, viewData);
    toolBar->EnableTool(MNU_VIEWFILTEREDDATA, viewData);
    toolBar->EnableTool(MNU_MAINTENANCE, maintenance);

	// Handle the menus associated with the buttons
	editMenu->Enable(MNU_CREATE, create);
	editMenu->Enable(MNU_DROP, drop);
	editMenu->Enable(MNU_PROPERTIES, properties);
	toolsMenu->Enable(MNU_CONNECT, false);
	toolsMenu->Enable(MNU_DISCONNECT, false);
	toolsMenu->Enable(MNU_SQL, sql);
	toolsMenu->Enable(MNU_QUERYBUILDER, sql);
	toolsMenu->Enable(MNU_MAINTENANCE, maintenance);
    toolsMenu->Enable(MNU_INDEXCHECK, false);
	toolsMenu->Enable(MNU_STATUS, sql);
	toolsMenu->Enable(MNU_VIEWDATA, viewData);
	toolsMenu->Enable(MNU_VIEWFILTEREDDATA, viewData);
	viewMenu->Enable(MNU_REFRESH, refresh);
    toolsMenu->Enable(MNU_RELOAD, false);

	treeContextMenu->Enable(MNU_CREATE, create);
	treeContextMenu->Enable(MNU_DROP, drop);
	treeContextMenu->Enable(MNU_CONNECT, false);
	treeContextMenu->Enable(MNU_DISCONNECT, false);
	treeContextMenu->Enable(MNU_REFRESH, refresh);
	treeContextMenu->Enable(MNU_PROPERTIES, properties);
	treeContextMenu->Enable(MNU_MAINTENANCE, maintenance);
	treeContextMenu->Enable(MNU_INDEXCHECK, false);
	treeContextMenu->Enable(MNU_VIEWDATA, viewData);
	treeContextMenu->Enable(MNU_VIEWFILTEREDDATA, viewData);
    treeContextMenu->Enable(MNU_RELOAD, false);
}




void frmMain::StartMsg(const wxString& msg)
{
   if (msgLevel++)
        return;

    timermsg.Printf(wxT("%s..."), msg.c_str());
    wxBeginBusyCursor();
    stopwatch.Start(0);
    wxLogStatus(timermsg);
    statusBar->SetStatusText(timermsg, 1);
    statusBar->SetStatusText(wxT(""), 2);
}


void frmMain::EndMsg()
{
    msgLevel--;

    if (!msgLevel)
    {
        // Get the execution time & display it
        float timeval = stopwatch.Time();
        wxString time;
        time.Printf(_("%.2f secs"), (timeval/1000));
        statusBar->SetStatusText(time, 2);

        // Display the 'Done' message
        statusBar->SetStatusText(timermsg + _(" Done."), 1);
        wxLogStatus(wxT("%s (%s)"), timermsg.c_str(), time.c_str());
        wxEndBusyCursor();
    }
}
