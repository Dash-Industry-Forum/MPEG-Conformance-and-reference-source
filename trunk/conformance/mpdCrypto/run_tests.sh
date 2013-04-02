#! /bin/bash

if [[ ! -f .project ]]; then
	echo "You're in the wrong directory. You must be in the same directory as $0";
	exit 1;
fi

export CLASSPATH=$CLASSPATH:.:bin:lib/commons-cli-1.2.jar:lib/commons-io-2.4.jar

# Encrypt cleartext .mp4 media from local disk
mkdir -p test/output/test1_enc
java mpdEncryptionUtil --encrypt --input-file test/input/cleartext/test1.mpd --output-dir test/output/test1_enc
echo "----------------------------------------------------------------"

# Encrypt cleartext MPEG-2 TS media from the Internet
mkdir test/output/test2_enc
java mpdEncryptionUtil --encrypt --input-file test/input/cleartext/test2.mpd --output-dir test/output/test2_enc
echo "----------------------------------------------------------------"

# Decrypt encrypted .mp4 media from local disk
mkdir test/output/test3_dec
java mpdEncryptionUtil --decrypt --input-file test/input/encrypted/test1_cleartext_encrypted/test1.mpd --output-dir test/output/test3_dec
