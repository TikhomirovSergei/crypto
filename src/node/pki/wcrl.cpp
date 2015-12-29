#include "../stdafx.h"

#include <node_buffer.h>
//#include <string_bytes.h>

#include "wcrl.h"
//#include "wcert.h"
#include "../helper.h"

void WCRL::Init(v8::Handle<v8::Object> exports){
	v8::Local<v8::String> className = Nan::New("CRL").ToLocalChecked();

	// Basic instance setup
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);

	tpl->SetClassName(className);
	tpl->InstanceTemplate()->SetInternalFieldCount(1); // req'd by ObjectWrap

	Nan::SetPrototypeMethod(tpl, "load", Load);
	Nan::SetPrototypeMethod(tpl, "import", Import);

	Nan::SetPrototypeMethod(tpl, "save", Save);
	Nan::SetPrototypeMethod(tpl, "export", Export);

	Nan::SetPrototypeMethod(tpl, "getVersion", GetVersion);
	Nan::SetPrototypeMethod(tpl, "getIssuerName", GetIssuerName);
	Nan::SetPrototypeMethod(tpl, "getLastUpdate", GetLastUpdate);
	Nan::SetPrototypeMethod(tpl, "getNextUpdate", GetNextUpdate);
	Nan::SetPrototypeMethod(tpl, "getCertificate", GetCertificate);

	// Store the constructor in the target bindings.
	constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
	
	exports->Set(Nan::New("CRL").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(WCRL::New){
	WCRL *obj = new WCRL();
	obj->data_ = new CRL();

	obj->Wrap(info.This());

	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(WCRL::GetVersion){
	try{

		UNWRAP_DATA(CRL);

		int version = _this->version();
		
		v8::Local<v8::Number> v8Version = Nan::New<v8::Number>(version);

		info.GetReturnValue().Set(v8Version);
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::GetIssuerName)
{
	try{

		UNWRAP_DATA(CRL);

		Handle<std::string> name = NULL;

		try{
			name = _this->issuerName();
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			return;
		}

		v8::Local<v8::String> v8Name = Nan::New<v8::String>(name->c_str()).ToLocalChecked();

		info.GetReturnValue().Set(v8Name);
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::GetLastUpdate)
{
	try{

		UNWRAP_DATA(CRL);

		Handle<std::string> time = NULL;

		try{
			time = _this->lastUpdate();
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			info.GetReturnValue().SetUndefined();
		}

		v8::Local<v8::String> v8Time = Nan::New<v8::String>(time->c_str()).ToLocalChecked();

		info.GetReturnValue().Set(v8Time);
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::GetNextUpdate)
{
	try{
		UNWRAP_DATA(CRL);

		Handle<std::string> time = NULL;

		try{
			time = _this->nextUpdate();
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			info.GetReturnValue().SetUndefined();
		}

		v8::Local<v8::String> v8Time = Nan::New<v8::String>(time->c_str()).ToLocalChecked();

		info.GetReturnValue().Set(v8Time);
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::GetCertificate)
{
	info.GetReturnValue().SetUndefined();
}

NAN_METHOD(WCRL::Load)
{
	try{
		if (info[0]->IsUndefined()){
			Nan::ThrowError("Parameter 1 is required");
			return;
		}

		v8::Local<v8::String> str = info[0].As<v8::String>();
		char *filename = copyBufferToUtf8String(str);
		if (filename == NULL) {
			Nan::ThrowError("Wrong filename");
			return;
		}

		std::string fname(filename);
		free(filename);

		UNWRAP_DATA(CRL);

		Handle<Bio> _in = NULL;

		try{
			_in = new Bio(BIO_TYPE_FILE, fname, "rb");
		}
		catch (Handle<Exception> e){
			Nan::ThrowError("File not found");
			return;
		}

		try{
			_this->read(_in, DataFormat::DER);
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			return;
		}

		info.GetReturnValue().Set(info.This());
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::Import)
{
	try{
		if (info[0]->IsUndefined()){
			Nan::ThrowError("Parameter 1 is required");
			info.GetReturnValue().SetUndefined();
		}

		//get data from buffer
		char* buf = node::Buffer::Data(info[0]);
		size_t buflen = node::Buffer::Length(info[0]);
		std::string buffer(buf, buflen);

		UNWRAP_DATA(CRL);

		Handle<Bio> in = NULL;

		try{
			Handle<Bio> in = new Bio(BIO_TYPE_MEM, buffer);

			_this->read(in, DataFormat::DER);
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			return;
		}

		info.GetReturnValue().Set(info.This());
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::Save)
{
	try{
		if (info[0]->IsUndefined()){
			Nan::ThrowError("Parameter 1 is required (filename)");
			info.GetReturnValue().SetUndefined();
		}

		v8::Local<v8::String> str = info[0].As<v8::String>();
		char *filename = copyBufferToUtf8String(str);
		if (filename == NULL) {
			Nan::ThrowError("Wrong filename");
			info.GetReturnValue().SetUndefined();
		}

		std::string fname(filename);
		free(filename);

		UNWRAP_DATA(CRL);

		try{
			Handle<Bio> out = new Bio(BIO_TYPE_FILE, fname, "wb");
			_this->write(out, DataFormat::DER);
		}
		catch (Handle<Exception> e){
			Nan::ThrowError(e->what());
			return;
		}

		info.GetReturnValue().Set(info.This());
		return;
	}
	TRY_END();
}

NAN_METHOD(WCRL::Export)
{
	try{
		UNWRAP_DATA(CRL);

		Handle<Bio> out = new Bio(BIO_TYPE_MEM, "");
		_this->write(out, DataFormat::DER);

		Handle<std::string> buf = out->read();

		info.GetReturnValue().Set(
			stringToBuffer(buf)
		);
		return;
	}
	TRY_END();
}