/*
 * Copyright 2011, Alexandre Deckner (alex@zappotek.com)
 * Distributed under the terms of the MIT License.
 *
 */


#include "MainWindow.h"

#include <Application.h>
//#include <Catalog.h>
#include <FilePanel.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>

#include <stdio.h>

#include "MainView.h"


#undef B_TRANSLATE_CONTEXT
#define B_TRANSLATE_CONTEXT "MainWindow"


const uint32 kMsgOpenFilePanel = 'opfp';


MainWindow::MainWindow(BRect frame, const char* title)
	:
	BWindow(frame, title, B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0)
{
	BMenuBar* menuBar = new BMenuBar(Bounds(), "menu");
	AddChild(menuBar);

	BRect mainFrame(Bounds());
	mainFrame.top = menuBar->Frame().bottom;

	fMainView = new MainView(mainFrame);
	AddChild(fMainView);

	// Setup menus

	// "File" menu
	BMenu* menu = new BMenu("File");
	BMenuItem* item = NULL;
	menu->AddItem(item = new BMenuItem("Open" B_UTF8_ELLIPSIS,
		new BMessage(kMsgOpenFilePanel)));
	item->SetShortcut('O', B_COMMAND_KEY);
	menu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED), 'Q'));
	menu->SetTargetForItems(this);

	menuBar->AddItem(menu);

	//
	fOpenPanel = new BFilePanel(B_OPEN_PANEL);
	fOpenPanel->SetTarget(this);

	//

	Show();
}


MainWindow::~MainWindow()
{
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kMsgOpenFilePanel:
			fOpenPanel->Show();
			break;

		case B_REFS_RECEIVED:
		case B_SIMPLE_DATA:
			_MessageDropped(message);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
MainWindow::_MessageDropped(BMessage* message)
{
	status_t status = B_MESSAGE_NOT_UNDERSTOOD;
	bool hasRef = false;

	entry_ref ref;
	if (message->FindRef("refs", &ref) != B_OK) {
		const void* data;
		ssize_t size;
		if (message->FindData("text/plain", B_MIME_TYPE, &data,
				&size) == B_OK) {
			printf("text/plain %s\n", (const char*)data);
		} else
			return;
	} else {
		//status = fSudokuView->SetTo(ref);
		//if (status == B_OK)
		//	be_roster->AddToRecentDocuments(&ref, kSignature);

		printf("ref = %s\n", ref.name);
		fMainView->LoadWave(ref);

		hasRef = true;
	}

	/*if (status < B_OK) {
		char buffer[1024];
		if (hasRef) {
			snprintf(buffer, sizeof(buffer),
				B_TRANSLATE("Could not open \"%s\":\n%s\n"), ref.name,
				strerror(status));
		} else {
			snprintf(buffer, sizeof(buffer),
				B_TRANSLATE("Could not set Sudoku:\n%s\n"),
				strerror(status));
		}

		(new BAlert(B_TRANSLATE("Sudoku request"),
			buffer, B_TRANSLATE("OK"), NULL, NULL,
			B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
	}*/
}

