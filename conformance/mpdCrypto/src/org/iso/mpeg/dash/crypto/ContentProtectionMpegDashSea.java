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
import java.security.SecureRandom;
import java.util.Iterator;

import javax.xml.bind.DatatypeConverter;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.iso.mpeg.dash.DescriptorType;
import org.iso.mpeg.dash.RepresentationType;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 */
public class ContentProtectionMpegDashSea extends ContentProtection {
	/**
	 * @param contentProtectionNode
	 * @throws DashCryptoException
	 */
	ContentProtectionMpegDashSea(DescriptorType contentProtectionDescriptor, long segmentStartNum, 
		RepresentationType representation) throws DashCryptoException {
		super(contentProtectionDescriptor, segmentStartNum, representation);
	}

	/**
	 * FIXME once sea schema is frozen, remove this abomination and generate JAXB objects
	 * @return
	 * @throws DashCryptoException 
	 * @deprecated Serialization to be done via JAXB in the near future
	 */
	private DescriptorType generateContentProtectionDescriptor() throws DashCryptoException {
		DescriptorType desc = new DescriptorType();
		desc.setSchemeIdUri(schemeIdUri);

		// xml motions
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	    DocumentBuilder builder;
		try {
			builder = factory.newDocumentBuilder();
		} catch (ParserConfigurationException e) {
			throw new DashCryptoException(e);
		}
	    Document document = builder.newDocument();
 
	    // SegmentEncryption element
		Element segmentEncryptionXml = (Element) document.createElement(XML_ELEMENT_NAME_SEGMENT_ENCRYPTION);
		segmentEncryptionXml.setAttribute(XML_ATTR_NAME_ENCRYPTION_SYSTEM_URI, 
			segmentEncryption.getEncryptionSystemUri());
		if (segmentEncryption.isIvEncryptionFlagBoolean() != null) {
			segmentEncryptionXml.setAttribute(XML_ATTR_NAME_IV_ENCRYPTION_FLAG, 
				segmentEncryption.isIvEncryptionFlagBoolean().toString());			
		}
		if (segmentEncryption.getEarlyAvailabilityDouble() != null) {
			segmentEncryptionXml.setAttribute(XML_ATTR_NAME_EARLY_AVAILABILITY, 
				segmentEncryption.getEarlyAvailabilityDouble().toString());
		}
		if (segmentEncryption.getKeyLengthInteger() != null) {
			segmentEncryptionXml.setAttribute(XML_ATTR_NAME_KEY_LENGTH, 
				segmentEncryption.getKeyLengthInteger().toString());
		}
		desc.getAnies().add(segmentEncryptionXml);

		// KeySystem elements
		for (Iterator<KeySystem> it = keySystems.iterator(); it.hasNext(); ) {
			KeySystem keySystem = it.next();
			Element keySystemXml = (Element) document.createElement(XML_ELEMENT_NAME_KEY_SYSTEM);
			keySystemXml.setAttribute(XML_ATTR_NAME_KEY_SYSTEM_URI, keySystem.getKeySystemUri());
			if (keySystem.getKeyLicenseUrlTemplate() != null)
				keySystemXml.setAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE, keySystem.getKeyLicenseUrlTemplate());
			desc.getAnies().add(keySystemXml);
		}

		// sole CryptoTimeline created by local encryption
		for (Iterator<CryptoPeriod> it=cryptoPeriods.iterator(); it.hasNext(); ) {
			CryptoPeriod cryptoPeriod = it.next();
			Element cryptoPeriodXml;
			
			switch (cryptoPeriod.type) {
			case CRYPTO_PERIOD: // add CryptoPeriod-specific attrs
				cryptoPeriodXml = (Element) document.createElement(XML_ELEMENT_NAME_CRYPTO_PERIOD);
				if (cryptoPeriod.getOffset() != null)
					cryptoPeriodXml.setAttribute(XML_ATTR_NAME_OFFSET, cryptoPeriod.getOffset().toString());
				break;
			case CRYPTO_TIMELINE: // add CryptoTimeline-specific attrs
				cryptoPeriodXml = (Element) document.createElement(XML_ELEMENT_NAME_CRYPTO_TIMELINE);
				CryptoTimeline cryptoTimeline = (CryptoTimeline) cryptoPeriod;
				if (cryptoTimeline.getNumCryptoPeriods() != null) {
					cryptoPeriodXml.setAttribute(XML_ATTR_NAME_NUM_CRYPTO_PERIODS, 
						Integer.toString(cryptoTimeline.getNumCryptoPeriods()));
				}
				if (cryptoTimeline.getFirstOffset() != null) {
					cryptoPeriodXml.setAttribute(XML_ATTR_NAME_FIRST_OFFSET, 
						Integer.toString(cryptoTimeline.getFirstOffset()));
				}
				break;
			default:
				throw new DashCryptoExceptionUnsupported("Unsupported CryptoPeriod type");
			}

			// add common fields
			cryptoPeriodXml.setAttribute(XML_ATTR_NAME_KEY_URI_TEMPLATE, cryptoPeriod.getKeyUriTemplate());
			if (cryptoPeriod.getNumSegments() != null)
				cryptoPeriodXml.setAttribute(XML_ATTR_NAME_NUM_SEGMENTS, cryptoPeriod.getNumSegments().toString());
			if (cryptoPeriod.getKeyLicenseUrlTemplate() != null)
				cryptoPeriodXml.setAttribute(XML_ATTR_NAME_KEY_LIC_URL_TEMPLATE, cryptoPeriod.getKeyLicenseUrlTemplate());
			if (cryptoPeriod.getIvUriTemplate() != null)
				cryptoPeriodXml.setAttribute(XML_ATTR_NAME_IV_URI_TEMPLATE, cryptoPeriod.getIvUriTemplate());
			if (cryptoPeriod.getIV() != null)
				cryptoPeriodXml.setAttribute(XML_ATTR_NAME_IV, DatatypeConverter.printHexBinary(cryptoPeriod.getIV()));
			
			desc.getAnies().add(cryptoPeriodXml);
		}

		return desc;
	}
	
	/**
	 * This constructor is used to generate a new instance of baseline content protection
	 * TODO clean up this function, 'cause, damn
	 * 
	 * @param contentProtectionNode
	 * @throws DashCryptoException
	 */
	ContentProtectionMpegDashSea(RepresentationType representation, long segmentStartNum) throws DashCryptoException {
		super(CONTENT_PROTECTION_SCHEME_ID_URI, segmentStartNum, representation);

		KeySystem baselineKeySys = new KeySystemBaselineHttp(this);
		keySystems.add(baselineKeySys);
		segmentEncryption = new SegmentEncryptionAES128CBC();
		
		// generate a random key to /tmp/key????????.bin
		SecureRandom secRnd = new SecureRandom();
		String keyPath = "/tmp/key" + Integer.toString(10000000 + secRnd.nextInt(90000000)) + ".bin";
		File keyFile = new File(keyPath);
		if (keyFile.exists())
			throw new DashCryptoException("Error - key file " + keyPath + " already exists");
		byte[] keyBytes = new byte[16];
		secRnd.nextBytes(keyBytes);
		try {
			FileUtils.writeByteArrayToFile(keyFile, keyBytes);
		} catch (IOException e) {
			throw new DashCryptoException(e);
		}

//		// generate a random IV
//		byte[] ivBytes = new byte[16];
//		secRnd.nextBytes(ivBytes);

		// create a single CryptoTimeline, covering all segments, including path to random key and implicit IV		
		int numCryptoPeriods = representation.getSegmentList().getSegmentURLs().size(), numSegmentsPerCryptoPeriod = 1;
		if (numCryptoPeriods % 2 == 0) { // if even number of segments, have 2-seg cryptoperiods
			numCryptoPeriods /= 2;
			numSegmentsPerCryptoPeriod *= 2;
		}
		CryptoTimeline cryptoTimeline = new CryptoTimeline(keyPath, this, numCryptoPeriods);
		cryptoTimeline.setNumSegments(numSegmentsPerCryptoPeriod); // one or two segments per cryptoperiod
//		cryptoTimeline.setIV(ivBytes);
		cryptoPeriods.add(cryptoTimeline);
		preAssignSegmentsToCryptoPeriods();

		// parse internal structure to descriptor (or should we do so lazily?)
		contentProtectionDescriptor = generateContentProtectionDescriptor();
	}

	/**
	 * Helper to encrypt/decrypt the input segment (URL) to output segment (file)
	 *  
	 * @param inputSegment
	 * @param outputSegment
	 * @param segmentNum
	 * @param encrypt
	 * @throws DashCryptoException
	 * @throws IOException
	 */
	private void baselineCryptSegmentHelper(InputStream inputSegment, File outputSegment, long segmentNum, 
			boolean encrypt) throws DashCryptoException, IOException {
		// determine which CryptoPeriod owns this segment (if any)
		CryptoPeriod relevantCryptoPeriod = cryptoPeriodForSegmentNumber(segmentNum);
		if (relevantCryptoPeriod != null) { // got info to encrypt/decrypt segment
			// fetch key via any KeySystem
			byte[] key = null;
			String keyUri = relevantCryptoPeriod.getKeyUri(segmentNum);
			for (Iterator<KeySystem> it=keySystems.iterator(); it.hasNext() && key == null; ) {
				KeySystem keySys = it.next();
				try {
					key = keySys.retrieveKey(keyUri);
				} catch (DashCryptoException dce) { // print a warning and skip to the next key system	
					System.out.println("WARNING: KeySystem \"" + keySys.getKeySystemUri() + 
						"\" cannot retrieve key at URI \"" + keyUri + "\": " + dce.getMessage());
				}
			}
			if (key == null)
				throw new DashCryptoException("Error: none of the available key systems could retrieve the key");
	
			// fetch or calculate iv
			byte[] iv = null;
			String ivUri = null;
			if (relevantCryptoPeriod.getIV() != null) // IV explicitly provided
				iv = relevantCryptoPeriod.getIV();
			else if ((ivUri = relevantCryptoPeriod.getIvUri(segmentNum)) != null) { // fetch IV
				InputSource ivInputSource = Util.inputSourceFromPath(ivUri);
				iv = IOUtils.toByteArray(ivInputSource.getInputStream());
			} else // derive IV from the segment number
				iv = segmentEncryption.deriveIvFromSegmentNumber(key, segmentNum);

			try {
				segmentEncryption.Crypt(inputSegment, outputSegment, key, iv, encrypt);
			} catch (GeneralSecurityException gse) {
				gse.printStackTrace();
				throw new DashCryptoException(gse);
			}
		} else { // just copy clear-text segments in pass-thru mode
			FileUtils.copyInputStreamToFile(inputSegment, outputSegment);
			System.out.println("INFO: Segment #" + segmentNum + " has no corresponding cryptoperiod, so passed " + 
				"through to \"" + outputSegment.getAbsolutePath() + "\" verbatim" + (encrypt ? " (unencrypted)" : ""));
		}
	}

	/**
	 * @see org.iso.mpeg.dash.crypto.ContentProtection#encryptSegment(java.io.InputStream, java.io.OutputStream, int)
	 */
	@Override
	public void encryptSegment(InputStream unencryptedSegment, File encryptedSegment, long segmentNum) 
		throws DashCryptoException, IOException {
		baselineCryptSegmentHelper(unencryptedSegment, encryptedSegment, segmentNum, true);
	}

	/**
	 * @see org.iso.mpeg.dash.crypto.ContentProtection#decryptSegment(java.io.InputStream, java.io.OutputStream, int)
	 */
	@Override
	public void decryptSegment(InputStream encryptedSegment, File unencryptedSegment, long segmentNum)
		throws DashCryptoException, IOException {
		baselineCryptSegmentHelper(encryptedSegment, unencryptedSegment, segmentNum, false);	
	}
}
