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
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Vector;

/**
 * Class for providing an XLink resolver.
 * 
 * The XLink resolver handles all cases defined in the standard
 * (e.g., only HTTP, no circular referencing)
 * 
 * @author Markus Waltl <markus.waltl@itec.uni-klu.ac.at>
 */
public class XLinkResolver {
	private static Vector<Document> documentList_ = new Vector<Document>();
	
	
	public void resolveXLinks(String fileToResolve) throws SAXException, IOException, ParserConfigurationException, XPathExpressionException, XLinkException, TransformerException {
		Document localDoc = parseXML(fileToResolve);
		documentList_.add(localDoc); // add start document to the list
			
		Document newDocument = handleNodeList(localDoc);
			
		// output temporary XML file
		writeXMLFile(newDocument);
	}
	
	private static Document handleNodeList(Document doc) throws XLinkException, XPathExpressionException, SAXException, IOException, ParserConfigurationException {
		NodeList nList = extractXLinkElements(doc);
		if (nList != null) {
			for (int i = 0; i < nList.getLength(); i++) {
				Node nNode = nList.item(i);
				
				if (Definitions.debug_)
					printNode(nNode);
				
				String link = extractXLinkHref(nNode);
				if (link != null) {
					Document remoteDoc = parseXML(link);
					
					// check if referencing element type and referenced element type are the same
					Element remoteRootElement = remoteDoc.getDocumentElement();
					if (!nNode.getNodeName().equals(remoteRootElement.getNodeName())) {
						throw new XLinkException("Referenced Document must contain same element type as referencing element!\n\n"
								+ "Referencing element: " + nNode.getNodeName() + "\nReferenced element: " + remoteDoc.getDocumentElement().getNodeName());
					}
					
					// check if we already used this document (direct or indirect circular reference)
					for (int j = 0; j < documentList_.size(); j++) {
						if (documentList_.get(j).isEqualNode(remoteDoc))
							throw new XLinkException("Circular referencing detected!");
					}
					
					documentList_.add(remoteDoc); // add the parsed document to the list for detecting circular referencing
										
					//printElement(remoteDoc.getDocumentElement());
					
					remoteDoc = handleNodeList(remoteDoc);
					
					// replace XLink included document
					Node parent = nNode.getParentNode();
					Node importedNode = doc.importNode(remoteRootElement, true);
					
					// re-add and reset original attributes
					NamedNodeMap nodeAttributes = nNode.getAttributes();
					if (nodeAttributes != null) {
						for (int j = 0; j < nodeAttributes.getLength(); j++) {
							Node attribute = nodeAttributes.item(j);							
							String name = attribute.getNodeName();
							String value = attribute.getNodeValue();
							String namespace = attribute.getNamespaceURI();
							
							// do not re-add the XLink attributes and empty attributes
							// also reset to original values of attributes in referenced document
							// having different values than in original document
							if (!Definitions.XLINK_NAMESPACE.equals(namespace) && !value.equals(""))
								((Element)importedNode).setAttribute(name, value);
						}
					}
					
					// replace node
					parent.replaceChild(importedNode, nNode);
				}
			}
		}
		
		return doc;
	}

	// needed to write or can be used directly?
	private static void writeXMLFile(Document doc) throws TransformerException, IOException {
		// set up a transformer
		TransformerFactory transfac = TransformerFactory.newInstance();
		Transformer trans = transfac.newTransformer();
		trans.setOutputProperty(OutputKeys.INDENT, "yes");
		trans.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "no");

		// create string from xml tree
		StringWriter sw = new StringWriter();
		StreamResult result = new StreamResult(sw);
		DOMSource source = new DOMSource(doc);
		trans.transform(source, result);
		String xmlString = sw.toString();

		// print xml
		if (Definitions.debug_)
			System.out.println(xmlString);

		// Write newly generated XML to temp folder
		FileWriter fstream = new FileWriter(Definitions.tmpOutputFile_);
		BufferedWriter out = new BufferedWriter(fstream);
		out.write(xmlString);
		out.close();
	}
	
	private static Document parseXML(String xmlURI) throws SAXException, IOException, ParserConfigurationException {
		DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
		dbFactory.setNamespaceAware(true); // resolve namespaces
		
		DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
		Document doc;
		try {
			URL myURL = new URL(xmlURI);
			doc = dBuilder.parse(myURL.openStream());
		}
		catch ( MalformedURLException e ) {
			doc = dBuilder.parse(xmlURI);
		}
		doc.getDocumentElement().normalize();
		
		return doc;
	}
	
	private static NodeList extractXLinkElements(Document doc) throws XPathExpressionException {
		XPath xpath = XPathFactory.newInstance().newXPath();
		return (NodeList)xpath.evaluate("//*[@*[namespace-uri()='" + Definitions.XLINK_NAMESPACE + "']]", doc, XPathConstants.NODESET);
	}
	
	/**
	 * Extracts the XLink href attribute and checks the protocol
	 * @param node Node to extract the link from
	 * @return String with the link
	 * @throws XLinkException thrown if an unsupported protocol is used
	 */
	private static String extractXLinkHref(Node node) throws XLinkException {
		String href = null;
		NamedNodeMap nnm = node.getAttributes();
		boolean found = false;
		for (int att = 0; att < nnm.getLength() && !found; att++) {
			Node n = nnm.item(att);
			// check for href attribute and if the correct namespace is used
			if (Definitions.XLINK_NAMESPACE.equals(n.getNamespaceURI()) && Definitions.HREF.equals(n.getLocalName())) {
				href = n.getNodeValue();
				found = true;
			}			
		}
		
		// we only allow http links
		if (href != null && (!href.startsWith(Definitions.PROTOCOL) && !href.startsWith(Definitions.SECURE_PROTOCOL)))
			throw new XLinkException("Only HTTP links are allowed!");
		
		return href;
	}
	
	private static void printNode(Node node) {
		System.out.println(node.getNodeName());
		NamedNodeMap nnm = node.getAttributes();
		for (int att = 0; att < nnm.getLength(); att++) {
			Node n = nnm.item(att);
			System.out.println(n.getNamespaceURI() + " -- " + n.getLocalName() + " -- " + n.getNodeValue());
		}
		System.out.println("");
	}
}