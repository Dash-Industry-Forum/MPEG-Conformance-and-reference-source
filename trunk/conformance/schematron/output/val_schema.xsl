<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:sch="http://www.ascc.net/xml/schematron"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:iso="http://purl.oclc.org/dsdl/schematron"
                xmlns:dash="urn:mpeg:dash:schema:mpd:2011"
                xmlns:xlink="http://www.w3.org/1999/xlink"
                xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                version="1.0"><!--Implementers: please note that overriding process-prolog or process-root is 
    the preferred method for meta-stylesheets to use where possible. -->
<xsl:param name="archiveDirParameter"/>
   <xsl:param name="archiveNameParameter"/>
   <xsl:param name="fileNameParameter"/>
   <xsl:param name="fileDirParameter"/>

   <!--PHASES-->


<!--PROLOG-->
<xsl:output xmlns:xs="http://www.w3.org/2001/XMLSchema"
               xmlns:schold="http://www.ascc.net/xml/schematron"
               xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
               method="xml"
               omit-xml-declaration="no"
               standalone="yes"
               indent="yes"/>

   <!--KEYS-->


<!--DEFAULT RULES-->


<!--MODE: SCHEMATRON-SELECT-FULL-PATH-->
<!--This mode can be used to generate an ugly though full XPath for locators-->
<xsl:template match="*" mode="schematron-select-full-path">
      <xsl:apply-templates select="." mode="schematron-get-full-path"/>
   </xsl:template>

   <!--MODE: SCHEMATRON-FULL-PATH-->
<!--This mode can be used to generate an ugly though full XPath for locators-->
<xsl:template match="*" mode="schematron-get-full-path">
      <xsl:apply-templates select="parent::*" mode="schematron-get-full-path"/>
      <xsl:text>/</xsl:text>
      <xsl:choose>
         <xsl:when test="namespace-uri()=''">
            <xsl:value-of select="name()"/>
            <xsl:variable name="p_1" select="1+    count(preceding-sibling::*[name()=name(current())])"/>
            <xsl:if test="$p_1&gt;1 or following-sibling::*[name()=name(current())]">[<xsl:value-of select="$p_1"/>]</xsl:if>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>*[local-name()='</xsl:text>
            <xsl:value-of select="local-name()"/>
            <xsl:text>' and namespace-uri()='</xsl:text>
            <xsl:value-of select="namespace-uri()"/>
            <xsl:text>']</xsl:text>
            <xsl:variable name="p_2"
                          select="1+   count(preceding-sibling::*[local-name()=local-name(current())])"/>
            <xsl:if test="$p_2&gt;1 or following-sibling::*[local-name()=local-name(current())]">[<xsl:value-of select="$p_2"/>]</xsl:if>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>
   <xsl:template match="@*" mode="schematron-get-full-path">
      <xsl:text>/</xsl:text>
      <xsl:choose>
         <xsl:when test="namespace-uri()=''">@<xsl:value-of select="name()"/>
         </xsl:when>
         <xsl:otherwise>
            <xsl:text>@*[local-name()='</xsl:text>
            <xsl:value-of select="local-name()"/>
            <xsl:text>' and namespace-uri()='</xsl:text>
            <xsl:value-of select="namespace-uri()"/>
            <xsl:text>']</xsl:text>
         </xsl:otherwise>
      </xsl:choose>
   </xsl:template>

   <!--MODE: SCHEMATRON-FULL-PATH-2-->
<!--This mode can be used to generate prefixed XPath for humans-->
<xsl:template match="node() | @*" mode="schematron-get-full-path-2">
      <xsl:for-each select="ancestor-or-self::*">
         <xsl:text>/</xsl:text>
         <xsl:value-of select="name(.)"/>
         <xsl:if test="preceding-sibling::*[name(.)=name(current())]">
            <xsl:text>[</xsl:text>
            <xsl:value-of select="count(preceding-sibling::*[name(.)=name(current())])+1"/>
            <xsl:text>]</xsl:text>
         </xsl:if>
      </xsl:for-each>
      <xsl:if test="not(self::*)">
         <xsl:text/>/@<xsl:value-of select="name(.)"/>
      </xsl:if>
   </xsl:template>

   <!--MODE: GENERATE-ID-FROM-PATH -->
<xsl:template match="/" mode="generate-id-from-path"/>
   <xsl:template match="text()" mode="generate-id-from-path">
      <xsl:apply-templates select="parent::*" mode="generate-id-from-path"/>
      <xsl:value-of select="concat('.text-', 1+count(preceding-sibling::text()), '-')"/>
   </xsl:template>
   <xsl:template match="comment()" mode="generate-id-from-path">
      <xsl:apply-templates select="parent::*" mode="generate-id-from-path"/>
      <xsl:value-of select="concat('.comment-', 1+count(preceding-sibling::comment()), '-')"/>
   </xsl:template>
   <xsl:template match="processing-instruction()" mode="generate-id-from-path">
      <xsl:apply-templates select="parent::*" mode="generate-id-from-path"/>
      <xsl:value-of select="concat('.processing-instruction-', 1+count(preceding-sibling::processing-instruction()), '-')"/>
   </xsl:template>
   <xsl:template match="@*" mode="generate-id-from-path">
      <xsl:apply-templates select="parent::*" mode="generate-id-from-path"/>
      <xsl:value-of select="concat('.@', name())"/>
   </xsl:template>
   <xsl:template match="*" mode="generate-id-from-path" priority="-0.5">
      <xsl:apply-templates select="parent::*" mode="generate-id-from-path"/>
      <xsl:text>.</xsl:text>
      <xsl:value-of select="concat('.',name(),'-',1+count(preceding-sibling::*[name()=name(current())]),'-')"/>
   </xsl:template>
   <!--MODE: SCHEMATRON-FULL-PATH-3-->
<!--This mode can be used to generate prefixed XPath for humans 
	(Top-level element has index)-->
<xsl:template match="node() | @*" mode="schematron-get-full-path-3">
      <xsl:for-each select="ancestor-or-self::*">
         <xsl:text>/</xsl:text>
         <xsl:value-of select="name(.)"/>
         <xsl:if test="parent::*">
            <xsl:text>[</xsl:text>
            <xsl:value-of select="count(preceding-sibling::*[name(.)=name(current())])+1"/>
            <xsl:text>]</xsl:text>
         </xsl:if>
      </xsl:for-each>
      <xsl:if test="not(self::*)">
         <xsl:text/>/@<xsl:value-of select="name(.)"/>
      </xsl:if>
   </xsl:template>

   <!--MODE: GENERATE-ID-2 -->
<xsl:template match="/" mode="generate-id-2">U</xsl:template>
   <xsl:template match="*" mode="generate-id-2" priority="2">
      <xsl:text>U</xsl:text>
      <xsl:number level="multiple" count="*"/>
   </xsl:template>
   <xsl:template match="node()" mode="generate-id-2">
      <xsl:text>U.</xsl:text>
      <xsl:number level="multiple" count="*"/>
      <xsl:text>n</xsl:text>
      <xsl:number count="node()"/>
   </xsl:template>
   <xsl:template match="@*" mode="generate-id-2">
      <xsl:text>U.</xsl:text>
      <xsl:number level="multiple" count="*"/>
      <xsl:text>_</xsl:text>
      <xsl:value-of select="string-length(local-name(.))"/>
      <xsl:text>_</xsl:text>
      <xsl:value-of select="translate(name(),':','.')"/>
   </xsl:template>
   <!--Strip characters--><xsl:template match="text()" priority="-1"/>

   <!--SCHEMA METADATA-->
<xsl:template match="/">
      <svrl:schematron-output xmlns:xs="http://www.w3.org/2001/XMLSchema"
                              xmlns:schold="http://www.ascc.net/xml/schematron"
                              xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                              title="Schema for validating MPDs"
                              schemaVersion="ISO19757-3">
         <xsl:comment>
            <xsl:value-of select="$archiveDirParameter"/>   
		 <xsl:value-of select="$archiveNameParameter"/>  
		 <xsl:value-of select="$fileNameParameter"/>  
		 <xsl:value-of select="$fileDirParameter"/>
         </xsl:comment>
         <svrl:ns-prefix-in-attribute-values uri="urn:mpeg:dash:schema:mpd:2011" prefix="dash"/>
         <svrl:ns-prefix-in-attribute-values uri="http://www.w3.org/1999/xlink" prefix="xlink"/>
         <svrl:ns-prefix-in-attribute-values uri="http://www.w3.org/2001/XMLSchema-instance" prefix="xsi"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M4"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M5"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M6"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M7"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M8"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M9"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M10"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M11"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M12"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M13"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M14"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M15"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M16"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M17"/>
         <svrl:active-pattern>
            <xsl:apply-templates/>
         </svrl:active-pattern>
         <xsl:apply-templates select="/" mode="M18"/>
      </svrl:schematron-output>
   </xsl:template>

   <!--SCHEMATRON PATTERNS-->
<svrl:text xmlns:xs="http://www.w3.org/2001/XMLSchema"
              xmlns:schold="http://www.ascc.net/xml/schematron"
              xmlns:svrl="http://purl.oclc.org/dsdl/svrl">Schema for validating MPDs</svrl:text>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:MPD" priority="1000" mode="M4">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:MPD"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@type = 'dynamic' and not(@availabilityStartTime)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@type = 'dynamic' and not(@availabilityStartTime)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If MPD is of type "dynamic" availabilityStartTime shall be defined.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@type = 'static' and @timeShiftBufferDepth) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@type = 'static' and @timeShiftBufferDepth) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If MPD is of type "static" timeShiftBufferDepth shall not be defined.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@type = 'static' and not(@mediaPresentationDuration)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@type = 'static' and not(@mediaPresentationDuration)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If MPD is of type "static" mediaPresentationDuration shall be defined.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@type = 'static' and descendant::dash:Period[1]/@start and (years-from-duration(descendant::dash:Period[1]/@start) + months-from-duration(descendant::dash:Period[1]/@start) + days-from-duration(descendant::dash:Period[1]/@start) + hours-from-duration(descendant::dash:Period[1]/@start) + minutes-from-duration(descendant::dash:Period[1]/@start) +  seconds-from-duration(descendant::dash:Period[1]/@start)) &gt; 0) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@type = 'static' and descendant::dash:Period[1]/@start and (years-from-duration(descendant::dash:Period[1]/@start) + months-from-duration(descendant::dash:Period[1]/@start) + days-from-duration(descendant::dash:Period[1]/@start) + hours-from-duration(descendant::dash:Period[1]/@start) + minutes-from-duration(descendant::dash:Period[1]/@start) + seconds-from-duration(descendant::dash:Period[1]/@start)) &gt; 0) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If MPD is of type "static" and the first period has a start attribute the start attribute shall be zero.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@mediaPresentationDuration) and not(@minimumUpdatePeriod)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@mediaPresentationDuration) and not(@minimumUpdatePeriod)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If mediaPresentationDuration is not defined for the MPD minimumUpdatePeriod shall be defined or vice versa.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@type = 'static' and @minimumUpdatePeriod) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@type = 'static' and @minimumUpdatePeriod) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If MPD is of type "static" minimumUpdatePeriod shall not be defined.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@profiles) or not(substring(@profiles,1,13) = 'urn:mpeg:dash') or (@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or @profiles = 'urn:mpeg:dash:profile:isoff-live:2011' or @profiles = 'urn:mpeg:dash:profile:isoff-main:2011' or @profiles = 'urn:mpeg:dash:profile:full:2011' or @profiles = 'urn:mpeg:dash:profile:mp2t-main:2011' or @profiles = 'urn:mpeg:dash:profile:mp2t-simple:2011')) then true() else false()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@profiles) or not(substring(@profiles,1,13) = 'urn:mpeg:dash') or (@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or @profiles = 'urn:mpeg:dash:profile:isoff-live:2011' or @profiles = 'urn:mpeg:dash:profile:isoff-main:2011' or @profiles = 'urn:mpeg:dash:profile:full:2011' or @profiles = 'urn:mpeg:dash:profile:mp2t-main:2011' or @profiles = 'urn:mpeg:dash:profile:mp2t-simple:2011')) then true() else false()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The On-Demand profile shall be identified by the URN "urn:mpeg:dash:profile:isoff-on-demand:2011". The live profile shall be identified by the URN "urn:mpeg:dash:profile:isoff-live:2011". The main profile shall be identified by the URN "urn:mpeg:dash:profile:isoff-main:2011". The full profile shall be identified by the URN "urn:mpeg:dash:profile:full:2011". The mp2t-main profile shall be identified by the URN "urn:mpeg:dash:profile:mp2t-main:2011". The mp2t-simple profile shall be identified by the URN "urn:mpeg:dash:profile:mp2t-simple:2011".</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011') or not(@type) or @type='static') then true() else false()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011') or not(@type) or @type='static') then true() else false()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>For On-Demand profile, the MPD @type shall be "static".</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M4"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M4"/>
   <xsl:template match="@*|node()" priority="-2" mode="M4">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M4"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:Period" priority="1000" mode="M5">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:Period"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (string(@bitstreamSwitching) = 'true' and string(child::dash:AdaptationSet/@bitstreamSwitching) = 'false') then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (string(@bitstreamSwitching) = 'true' and string(child::dash:AdaptationSet/@bitstreamSwitching) = 'false') then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If bitstreamSwitching is set to true all bitstreamSwitching declarations for AdaptationSet within this Period shall not be set to false.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@id = preceding::dash:Period/@id) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@id = preceding::dash:Period/@id) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The id of each Period shall be unique.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((years-from-duration(@start) + months-from-duration(@start) + days-from-duration(@start) + hours-from-duration(@start) + minutes-from-duration(@start) +  seconds-from-duration(@start)) &gt; (years-from-duration(following-sibling::dash:Period/@start) + months-from-duration(following-sibling::dash:Period/@start) + days-from-duration(following-sibling::dash:Period/@start) + hours-from-duration(following-sibling::dash:Period/@start) + minutes-from-duration(following-sibling::dash:Period/@start) +  seconds-from-duration(following-sibling::dash:Period/@start))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((years-from-duration(@start) + months-from-duration(@start) + days-from-duration(@start) + hours-from-duration(@start) + minutes-from-duration(@start) + seconds-from-duration(@start)) &gt; (years-from-duration(following-sibling::dash:Period/@start) + months-from-duration(following-sibling::dash:Period/@start) + days-from-duration(following-sibling::dash:Period/@start) + hours-from-duration(following-sibling::dash:Period/@start) + minutes-from-duration(following-sibling::dash:Period/@start) + seconds-from-duration(following-sibling::dash:Period/@start))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Periods shall be physically ordered in the MPD file in increasing order of their start time.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>At most one of SegmentBase, SegmentTemplate and SegmentList shall be defined in Period.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@id) and ancestor::dash:MPD/@type = 'dynamic') then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@id) and ancestor::dash:MPD/@type = 'dynamic') then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If the MPD is dynamic the Period element shall have an id.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(descendant-or-self::dash:BaseURL) and not(descendant-or-self::dash:SegmentTemplate) and not(descendant-or-self::dash:SegmentList)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(descendant-or-self::dash:BaseURL) and not(descendant-or-self::dash:SegmentTemplate) and not(descendant-or-self::dash:SegmentList)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>At least one BaseURL, SegmentTemplate or SegmentList shall be defined in Period, AdaptationSet or Representation.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M5"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M5"/>
   <xsl:template match="@*|node()" priority="-2" mode="M5">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M5"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:AdaptationSet" priority="1000" mode="M6">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:AdaptationSet"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@id = preceding-sibling::dash:AdaptationSet/@id) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@id = preceding-sibling::dash:AdaptationSet/@id) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The id of each AdaptationSet within a Period shall be unique.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@lang = descendant::dash:ContentComponent/@lang) or (@contentType = descendant::dash:ContentComponent/@contentType) or (@par = descendant::dash:ContentComponent/@par)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@lang = descendant::dash:ContentComponent/@lang) or (@contentType = descendant::dash:ContentComponent/@contentType) or (@par = descendant::dash:ContentComponent/@par)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Attributes from the AdaptationSet shall not be repeated in the descendanding ContentComponent elements.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@profiles and descendant::dash:Representation/@profiles) or (@width and descendant::dash:Representation/@width) or (@height and descendant::dash:Representation/@height) or (@sar and descendant::dash:Representation/@sar) or (@frameRate and descendant::dash:Representation/@frameRate) or (@audioSamplingRate and descendant::dash:Representation/@audioSamplingRate) or (@mimeType and descendant::dash:Representation/@mimeType) or (@segmentProfiles and descendant::dash:Representation/@segmentProfiles) or (@codecs and descendant::dash:Representation/@codecs) or (@maximumSAPPeriod and descendant::dash:Representation/@maximumSAPPeriod) or (@startWithSAP and descendant::dash:Representation/@startWithSAP) or (@maxPlayoutRate and descendant::dash:Representation/@maxPlayoutRate) or (@codingDependency and descendant::dash:Representation/@codingDependency) or (@scanType and descendant::dash:Representation/@scanType)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@profiles and descendant::dash:Representation/@profiles) or (@width and descendant::dash:Representation/@width) or (@height and descendant::dash:Representation/@height) or (@sar and descendant::dash:Representation/@sar) or (@frameRate and descendant::dash:Representation/@frameRate) or (@audioSamplingRate and descendant::dash:Representation/@audioSamplingRate) or (@mimeType and descendant::dash:Representation/@mimeType) or (@segmentProfiles and descendant::dash:Representation/@segmentProfiles) or (@codecs and descendant::dash:Representation/@codecs) or (@maximumSAPPeriod and descendant::dash:Representation/@maximumSAPPeriod) or (@startWithSAP and descendant::dash:Representation/@startWithSAP) or (@maxPlayoutRate and descendant::dash:Representation/@maxPlayoutRate) or (@codingDependency and descendant::dash:Representation/@codingDependency) or (@scanType and descendant::dash:Representation/@scanType)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Common attributes for AdaptationSet and Representation shall either be in one of the elements but not in both.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@minWidth &gt; @maxWidth) or (@minHeight &gt; @maxHeight) or (@minBandwidth &gt; @maxBandwidth)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@minWidth &gt; @maxWidth) or (@minHeight &gt; @maxHeight) or (@minBandwidth &gt; @maxBandwidth)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Each minimum value (minWidth, minHeight, minBandwidth) shall be larger than the maximum value.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (descendant::dash:Representation/@bandwidth &lt; @minBandwidth or descendant::dash:Representation/@bandwidth &gt; @maxBandwidth) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (descendant::dash:Representation/@bandwidth &lt; @minBandwidth or descendant::dash:Representation/@bandwidth &gt; @maxBandwidth) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of the bandwidth attribute shall be in the range defined by the AdaptationSet.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (descendant::dash:Representation/@width &lt; @minWidth or descendant::dash:Representation/@width &gt; @maxWidth) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (descendant::dash:Representation/@width &lt; @minWidth or descendant::dash:Representation/@width &gt; @maxWidth) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of the width attribute shall be in the range defined by the AdaptationSet.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (descendant::dash:Representation/@height &lt; @minHeight or descendant::dash:Representation/@height &gt; @maxHeight) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (descendant::dash:Representation/@height &lt; @minHeight or descendant::dash:Representation/@height &gt; @maxHeight) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of the height attribute shall be in the range defined by the AdaptationSet.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (count(child::dash:Representation)=0) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (count(child::dash:Representation)=0) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>An AdaptationSet shall have at least one Representation element.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or ancestor::dash:MPD/@profiles  = 'urn:mpeg:dash:profile:isoff-on-demand:2011' ) and (@mimeType and (@mimeType != 'video/mp4' and @mimeType != 'audio/mp4' and @mimeType != 'application/mp4'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or ancestor::dash:MPD/@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' ) and (@mimeType and (@mimeType != 'video/mp4' and @mimeType != 'audio/mp4' and @mimeType != 'application/mp4'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>For ISO BMFF On-Demand profile, the @mimeType shall equal video/mp4, audio/mp4 or application/mp4.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>At most one of SegmentBase, SegmentTemplate and SegmentList shall be defined in AdaptationSet.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M6"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M6"/>
   <xsl:template match="@*|node()" priority="-2" mode="M6">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M6"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:ContentComponent" priority="1000" mode="M7">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:ContentComponent"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@id = preceding-sibling::dash:ContentComponent/@id) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@id = preceding-sibling::dash:ContentComponent/@id) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The id of each ContentComponent within an AdaptationSet shall be unique.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M7"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M7"/>
   <xsl:template match="@*|node()" priority="-2" mode="M7">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M7"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:Representation" priority="1000" mode="M8">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:Representation"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@mimeType) and not(parent::dash:AdaptationSet/@mimeType)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@mimeType) and not(parent::dash:AdaptationSet/@mimeType)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Either the Representation or the containing AdaptationSet shall have the mimeType attribute.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or ancestor::dash:MPD/@profiles  = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or parent::dash:AdaptationSet/@profiles  = 'urn:mpeg:dash:profile:isoff-on-demand:2011') and (@mimeType and (@mimeType != 'video/mp4' and @mimeType != 'audio/mp4' and @mimeType != 'application/mp4'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or ancestor::dash:MPD/@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011' or parent::dash:AdaptationSet/@profiles = 'urn:mpeg:dash:profile:isoff-on-demand:2011') and (@mimeType and (@mimeType != 'video/mp4' and @mimeType != 'audio/mp4' and @mimeType != 'application/mp4'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>For ISO BMFF On-Demand profile, the @mimeType shall equal video/mp4, audio/mp4 or application/mp4.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(child::dash:SegmentTemplate or parent::dash:AdaptationSet/dash:SegmentTemplate or ancestor::dash:Period/dash:SegmentTemplate) and (@profiles = 'urn:mpeg:dash:profile:isoff-live:2011' or ancestor::dash:MPD/@profiles  = 'urn:mpeg:dash:profile:isoff-live:2011' or parent::dash:AdaptationSet/@profiles  = 'urn:mpeg:dash:profile:isoff-live:2011')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(child::dash:SegmentTemplate or parent::dash:AdaptationSet/dash:SegmentTemplate or ancestor::dash:Period/dash:SegmentTemplate) and (@profiles = 'urn:mpeg:dash:profile:isoff-live:2011' or ancestor::dash:MPD/@profiles = 'urn:mpeg:dash:profile:isoff-live:2011' or parent::dash:AdaptationSet/@profiles = 'urn:mpeg:dash:profile:isoff-live:2011')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>For live profile, the SegmentTemplate element shall be present on at least one of the three levels, the Period level containing the Representation, the Adaptation Set containing the Representation, or on Representation level itself.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((child::dash:SegmentBase and child::dash:SegmentTemplate and child::dash:SegmentList) or (child::dash:SegmentBase and child::dash:SegmentTemplate) or (child::dash:SegmentBase and child::dash:SegmentList) or (child::dash:SegmentTemplate and child::dash:SegmentList)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>At most one of SegmentBase, SegmentTemplate and SegmentList shall be defined in Representation.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M8"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M8"/>
   <xsl:template match="@*|node()" priority="-2" mode="M8">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M8"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:SubRepresentation" priority="1000" mode="M9">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:SubRepresentation"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@level and not(@bandwidth)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@level and not(@bandwidth)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If the level attribute is defined for a SubRepresentation also the bandwidth attribute shall be defined.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M9"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M9"/>
   <xsl:template match="@*|node()" priority="-2" mode="M9">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M9"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:SegmentTemplate" priority="1000" mode="M10">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:SegmentTemplate"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@duration) and not(child::dash:SegmentTimeline)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@duration) and not(child::dash:SegmentTimeline)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If more than one Media Segment is present the duration attribute or SegmentTimeline element shall be present.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@duration and child::dash:SegmentTimeline) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@duration and child::dash:SegmentTimeline) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Either the duration attribute or SegmentTimeline element shall be present but not both.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@indexRange) and @indexRangeExact) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@indexRange) and @indexRangeExact) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If indexRange is not present indexRangeExact shall not be present.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@initialization and (matches(@initialization, '\$Number(%.[^\$]*)?\$') or matches(@initialization, '\$Time(%.[^\$]*)?\$'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@initialization and (matches(@initialization, '\$Number(%.[^\$]*)?\$') or matches(@initialization, '\$Time(%.[^\$]*)?\$'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Neither $Number$ nor the $Time$ identifier shall be included in the initialization attribute.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@bitstreamSwitching and (matches(@bitstreamSwitching, '\$Number(%.[^\$]*)?\$') or matches(@bitstreamSwitching, '\$Time(%.[^\$]*)?\$'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@bitstreamSwitching and (matches(@bitstreamSwitching, '\$Number(%.[^\$]*)?\$') or matches(@bitstreamSwitching, '\$Time(%.[^\$]*)?\$'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Neither $Number$ nor the $Time$ identifier shall be included in the bitstreamSwitching attribute.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (matches(@media, '\$.[^\$]*\$')) then every $y in (for $x in tokenize(@media, '\$(Bandwidth|Time|Number|RepresentationID)(%.[^\$]*)?\$') return matches($x, '\$.[^\$]*\$')) satisfies $y eq false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (matches(@media, '\$.[^\$]*\$')) then every $y in (for $x in tokenize(@media, '\$(Bandwidth|Time|Number|RepresentationID)(%.[^\$]*)?\$') return matches($x, '\$.[^\$]*\$')) satisfies $y eq false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Only identifiers such as $Bandwidth$, $Time$, $RepresentationID$, or $Number$ shall be used.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (matches(@media, '\$RepresentationID%.[^\$]*\$')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (matches(@media, '\$RepresentationID%.[^\$]*\$')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>$RepresentationID$ shall not have a format tag.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M10"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M10"/>
   <xsl:template match="@*|node()" priority="-2" mode="M10">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M10"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:SegmentList" priority="1000" mode="M11">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:SegmentList"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@duration) and not(child::dash:SegmentTimeline)) then if (count(child::dash:SegmentURL) &gt; 1) then false() else true() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@duration) and not(child::dash:SegmentTimeline)) then if (count(child::dash:SegmentURL) &gt; 1) then false() else true() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If more than one Media Segment is present the duration attribute or SegmentTimeline element shall be present.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (@duration and child::dash:SegmentTimeline) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (@duration and child::dash:SegmentTimeline) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>Either the duration attribute or SegmentTimeline element shall be present but not both.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@indexRange) and @indexRangeExact) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@indexRange) and @indexRangeExact) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If indexRange is not present indexRangeExact shall not be present.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M11"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M11"/>
   <xsl:template match="@*|node()" priority="-2" mode="M11">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M11"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:SegmentBase" priority="1000" mode="M12">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:SegmentBase"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@indexRange) and @indexRangeExact) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@indexRange) and @indexRangeExact) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If indexRange is not present indexRangeExact shall not be present.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M12"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M12"/>
   <xsl:template match="@*|node()" priority="-2" mode="M12">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M12"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:SegmentTimeline" priority="1000" mode="M13">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:SegmentTimeline"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((if (ancestor::dash:*[1]/@timescale) then (child::dash:S/@d div ancestor::dash:*[1]/@timescale) else child::dash:S/@d) &gt; (years-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + months-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + days-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + hours-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + minutes-from-duration(ancestor::dash:MPD/@maxSegmentDuration) +  seconds-from-duration(ancestor::dash:MPD/@maxSegmentDuration))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((if (ancestor::dash:*[1]/@timescale) then (child::dash:S/@d div ancestor::dash:*[1]/@timescale) else child::dash:S/@d) &gt; (years-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + months-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + days-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + hours-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + minutes-from-duration(ancestor::dash:MPD/@maxSegmentDuration) + seconds-from-duration(ancestor::dash:MPD/@maxSegmentDuration))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The d attribute of a SegmentTimeline shall not exceed the value give bei the MPD maxSegmentDuration attribute.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M13"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M13"/>
   <xsl:template match="@*|node()" priority="-2" mode="M13">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M13"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:ProgramInformation" priority="1000" mode="M14">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:ProgramInformation"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (count(parent::dash:MPD/dash:ProgramInformation) &gt; 1 and not(@lang)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (count(parent::dash:MPD/dash:ProgramInformation) &gt; 1 and not(@lang)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>If more than one ProgramInformation element is given each ProgramInformation element shall have a lang attribute.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M14"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M14"/>
   <xsl:template match="@*|node()" priority="-2" mode="M14">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M14"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:ContentProtection" priority="1000" mode="M15">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:ContentProtection"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:mp4protection:2011') and not(string-length(@value) = 4)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:mp4protection:2011') and not(string-length(@value) = 4)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of ContentProtection shall be the 4CC contained in the Scheme Type Box</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:CA_descriptor:2011') and not(string-length(@value) = 4)) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:CA_descriptor:2011') and not(string-length(@value) = 4)) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of ContentProtection shall be the 4-digit lower-case hexadecimal Representation.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M15"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M15"/>
   <xsl:template match="@*|node()" priority="-2" mode="M15">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M15"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:FramePacking" priority="1000" mode="M16">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:FramePacking"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(contains(parent::dash:AdaptationSet/@codecs, 'avc') or contains(parent::dash:AdaptationSet/@codecs, 'svc') or contains(parent::dash:AdaptationSet/@codecs, 'mvc')) and not(contains(parent::dash:Representation/@codecs, 'avc') or contains(parent::dash:Representation/@codecs, 'svc') or contains(parent::dash:Representation/@codecs, 'mvc'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(contains(parent::dash:AdaptationSet/@codecs, 'avc') or contains(parent::dash:AdaptationSet/@codecs, 'svc') or contains(parent::dash:AdaptationSet/@codecs, 'mvc')) and not(contains(parent::dash:Representation/@codecs, 'avc') or contains(parent::dash:Representation/@codecs, 'svc') or contains(parent::dash:Representation/@codecs, 'mvc'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The URI urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011 is used for Adaptation Sets or Representations that contain a video component that conforms to ISO/IEC 14496-10.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011') and not(parent::dash:AdaptationSet/@mimeType = 'video/mp2t') and not(parent::dash:Representation/@mimeType = 'video/mp2t')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011') and not(parent::dash:AdaptationSet/@mimeType = 'video/mp2t') and not(parent::dash:Representation/@mimeType = 'video/mp2t')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The URI urn:mpeg:dash:13818:1:stereo_video_format_type:2011 is used for Adaptation Sets or Representations that contain a video component that conforms to ISO/IEC 13818-1.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(@value = '0' or @value = '1' or @value = '2' or @value = '3' or @value = '4' or @value = '5')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(@value = '0' or @value = '1' or @value = '2' or @value = '3' or @value = '4' or @value = '5')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of FramePacking shall be 0 (checkerboard interleaving), 1 (column interleaving), 2 (row interleaving), 3 (side-by-side), 4 (top-bottom) or 5 (temporal interleaving).</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011') and not(@value = '0000011' or @value = '0000100' or @value = '0001000')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011') and not(@value = '0000011' or @value = '0000100' or @value = '0001000')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of FramePacking shall be 0000011 (stereo side by side), 0000100 (stereo top and bottom) or 0001000 (2D video).</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@schemeIdUri = 'urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011') and not(@schemeIdUri = 'urn:mpeg:dash:13818:1:stereo_video_format_type:2011')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>schemeIdUri for FramePacking descriptor shall be urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011 or urn:mpeg:dash:13818:1:stereo_video_format_type:2011.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M16"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M16"/>
   <xsl:template match="@*|node()" priority="-2" mode="M16">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M16"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:AudioChannelConfiguration" priority="1000" mode="M17">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:AudioChannelConfiguration"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:23003:3:audio_channel_configuration:2011') and not(@value = '0' or @value = '1' or @value = '2' or @value = '3' or @value = '4' or @value = '5' or @value = '6' or @value = '7' or @value = '8' or @value = '9' or @value = '10' or @value = '11' or @value = '12' or @value = '13')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:23003:3:audio_channel_configuration:2011') and not(@value = '0' or @value = '1' or @value = '2' or @value = '3' or @value = '4' or @value = '5' or @value = '6' or @value = '7' or @value = '8' or @value = '9' or @value = '10' or @value = '11' or @value = '12' or @value = '13')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of AudioChannelConfiguration shall be 0, 1 (1/0.0),  2 (2/0.0), 3 (3/0.0), 4 (3/1.0), 5 (3/2.0), 6 (3/2.1), 7 (5/2.1), 8 (1+1), 9 (2/1.0), 10 (2/2.0), 11 (3/3.1), 12 (3/4.1) or 13 (11/11.2) [Front/Surr.LFE].</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@schemeIdUri = 'urn:mpeg:dash:23003:3:audio_channel_configuration:2011')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@schemeIdUri = 'urn:mpeg:dash:23003:3:audio_channel_configuration:2011')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>schemeIdUri for AudioChannelConfiguration descriptor shall be urn:mpeg:dash:23003:3:audio_channel_configuration:2011.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M17"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M17"/>
   <xsl:template match="@*|node()" priority="-2" mode="M17">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M17"/>
   </xsl:template>

   <!--PATTERN -->


	<!--RULE -->
<xsl:template match="dash:Role" priority="1000" mode="M18">
      <svrl:fired-rule xmlns:xs="http://www.w3.org/2001/XMLSchema"
                       xmlns:schold="http://www.ascc.net/xml/schematron"
                       xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                       context="dash:Role"/>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:role:2011') and not(@value = 'caption' or @value = 'subtitle' or @value = 'main' or @value = 'alternate' or @value = 'supplementary' or @value = 'commentary' or @value = 'dub')) then false else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:role:2011') and not(@value = 'caption' or @value = 'subtitle' or @value = 'main' or @value = 'alternate' or @value = 'supplementary' or @value = 'commentary' or @value = 'dub')) then false else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of Role (role) shall be caption, subtitle, main, alternate, supplementary, commentary or dub.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if ((@schemeIdUri = 'urn:mpeg:dash:stereoid:2011') and not(starts-with(@value, 'l') or starts-with(@value, 'r'))) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if ((@schemeIdUri = 'urn:mpeg:dash:stereoid:2011') and not(starts-with(@value, 'l') or starts-with(@value, 'r'))) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>The value of Role (stereoid) shall start with 'l' or 'r'.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>

		    <!--ASSERT -->
<xsl:choose>
         <xsl:when test="if (not(@schemeIdUri = 'urn:mpeg:dash:role:2011') and not(@schemeIdUri = 'urn:mpeg:dash:stereoid:2011')) then false() else true()"/>
         <xsl:otherwise>
            <svrl:failed-assert xmlns:xs="http://www.w3.org/2001/XMLSchema"
                                xmlns:schold="http://www.ascc.net/xml/schematron"
                                xmlns:svrl="http://purl.oclc.org/dsdl/svrl"
                                test="if (not(@schemeIdUri = 'urn:mpeg:dash:role:2011') and not(@schemeIdUri = 'urn:mpeg:dash:stereoid:2011')) then false() else true()">
               <xsl:attribute name="location">
                  <xsl:apply-templates select="." mode="schematron-get-full-path"/>
               </xsl:attribute>
               <svrl:text>schemeIdUri for Role descriptor shall be urn:mpeg:dash:role:2011 or urn:mpeg:dash:stereoid:2011.</svrl:text>
            </svrl:failed-assert>
         </xsl:otherwise>
      </xsl:choose>
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M18"/>
   </xsl:template>
   <xsl:template match="text()" priority="-1" mode="M18"/>
   <xsl:template match="@*|node()" priority="-2" mode="M18">
      <xsl:apply-templates select="*|comment()|processing-instruction()" mode="M18"/>
   </xsl:template>
</xsl:stylesheet>