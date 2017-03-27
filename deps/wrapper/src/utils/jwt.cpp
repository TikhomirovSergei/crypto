#include "../stdafx.h"

#include "wrapper/utils/jwt.h"

bool Jwt::checkLicense() {
	LOGGER_FN();

	try{
		int errorCode;

#ifndef JWT_NO_LICENSING
		if (!ctlicense_verify_file(&errorCode)) {
			THROW_EXCEPTION(0, Jwt, NULL, "verify jwt license failed(error code %d)", errorCode);
		}

		return true;
#else
		THROW_EXCEPTION(0, Jwt, NULL, "Only if undefined JWT_NO_LICENSING");
#endif
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Jwt, e, "Error check license");
	}
}

bool Jwt::checkLicense(Handle<std::string> lic) {
	LOGGER_FN();

	try{
		int errorCode;

#ifndef JWT_NO_LICENSING
		if (!ctlicense_verify_str((char *)lic->c_str(), &errorCode)) {
			THROW_EXCEPTION(0, Jwt, NULL, "verify jwt license failed(error code %d)", errorCode);
		}

		return true;
#else
		THROW_EXCEPTION(0, Jwt, NULL, "Only if undefined JWT_NO_LICENSING");
#endif
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, Jwt, e, "Error check license");
	}
}
