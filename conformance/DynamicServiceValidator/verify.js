/*
For each selected represenation, the client needs to decide, LSN, SAST(SN),check update for MPD, and closer live edge
LSN=floor((NTP-(AST+PS)-d)/d)+SSN; //lastest segment number
SAST(SN)=AST+PS+(SN-SSN+1)*d; //segment availability start time for a segment
GSN=ceil((FT+MUP-(AST+PS)-d)/d)+SSN;  //greatest segment number to check the validity of an MPD
Scheduling a playout: by offering an MPD it is guaranteed that
  1.each segment in this period is available prior to its earliest presentation time, i.e,for all segments, EPT(SN)>=SAST(SN)-(AST+PS) 
  2.if each segment with segment number SN is delivered starting at SAST(SN) over a constant bitrate channel with bitrate equal to value of the @bandwidth attribute of 
    the Representation and taking into account the @minBufferTime value denoted as MBT then each presentation time PT is available at the client latest at time PT+(AST+PS)+MBT
  3.a recommended playout MPTS(PT) for a presentation time when operating in sync with other clients is MPTS(PT)=(AST+PS)+PT+SPD
  4.each segment in this period is available at least until SAST(SN)+TSBD+d

The typical sequence of operations when working with XMLHttpRequest is as follows:
1.Create an instance of the XMLHttpRequest object.
2.Use the XMLHttpRequest object to make an asynchronous call to a server page, defining a callback function that will be executed automatically when the server response is received.
3.Read the server's response in the callback function.
4.Update the web page using the data received from the server.
5.Go to step 2.
*/
/*****************************************************************************************************************************************************************************************************/

var MPD = {xmlHttpMPD: createXMLHttpRequestObject(), xmlData: null, FT: null, Periods: new Array()};

var maxPastSegments = 100;

function createXMLHttpRequestObject(){ 
  var xmlHttp; // xmlHttp will store the reference to the XMLHttpRequest object
  try{         // try to instantiate the native XMLHttpRequest object
    xmlHttp = new XMLHttpRequest(); // create an XMLHttpRequest object
  }
  catch(e) {
    try     // assume IE6 or older
    {
      xmlHttp = new ActiveXObject("Microsoft.XMLHttp");
    }
    catch(e) { }
  }
  if (!xmlHttp)       // return the created object or display an error message
    alert("Error creating the XMLHttpRequest object.");
  else 
    return xmlHttp;
}

// performs a server request and assigns a callback function

function process(){

  if (MPD.xmlHttpMPD)     // continue only if xmlHttp isn't void
  {
    try          // try to connect to the server
    {	
	  
	  var mpd_url=document.getElementById("foo").value;
	  MPD.xmlHttpMPD.open("GET", mpd_url, false);  // initiate server request
      MPD.xmlHttpMPD.onreadystatechange = handleRequestStateChange;
      MPD.xmlHttpMPD.send(null);
    }
    catch (e)      // display an error in case of failure
    {
      alert("Can't connect to server when request the MPD:\n" + e.toString());
    }
  }
}

function reqListener() {
	
    var statusReported = false;
    var totalSASRequestsDispatched = 0;
    var totalSASRequestsProcessed = 0;
    var totalSAERequestsDispatched = 0;
    var totalSAERequestsProcessed = 0;
    

    for(var periodIndex = 0; periodIndex < MPD.Periods.length ; periodIndex++)
    {
        var Period = MPD.Periods[periodIndex];
        
        for(var asIndex = 0; asIndex < Period.AdaptationSets.length ; asIndex++)
        {
            var AdaptationSet = Period.AdaptationSets[asIndex];
            
            for(var repIndex = 0; repIndex < AdaptationSet.Representations.length ; repIndex++)
            {
                var Representation = AdaptationSet.Representations[repIndex];
                var indexFound = false;
                var segmentIndex = 0;
                var requestType="";
                var indexFound = false;
                var segmentIndex = 0;
                var requestType="";
                var printString = "";

                for(var index = Representation.firstAvailableSsegment ; index <= Representation.GSN ; index++)
                {
                    if(Representation.Segments[index].SAS.xmlHttp == this)
                    {
                        segmentIndex = index;
                        indexFound = true;
                        requestType = "SAS";
                        break;
                    }

                    if(Representation.Segments[index].SAE.xmlHttp == this)
                    {
                        segmentIndex = index;
                        indexFound = true;
                        requestType = "SAE";
                        break;
                    }
                }

                if(!indexFound)
                {
                    totalSASRequestsDispatched += Representation.dispatchedSASRequests;
                    totalSASRequestsProcessed += Representation.processedSASRequests;
                    totalSAERequestsDispatched += Representation.dispatchedSAERequests;
                    totalSAERequestsProcessed += Representation.processedSAERequests;
                    continue;
                }
                else
                {
                    if(requestType == "SAS")
                    {
                        printString += "Segment Start check: "+ Representation.Segments[segmentIndex].url+", @SAST: "+ Representation.Segments[segmentIndex].SAS.time;
                        if(Representation.Segments[segmentIndex].SAS.deltaTime < 0)
                            printString += ", Prior available Segment";

                        Representation.processedSASRequests ++;
                    }
                    else
                    {
                        printString += "Segment End check: "+ Representation.Segments[segmentIndex].url+", @SAET: "+ Representation.Segments[segmentIndex].SAE.time;
                        Representation.processedSAERequests ++;
                    }

                }

                statusReported = true;

                if (this.status === 200)
                {
                    printString += '<span style="color:blue">'+", Status: "+this.statusText+'</span>'+"<br/>";						 
                }
                else
                {
                    printString += '<span style="color:red">'+", Status: "+this.statusText+'</span>'+"<br/>";						 
                }

                printOutput(printString);

                totalSASRequestsDispatched += Representation.dispatchedSASRequests;
                totalSASRequestsProcessed += Representation.processedSASRequests;
                totalSAERequestsDispatched += Representation.dispatchedSAERequests;
                totalSAERequestsProcessed += Representation.processedSAERequests;


                //if(statusReported)break;
            }
            
            //if(statusReported)break;
        }
        
        //if(statusReported)break;
    }

    var progress =document.getElementById('Progress');
    progress.innerHTML =totalSASRequestsProcessed + "/" + totalSASRequestsDispatched + " Segment Availabilty Start checks processed.<br/>";
    progress.innerHTML+=totalSAERequestsProcessed + "/" + totalSAERequestsDispatched + " Segment Availabilty End checks processed.";


    if(!statusReported)
        printOutput('<span style="color:red">'+"Segment report match could not be made!! <br/>");
};

function checkURL(check,xmlHttp)
{
    xmlHttp.onload = reqListener;
    xmlHttp.open("HEAD",check.url,true);   //Head, not get, we just need to check if segment is available, not get it
    xmlHttp.send(null);//if the request method is post, it will not be null
}

// function executed when the state of the request changes


function handleRequestStateChange(){
        
  if (MPD.xmlHttpMPD.readyState == 4){    // continue if the process is completed
    if (MPD.xmlHttpMPD.status == 200) {       // continue only if HTTP status is "OK"   
      try {
        MPD.FT = new Date();
        
        response = MPD.xmlHttpMPD.responseXML;          // retrieve the response
        
        var mpdOP =document.getElementById('MPDOutput');
        mpdOP.innerHTML="xmlHttpMPD.statusXML(response for MPD request):"+ MPD.xmlHttpMPD.status + mpdOP.innerHTML;						 
        
        // do something with the response
        MPD.xmlData = MPD.xmlHttpMPD.responseXML.documentElement;	
                
		processMPD(MPD.xmlData);

        dispatchChecks(MPD);
        		
      }
      catch(e)
      {
        alert("Error dispatching request for index: " + i + ", error: " + e.toString());          // display error message
      }
    } 
    else
    {
      alert("There was a problem retrieving the data (MPD):\n" + MPD.xmlHttpMPD.status);        // display status message
    }
  }
}

function dispatchChecks(mpd)
{
    for(var periodIndex = 0; periodIndex < MPD.Periods.length ; periodIndex++)
    {
        var Period = MPD.Periods[periodIndex];
        
        for(var asIndex = 0; asIndex < Period.AdaptationSets.length ; asIndex++)
        {
            var AdaptationSet = Period.AdaptationSets[asIndex];
            
            for(var repIndex = 0; repIndex < AdaptationSet.Representations.length ; repIndex++)
            {
                var Representation = AdaptationSet.Representations[repIndex];
                var SSN = Representation.SSN;
                var GSN = Representation.GSN;
                var pastSegments = 0;
                
                var now = new Date();
                
                for(var i = Representation.firstAvailableSsegment; i <= GSN ; i ++)
                    if(Math.ceil(Representation.Segments[i].SAS.time.getTime()-now.getTime()) < 0)
                        pastSegments ++;
                    else
                        break;

                //alert("To dispatch: " + (GSN - (SSN + Math.max(0,pastSegments-maxPastSegments))));
                
                for(var i = (Representation.firstAvailableSsegment + Math.max(0,pastSegments-maxPastSegments)) ; i <= GSN ; i ++)
                {           			
                   try
            	   	{	
                        var now = new Date();							 
                        Representation.Segments[i].SAS.deltaTime = Math.ceil(Representation.Segments[i].SAS.time.getTime()-now.getTime());
                        Representation.Segments[i].SAE.deltaTime = Math.ceil(Representation.Segments[i].SAE.time.getTime()-now.getTime());

                        if(Representation.Segments[i].SAE.deltaTime >= 0)   //Still some time to expiry of segment
                        {
                            Representation.Segments[i].SAS.xmlHttp = createXMLHttpRequestObject();
                            Representation.Segments[i].SAE.xmlHttp = createXMLHttpRequestObject();
                            
                            if(Representation.Segments[i].SAS.deltaTime > 0)  //Still some time to availabilty of segment
                            {
                                Representation.Segments[i].SAS.timeOutRet=setTimeout(checkURL, Representation.Segments[i].SAS.deltaTime,Representation.Segments[i],Representation.Segments[i].SAS.xmlHttp);
                            }						   						   
                            else
                            {	
                                checkURL(Representation.Segments[i],Representation.Segments[i].SAS.xmlHttp);						 
                            }
                            
                            Representation.dispatchedSASRequests ++;

                            Representation.Segments[i].SAE.timeOutRet=setTimeout(checkURL, Representation.Segments[i].SAE.deltaTime,Representation.Segments[i],Representation.Segments[i].SAE.xmlHttp);

                            Representation.dispatchedSAERequests ++;
                        }
            		}
                   catch(e)
                    {
                         printOutput("Exception while dispatching for segment: "+ 1+", Error: "+e.toString()+"<br/>");						 
                    }		              
                }

                //printOutput("Dispatched " + Representation.dispatchedSASRequests + " SAS reqs, " + Representation.dispatchedSAERequests + "  SAE reqs for AS " + asIndex + " Rep "+ repIndex + "<br/>");						 

            }
        }
    }
}

function processRepresentation(Representation, Period)
{
    var SegmentTemplate = getChildByTagName(Representation,"SegmentTemplate");
    
    if( SegmentTemplate != null )
        Representation.SegmentTemplate = SegmentTemplate;
    
    var init=Representation.SegmentTemplate.getAttribute("initialization");
    var media=Representation.SegmentTemplate.getAttribute("media");

    var SSN = Representation.SegmentTemplate.getAttribute("startNumber");

    if(SSN != null)
        Representation.SSN = parseInt(SSN);
    else
        Representation.SSN = 1;

    var mpd = MPD;
    
    //if timescale is not defined, then its default value is 1
    if(Representation.SegmentTemplate.getAttribute("timescale")){
      Representation.duration=parseFloat(Representation.SegmentTemplate.getAttribute("duration"))/parseFloat(Representation.SegmentTemplate.getAttribute("timescale"));
    }else{
      Representation.duration=parseFloat(Representation.SegmentTemplate.getAttribute("duration"));
    }

    var d = Representation.duration;

    // start to check from the start number, but we need to to calculate the GSN 
    var LSN            = Math.floor((mpd.FT.getTime()                            - ( getAST(mpd.xmlData).getTime() + Period.PeriodStart*1000 ) - d*1000 )/ (d*1000) ) +  Representation.SSN;  
    Representation.GSN = Math.ceil(( mpd.FT.getTime() + getMUP(mpd.xmlData)*1000 - ( getAST(mpd.xmlData).getTime() + Period.PeriodStart*1000 ) - d*1000 )/ (d*1000) ) +  Representation.SSN;
    Representation.firstAvailableSsegment = Math.max(LSN - Math.ceil(getTSBD(mpd.xmlData)+d)/d - 100, Representation.SSN);

    //printOutput("About to process: " + (Representation.GSN - Representation.SSN) + ", GSN: " + Representation.GSN + ", LSN: " + LSN + ", SSN: " + Representation.SSN + ", Earliest Avail: " + Representation.firstAvailableSsegment + "<br/>");
    //exit();

    var num=Representation.firstAvailableSsegment;//Representation.SSN;
    do{
        var urlObj = getBaseURL(mpd.xmlData)+((media.replace("$RepresentationID$",Representation.xmlData.getAttribute("id"))).replace("$Number$",num)).replace("$Bandwidth$",Representation.xmlData.getAttribute("bandwidth"));
        var sasObj = {time: new Date((getAST(mpd.xmlData).getTime()+Period.PeriodStart*1000+(num-Representation.SSN+1)*d*1000)), deltaTime: 0, timeOutRet : 0, xmlHttp: null};
        var saeObj = {time: new Date((getAST(mpd.xmlData).getTime()+Period.PeriodStart*1000+(num-Representation.SSN+1)*d*1000)+getTSBD(mpd.xmlData)*1000), deltaTime: 0, timeOutRet : 0, xmlHttp: null};
        Representation.Segments[num] = {SAS: sasObj, SAE: saeObj, segNum: num, url: urlObj};
        num++;

    }while(num <= Representation.GSN);
}

function processAdaptationSet(AdaptationSet,Period)
{
    var numRepresentations = AdaptationSet.xmlData.getElementsByTagName("Representation").length;

    var SegmentTemplate = getChildByTagName(AdaptationSet,"SegmentTemplate");
    
    if( SegmentTemplate != null )
        AdaptationSet.SegmentTemplate = SegmentTemplate;
    
    for(var repIndex = 0; repIndex < numRepresentations ; repIndex++)
    {
        //printOutput("Processing rep: " + (repIndex+1));
        AdaptationSet.Representations[repIndex] = {xmlData: AdaptationSet.xmlData.getElementsByTagName("Representation")[repIndex], SegmentTemplate: AdaptationSet.SegmentTemplate, Segments: new Array(), duration: 0, SSN: 0, GSN: 0, 
                                                    firstAvailableSsegment: 0, dispatchedSASRequests: 0, dispatchedSAERequests: 0, processedSASRequests: 0, processedSAERequests: 0}; 
        processRepresentation(AdaptationSet.Representations[repIndex],Period);
    }
}


function processPeriod(Period)
{
    Period.SegmentTemplate = getChildByTagName(Period,"SegmentTemplate");
    
    var numAdaptationSets = Period.xmlData.getElementsByTagName("AdaptationSet").length;

    var r1=/[-+]?[0-9]*\.?[0-9]+/g; //sometimes they are in the form of "PT5M8S"
    if(Period.xmlData.getAttribute("start")){
       var ps_string = Period.xmlData.getAttribute("start").match(r1);
       var ps_len=ps_string.length; 

       for(i=ps_len-1;i>=0;i--){
          Period.PeriodStart=Period.PeriodStart+parseFloat(ps_string[i])*Math.pow(60,ps_len-i-1);
       }
    }	

    for(var asIndex = 0; asIndex < numAdaptationSets ; asIndex++)
    {
        //printOutput("Processing AS: " + (asIndex+1));
        Period.AdaptationSets[asIndex] = {xmlData: Period.xmlData.getElementsByTagName("AdaptationSet")[asIndex], SegmentTemplate: Period.SegmentTemplate, Representations: new Array()}; 
        processAdaptationSet(Period.AdaptationSets[asIndex],Period);
    }
}

function processMPD(MPDxmlData)
{
    var numPeriods = MPDxmlData.getElementsByTagName("Period").length;
    
    if(numPeriods > 1)
        alert("Found " + numPeriods + "periods, current implementation will only handle the first one!");

    for(var periodIndex = 0; periodIndex < 1/*numPeriods*/ ; periodIndex++) //Currently only first period
    {
        MPD.Periods[periodIndex] = {xmlData: MPDxmlData.getElementsByTagName("Period")[periodIndex], PeriodStart: 0, SegmentTemplate: null, AdaptationSets: new Array()}; 
        processPeriod(MPD.Periods[periodIndex]);
    }

}

function getChildByTagName(parent,tagName)
{
    var matches = parent.xmlData.getElementsByTagName(tagName);

    for(var index = 0 ; index < matches.length ; index++)
    {
        if(matches[index].parentNode == parent.xmlData)
        {
            //alert("Found match for: " + tagName + ", parent: " + parent.xmlData.tagName);
            return matches[index];
        }
    }

    return null;
}

/******************************get MediaPresentationDuration******************************************/
function getMediaPresentationDuration(mpd){
//case 1:mediaPresentationDuration="PT9M57S"
var r1=/[-+]?[0-9]*\.?[0-9]+/g;
//Case 2:sometimes they are in the form of "PT7200S"
var MediaPresentationDuration=0;
//s.match(r):object, Number(s.match(r)):number  
if(mpd.getAttribute("mediaPresentationDuration")){
   var num_string=mpd.getAttribute("mediaPresentationDuration").match(r1);
   var num_len=(mpd.getAttribute("mediaPresentationDuration").match(r1)).length;   
   for(i=num_len-1;i>=0;i--){
	MediaPresentationDuration=MediaPresentationDuration+parseFloat(num_string[i])*Math.pow(60,num_len-i-1);
   }
}else{
   MediaPresentationDuration=100;
} 
return MediaPresentationDuration;
}

/******************find the URL(the top level baseURL)********************************/
function getBaseURL(mpd){
  var BaseURL1;
  if(mpd.getElementsByTagName("BaseURL")[0]){
    URLs=mpd.getElementsByTagName("BaseURL");
	BaseURL1=URLs.item(0).firstChild.data;
  }else{	
    var mpdurl=document.getElementById("foo").value;
    var n=mpdurl.lastIndexOf("/"); 
    BaseURL1= mpdurl.substring(0,n+1);  
  }
//alert("BaseURL:"+BaseURL1);
return BaseURL1; 
}

/************************* Period@start ****************************************/
// nomally the number of period is 1, but it can be multiple
function getPS(mpd){
var len=mpd.getElementsByTagName("Period").length;
var ps_string= new Array(len);
var ps= new Array(len);
for(var i=0; i<len;i ++){
    if(mpd.getElementsByTagName("Period")[i].getAttribute("start")){
       ps_string[i]=mpd.getElementsByTagName("Period")[i].getAttribute("start");  // ps[0]=PT0S, string, if not defined, then it is value if null
	 } else{
	   ps_string[i]="PT0S";
	 }
	var ps=parseFloat(ps_string);
    alert("ps[i]:"+ps[i]);
	alert("period:"+mpd.getElementsByTagName("Period")[i]);
}
return ps;
}

/******************get the attributes of MPD(root) element********************************/
// get fetch time(FT), which is normally earlier than MPD@availabilityStartTime
//MPD@availabilityStartTime, it is just a value
function getAST(mpd){
var ast_string=mpd.getAttribute("availabilityStartTime"); //string
var ast=new Date(ast_string);		
//alert("availabilityStartTime in milliseconds in MPD:"+ast); // need to check the type to make sure 

//in MPD:2013-06-26T11:19:00+00:00, but here it is 2013-06-26T13:19:00+00:00,exactly two hours ahead of that
//alert("type of availabilityStartTime:"+ typeof(ast)); //object

var ast1=Date.parse(ast_string);
//alert("availabilityStartTime in the MPD:"+ast1); // 12323243434
//alert("type of availabilityStartTime1:"+ typeof(ast1)); // number
return ast;
}
//MPD@timeShiftBufferDepth
function getTSBD(mpd){

var r1=/[-+]?[0-9]*\.?[0-9]+/g;
//Case 2:sometimes they are in the form of "PT7200S"
var tsbd=0;
//s.match(r):object, Number(s.match(r)):number  
if(mpd.getAttribute("timeShiftBufferDepth")){
   var tsbd_string=mpd.getAttribute("timeShiftBufferDepth").match(r1);
   var tsbd_len=tsbd_string.length;   
   for(i=tsbd_len-1;i>=0;i--){
	tsbd=tsbd+parseFloat(tsbd_string[i])*Math.pow(60,tsbd_len-i-1);
   }
}else{
   tsbd=86400000; //Lets stick to 10000 days for now, Infinity not working as expected;
}
//alert("tsbd:"+tsbd);
return tsbd;
}

// MPD@suggestedPresentationDelay, it is just a value
function getSPD(mpd){
var r1=/[-+]?[0-9]*\.?[0-9]+/g;
//Case 2:sometimes they are in the form of "PT7200S"
var spd=0;
//s.match(r):object, Number(s.match(r)):number  
if(mpd.getAttribute("suggestedPresentationDelay")){
   var spd_string=mpd.getAttribute("suggestedPresentationDelay").match(r1);
   var spd_len=spd_string.length;   
   for(i=spd_len-1;i>=0;i--){
	spd=spd+parseFloat(spd_string[i])*Math.pow(60,spd_len-i-1);
   }
}else{
   spd=5;
}
return spd;
}

 //MPD@minimumUpdatePeriod, it is just a value
function getMUP(mpd){
var r1=/[-+]?[0-9]*\.?[0-9]+/g;
//Case 2:sometimes they are in the form of "PT7200S"
var mup=0;
//s.match(r):object, Number(s.match(r)):number  
if(mpd.getAttribute("minimumUpdatePeriod")){
   var mup_string=mpd.getAttribute("minimumUpdatePeriod").match(r1);
   var mup_len=mup_string.length;   
   for(i=mup_len-1;i>=0;i--){
	mup=mup+parseFloat(mup_string[i])*Math.pow(60,mup_len-i-1);
   }
}else{
   mup=100;
}
return mup;
}
// MPD@minBufferTime, it is just a value
function getMBT(mpd){
var r1=/[-+]?[0-9]*\.?[0-9]+/g;
//Case 2:sometimes they are in the form of "PT7200S"
var mbt=0;
//s.match(r):object, Number(s.match(r)):number  
if(mpd.getAttribute("minBufferTime")){
   var mbt_string=mpd.getAttribute("minBufferTime").match(r1);
   var mbt_len=mbt_string.length;   
   for(i=mbt_len-1;i>=0;i--){
	mbt=mbt+parseFloat(mbt_string[i])*Math.pow(60,mbt_len-i-1);
   }
}else{
   mbt=10;
}
return mbt;
}

// MPD@publishTime  (i suppose this is somehow related to FT (wall-clock-time))
function getPT(mpd){
var pt;
if(mpd.getAttribute("publishTime")){
   var pt_string=mpd.getAttribute("publishTime");	
   var pt=new Date(parseInt(pt_string));	// convert 13223444343 to the same format as AST
   //alert("publishTime:"+pt); 
   //alert("type of publishTime:"+ typeof(pt)); // object
}else{
   pt=new Date();
}
return pt;
}

function windup()
{
}

function printOutput(string)
{
    theD =document.getElementById('SegmentOutput');
    theD.innerHTML=string + theD.innerHTML;						 
}

function exit( status ) {
    var i;

    if (typeof status === 'string') {
        alert(status);
    }

    window.addEventListener('error', function (e) {e.preventDefault();e.stopPropagation();}, false);

    var handlers = [
        'copy', 'cut', 'paste',
        'beforeunload', 'blur', 'change', 'click', 'contextmenu', 'dblclick', 'focus', 'keydown', 'keypress', 'keyup', 'mousedown', 'mousemove', 'mouseout', 'mouseover', 'mouseup', 'resize', 'scroll',
        'DOMNodeInserted', 'DOMNodeRemoved', 'DOMNodeRemovedFromDocument', 'DOMNodeInsertedIntoDocument', 'DOMAttrModified', 'DOMCharacterDataModified', 'DOMElementNameChanged', 'DOMAttributeNameChanged', 'DOMActivate', 'DOMFocusIn', 'DOMFocusOut', 'online', 'offline', 'textInput',
        'abort', 'close', 'dragdrop', 'load', 'paint', 'reset', 'select', 'submit', 'unload'
    ];

    function stopPropagation (e) {
        e.stopPropagation();
    }
    for (i=0; i < handlers.length; i++) {
        window.addEventListener(handlers[i], function (e) {stopPropagation(e);}, true);
    }

    if (window.stop) {
        window.stop();
    }

    throw '';
}


