Validator for DASH
------------------

created by Markus Waltl (Alpen-Adria-Universitaet Klagenfurt)

The structure of the package is as following:

|- bin/				<- pre-compiled version of the conformance software
|- schemas/			<- schema files for DASH
|- schematron/			<- Schematron Rule based validator (stand-alone version included)
|- src/				<- source file of the validator
|- validator_examples/		<- examples for schema validation tests
|- xlink_examples/		<- examples for testing the XLinkResolver
|- build.xml			<- ant build file
|- changelog.txt		<- file for indicating changes
|- compile_and_run.txt		<- file describing the compilation and executation procedure
|- readme.txt			<- this file
|- saxon9.jar			<- file needed by Saxon XSLT
|- saxon9-dom.jar		<- file needed by Saxon DOM transformation
|- xercesImpl.jar		<- file needed by Xerces


Note: you need at least Java 1.6.0 Update 17 because JAXB is already added in this version.

