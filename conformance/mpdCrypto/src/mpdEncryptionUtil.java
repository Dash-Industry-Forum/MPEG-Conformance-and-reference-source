import java.io.File;
import java.io.IOException;
import java.util.Iterator;
import java.util.List;

import javax.xml.bind.JAXBElement;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.OptionBuilder;
import org.apache.commons.cli.OptionGroup;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.apache.commons.cli.Parser;
import org.apache.commons.cli.PosixParser;
import org.iso.mpeg.dash.AdaptationSetType;
import org.iso.mpeg.dash.DescriptorType;
import org.iso.mpeg.dash.MPD;
import org.iso.mpeg.dash.PeriodType;
import org.iso.mpeg.dash.RepresentationType;
import org.iso.mpeg.dash.SegmentListType;
import org.iso.mpeg.dash.SegmentURLType;
import org.iso.mpeg.dash.crypto.Const;
import org.iso.mpeg.dash.crypto.ContentProtection;
import org.iso.mpeg.dash.crypto.DashCryptoException;
import org.iso.mpeg.dash.crypto.DashCryptoExceptionUnsupported;
import org.iso.mpeg.dash.crypto.InputSource;
import org.iso.mpeg.dash.crypto.Util;

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

/**
 * @author Vlad Zbarsky <zbarsky@cornell.edu>
 */
public class mpdEncryptionUtil implements Const {
	private static Options cmdlineOptions;

	private static final Option ARG_ENCRYPT = new Option("e", "encrypt", false, "Encrypt cleartext segments");
	private static final Option ARG_DECRYPT = new Option("d", "decrypt", false, "Decrypt encrypted segments");
	private static final Option ARG_SIGN	= new Option("s", "sign", false, "Not yet supported");
	@SuppressWarnings("static-access")
	private static final Option ARG_INPUT_FILE = OptionBuilder.withLongOpt("input-file")
		.withDescription("MPD file path or URL, we'll fetch segments ourselves")
		.isRequired()
        .hasArg()
        .withArgName("inputfile.mpd")
        .create("i");
	@SuppressWarnings("static-access")
	private static final Option ARG_OUTPUT_DIR = OptionBuilder.withLongOpt("output-dir")
		.withDescription("Directory to write encrypted or decrypted MPD and segments to")
		.isRequired()
        .hasArg()
        .withArgName("output dir")
        .create("o");
	@SuppressWarnings("static-access")
	private static final Option ARG_VALIDATE_SCHEMATRON = OptionBuilder.withLongOpt("validate-schematron")
		.withDescription("Whether to validate MPD via DASH schema and schematron (default: no)")
		.create();

	static
	{
		cmdlineOptions = new Options();
		
		OptionGroup cipherOp = new OptionGroup();
		cipherOp.setRequired(true);
		cipherOp.addOption(ARG_ENCRYPT);
		cipherOp.addOption(ARG_DECRYPT);
		cmdlineOptions.addOptionGroup(cipherOp);
		
		cmdlineOptions.addOption(ARG_SIGN);		
		cmdlineOptions.addOption(ARG_INPUT_FILE);
		cmdlineOptions.addOption(ARG_OUTPUT_DIR);
		cmdlineOptions.addOption(ARG_VALIDATE_SCHEMATRON);		
	}

	private boolean encrypt = true, sign = false, validateSchematron = false;
	private String inputMpdPath = null, outputDirPath = null;
	private InputSource inputMpdSource = null;
	private ContentProtection contentProtection = null;
	private RepresentationType representation = null;

	private static void printUsageWithError(String errorMsg) {
		HelpFormatter formatter = new HelpFormatter();
		formatter.setWidth(110);
		formatter.printHelp(mpdEncryptionUtil.class.getSimpleName(), "" /* header */, cmdlineOptions,
				(errorMsg == null ? null : "\nERROR: " + errorMsg), true);
	}
	
	private static void printUsage() {
		printUsageWithError(null);
	}

	private boolean parseCmdlineParams(String[] args) throws DashCryptoException {
		if (args == null || args.length == 0) {
			printUsage();
			return false;
		}
		Parser cmdlineParser = new PosixParser();
		CommandLine cmdline = null;
		try {
			cmdline = cmdlineParser.parse(cmdlineOptions, args);
		} catch (ParseException e) {
			printUsageWithError(e.getMessage());
			return false;
		}

		if (cmdline.hasOption(ARG_DECRYPT.getOpt()))
			encrypt = false;

		if (cmdline.hasOption(ARG_SIGN.getOpt())) {
			sign = true;
			if (!encrypt) {
				printUsageWithError("Combination of options -d and -s is meaningless");
				return false;
			}
		}
		
		if (cmdline.hasOption(ARG_VALIDATE_SCHEMATRON.getLongOpt()))
			validateSchematron = true;

		// verify input file to be readable
		inputMpdPath = cmdline.getOptionValue(ARG_INPUT_FILE.getOpt());
		try {
			inputMpdSource = Util.inputSourceFromPath(inputMpdPath);
		} catch (IOException ioe) {
			ioe.printStackTrace();
			throw new DashCryptoException("ERROR getting a source for MPD at \"" + inputMpdPath + "\"", ioe);
		}

		// verify output dir to be writable
		String _outputDirPath = cmdline.getOptionValue(ARG_OUTPUT_DIR.getOpt());
		File outputDir = new File(_outputDirPath);
		if (!outputDir.exists()) {
			printUsageWithError("Output directory \"" + _outputDirPath + "\" does not exist");
			return false;
		}
		if (!outputDir.canWrite()) {
			printUsageWithError("Output directory \"" + _outputDirPath + "\" is not readable");
			return false;
		}
		try {
			outputDirPath = outputDir.getCanonicalPath();
		} catch (IOException e) {
			e.printStackTrace();
			throw new DashCryptoException("ERROR: cannot get canonical path of output dir \""
				+ _outputDirPath + "\"", e);
		}
		
		return true;
	}

	/**
	 * 
	 * @param input
	 * @param encrypt
	 * @param segmentNum
	 * @return path to the output file
	 * @throws DashCryptoException
	 */
	private String segmentHelper(String input, boolean encrypt, long segmentNum) throws DashCryptoException {
		InputSource inputSource = null;
		try {
			inputSource = Util.inputSourceFromPath(input);
		} catch (IOException ioe) {
			ioe.printStackTrace();
			throw new DashCryptoException("ERROR getting a source for segment at \"" + input + "\"", ioe);
		}
		
		String outputSegmentPath = outputDirPath + File.separator + inputSource.getFileName();
		File outputSegment = new File(outputSegmentPath);
		if (outputSegment.exists())
			throw new DashCryptoException("ERROR: output segment file \"" + outputSegmentPath + "\" already exists");

		try {
			if (encrypt)
				contentProtection.encryptSegment(inputSource.getInputStream(), outputSegment, segmentNum);
			else
				contentProtection.decryptSegment(inputSource.getInputStream(), outputSegment, segmentNum);
		} catch (IOException ioe) {
			throw new DashCryptoException("ERROR " + (encrypt ? "en" : "de" ) + "crypting the segment #" + segmentNum, ioe);
		}

		return outputSegmentPath;
	}

	private int runInstance(String[] args) throws DashCryptoException {
		if (!parseCmdlineParams(args))
			return -1;
//		System.out.println("Finished parsing cmdline arguments");

		// load MPD, verify it to be valid XML and optionally conforming to DASH schema
		JAXBElement<MPD> mpd = Util.loadMpd(inputMpdSource, validateSchematron);
		System.out.println("Loaded MPD \"" + inputMpdPath + "\"");

		// TODO optionally validate MPD via schematron
		
		// extract first period
		List<PeriodType> periods = mpd.getValue().getPeriods();
		if (periods.isEmpty())
			throw new DashCryptoException("ERROR: no periods in this MPD");
		if (periods.size() > 1)
			System.err.println("WARNING: multiple periods in this MPD, processing only the first one");
		PeriodType period = periods.get(0);
		
		// extract its first adaptation set
		List<AdaptationSetType> adaptationSets = period.getAdaptationSets();
		if (adaptationSets.isEmpty())
			throw new DashCryptoException("ERROR: no adaptation sets in this period");
		if (adaptationSets.size() > 1)
			System.err.println("WARNING: multiple adaptation sets in this period, processing only the first one");
		AdaptationSetType adaptationSet = adaptationSets.get(0);

		// extract first representation
		List<RepresentationType> representations = adaptationSet.getRepresentations();
		if (representations.isEmpty())
			throw new DashCryptoException("ERROR: no [supported] representations in this adaptation set");
		if (representations.size() > 1)
			System.err.println("WARNING: multiple representations in this adaptation set, processing only the first one");
		representation = representations.get(0);

		// list its segments
		SegmentListType segmentList = representation.getSegmentList();
		List<SegmentURLType> segmentUrls = segmentList.getSegmentURLs();
		if (segmentUrls.isEmpty())
			System.err.println("WARNING: no segments in the segment URL list of this representation");
		Long segmentStartNum = segmentList.getStartNumber();
		if (segmentStartNum == null)
			segmentStartNum = DEFAULT_SEGMENT_START_NUMBER;

		// finally, extract our content protection entry, and note whether we encounter multiple supported entries
		boolean multipleSupportedContentProtectionEntries = false;
		int ourContentProtectionIndex = 0;
		List<DescriptorType> contentProtections = adaptationSet.getContentProtections();
		for (int i=0; i<contentProtections.size(); i++) {
			DescriptorType curr = contentProtections.get(i);
			ContentProtection _contentProt = null;
			try {
				_contentProt = ContentProtection.getInstance(curr, segmentStartNum, representation);
			} catch (DashCryptoExceptionUnsupported dceus) {
				// do nothing, continue
			}

			if (_contentProt != null) {
				if (contentProtection == null) {
					contentProtection = _contentProt;
					ourContentProtectionIndex = i;
				} else {
					multipleSupportedContentProtectionEntries = true;
					break;
				}
			}
		}

		// transform input MPD into output form
		// 	- TODO use XSLT in the future
		if (encrypt) {	// for encryption, add metadata manually for now
			if (contentProtection != null)
				throw new DashCryptoException("ERROR: trying to encrypt already encrypted MPD");
			contentProtection = ContentProtection.generateNewInstance(representation, segmentStartNum);
			contentProtections.add(0, contentProtection.getDescriptor());
		} else {		// for decryption, just erase the ContentProtection descriptor
			if (contentProtection == null)
				throw new DashCryptoExceptionUnsupported("ERROR: trying to decrypt MPD without any supported content schemes");
			else if (multipleSupportedContentProtectionEntries)
				throw new DashCryptoExceptionUnsupported("ERROR: trying to decrypt MPD with multiple supported content schemes");

			contentProtections.remove(ourContentProtectionIndex);
		}

		// encrypt/decrypt input segments into output dir
		//	- TODO in the future support hashing/signing
		//	- TODO in the future pipeline the process of fetching and encrypting/decrypting
		//		- and, if pipelining, might as well multithread
		System.out.println(); // finally, do ciphering, and renaming
		long segmentNum = segmentStartNum;
		for (Iterator<SegmentURLType> it = segmentUrls.iterator(); it.hasNext(); segmentNum++) {
			SegmentURLType segmentUrl = it.next();
			String outputSegmentPath = segmentHelper(segmentUrl.getMedia(), encrypt, segmentNum);
			System.out.println((encrypt ? "En" : "De") + "crypted segment #" + segmentNum +" \"" + segmentUrl.getMedia() 
				+ "\" to \"" + outputSegmentPath + "\"");
			segmentUrl.setMedia(outputSegmentPath);
		}
		System.out.println();

		// now marshal (deserialize) the modified MPD into output dir
		String mpdOutputFilePath = outputDirPath + File.separator + inputMpdSource.getFileName();
		Util.saveModifiedMpd(mpd, mpdOutputFilePath);

		System.out.println("SUCCESS " + (encrypt ? "encrypt" : "decrypt") + "ing \"" + inputMpdPath + "\" to \"" +
			outputDirPath + File.separator + inputMpdSource.getFileName() + "\"");
		return 0;
	}

	private mpdEncryptionUtil() {}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		mpdEncryptionUtil instance = new mpdEncryptionUtil();
		try {
			System.exit(instance.runInstance(args));
		} catch (DashCryptoException e) {
			e.printStackTrace();
			System.out.println();
			printUsageWithError(e.getMessage());
			System.exit(-1);
		}
	}
}
