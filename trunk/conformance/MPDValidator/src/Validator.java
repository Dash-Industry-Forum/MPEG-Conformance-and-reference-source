/** ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla  Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and  limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is Alpen-Adria-Universitaet Klagenfurt, Austria
 * Portions created by the Initial Developer are Copyright (C) 2011
 * All Rights Reserved.
 *
 * The Initial Developer: Markus Waltl 
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.xpath.XPathExpressionException;

import org.iso.mpeg.dash.MPD;
import org.xml.sax.SAXException;


/**
 * Program for validating MPEG-DASH
 *	  
 * @author Markus Waltl <markus.waltl@itec.uni-klu.ac.at>
 */
public class Validator {
	private static String system_os = "windows";
	
	/**
	 * Entry point
	 * @param args see printUsage()
	 */
	public static void main(String[] args) {
	    // check for minimum java version (this piece of software can only
	    // be run with JRE 1.6.0_17 or higher)
        String version = System.getProperty("java.version");
        System.out.println("Your JRE version: " + version);
        char minor = version.charAt(2);
        int pos = version.indexOf("_");
        int update = -1;
       
        if (pos != -1) {
    	   try {
    		   update = Integer.valueOf(version.substring(pos+1));
    	   } catch (NumberFormatException e) { }
        }
       
        if(minor < '6') {
    	   System.out.println("\nYou need at least JRE 1.6.0_17 to run this program!");
    	   return;
        }
        else {
    	   if (minor == '6' && update < 17) {
    		   System.out.println("\nYou need at least JRE 1.6.0_17 to run this program!");
        	   return;
    	   }
        }
		
        if (args.length != 1) {
   		   System.out.println("Needing a file to validate");
   		   printUsage();
   		   return;
		}

		try {
			String OS = System.getProperty("os.name").toLowerCase();
			if (OS.indexOf("windows") == -1)
				system_os = "unix";
			
			/*File f = new File(args[0]);
			if (!f.exists()) {
				System.out.println("\nFile does not exist!\n");
				return;
			}*/		
			
			boolean retVal = false;

			// Step 1:
			// XLink resolving and validation
			System.out.println("\nStart XLink resolving\n=====================\n");
			
			XLinkResolver xlinkResolver = new XLinkResolver();
			xlinkResolver.resolveXLinks(args[0]);	
			
			System.out.println("XLink resolving successful\n\n");
			
			URL url = null;
			if (system_os.equals("windows")) {
				url = new URL("file:///" + Definitions.tmpOutputFile_);
			}
			else { // in linux there are different separators
				url = new URL("file://" + Definitions.tmpOutputFile_);
				String separator = System.getProperty("file.separator");
				Definitions.DASHXSDNAME = Definitions.DASHXSDNAME.replace("\\", separator);
				Definitions.XSLTFILE = Definitions.XSLTFILE.replace("\\", separator);
			}			
			
			// Step 2:
			// MPD validation
			System.out.println("\nStart MPD validation\n====================\n");
			retVal = parseDASH(url);
			if (retVal)
				System.out.println("MPD validation successful - DASH is valid!\n\n");
			else {
				System.out.println("MPD validation not successful - DASH is not valid!\n\n");
				return;
			}
			
			// Step 3:
			// Schematron check
			System.out.println("\nStart Schematron validation\n===========================\n");
			retVal = XSLTTransformer.transform(Definitions.tmpOutputFile_, Definitions.XSLTFILE);
			if (retVal)
				System.out.println("Schematron validation successful - DASH is valid!\n\n");
			else
				System.out.println("Schematron validation not successful - DASH is not valid!\n\n");
		} catch (MalformedURLException e) {
			System.out.println("Malformed URL: " + e.getMessage());
		} catch (SAXException e) {
			System.out.println("SAXException: " + e.getMessage());
		} catch (IOException e) {
			System.out.println("IOException: " + e.getMessage());
		} catch (ParserConfigurationException e) {
			System.out.println("ParserConfigurationException: " + e.getMessage());
		} catch (XPathExpressionException e) {
			System.out.println("XPathExpressionException: " + e.getMessage());
		} catch (XLinkException e) {
			System.out.println("XLinkException: " + e.getMessage());
		} catch (TransformerException e) {
			System.out.println("TransformerException: " + e.getMessage());
		} catch (Exception e) {
			System.out.println("Unexpected error: " + e.getMessage());
		} finally {
			// delete the temporary file
			File tmpFile = new File(Definitions.tmpOutputFile_);
			if (tmpFile != null && tmpFile.exists())
				tmpFile.delete();
		}
	}

	private static boolean parseDASH(URL pathToFile) throws URISyntaxException {
		JAXBContext jaxbContext;
		try {
			jaxbContext = JAXBContext.newInstance(MPD.class);
			Unmarshaller unmarshaller = jaxbContext.createUnmarshaller();
			SchemaFactory sf = SchemaFactory.newInstance(javax.xml.XMLConstants.W3C_XML_SCHEMA_NS_URI);
			File f = new File(Definitions.DASHXSDNAME);
			if (!f.exists()) {
				System.out.println("Schema cannot be found!");
				throw new JAXBException("Schema not found");
			}
			
			Schema schema = sf.newSchema(f);
			unmarshaller.setSchema(schema);
			EventHandler e = new EventHandler();
			unmarshaller.setEventHandler(e);
			unmarshaller.unmarshal(new StreamSource(pathToFile.openStream()), MPD.class);
			if (e.hasErrors())
				return false;
				
			return true;
		} catch (JAXBException e) {
			System.out.println("Parsing error: " + e.getMessage());
			return false; // we have errors
		} catch (Exception e) {
			System.out.println("Unexpected error: " + e.getMessage());
			return false; // we have errors
		}
	}
	
	public static void printUsage() {
		System.out.println("usage: Validator <file>");
		System.out.println("=======================");
		System.out.println("<file>   file to be validated");
		System.out.println("");
	}

}
