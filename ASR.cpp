// ASR.cpp - Main application for testing speech recognition.
// Date: December 2011
// Author: Scott Halsall

#include"Speech.h"

bool done = false;

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{	
	std::wcout << "Exiting . . ." << std::endl;
	done = true;

	return TRUE;
}

CSpeech speak;

int wmain(int argc, wchar_t **argv)
{	
	//Standard stuff below.
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// Break if we can't begin the speech recogniser (any aspect of) or load the grammar rules
	if (!speak.Initialize() || !speak.SetGrammar() )
	{
		//delete ( speak );
		return 0;
	}

	LPWSTR flee = L"Automatic Shutdown Announced";
	while ( !done )
	{
		// If the recogniser returns flee, quit.
		if( wcscmp( flee, speak.Update() ) == 0 )
		{
			done = true;
		}
	}
  
	//
	//delete speak;
	

	cout << "Pausing..." << endl;
	system( "pause" );

	return 0;
}
