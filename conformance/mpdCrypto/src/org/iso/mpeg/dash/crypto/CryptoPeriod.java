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

import javax.xml.bind.DatatypeConverter;

import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 * CryptoTimeline extenda and overrides this
 */
public class CryptoPeriod implements Const {
	protected CryptoPeriodTypes type;
	protected ContentProtection parent;
	
	// fields common both to CryptoPerod and CryptoTimeline
	protected Integer numSegments = null;
	protected String keyUriTemplate = null;
	protected String keyLicenseUrlTemplate = null;
	protected byte[] IV = null;
	protected String ivUriTemplate = null;

	// fields specific to CryptoPeriod
	private Integer offset = null;
	
	// fields for indexing the segments, which belong to this CryptoPeriod or CryptoTimeline
	protected Long firstSegNum = null;

	protected CryptoPeriod(Element cryptoPeriodXml, ContentProtection parent, CryptoPeriodTypes type) throws DashCryptoException {
		this.type = type;
		this.parent = parent;
		
		// parse attrs, common both to CryptoPeriod and CryptoTimeline
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_NUM_SEGMENTS))
			numSegments = new Integer(cryptoPeriodXml.getAttribute(XML_ATTR_NAME_NUM_SEGMENTS));
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_KEY_URI_TEMPLATE)) // mandatory attr
			keyUriTemplate = cryptoPeriodXml.getAttribute(XML_ATTR_NAME_KEY_URI_TEMPLATE);
		else throw new DashCryptoException("Error - " + (type == CryptoPeriodTypes.CRYPTO_PERIOD ? 
					XML_ELEMENT_NAME_CRYPTO_PERIOD : XML_ELEMENT_NAME_CRYPTO_TIMELINE) + 
				" does not have a mandatory attribute " + XML_ATTR_NAME_KEY_URI_TEMPLATE);
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE))
			keyLicenseUrlTemplate = cryptoPeriodXml.getAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE);
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_IV))
			IV = DatatypeConverter.parseHexBinary(cryptoPeriodXml.getAttribute(XML_ATTR_NAME_IV));
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_IV_URI_TEMPLATE))
			ivUriTemplate = cryptoPeriodXml.getAttribute(XML_ATTR_NAME_IV_URI_TEMPLATE);
		
		// parse attrs, specific to CryptoPeriod
		if (type == CryptoPeriodTypes.CRYPTO_PERIOD) {
			if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_OFFSET))
				offset = new Integer(cryptoPeriodXml.getAttribute(XML_ATTR_NAME_OFFSET));
		}			
	}

	static CryptoPeriod getInstance(Element cryptoPeriodXml, ContentProtection parent) throws DashCryptoException {
		if (cryptoPeriodXml.getLocalName().equals(XML_ELEMENT_NAME_CRYPTO_TIMELINE))
			return new CryptoTimeline(cryptoPeriodXml, parent);
		else if (cryptoPeriodXml.getLocalName().equals(XML_ELEMENT_NAME_CRYPTO_PERIOD))
			return new CryptoPeriod(cryptoPeriodXml, parent, CryptoPeriodTypes.CRYPTO_PERIOD);
		else {
			throw new DashCryptoExceptionUnsupported("Error: crypto period of type\"" + 
				cryptoPeriodXml.getLocalName() + "\" not supported yet");
		}
	}

	protected CryptoPeriod(String keyUriTemplate, ContentProtection parent, CryptoPeriodTypes type) {
		this.keyUriTemplate = keyUriTemplate;
		this.type = type;
		this.parent = parent;
	}

	/**
	 * @return the type
	 */
	public CryptoPeriodTypes getType() {
		return type;
	}

	/**
	 * @return number of segments in this CryptoPeriod, or in each cryptoperiod of this CryptoTimeline
	 */
	public Integer getNumSegments() throws DashCryptoException {
		return numSegments;
	}
	
	void setNumSegments(int numSegments) {
		this.numSegments = numSegments;
	}

	/**
	 * @return total number of segments (numSegments for CryptoPeriod, numSegments * numCryptoPeriods for CryptoTimeline 
	 * @throws DashCryptoException
	 */
	public Integer getTotalNumSegments() throws DashCryptoException {
		return numSegments;
	}
	
	protected long getCryptoPeriodBase(long segmentNum) throws DashCryptoException {
		return firstSegNum;
	}

	private String preParseTemplate(String initialTemplate, long segmentNum) throws DashCryptoException {
		if (initialTemplate == null)
			return null;
		return initialTemplate.replace(TEMPLATE_VAR_NUMBER, Long.toString(getCryptoPeriodBase(segmentNum)));
	}

	/**
	 * @return the keyUriTemplate
	 */
	String getKeyUriTemplate() {
		return keyUriTemplate;
	}

	/**
	 * @return the key URI, resolved in context of particular segment
	 * @throws DashCryptoException 
	 */
	public String getKeyUri(long segmentNum) throws DashCryptoException {
		// TODO preparse $Time$ also
		return parent.resolveTemplate(preParseTemplate(keyUriTemplate, segmentNum), segmentNum);
	}

	/**
	 * @return the ivUriTemplate
	 * @throws DashCryptoException 
	 */
	String getIvUriTemplate() throws DashCryptoException {
		return ivUriTemplate;
	}

	/**
	 * @return the iv URI, resolved in context of particular segment
	 * @throws DashCryptoException 
	 */
	public String getIvUri(long segmentNum) throws DashCryptoException {
		return parent.resolveTemplate(preParseTemplate(ivUriTemplate, segmentNum), segmentNum);
	}

	/**
	 * @return the keyLicenseUrlTemplate
	 */
	String getKeyLicenseUrlTemplate() {
		return keyLicenseUrlTemplate;
	}

	/**
	 * @return the key license URL, resolved in context of particular segment
	 * @throws DashCryptoException 
	 */
	public String getKeyLicenseUrl(int segmentNum) throws DashCryptoException {
		return parent.resolveTemplate(keyLicenseUrlTemplate, segmentNum);
	}

	/**
	 * @return the iV
	 */
	public byte[] getIV() {
		return IV;
	}

	/**
	 * @return offset iff this element is of type CryptoPeriod
	 * @throws DashCryptoException 
	 */
	public Integer getOffset() throws DashCryptoException {
		if (type == CryptoPeriodTypes.CRYPTO_PERIOD)
			return offset;
		else {
			throw new DashCryptoException("Atrribute \"" + XML_ATTR_NAME_OFFSET + 
				"\" is specific to CryptoPeriod elements");
		}
	}

	/**
	 * @param iV the iV to set
	 */
	void setIV(byte[] iV) {
		IV = iV;
	}

	/**
	 * @return the absolute index of the first segment corresponding to this CryptoPeriod or CryptoTimeline
	 */
	public long getFirstSegNum() {
		return firstSegNum;
	}

	/**
	 * @param firstSegNum set the absolute index of the first segment number belonging to us
	 */
	void setFirstSegNum(long firstSegNum) {
		this.firstSegNum = firstSegNum;
	}
}
