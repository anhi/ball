### list all filenames of the directory here ###
SET(SOURCES_LIST
	bruker1DFile.C
	bruker2DFile.C
	DCDFile.C
	DSN6File.C
	GAMESSDatFile.C
#	GAMESSLogFile.C
	genericMolFile.C
	HINFile.C
	INIFile.C
	JCAMPFile.C
	KCFFile.C
	lineBasedFile.C
	MOLFile.C
	SDFile.C
	MOL2File.C
	NMRStarFile.C
	parameters.C
	parameterSection.C
	PDBdefs.C
	PDBFileGeneral.C
	PDBFileDetails.C
	PDBInfo.C
	PDBRecords.C
	resourceFile.C
	trajectoryFile.C
	TRRFile.C
	XYZFile.C
	SCWRLRotamerFile.C
	molFileFactory.C
)

IF (BALL_HAS_QTSQL)
	SET(SOURCES_LIST ${SOURCES_LIST} DBInterface.C)
ENDIF()

ADD_BALL_SOURCES("FORMAT" "${SOURCES_LIST}")

ADD_BALL_PARSER_LEXER("FORMAT" "GAMESSDatParser" "GAMESSDatParser")
#ADD_BALL_PARSER_LEXER("FORMAT" "GAMESSLogParser" "GAMESSLogParser")