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
import java.util.Arrays;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.xml.bind.DatatypeConverter;

import org.apache.commons.io.FileUtils;
import org.w3c.dom.Element;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 */
public class SegmentEncryptionAES128CBC extends SegmentEncryption {
	/**
	 * @param segmentEncryptionElement
	 * @param encryptionSystemUri
	 */
	public SegmentEncryptionAES128CBC(Element segmentEncryptionElement, String encryptionSystemUri) {
		super(segmentEncryptionElement, encryptionSystemUri);
	}

	SegmentEncryptionAES128CBC() {
		super(ENCRYPTION_SYSTEM_URI_AES128_CBC);
		ivEncryptionFlag = true;
	}

	/* (non-Javadoc)
	 * @see org.iso.mpeg.dash.crypto.SegmentEncryption#Crypt(java.io.InputStream, java.io.File, byte[], byte[], boolean)
	 */
	@Override
	public void Crypt(InputStream inputSegment, File outputSegment, byte[] key, byte[] iv, boolean encrypt)
			throws DashCryptoException, GeneralSecurityException, IOException {
		SecretKeySpec keySpec = new SecretKeySpec(key, SEGMENT_ENCRYPTION_BASE_CIPHER_ALGO);
		IvParameterSpec ivSpec = new IvParameterSpec(iv);
		
		Cipher cipher = Cipher.getInstance(SEGMENT_ENCRYPTION_BASE_CIPHER_DESC);
		cipher.init(encrypt ? Cipher.ENCRYPT_MODE : Cipher.DECRYPT_MODE, keySpec, ivSpec);

		CipherInputStream cis = new CipherInputStream(inputSegment, cipher);
		FileUtils.copyInputStreamToFile(cis, outputSegment);
		cis.close();
	}

	/* (non-Javadoc)
	 * @see org.iso.mpeg.dash.crypto.SegmentEncryption#deriveIvFromSegmentNumber(byte[], int)
	 */
	@Override
	public byte[] deriveIvFromSegmentNumber(byte[] key, long segmentNum) throws DashCryptoException {
		int ivStrLen = 2 * AES_BLOCK_LEN;
		String segNumHex = Long.toHexString(segmentNum);
		if (segNumHex.length() > ivStrLen)
			throw new DashCryptoException("Segment number " + segmentNum + " is too large for IV derivation");
		char leftPaddingZerosArr[] = new char[ivStrLen - segNumHex.length()];
		Arrays.fill(leftPaddingZerosArr, '0');
		String leftPaddingZerosStr = new String(leftPaddingZerosArr);
		byte[] iv = DatatypeConverter.parseHexBinary(leftPaddingZerosStr + segNumHex);

		if (ivEncryptionFlag) try {
			SecretKeySpec keySpec = new SecretKeySpec(key, SEGMENT_ENCRYPTION_BASE_CIPHER_ALGO);			
			Cipher cipher = Cipher.getInstance(SEGMENT_ENCRYPTION_IV_GEN_CIPHER_MODE);
			cipher.init(Cipher.ENCRYPT_MODE, keySpec);
			iv = cipher.doFinal(iv);
		} catch (GeneralSecurityException gse) {
			throw new DashCryptoException(gse);
		}

		return iv;
	}
}
