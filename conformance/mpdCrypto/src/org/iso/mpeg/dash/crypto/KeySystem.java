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

import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 * 
 * This class is responsible for retrieving encryption/decryption keys for particular segments or segment ranges.
 *
 * This class cannot be instantiated directly, use getInstance() to create an instance of one of its descendants.
 */
public abstract class KeySystem implements Const {
	protected String keySystemUri = null;
	protected String keyLicenseUrlTemplate = null;
	
	protected ContentProtection parent;

	protected KeySystem(Element keySystemElement, String keySystemUri, ContentProtection parent) {
		this.keySystemUri = keySystemUri;
		this.parent = parent;
		if (keySystemElement.hasAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE))
			keyLicenseUrlTemplate = keySystemElement.getAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE);
	}
	
	protected KeySystem(String keySystemUri, ContentProtection parent) {
		this.keySystemUri = keySystemUri;
		this.parent = parent;
	}

	static KeySystem getInstance(Element keySystemElement, ContentProtection parent) throws DashCryptoException {
		String keySystemUri = keySystemElement.getAttribute(XML_ATTR_NAME_KEY_SYSTEM_URI);
		if (keySystemUri.equals(KEY_SYSTEM_URI_BASELINE_HTTP))
			return new KeySystemBaselineHttp(keySystemElement, keySystemUri, parent);
		else
			throw new DashCryptoExceptionUnsupported("Error: key system \"" + keySystemUri + "\" not yet supported");
	}

	/**
	 * @param keyUri (already resolved from a template)
	 * @param segmentNum
	 * @return
	 * @throws DashCryptoException
	 */
	abstract byte[] retrieveKey(String keyUri) throws DashCryptoException;

	/**
	 * @return the keySystemUri
	 */
	public String getKeySystemUri() {
		return keySystemUri;
	}

	/**
	 * @return the keyLicenseUrlTemplate
	 */
	public String getKeyLicenseUrlTemplate() {
		return keyLicenseUrlTemplate;
	}
	/**
	 * @return the key license URL, resolved in context of the segment
	 * @throws DashCryptoException 
	 */
	public String getKeyLicenseUrl(int segmentNum) throws DashCryptoException {
		return parent.resolveTemplate(keyLicenseUrlTemplate, segmentNum);
	}
}
