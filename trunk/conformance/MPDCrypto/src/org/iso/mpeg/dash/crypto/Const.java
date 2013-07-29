/**
	Copyright (c) 2013- Vlad Zbarsky <zbarsky@cornell.edu>
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	* Neither the name of the <organization> nor the
	names of its contributors may be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR COmpeg transport streamNSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	@author	Vlad Zbarsky <zbarsky@cornell.edu>
	@since	Jan 13, 2013
*/
package org.iso.mpeg.dash.crypto;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 */
public interface Const {
//	public static final String DASH_SCHEMA_PATH						= "schemas\\sea.xsd";
	public static final String DASH_SCHEMA_PATH						= "schema\\DASH-MPD.xsd";	
	
		   static final String URI_PREFIX							= "urn:mpeg:dash:sea";
	public static final String CONTENT_PROTECTION_SCHEME_ID_URI		= URI_PREFIX + ":enc:2012";
	public static final String ENCRYPTION_SYSTEM_URI_AES128_CBC		= URI_PREFIX + ":aes128-cbc:2013";
	public static final String KEY_SYSTEM_URI_BASELINE_HTTP			= URI_PREFIX + ":keys:baseline:http:2013";
	public static final String CONTENT_AUTH_PROP_SCHEME_ID_URI		= URI_PREFIX + ":auth:2012";
	public static final String CONTENT_AUTH_SCHEME_ID_URI_SHA256	= URI_PREFIX + ":sha256";
	public static final String CONTENT_AUTH_SCHEME_ID_URI_HMAC_SHA1	= URI_PREFIX + ":hmac-sha1";

	public static final String XML_ELEMENT_NAME_CONTENT_PROTECTION	= "ContentProtection";
	public static final String XML_ELEMENT_NAME_SEGMENT_ENCRYPTION	= "SegmentEncryption";
	public static final String XML_ELEMENT_NAME_KEY_SYSTEM			= "KeySystem";
	public static final String XML_ELEMENT_NAME_CRYPTO_PERIOD		= "CryptoPeriod";
	public static final String XML_ELEMENT_NAME_CRYPTO_TIMELINE		= "CryptoTimeline";
	public static final String XML_ELEMENT_NAME_CONTENT_AUTH		= "ContentAuthenticity";
	
	// SegmentEncryption attributes
	public static final String XML_ATTR_NAME_ENCRYPTION_SYSTEM_URI	= "encryptionSystemUri";
	public static final String XML_ATTR_NAME_KEY_LENGTH				= "keyLength";
	public static final String XML_ATTR_NAME_EARLY_AVAILABILITY		= "earlyAvailability";
	public static final String XML_ATTR_NAME_IV_ENCRYPTION_FLAG		= "ivEncryptionFlag";

	// KeySystem attributes
	public static final String XML_ATTR_NAME_KEY_SYSTEM_URI			= "keySystemUri";
	public static final String XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE	= "keyLicenseUrlTemplate";

	// ContentAuthenticity attributes
	public static final String XML_ATTR_NAME_AUTH_SCHEME_ID_URI		= "authSchemeIdUri";
	public static final String XML_ATTR_NAME_AUTH_URL_TEMPLATE		= "authUrlTemplate";
	public static final String XML_ATTR_NAME_AUTH_TAG_LEN			= "authTagLength";
	
	// CryptoPeriod and CryptoTimeline common attributes
	public static final String XML_ATTR_NAME_NUM_SEGMENTS			= "numSegments";
	public static final String XML_ATTR_NAME_KEY_URI_TEMPLATE		= "keyUriTemplate";
	public static final String XML_ATTR_NAME_IV						= "IV";
	public static final String XML_ATTR_NAME_IV_URI_TEMPLATE		= "ivUriTemplate";

	// CryptoPeriod-specific attributes
	public static final String XML_ATTR_NAME_OFFSET					= "offset";

	// CryptoTimeline-specific attributes
	public static final String XML_ATTR_NAME_NUM_CRYPTO_PERIODS		= "numCryptoPeriods";
	public static final String XML_ATTR_NAME_FIRST_OFFSET			= "firstOffset";
	
	public static enum CryptoPeriodTypes { CRYPTO_PERIOD, CRYPTO_TIMELINE };
	
	public static final String SEGMENT_ENCRYPTION_BASE_CIPHER_ALGO	= "AES";
	public static final String SEGMENT_ENCRYPTION_BASE_CIPHER_DESC	= SEGMENT_ENCRYPTION_BASE_CIPHER_ALGO + "/CBC/NoPadding";
	public static final String SEGMENT_ENCRYPTION_IV_GEN_CIPHER_MODE= SEGMENT_ENCRYPTION_BASE_CIPHER_ALGO + "/ECB/NoPadding";

	public static final int AES_BLOCK_LEN							= 16;
	
	// variables used in URI templates
	public static final String TEMPLATE_VAR_REPRESENTATION_ID		= "$RepresentationID$";
	public static final String TEMPLATE_VAR_NUMBER					= "$Number$";
	public static final String TEMPLATE_VAR_BANDWIDTH				= "$Bandwidth$";
	public static final String TEMPLATE_VAR_TIME					= "$Time$";
	
	public static final String TEMPLATE_VAR_ESCAPE_DOLLAR			= "$$";
	
	public static final long DEFAULT_SEGMENT_START_NUMBER			= 1;
}
