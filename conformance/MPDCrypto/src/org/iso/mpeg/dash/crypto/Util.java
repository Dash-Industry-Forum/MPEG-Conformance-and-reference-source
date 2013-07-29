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
	@since	Jan 19, 2013
*/
package org.iso.mpeg.dash.crypto;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.iso.mpeg.dash.MPD;

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 *
 */
public class Util implements Const {

	// never to be instantiated
	private Util() {}
	
	/**
	 * Extracts base file name of the input file, and provides an InputStream to its contents
	 * 
	 * @param path (can be either a filesystem path, or a URL)
	 * @return
	 * @throws DashCryptoException
	 * @throws IOException 
	 */
	public static InputSource inputSourceFromPath(String path) throws DashCryptoException, IOException {
		String segmentBaseFileName = null;
		InputStream segmentInputStream = null;
	
		File inputFile = new File(path);
		if (inputFile.exists()) { // a valid file path, construct a filesystem URL
			if (!inputFile.canRead())
				throw new DashCryptoException("Input file \"" + path + 
						"\" exists on local filesystem, but it's not readable");
			segmentInputStream = new FileInputStream(inputFile);
			segmentBaseFileName = inputFile.getName();
		} else try { // not a valid filesystem path, try it as URL
			URL segmentURL = new URL(path);
			segmentInputStream = segmentURL.openStream();
			File urlFilePath = new File(segmentURL.getFile());
			segmentBaseFileName = urlFilePath.getName();
		} catch (MalformedURLException e) {
			// TODO if URL is malformed, try decoding input string via URI.toURL()
			e.printStackTrace();
			throw new DashCryptoException("Error creating URL from \"" + path + "\"", e);
		}
		
		return new InputSource(segmentInputStream, segmentBaseFileName);
	}
	
	public static JAXBElement<MPD> loadMpd(InputSource inputMpdSource, boolean validateSchematron) 
			throws DashCryptoException {
		try {
			JAXBContext jaxbContext = JAXBContext.newInstance(MPD.class);
			Unmarshaller unmarshaller = jaxbContext.createUnmarshaller();
			
			if (validateSchematron) {
				// FIXME doesn't work for now, gotta resolve xlinks
				System.out.println("Validating against DASH schema");
				SchemaFactory sf = SchemaFactory.newInstance(javax.xml.XMLConstants.W3C_XML_SCHEMA_NS_URI);
				File schemaFile = new File(DASH_SCHEMA_PATH); // TODO change to URL, and load schema from MPD itself
				if (!schemaFile.canRead())
					throw new DashCryptoException("Schema not found at \"" + DASH_SCHEMA_PATH + "\"");
				Schema schema = sf.newSchema(schemaFile);
				unmarshaller.setSchema(schema);
			}

			EventHandler e = new EventHandler();
			unmarshaller.setEventHandler(e);

			JAXBElement<MPD> mpd = unmarshaller.unmarshal(new StreamSource(inputMpdSource.getInputStream()), MPD.class);
			if (e.hasErrors())
				throw new DashCryptoException(e.getErrorMessage());

			return mpd;
		} catch (JAXBException e) {
			throw new DashCryptoException("Parsing error: " + e.getMessage(), e);
		} catch (Exception e) { // TODO specialize here
			throw new DashCryptoException("Unexpected error: " + e.getMessage(), e);
		}
	}

	/**
	 * @param modified MPD to be saved to output directory
	 * @throws DashCryptoException 
	 */
	public static void saveModifiedMpd(JAXBElement<MPD> mpd, String mpdOutputFilePath)
			throws DashCryptoException {
		try {
			JAXBContext jaxbContext = JAXBContext.newInstance(MPD.class);
			Marshaller marshaller = jaxbContext.createMarshaller();
			EventHandler e = new EventHandler();
			marshaller.setEventHandler(e);
			marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);

			// TODO do we need to do schema & schematron validation on export also?

			File mpdOutputFile = new File(mpdOutputFilePath);
			if (mpdOutputFile.exists())
				throw new DashCryptoException("ERROR: output MPD file \"" + mpdOutputFilePath + "\" already exists");			
//			if (!mpdOutputFile.canWrite())
//				throw new DashCryptoException("ERROR: cannot write output MPD file \"" + mpdOutputFilePath + "\"");

			marshaller.marshal(mpd, mpdOutputFile);
			if (e.hasErrors())
				throw new DashCryptoException(e.getErrorMessage());
		} catch (JAXBException e) {
			throw new DashCryptoException("Parsing error: " + e.getMessage(), e);
		} catch (Exception e) { // TODO specialize here
			throw new DashCryptoException("Unexpected error: " + e.getMessage(), e);
		}		
	}	
}
