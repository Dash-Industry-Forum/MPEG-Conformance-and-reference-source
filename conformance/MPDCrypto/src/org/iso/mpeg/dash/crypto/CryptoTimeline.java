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
 */
public class CryptoTimeline extends CryptoPeriod {
	// fields specific to CryptoTimeline
	private Integer numCryptoPeriods = null;
	private Integer firstOffset = null;
	
	/**
	 * @param cryptoPeriodXml
	 * @param type
	 * @throws DashCryptoException 
	 */
	CryptoTimeline(Element cryptoPeriodXml, ContentProtection parent) throws DashCryptoException {
		super(cryptoPeriodXml, parent, CryptoPeriodTypes.CRYPTO_TIMELINE);

		// parse attrs, specific to CryptoPeriod
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_NUM_CRYPTO_PERIODS))
			numCryptoPeriods = new Integer(cryptoPeriodXml.getAttribute(XML_ATTR_NAME_NUM_CRYPTO_PERIODS));
		if (cryptoPeriodXml.hasAttribute(XML_ATTR_NAME_FIRST_OFFSET))
			firstOffset = new Integer(cryptoPeriodXml.getAttribute(XML_ATTR_NAME_FIRST_OFFSET));		
	}

	CryptoTimeline(String keyUriTemplate, ContentProtection parent, int numCryptoPeriods) {
		super(keyUriTemplate, parent, CryptoPeriodTypes.CRYPTO_TIMELINE);
		this.numCryptoPeriods = numCryptoPeriods;
	}

	/**
	 * @return first offset
	 */
	public Integer getFirstOffset() {
		return firstOffset;
	}

	/**
	 * @return the number of crypto periods
	 * @throws DashCryptoException 
	 */
	public Integer getNumCryptoPeriods() throws DashCryptoException {
		if (numCryptoPeriods == null) {
			throw new DashCryptoException("Error: CryptoTimeline elements must possess mandatory @" 
				+ XML_ATTR_NAME_NUM_CRYPTO_PERIODS);
		}
		return numCryptoPeriods;
	}
	
	void setNumCryptoPeriods(int numCryptoPeriods) {
		this.numCryptoPeriods = numCryptoPeriods;
	}

	/* (non-Javadoc)
	 * @see org.iso.mpeg.dash.crypto.CryptoPeriod#getNumSegments()
	 */
	@Override
	public Integer getNumSegments() throws DashCryptoException {
		if (numSegments == null)
			throw new DashCryptoException("Error: CryptoTimeline elements must possess @" + XML_ATTR_NAME_NUM_SEGMENTS);
		return super.getNumSegments();
	}
	
	/* (non-Javadoc)
	 * @see org.iso.mpeg.dash.crypto.CryptoPeriod#getTotalNumSegments()
	 */
	@Override
	public Integer getTotalNumSegments() throws DashCryptoException {
		return getNumSegments() * getNumCryptoPeriods();
	}

	/* (non-Javadoc)
	 * @see org.iso.mpeg.dash.crypto.CryptoPeriod#getCryptoPeriodBase(long)
	 */
	@Override
	protected long getCryptoPeriodBase(long segmentNum) throws DashCryptoException {
		// gotta find the base of the particular cryptoperiod this segment belongs to
		long remainder = (segmentNum - firstSegNum) % getNumSegments();
		return segmentNum - remainder;
	}
}
