#include <string>
#include <node.h>
#include <v8.h>

using namespace v8;

std::string Utf8ToGbk(char* ubuf) {
	// 1. UTF8 -> UNICODE
	int wsize = MultiByteToWideChar(CP_UTF8, 0, ubuf, -1, NULL, 0);
	wchar_t* wbuf = new wchar_t[wsize];
	MultiByteToWideChar(CP_UTF8, 0, ubuf, -1, wbuf, wsize);
	// 2. UNICODE -> GBK
	int gsize = WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, FALSE);
	char* gbuf = new char[gsize];
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, gbuf, gsize, NULL, FALSE);

	std::string gbk = gbuf;
	delete []gbuf;
	delete []wbuf;
	return gbk;
}

typedef void (__stdcall *GdxSetup)(int a, int b, int c, int d, int e, int f); GdxSetup gdx_setup = NULL;
typedef void (__stdcall *GdxOpenPort)(LPSTR n); GdxOpenPort gdx_openport = NULL;
typedef void (__stdcall *GdxOpenUSB)(LPSTR n); GdxOpenUSB gdx_openusb = NULL;
typedef void (__stdcall *GdxClosePort)(); GdxClosePort gdx_closeport = NULL;
typedef void (__stdcall *GdxFindFirstUSB)(char* usbid); GdxFindFirstUSB gdx_findfirstusb = NULL;
typedef void (__stdcall *GdxSendCommand)(char* cmd); GdxSendCommand gdx_sendcommand = NULL;
typedef void (__stdcall *GdxEcTextOut)(int x, int y, int b, const char* c, const char* d); GdxEcTextOut gdx_ectextout = NULL;

bool LoadGoDexDll()
{
	HINSTANCE dll = LoadLibrary("Trace.dll");
	if (!dll) {
		printf("Load Trace.dll failed!");
		return false;
	}
	gdx_setup = (GdxSetup)GetProcAddress(dll, "setup");
	if (!gdx_setup) {
		printf("Can't load [Trace.dll-setup].");
		return false;
	}
	gdx_openport = (GdxOpenPort)GetProcAddress(dll, "openport");
	if (!gdx_openport) {
		printf("Can't load [Trace.dll-openport].");
		return false;
	}
	gdx_closeport = (GdxClosePort)GetProcAddress(dll, "closeport");
	if (!gdx_closeport) {
		printf("Can't load [Trace.dll-closeport].");
		return false;
	}
	gdx_openusb = (GdxOpenUSB)GetProcAddress(dll, "OpenUSB");
	if (!gdx_openusb) {
		printf("Can't load [Trace.dll-OpenUSB].");
		return false;
	}
	gdx_findfirstusb = (GdxFindFirstUSB)GetProcAddress(dll, "FindFirstUSB");
	if (!gdx_findfirstusb) {
		printf("Can't load [Trace.dll-FindFirstUSB].");
		return false;
	}
	gdx_sendcommand = (GdxSendCommand)GetProcAddress(dll, "sendcommand");
	if (!gdx_sendcommand) {
		printf("Can't load [Trace.dll-sendcommand].");
		return false;
	}
	gdx_ectextout = (GdxEcTextOut)GetProcAddress(dll, "ecTextOut");
	if (!gdx_ectextout) {
		printf("Can't load [Trace.dll-ecTextOut].");
		return false;
	}

	return true;
}

Handle<Value> Setup(const Arguments& args) {
  HandleScope scope;
  if (args.Length() == 6) {
  	int params[6];
  	for (int i = 0; i < 6; i++) {
  		if (!args[i]->IsInt32())
  			return scope.Close(Null());
  		Local<Integer> value = Local<Integer>::Cast(args[i]);
  		params[i] = (int)(value->Int32Value());
  	}
  	gdx_setup(params[0], params[1], params[2], params[3], params[4], params[5]);
  }
  return scope.Close(Null());
}

Handle<Value> OpenPort(const Arguments& args) {
  HandleScope scope;
  gdx_openport("6");
  return scope.Close(Null());
}

Handle<Value> SendCommand(const Arguments& args) {
  HandleScope scope;
  if (args.Length() > 0 && args[0]->IsString()) {
      String::AsciiValue string(args[0]);
      gdx_sendcommand(*string);
  }
  return scope.Close(Null());
}

Handle<Value> ClosePort(const Arguments& args) {
  HandleScope scope;
  gdx_closeport();
  return scope.Close(Null());
}

Handle<Value> EcTextOut(const Arguments& args) {
  HandleScope scope;
  if (args.Length() == 5) {
  	int params[3];
  	for (int i = 0; i < 3; i++) {
  		if (!args[i]->IsInt32())
  			return scope.Close(Null());
  		Local<Integer> value = Local<Integer>::Cast(args[i]);
  		params[i] = (int)(value->Int32Value());
  	}
  	if (args[3]->IsString() && args[4]->IsString()) {
  		String::Utf8Value string3(args[3]);
  		String::Utf8Value string4(args[4]);
  		std::string gbkstr3 = Utf8ToGbk(*string3);
  		std::string gbkstr4 = Utf8ToGbk(*string4);
  		gdx_ectextout(params[0], params[1], params[2], gbkstr3.c_str(), gbkstr4.c_str());
  	}
  }
  return scope.Close(Null());
}

void Init(Handle<Object> exports) {
	if (!LoadGoDexDll())
		return;

	exports->Set(String::NewSymbol("setup"),
		FunctionTemplate::New(Setup)->GetFunction());	
	exports->Set(String::NewSymbol("openport"),
		FunctionTemplate::New(OpenPort)->GetFunction());
    exports->Set(String::NewSymbol("sendcommand"),
        FunctionTemplate::New(SendCommand)->GetFunction());
    exports->Set(String::NewSymbol("closeport"),
        FunctionTemplate::New(ClosePort)->GetFunction());
    exports->Set(String::NewSymbol("ectextout"),
    	FunctionTemplate::New(EcTextOut)->GetFunction());
}

NODE_MODULE(godex, Init)