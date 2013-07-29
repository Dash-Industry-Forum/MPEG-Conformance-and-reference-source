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

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;

import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 * This class is responsible for actual ciphering of segments.
 */
public abstract class SegmentEncryption implements Const {
	protected String encryptionSystemUri = null;
	protected Integer keyLength = null;
	protected Double earlyAvailability = null;
	protected Boolean ivEncryptionFlag = null;

	protected SegmentEncryption(Element segmentEncryptionElement, String encryptionSystemUri) {
		this.encryptionSystemUri = encryptionSystemUri;
		if (segmentEncryptionElement.hasAttribute(XML_ATTR_NAME_KEY_LENGTH))
			keyLength = new Integer(segmentEncryptionElement.getAttribute(XML_ATTR_NAME_KEY_LENGTH));
		if (segmentEncryptionElement.hasAttribute(XML_ATTR_NAME_EARLY_AVAILABILITY))
			earlyAvailability = new Double(segmentEncryptionElement.getAttribute(XML_ATTR_NAME_EARLY_AVAILABILITY));
		if (segmentEncryptionElement.hasAttribute(XML_ATTR_NAME_IV_ENCRYPTION_FLAG))
			ivEncryptionFlag = new Boolean(segmentEncryptionElement.getAttribute(XML_ATTR_NAME_IV_ENCRYPTION_FLAG));
	}

	static SegmentEncryption getInstance(Element segmentEncryptionElement) throws DashCryptoException {
		String encryptionSystemUri = segmentEncryptionElement.getAttribute(XML_ATTR_NAME_ENCRYPTION_SYSTEM_URI);
		if (encryptionSystemUri.equals(ENCRYPTION_SYSTEM_URI_AES128_CBC))
			return new SegmentEncryptionAES128CBC(segmentEncryptionElement, encryptionSystemUri);
		else
			throw new DashCryptoExceptionUnsupported("Error: segment encryption system \"" + encryptionSystemUri + 
					"\" not yet supported");
	}
	
	protected SegmentEncryption(String encryptionSystemUri) {
		this.encryptionSystemUri = encryptionSystemUri;
	}

	/**
	 * Does the actual ciphering
	 * 
	 * @param inputSegment (will be closed after reading)
	 * @param outputSegment
	 * @param key
	 * @param iv
	 * @param encrypt Whether to encrypt or decrypt
	 */
	public abstract void Crypt(InputStream inputSegment, File outputSegment, byte[] key, byte[] iv, boolean encrypt)
		throws DashCryptoException, GeneralSecurityException, IOException;

	/**
	 * Calculates the IV, automatically derived from segment number.
	 * 
	 * @param key
	 * @param segmentNum
	 * @return
	 * @throws DashCryptoException
	 */
	public abstract byte[] deriveIvFromSegmentNumber(byte[] key, long segmentNum) throws DashCryptoException;
	
	/**
	 * @return the encryptionSystemUri
	 */
	public String getEncryptionSystemUri() {
		return encryptionSystemUri;
	}

	/**
	 * @return the key length
	 */
	public Integer getKeyLengthInteger() {
		return keyLength;
	}

	/**
	 * @return the early availability, seconds
	 */
	public Double getEarlyAvailabilityDouble() {
		return earlyAvailability;
	}

	/**
	 * @return the IV encryption flag
	 */
	public Boolean isIvEncryptionFlagBoolean() {
		return ivEncryptionFlag;
	}
}
