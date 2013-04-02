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
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.iso.mpeg.dash.DescriptorType;
import org.iso.mpeg.dash.RepresentationType;
import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 * This class cannot be instantiated directly, use getInstance() to create an instance of one of its descendants.
 *
 * TODO in the future to be instantiated directly from JAXB-unmarshalled ContentProtection XML element
 * TODO override some DashDescriptor interface
 */
public abstract class ContentProtection implements Const {
	protected String schemeIdUri = null;
	protected DescriptorType contentProtectionDescriptor = null;

	protected SegmentEncryption segmentEncryption = null;
	protected List<KeySystem> keySystems = new ArrayList<KeySystem>();
	
	// TODO we'll probably need some index to look up crypto period owning a particular segment # 
	//	- note that crypto periods specify ranges of segment #s
	protected List<CryptoPeriod> cryptoPeriods = new ArrayList<CryptoPeriod>();

	// data for assigning segments to crypto periods
	protected Long segmentStartNum = null;
	
	// reference to owning Representation, used to resolve templates
	protected RepresentationType representation;

	protected ContentProtection(DescriptorType contentProtectionDescriptor, long segmentStartNum, 
		RepresentationType representation) throws DashCryptoException {
		schemeIdUri = contentProtectionDescriptor.getSchemeIdUri();
		this.contentProtectionDescriptor = contentProtectionDescriptor;
		this.representation = representation;
		this.segmentStartNum = segmentStartNum;

		// in this loop we're assuming that XML was verified against schema, and we won't encounter duplicate
		// entries of SegmentEncryption, for example
		List<Element> childrenElements = contentProtectionDescriptor.getAnies();
		for (Iterator<Element> it = childrenElements.iterator(); it.hasNext(); ) {
			Element curr = it.next();
			String currElementName = curr.getLocalName();
			if (currElementName.equals(XML_ELEMENT_NAME_SEGMENT_ENCRYPTION))
				segmentEncryption = SegmentEncryption.getInstance(curr);
			else if (currElementName.equals(XML_ELEMENT_NAME_KEY_SYSTEM)) {
				try {
					KeySystem keySys = KeySystem.getInstance(curr, this);
					keySystems.add(keySys);
				} catch (DashCryptoExceptionUnsupported dceu) {
					// ignore unsupported key systems for now
				}
			} else if (currElementName.equals(XML_ELEMENT_NAME_CRYPTO_PERIOD) 
				  || currElementName.equals(XML_ELEMENT_NAME_CRYPTO_TIMELINE))
				cryptoPeriods.add(CryptoPeriod.getInstance(curr, this));
			// else warn about unknown child element
		}
		
		preAssignSegmentsToCryptoPeriods();
		
		if (keySystems.isEmpty())
			throw new DashCryptoException("Error - no supported key systems in this ContentProtection element");
		if (cryptoPeriods.isEmpty())
			throw new DashCryptoException("Error - no supported Crypto Periods in this ContentProtection element");
		
	}

	/**
	 * @param schemeIdUriMpegDashSea
	 */
	protected ContentProtection(String schemeIdUri, long segmentStartNum, RepresentationType representation) {
		this.schemeIdUri = schemeIdUri;
		this.representation = representation;
		this.segmentStartNum = segmentStartNum;
	}

	/**
	 * Instantiates ContentProtection from existing descriptor
	 * 
	 * @param contentProtectionDescriptor
	 * @return
	 * @throws DashCryptoException
	 */
	public static ContentProtection getInstance(DescriptorType contentProtectionDescriptor, long segmentStartNum, 
			RepresentationType representation) throws DashCryptoException {
		String schemeIdUri = contentProtectionDescriptor.getSchemeIdUri();
		if (schemeIdUri.equals(CONTENT_PROTECTION_SCHEME_ID_URI))
			return new ContentProtectionMpegDashSea(contentProtectionDescriptor, segmentStartNum, representation);
		else
			throw new DashCryptoExceptionUnsupported("Error: Content protection descriptors with scheme ID URI \"" + 
					schemeIdUri + "\" are not yet supported");
	}

	/**
	 * Generates a new content protection with default scheme:
	 * - Will generate a single CryptoTimeline covering all segments in the representation
	 * 	- 2 segments per cryptoperiod, if total number of segments is even
	 * 	- 1 segment per cryptoperiod, if total number of segments is odd
	 * - Will generate a random AES-128 key into /tmp/key????????.bin 
	 * - Will not specify explicit IV (leaving them to be derived as per sec. 6.4.4.2 of ISO-IEC 23009-4)
	 * - Will include a fully furnished descriptor element inside
	 * 
	 * @return new ContentProtection instance
	 * @throws DashCryptoException
	 */
	public static ContentProtection generateNewInstance(RepresentationType representation, long segmentStartNum)
			throws DashCryptoException {
		return new ContentProtectionMpegDashSea(representation, segmentStartNum);
	}

	/**
	 * Encrypts the given segment.
	 * 
	 * @param unencryptedSegment
	 * @param encryptedSegment
	 * @param segmentNum
	 */
	public abstract void encryptSegment(InputStream unencryptedSegment, File encryptedSegment, long segmentNum)
			throws DashCryptoException, IOException;

	/**
	 * Decrypts the given segment.
	 * 
	 * @param encryptedSegment
	 * @param unencryptedSegment
	 * @param segmentNum
	 */
	public abstract void decryptSegment(InputStream encryptedSegment, File unencryptedSegment, long segmentNum)
			throws DashCryptoException, IOException;
		
	/**
	 * @return the segment encryption
	 */
	public SegmentEncryption getSegmentEncryption() {
		return segmentEncryption;
	}

	/**
	 * @return the key system
	 */
	public List<KeySystem> getKeySystems() {
		return keySystems;
	}

	/**
	 * @return the crypto period[s] or timeline[s]
	 */
	public List<CryptoPeriod> getCryptoPeriods() {
		return cryptoPeriods;
	}

	/**
	 * @return the scheme ID URI
	 */
	public String getSchemeIdUri() {
		return schemeIdUri;
	}

	/**
	 * @return the contentProtectionDescriptor
	 */
	public DescriptorType getDescriptor() {
		return contentProtectionDescriptor;
	}

	/**
	 * Resolves template URIs in context of the current Representation
	 * 
	 * @param uriTemplate one to resolve
	 * @param segmentNum number of current segment
	 * @return resolved URI
	 * @throws DashCryptoException
	 */
	String resolveTemplate(String uriTemplate, long segmentNum) throws DashCryptoException {
		if (uriTemplate == null)
			return null;
		
		String result = uriTemplate;
		
		// vars
		result.replace(TEMPLATE_VAR_REPRESENTATION_ID, representation.getId());
		result.replace(TEMPLATE_VAR_NUMBER, Long.toString(segmentNum));
		result.replace(TEMPLATE_VAR_BANDWIDTH, Long.toString(representation.getBandwidth()));
		if (result.contains(TEMPLATE_VAR_TIME)) {
			throw new DashCryptoExceptionUnsupported("Error: resolving template variable \"" + TEMPLATE_VAR_TIME 
				+ "\" not yet supported");
		}
		
		// escapes
		result.replace(TEMPLATE_VAR_ESCAPE_DOLLAR, TEMPLATE_VAR_ESCAPE_DOLLAR.substring(1));
		
		return result;
	}

	/**
	 * @return the segmentStartNum
	 */
	public long getSegmentStartNum() {
		return segmentStartNum;
	}

	/**
	 * Determines which CryptoPeriod owns this segment (if any)
	 * TODO do something better than linear search, like range->cryptoperiod index (arithmetic coding?)
	 * 
	 * @param segmentNum
	 * @return CryptoPeriod (or CryptoTimeline, or null) corresponding to this particular segment
	 * @throws DashCryptoException 
	 */
	public CryptoPeriod cryptoPeriodForSegmentNumber(long segmentNum) throws DashCryptoException {
		for (int i=0, last=cryptoPeriods.size()-1; i<=last; i++) {
			CryptoPeriod curr = cryptoPeriods.get(i);
			if (segmentNum < curr.getFirstSegNum())
				return null; // this segment is not encrypted
			else if (curr.getNumSegments() == null) {
				if (i == last && curr.getType() == CryptoPeriodTypes.CRYPTO_PERIOD)
					return curr; // last CryptoPeriod is unbounded, and covers this segment
				else throw new DashCryptoException("Error: only last CryptoPeriod may be missing @" 
					+ XML_ATTR_NAME_NUM_SEGMENTS);
			} else if (segmentNum < curr.getFirstSegNum() + curr.getTotalNumSegments())
				return curr; // segment falls within explicit range of this crypto period
			// else continue looping
		}
		return null; // if no cryptoperiods matched, then this segment is not encrypted
	}

	/**
	 * This one MUST be run after initializing or changing the list of Crypto Periods
	 * 
	 * Recording the absolute start of the segment range belonging to this CryptoPeriod or CryptoTimeline
	 * @throws DashCryptoExceptionUnsupported 
	 * 
	 */
	protected void preAssignSegmentsToCryptoPeriods() throws DashCryptoException {
		long currNumSegments = (long) 0, currSegBase = segmentStartNum;

		for (int i=0, last=cryptoPeriods.size()-1; i<=last; i++) {
			CryptoPeriod curr = cryptoPeriods.get(i);
			switch (curr.getType()) {
			case CRYPTO_PERIOD:
				if (curr.getOffset() != null)
					currSegBase += curr.getOffset(); 
				if (curr.getNumSegments() != null)
					currNumSegments = curr.getNumSegments();
				else if (i != last) {
					throw new DashCryptoException("Error: only last CryptoPeriod may be missing @" 
						+ XML_ATTR_NAME_NUM_SEGMENTS);
				}
				break;
			case CRYPTO_TIMELINE:
				CryptoTimeline currTimeline = (CryptoTimeline) curr;
				if (currTimeline.getFirstOffset() != null)
					currSegBase += currTimeline.getFirstOffset();
				currNumSegments = currTimeline.getTotalNumSegments();				
				break;
			default:
				throw new DashCryptoExceptionUnsupported("Unsupported CryptoPeriod type");			
			}

			curr.setFirstSegNum(currSegBase);
			currSegBase += currNumSegments;
		}
	}
}
