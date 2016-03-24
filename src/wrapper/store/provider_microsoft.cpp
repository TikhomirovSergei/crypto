#include "../stdafx.h"

#include "provider_microsoft.h"

ProviderMicrosoft::ProviderMicrosoft(){
	LOGGER_FN();

	try{
		type = new std::string("MICROSOFT");
		providerItemCollection = new PkiItemCollection();

		init();
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Cannot be constructed ProviderMicrosoft");
	}
}
void ProviderMicrosoft::init(){
	LOGGER_FN();

	try{
		std::string listStore[] = {
			"MY",
			"AddressBook",
			"ROOT",
			"TRUST",
			"CA",
			"Request"
		};

		HCERTSTORE hCertStore;

		for (int i = 0, c = sizeof(listStore) / sizeof(*listStore); i < c; i++){
			std::wstring widestr = std::wstring(listStore[i].begin(), listStore[i].end());
			hCertStore = CertOpenStore(
				CERT_STORE_PROV_SYSTEM,
				PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
				NULL,
				CERT_SYSTEM_STORE_CURRENT_USER,
				widestr.c_str()
				);

			if (!hCertStore) {
				THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Error open store");
			}

			enumCertificates(hCertStore, &listStore[i]);
			enumCrls(hCertStore, &listStore[i]);

			if (hCertStore) {
				CertCloseStore(hCertStore, 0);
			}
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error init microsoft provider");
	}	
}

void ProviderMicrosoft::enumCertificates(HCERTSTORE hCertStore, std::string *category){
	LOGGER_FN();

	try{
		X509 *cert = NULL;
		const unsigned char *p;

		if (!hCertStore){
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Certificate store not opened");
		}

		PCCERT_CONTEXT pCertContext = NULL;
		do
		{
			pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext);
			if (pCertContext){
				p = pCertContext->pbCertEncoded;
				LOGGER_OPENSSL(d2i_X509);
				if (!(cert = d2i_X509(NULL, &p, pCertContext->cbCertEncoded))) {
					THROW_OPENSSL_EXCEPTION(0, ProviderMicrosoft, NULL, "'d2i_X509' Error decode len bytes");
				}

				Handle<PkiItem> item = objectToPKIItem(new Certificate(cert));
				item->category = new std::string(*category);

				DWORD * pdwKeySpec;
				BOOL * pfCallerFreeProv;
				HCRYPTPROV m_hProv;

				if (CryptAcquireCertificatePrivateKey(pCertContext, NULL, NULL, &m_hProv, pdwKeySpec, pfCallerFreeProv)){
					item->certKey = new std::string("1");
				}

				providerItemCollection->push(item);
			}			
		} while (pCertContext != NULL);

		if (pCertContext){
			CertFreeCertificateContext(pCertContext);
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error enum certificates in store");
	}
}

void ProviderMicrosoft::enumCrls(HCERTSTORE hCertStore, std::string *category){
	LOGGER_FN();

	try{
		X509_CRL *crl = NULL;
		const unsigned char *p;

		if (!hCertStore){
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Certificate store not opened");
		}

		PCCRL_CONTEXT pCrlContext = NULL;
		do
		{
			pCrlContext = CertEnumCRLsInStore(hCertStore, pCrlContext);
			if (pCrlContext){
				p = pCrlContext->pbCrlEncoded;
				LOGGER_OPENSSL(d2i_X509_CRL);
				if (!(crl = d2i_X509_CRL(NULL, &p, pCrlContext->cbCrlEncoded))) {
					THROW_OPENSSL_EXCEPTION(0, ProviderMicrosoft, NULL, "'d2i_X509_CRL' Error decode len bytes");
				}			

				Handle<PkiItem> item = objectToPKIItem(new CRL(crl));
				item->category = new std::string(*category);
				providerItemCollection->push(item);
			}
		} while (pCrlContext != NULL);

		if (pCrlContext){
			CertFreeCRLContext(pCrlContext);
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error enum CRLs in store");
	}
}

Handle<PkiItem> ProviderMicrosoft::objectToPKIItem(Handle<Certificate> cert){
	LOGGER_FN();

	try{
		if (cert.isEmpty()){
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Certificate empty");
		}

		Handle<PkiItem> item = new PkiItem();

		item->format = new std::string("DER");
		item->type = new std::string("CERTIFICATE");
		item->provider = new std::string("MICROSOFT");

		char * hexHash;
		Handle<std::string> hhash = cert->getThumbprint();
		PkiStore::bin_to_strhex((unsigned char *)hhash->c_str(), hhash->length(), &hexHash);
		item->hash = new std::string(hexHash);

		item->certSubjectName = cert->getSubjectName();
		item->certSubjectFriendlyName = cert->getSubjectFriendlyName();
		item->certIssuerName = cert->getIssuerName();
		item->certIssuerFriendlyName = cert->getIssuerFriendlyName();
		item->certSerial = cert->getSerialNumber();

		item->certNotBefore = cert->getNotBefore();
		item->certNotAfter = cert->getNotAfter();

		return item;		
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error create PkiItem from certificate");
	}
}

Handle<PkiItem> ProviderMicrosoft::objectToPKIItem(Handle<CRL> crl){
	LOGGER_FN();

	try{
		if (crl.isEmpty()){
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "CRL empty");
		}

		Handle<PkiItem> item = new PkiItem();

		item->format = new std::string("DER");
		item->type = new std::string("CRL");
		item->provider = new std::string("MICROSOFT");

		char * hexHash;
		Handle<std::string> hhash = crl->getThumbprint();
		PkiStore::bin_to_strhex((unsigned char *)hhash->c_str(), hhash->length(), &hexHash);
		item->hash = new std::string(hexHash);

		item->crlIssuerName = crl->issuerName();
		item->crlIssuerFriendlyName = crl->issuerFriendlyName();
		item->crlLastUpdate = crl->getThisUpdate();
		item->crlNextUpdate = crl->getNextUpdate();

		return item;
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error create PkiItem from crl");
	}
}

Handle<Certificate> ProviderMicrosoft::getCert(Handle<std::string> hash, Handle<std::string> category){
	LOGGER_FN();

	X509 *hcert = NULL;

	try{
		HCERTSTORE hCertStore;
		PCCERT_CONTEXT pCertContext = NULL;
		
		const unsigned char *p;

		std::wstring wCategory = std::wstring(category->begin(), category->end());

		char cHash[20] = { 0 };
		hex2bin(hash->c_str(), cHash);

		CRYPT_HASH_BLOB cblobHash;
		cblobHash.pbData = (BYTE *)cHash;
		cblobHash.cbData = (DWORD)20;

		hCertStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
			NULL,
			CERT_SYSTEM_STORE_CURRENT_USER,
			wCategory.c_str()
			);

		if (!hCertStore) {
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Error open store");
		}

		pCertContext = CertFindCertificateInStore(
			hCertStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			0,
			CERT_FIND_HASH,
			&cblobHash,
			NULL);

		if (pCertContext) {
			p = pCertContext->pbCertEncoded;
			LOGGER_OPENSSL(d2i_X509);

			if (!(hcert = d2i_X509(NULL, &p, pCertContext->cbCertEncoded))) {
				THROW_OPENSSL_EXCEPTION(0, ProviderMicrosoft, NULL, "'d2i_X509' Error decode len bytes");
			}

			if (pCertContext){
				CertFreeCertificateContext(pCertContext);
			}
				
			if (hCertStore) {
				CertCloseStore(hCertStore, 0);
			}

			return new Certificate(hcert);
		}
		else{
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Cannot find certificate in store");
		}
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error get certificate");
	}
}

Handle<CRL> ProviderMicrosoft::getCRL(Handle<std::string> hash, Handle<std::string> category){
	LOGGER_FN();

	try{
		HCERTSTORE hCertStore;
		PCCERT_CONTEXT pCertContext = NULL;
		PCCRL_CONTEXT pCrlContext = NULL;
		CERT_INFO certInfo = { 0 };

		const unsigned char *p;

		std::wstring wCategory = std::wstring(category->begin(), category->end());

		hCertStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
			NULL,
			CERT_SYSTEM_STORE_CURRENT_USER,
			wCategory.c_str()
			);

		if (!hCertStore) {
			THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Error open store");
		}

		do
		{
			pCrlContext = CertEnumCRLsInStore(hCertStore, pCrlContext);
			if (pCrlContext){
				X509_CRL *tempCrl = NULL;
				p = pCrlContext->pbCrlEncoded;
				LOGGER_OPENSSL(d2i_X509_CRL);
				if (!(tempCrl = d2i_X509_CRL(NULL, &p, pCrlContext->cbCrlEncoded))) {
					THROW_OPENSSL_EXCEPTION(0, ProviderMicrosoft, NULL, "'d2i_X509_CRL' Error decode len bytes");
				}

				Handle<CRL> hTempCrl = new CRL(tempCrl);

				char * hexHash;
				Handle<std::string> hhash = hTempCrl->getThumbprint();
				PkiStore::bin_to_strhex((unsigned char *)hhash->c_str(), hhash->length(), &hexHash);
				std::string sh(hexHash);

				if (strcmp(sh.c_str(), hash->c_str()) == 0){
					return hTempCrl;
				}
			}
		} while (pCrlContext != NULL);

		if (pCrlContext){
			CertFreeCRLContext(pCrlContext);
		}

		if (hCertStore) {
			CertCloseStore(hCertStore, 0);
		}

		THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Cannot find CRL in store");
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error get CRL");
	}
}

int ProviderMicrosoft::char2int(char input) {
	LOGGER_FN();

	try{
		if (input >= '0' && input <= '9'){
			return input - '0';
		}
			
		if (input >= 'A' && input <= 'F'){
			return input - 'A' + 10;
		}
			
		if (input >= 'a' && input <= 'f'){
			return input - 'a' + 10;
		}
		
		THROW_EXCEPTION(0, ProviderMicrosoft, NULL, "Invalid input string");
	}
	catch (Handle<Exception> e){
		THROW_EXCEPTION(0, ProviderMicrosoft, e, "Error char to int");
	}
	
}

void ProviderMicrosoft::hex2bin(const char* src, char* target) {
	LOGGER_FN();

	while (*src && src[1]){
		*(target++) = char2int(*src) * 16 + char2int(src[1]);
		src += 2;
	}
}