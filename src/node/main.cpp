#include "stdafx.h"

#include <openssl/cms.h>
#include <openssl/engine.h>
#include <openssl/err.h>

#include <nan.h>

#include "utils/wlog.h"

#include "pki/wkey.h"
#include "pki/wcert.h"
#include "pki/wcerts.h"
#include "pki/wcrl.h"
#include "pki/woid.h"
#include "pki/walg.h"
#include "pki/wattr.h"
#include "pki/wcertRegInfo.h"
#include "pki/wcertReg.h"
#include "pki/wcsr.h"
#include "pki/wcipher.h"
#include "pki/wchain.h"
#include "store/wpkistore.h"
#include "store/wsystem.h"
#include "store/wcashjson.h"

#include "cms/wsigned_data.h"
#include "cms/wsigner.h"
#include "cms/wsigners.h"
#include "cms/wsigner_attrs.h"

#include <node_object_wrap.h>

void init(v8::Handle<v8::Object> target) {
	// logger.start("/tmp/trustedtls/node.log", -1); // -1 = all levels bits
	 logger.start("logger.txt", LoggerLevel::All );

	// On Windows, we can't use Node's OpenSSL, so we link
	// to a standalone OpenSSL library. Therefore, we need
	// to initialize OpenSSL separately.

	// TODO: Do I need to free these?
	// I'm not sure where to call ERR_free_strings() and EVP_cleanup()

	// LOGGER_TRACE("OpenSSL init");

	OpenSSL::run();

	v8::Local<v8::Object> Pki = Nan::New<v8::Object>();

	target->Set(Nan::New("PKI").ToLocalChecked(), Pki);
	WCertificate::Init(Pki);
	WCertificateCollection::Init(Pki);
	WCRL::Init(Pki);
	WOID::Init(Pki);
	WAlgorithm::Init(Pki);
	WAttribute::Init(Pki);
	WKey::Init(Pki);
	WCSR::Init(Pki);
	WCertificationRequestInfo::Init(Pki);
	WCertificationRequest::Init(Pki);
	WCipher::Init(Pki);
	WChain::Init(Pki);
	WPkiStore::Init(Pki);
	WProvider_System::Init(Pki);
	WFilter::Init(Pki);
	WPkiItem::Init(Pki);
	WCashJson::Init(Pki);

	v8::Local<v8::Object> Cms = Nan::New<v8::Object>();
	target->Set(Nan::New("CMS").ToLocalChecked(), Cms);
	WSignedData::Init(Cms);
	WSigner::Init(Cms);
	WSignerCollection::Init(Cms);
	WSignerAttributeCollection::Init(Cms);

	// target->Set(NanNew<v8::String>("utils"), NanNew<v8::Object>());
	// WLogger::Init(target->Get(NanNew<v8::String>("utils"))->ToObject());


	// logger.start("log-node.txt", LoggerLevel::Debug);
}

NODE_MODULE(trusted, init)