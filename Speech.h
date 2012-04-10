// Speech.h - Class for creating a speech recognition engine indepenant of Windows Help Commands
// using SAPI 5.1 and default audio input device.
// Date: December 2011
// Author: Scott Halsall

#include <windows.h>
#include <sphelper.h>

#pragma warning(disable:4995) // Fixing problems with shelper.h
#include <iostream>
using namespace std;

class CSpeech
{
private:
	LPWSTR pwszText;
	HANDLE hEvent;
	HRESULT hr;
	CSpEvent evt;
	SPPHRASE *pParts;
	ISpPhrase *pPhrase;
	ULONGLONG ullEvents;
	CComPtr<ISpRecognizer> cpEngine;
	CComPtr<ISpRecoContext> cpRecoCtx; // Recogniser Context
	CComPtr<ISpRecoGrammar> cpGram; // Grammar Rules

	// Audio input setup. This is for creating our engine and getting rid of the lovely "help" Microsoft gives us.
	CComPtr<ISpObjectToken> cpAudioToken; // audio device
    CComPtr<ISpAudio> cpAudio; // Where the device connects to the engine.
public:
	CSpeech();
	~CSpeech();
	bool Initialize();
	bool SetGrammar();
	LPWSTR Update();

};

CSpeech::CSpeech()
{
	// Nothing to sort in constructor
}

CSpeech::~CSpeech()
{
	// Release and uninialize
	cpGram.Release();
	cpRecoCtx.Release();
	cpEngine.Release();

	CoUninitialize();  
}

bool CSpeech::Initialize()
{
	CoInitialize(0);

	// Create speech recognition engine.
	hr = cpEngine.CoCreateInstance(CLSID_SpInprocRecognizer);
	if( FAILED(hr) )
	{
		return false;
	}

	// Try the default audio input device
	hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpAudioToken);
	if( FAILED(hr) )
	{
		return false;
	}

	// Connect the Device
	hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &cpAudio);
	if( FAILED(hr) )
	{
		return false;
	}

	hr = cpEngine->SetInput(cpAudio, TRUE);	
	if( FAILED(hr) )
	{
		return false;
	}

	// Creae recognition context to recieve events.
	hr = cpEngine->CreateRecoContext(&cpRecoCtx);
	if( FAILED(hr) )
	{
		return false;
	}

	// Use Win32 events.
	hr = cpRecoCtx->SetNotifyWin32Event();
	if( FAILED(hr) )
	{
		return false;
	}

	hEvent = cpRecoCtx->GetNotifyEventHandle();

	// Set events we are interested in (speech recognised in grammar // Not recognised in grammar).
	ullEvents = SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_FALSE_RECOGNITION);

	// Set events to queue, interested in everything whether it is right or wrong, will queue them.
	hr = cpRecoCtx->SetInterest(ullEvents, ullEvents);
	if( FAILED(hr) )
	{
		return false;
	}

	return true;
}

bool CSpeech::SetGrammar()
{
	// Create and finally load the grammar.
	hr = cpRecoCtx->CreateGrammar(1, &cpGram);
	if( FAILED(hr) )
	{
		return false;
	}

	hr = cpGram->LoadCmdFromFile( L"plancommands.cfg", SPLO_DYNAMIC);
	if( FAILED(hr) )
	{
		return false;
	}

	// Activate the grammar rules
	hr = cpGram->SetRuleState(0, 0, SPRS_ACTIVE);
	if( FAILED(hr) )
	{
		return false;
	}

	return true;
}

LPWSTR CSpeech::Update()
{
	// Set to 0 so it wont interrupt other code.
	WaitForSingleObject(hEvent, 0);

	if ( evt.GetFrom(cpRecoCtx) == S_OK )
	{
		if ( evt.eEventId == SPEI_FALSE_RECOGNITION )
		{
			cout << "No recognition" << endl;
			return L"No recognition";
		}
		else
		{
			// Free this up before we go in.
			// Since it's the return can't do it after we've got the data we want.
			CoTaskMemFree( pwszText );

			pPhrase = evt.RecoResult();
			hr = pPhrase->GetPhrase(&pParts);
			hr = pPhrase->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &pwszText, 0);

			// Need to use wcout as we're using a LPWSTR.
			wcout << pwszText << " (" << pParts->Rule.Confidence << ") " << endl;
			
			// Release what we no longer need.
			CoTaskMemFree(pParts);
			//CoTaskMemFree(pwszText);
			return pwszText;
		}
	}

	return L"Nothing";
}
