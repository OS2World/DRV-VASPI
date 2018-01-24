/**************************************************************
*
* REXX script to convert MASM ".lst" file into an ".asm" file.
* An output .ASM and header file will be created based upon
* keywords already in the source.
*
**************************************************************/

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadfuncs'
call SysLoadFuncs

/*
** 2 input parameters: the name of the input .LST file
**                     the name of the output .ASM file
*/

parse arg inputfile outputfile

if inputfile = '' | outputfile = ''  then

   do
      say "You must enter an input and output file "
      say ""
      say "    Example:  CODE2DAT INFILE.LST OUTFILE.ASM"
      exit
   end

/*
** Set some defaults
*/

lastaddr = "!No code generated!"
headerfile = "\dev\nul"
procname = "!No entry point specified!"
lengthname = "@"
inextract = 0
tab = D2C( 9 )


/*
** Open the input .LST file and output .ASM file and write some
** header information to the output file
*/

call SysFileDelete( outputfile )

rc = LINEOUT( outputfile,,1 )
rc = LINEIN(inputfile,,0 )

rc = LINEOUT( outputfile, "; DO NOT ALTER THIS FILE")
rc = LINEOUT( outputfile, ";   It is created automagically by an rexx script")
rc = LINEOUT( outputfile, ";   Just sit back and enjoy")
rc = LINEOUT( outputfile, "")
rc = LINEOUT( outputfile, "  .xlist")
rc = LINEOUT( outputfile, "  include mvdm.inc")
rc = LINEOUT( outputfile, "  include vddseg.inc")
rc = LINEOUT( outputfile, "  .list")
rc = LINEOUT( outputfile, "")

/*
** The main processing loop for the input file
*/

do while LINES( inputfile ) <> 0

   currline = LINEIN( inputfile )

   /*
   ** Check to see if any of the following strings are present on the
   ** line and process accordingly
   */

   keypos = POS( "#### HEADERFILE", currline )
   if( keypos <> 0 ) then
   do

     headerfile = SUBSTR( currline, keypos+16 )
     rc = LINEOUT( outputfile, "; header information is in ["headerfile"]" )

     call SysFileDelete( headerfile )

     rc = LINEOUT( headerfile,,1 )

     rc = LINEOUT( headerfile, "/* "headerfile" -- automagic header file */" )
     rc = LINEOUT( headerfile, "/* This file is created by an rexx script. */" );
     rc = LINEOUT( headerfile, "/* Do not modify it directly. */" );

   end

   keypos = POS( "#### ENTRYPOINT", currline )
   if( keypos <> 0 ) then
   do

     procname = SUBSTR( currline, POS( "=", currline ) + 1 )

     rc = LINEOUT( outputfile, " DefData EXPORT,SWAP,C" )
     rc = LINEOUT( outputfile, "VOID "procname )

     rc = LINEOUT( headerfile, "extern UCHAR _"procname"[];" )

   end

   keypos = POS( "#### LENGTH", currline )
   if( keypos <> 0 ) then
   do

     lengthname = SUBSTR( currline, POS( "=", currline ) + 1 )

   end

   keypos = POS( "#### LABEL", currline )
   if( keypos <> 0 ) then
   do

     labelname = SUBSTR( currline, POS( "=", currline ) + 1 )
     rc = LINEOUT( headerfile, "#define "labelname" 0x"SUBSTR(currline,2,4) )

   end

   keypos = POS( "#### BEGIN EXTRACTION", currline )
   if( keypos <> 0 ) then
   do

     inextract = 1

   end

   keypos = POS( "#### END EXTRACTION", currline )
   if( keypos <> 0 ) then
   do

     inextract = 0

   end

   /*
   ** If we are in a begin-end extract construct write out the
   ** opcodes to the output file
   */


   if( inextract = 1 ) then
   do

     if( SUBSTR( currline, 1, 1 ) == " " ) then
     do

       /*
       ** First, split up the opcodes into  a space-delimited
       ** fields
       */

       tempvar = SUBSTR( currline, 8 )

       j = pos( tab,  tempvar )

       if j <> 0 then 
         opcodes = left( tempvar, j - 1 )
       else
         opcodes = ''

       numopcodes = WORDS( opcodes )

       lastaddr = SUBSTR( currline, 2, 4 )

       /*
       ** Now step through the fields
       */

       i = 1

       do while i <= numopcodes

         currword = WORD( opcodes, i )

         /*
         ** Ignore relocation markers
         */

         if( currword = "R" ) then
         do
           i = i + 1
           iterate
         end

         if( currword = "----" ) then currword = "0000"

         /*
         ** Kill any segment override marker
         */

         keypos = POS( ":", currword )
         if( keypos <> 0 ) then
         do
           currword = OVERLAY( " ", currword, keypos )
           currword = STRIP( currword, "trailing" )
         end

         /*
         ** Now output the appropriate data statement
         */

         wordlength = LENGTH( currword )

         Select

           when wordlength = 2 then
           do
             rc = LINEOUT( outputfile, " db 0"currword"h" )
           end

           when wordlength = 4 then
           do
             rc = LINEOUT( outputfile, " dw 0"currword"h" )
           end

           when wordlength = 8 then
           do
             rc = LINEOUT( outputfile, " dd 0"currword"h" )
           end

           otherwise

         end   /* End Select  */


         i = i + 1

       end  /* END do while for numopcodes */

     end  /* END do checking if first char is a blank */

   end  /* END do if we are in an extract */

end   /* END DO WHILE for input file */

rc = LINEOUT( outputfile, " IF THIS BYTE - "procname" NE 0"lastaddr"h" )
rc = LINEOUT( outputfile, "   .ERR Code length incorrect." )
rc = LINEOUT( outputfile, " ENDIF" )
rc = LINEOUT( outputfile, " EndData EXPORT,SWAP,C" )
rc = LINEOUT( outputfile, " END" )

if( lengthname <> "@" ) then rc = LINEOUT( headerfile, "#define "lengthname" 0x"lastaddr )

call stream inputfile,  'Command', 'Close'
call stream outputfile, 'Command', 'Close'
call stream headerfile, 'Command', 'Close'

