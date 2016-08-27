<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.4">
<drawing>
<settings>
<setting alwaysvectorfont="yes"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="101" name="LcdOutline" color="7" fill="1" visible="yes" active="yes"/>
<layer number="250" name="Descript" color="7" fill="1" visible="yes" active="yes"/>
<layer number="251" name="SMDround" color="7" fill="1" visible="yes" active="yes"/>
</layers>
<schematic>
<libraries>
<library name="frames">
<packages>
</packages>
<symbols>
<symbol name="A4L-LOC">
<wire x1="256.54" y1="3.81" x2="256.54" y2="8.89" width="0.1016" layer="94"/>
<wire x1="256.54" y1="8.89" x2="256.54" y2="13.97" width="0.1016" layer="94"/>
<wire x1="256.54" y1="13.97" x2="256.54" y2="19.05" width="0.1016" layer="94"/>
<wire x1="256.54" y1="19.05" x2="256.54" y2="24.13" width="0.1016" layer="94"/>
<wire x1="161.29" y1="3.81" x2="161.29" y2="24.13" width="0.1016" layer="94"/>
<wire x1="161.29" y1="24.13" x2="215.265" y2="24.13" width="0.1016" layer="94"/>
<wire x1="215.265" y1="24.13" x2="256.54" y2="24.13" width="0.1016" layer="94"/>
<wire x1="246.38" y1="3.81" x2="246.38" y2="8.89" width="0.1016" layer="94"/>
<wire x1="246.38" y1="8.89" x2="256.54" y2="8.89" width="0.1016" layer="94"/>
<wire x1="246.38" y1="8.89" x2="215.265" y2="8.89" width="0.1016" layer="94"/>
<wire x1="215.265" y1="8.89" x2="215.265" y2="3.81" width="0.1016" layer="94"/>
<wire x1="215.265" y1="8.89" x2="215.265" y2="13.97" width="0.1016" layer="94"/>
<wire x1="215.265" y1="13.97" x2="256.54" y2="13.97" width="0.1016" layer="94"/>
<wire x1="215.265" y1="13.97" x2="215.265" y2="19.05" width="0.1016" layer="94"/>
<wire x1="215.265" y1="19.05" x2="256.54" y2="19.05" width="0.1016" layer="94"/>
<wire x1="215.265" y1="19.05" x2="215.265" y2="24.13" width="0.1016" layer="94"/>
<text x="217.17" y="15.24" size="2.54" layer="94" font="vector">&gt;DRAWING_NAME</text>
<text x="217.17" y="10.16" size="2.286" layer="94" font="vector">&gt;LAST_DATE_TIME</text>
<text x="230.505" y="5.08" size="2.54" layer="94" font="vector">&gt;SHEET</text>
<text x="216.916" y="4.953" size="2.54" layer="94" font="vector">Sheet:</text>
<frame x1="0" y1="0" x2="260.35" y2="179.07" columns="6" rows="4" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="A4L-LOC" prefix="FRAME" uservalue="yes">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A4, landscape with location and doc. field</description>
<gates>
<gate name="G$1" symbol="A4L-LOC" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="  merged">
<packages>
<package name="VQFP-100">
<wire x1="-7" y1="7" x2="7" y2="7" width="0.254" layer="21"/>
<wire x1="7" y1="7" x2="7" y2="-7" width="0.254" layer="21"/>
<wire x1="7" y1="-7" x2="-7" y2="-7" width="0.254" layer="21"/>
<wire x1="-7" y1="-7" x2="-7" y2="7" width="0.254" layer="21"/>
<circle x="-5" y="5" radius="1" width="0.254" layer="21"/>
<smd name="26" x="-6" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="27" x="-5.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="28" x="-5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="29" x="-4.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="30" x="-4" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="31" x="-3.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="32" x="-3" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="33" x="-2.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="34" x="-2" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="35" x="-1.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="36" x="-1" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="37" x="-0.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="38" x="0" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="39" x="0.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="40" x="1" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="41" x="1.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="42" x="2" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="43" x="2.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="44" x="3" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="45" x="3.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="46" x="4" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="47" x="4.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="48" x="5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="49" x="5.5" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="50" x="6" y="-8" dx="0.25" dy="1.5" layer="1"/>
<smd name="100" x="-6" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="99" x="-5.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="98" x="-5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="97" x="-4.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="96" x="-4" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="95" x="-3.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="94" x="-3" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="93" x="-2.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="92" x="-2" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="91" x="-1.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="90" x="-1" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="89" x="-0.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="88" x="0" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="87" x="0.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="86" x="1" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="85" x="1.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="84" x="2" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="83" x="2.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="82" x="3" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="81" x="3.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="80" x="4" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="79" x="4.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="78" x="5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="77" x="5.5" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="76" x="6" y="8" dx="0.25" dy="1.5" layer="1"/>
<smd name="51" x="8" y="-6" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="52" x="8" y="-5.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="53" x="8" y="-5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="54" x="8" y="-4.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="55" x="8" y="-4" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="56" x="8" y="-3.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="57" x="8" y="-3" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="58" x="8" y="-2.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="59" x="8" y="-2" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="60" x="8" y="-1.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="61" x="8" y="-1" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="62" x="8" y="-0.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="63" x="8" y="0" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="64" x="8" y="0.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="65" x="8" y="1" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="66" x="8" y="1.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="67" x="8" y="2" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="68" x="8" y="2.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="69" x="8" y="3" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="70" x="8" y="3.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="71" x="8" y="4" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="72" x="8" y="4.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="73" x="8" y="5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="74" x="8" y="5.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="75" x="8" y="6" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="25" x="-8" y="-6" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="24" x="-8" y="-5.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="23" x="-8" y="-5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="22" x="-8" y="-4.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="21" x="-8" y="-4" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="20" x="-8" y="-3.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="19" x="-8" y="-3" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="18" x="-8" y="-2.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="17" x="-8" y="-2" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="16" x="-8" y="-1.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="15" x="-8" y="-1" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="14" x="-8" y="-0.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="13" x="-8" y="0" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="12" x="-8" y="0.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="11" x="-8" y="1" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="10" x="-8" y="1.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="9" x="-8" y="2" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="8" x="-8" y="2.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="7" x="-8" y="3" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="6" x="-8" y="3.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="5" x="-8" y="4" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="4" x="-8" y="4.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="3" x="-8" y="5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="2" x="-8" y="5.5" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<smd name="1" x="-8" y="6" dx="0.25" dy="1.25" layer="1" rot="R90"/>
<text x="6" y="-4" size="1.9304" layer="21" ratio="14" rot="R180">&gt;NAME</text>
</package>
<package name="LQFP-64">
<wire x1="-0.25" y1="11.5" x2="-1.75" y2="11.5" width="0.2" layer="21"/>
<wire x1="-1.75" y1="11.5" x2="-1.75" y2="10" width="0.2" layer="21"/>
<wire x1="-0.25" y1="0.5" x2="-1.75" y2="0.5" width="0.2" layer="21"/>
<wire x1="-1.75" y1="2" x2="-1.75" y2="0.5" width="0.2" layer="21"/>
<wire x1="9.25" y1="0.5" x2="7.75" y2="0.5" width="0.2" layer="21"/>
<wire x1="9.25" y1="2" x2="9.25" y2="0.5" width="0.2" layer="21"/>
<wire x1="9.25" y1="11.5" x2="7.75" y2="11.5" width="0.2" layer="21"/>
<wire x1="9.25" y1="11.5" x2="9.25" y2="10" width="0.2" layer="21"/>
<circle x="-0.3467" y="1.8033" radius="0.3807" width="0.2" layer="21"/>
<smd name="48" x="0" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="47" x="0.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="46" x="1" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="45" x="1.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="44" x="2" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="43" x="2.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="42" x="3" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="41" x="3.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="40" x="4" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="39" x="4.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="38" x="5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="37" x="5.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="36" x="6" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="35" x="6.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="34" x="7" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="33" x="7.5" y="12" dx="0.25" dy="2" layer="1"/>
<smd name="1" x="0" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="2" x="0.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="3" x="1" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="4" x="1.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="5" x="2" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="6" x="2.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="7" x="3" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="8" x="3.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="9" x="4" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="10" x="4.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="11" x="5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="12" x="5.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="13" x="6" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="14" x="6.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="15" x="7" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="16" x="7.5" y="0" dx="0.25" dy="2" layer="1"/>
<smd name="64" x="-2.25" y="2.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="63" x="-2.25" y="2.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="62" x="-2.25" y="3.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="61" x="-2.25" y="3.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="60" x="-2.25" y="4.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="59" x="-2.25" y="4.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="58" x="-2.25" y="5.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="57" x="-2.25" y="5.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="56" x="-2.25" y="6.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="55" x="-2.25" y="6.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="54" x="-2.25" y="7.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="53" x="-2.25" y="7.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="52" x="-2.25" y="8.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="51" x="-2.25" y="8.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="50" x="-2.25" y="9.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="49" x="-2.25" y="9.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="17" x="9.75" y="2.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="18" x="9.75" y="2.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="19" x="9.75" y="3.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="20" x="9.75" y="3.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="21" x="9.75" y="4.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="22" x="9.75" y="4.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="23" x="9.75" y="5.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="24" x="9.75" y="5.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="25" x="9.75" y="6.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="26" x="9.75" y="6.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="27" x="9.75" y="7.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="28" x="9.75" y="7.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="29" x="9.75" y="8.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="30" x="9.75" y="8.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="31" x="9.75" y="9.25" dx="0.25" dy="2" layer="1" rot="R90"/>
<smd name="32" x="9.75" y="9.75" dx="0.25" dy="2" layer="1" rot="R90"/>
<text x="0.5376" y="1.4397" size="1.27" layer="21">1</text>
<text x="-0.3641" y="9.155" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="RLC_0805">
<wire x1="-1.651" y1="0.9144" x2="1.651" y2="0.9144" width="0.254" layer="21"/>
<wire x1="1.651" y1="0.9144" x2="1.651" y2="-0.9144" width="0.254" layer="21"/>
<wire x1="1.651" y1="-0.9144" x2="-1.651" y2="-0.9144" width="0.254" layer="21"/>
<wire x1="-1.651" y1="-0.9144" x2="-1.651" y2="0.9144" width="0.254" layer="21"/>
<smd name="1" x="-0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<text x="-1.4732" y="-0.635" size="1.27" layer="51">&gt;NAME</text>
</package>
<package name="RLC_1210">
<wire x1="-2.4638" y1="1.6764" x2="2.4638" y2="1.6764" width="0.3048" layer="21"/>
<wire x1="2.4638" y1="1.6764" x2="2.4638" y2="-1.651" width="0.3048" layer="21"/>
<wire x1="2.4638" y1="-1.651" x2="-2.4638" y2="-1.651" width="0.3048" layer="21"/>
<wire x1="-2.4638" y1="-1.651" x2="-2.4638" y2="1.6764" width="0.3048" layer="21"/>
<smd name="1" x="-1.5" y="0" dx="1.5" dy="2.9" layer="1"/>
<smd name="2" x="1.5" y="0" dx="1.5" dy="2.9" layer="1"/>
<text x="-2.2096" y="-0.736" size="1.524" layer="51">&gt;NAME</text>
</package>
<package name="XTAL-SMD-CITIZEN-CS10">
<wire x1="-0.75" y1="1.65" x2="5.25" y2="1.65" width="0.127" layer="21"/>
<wire x1="-0.75" y1="-1.65" x2="5.25" y2="-1.65" width="0.127" layer="21"/>
<smd name="A" x="-0.2" y="0" dx="2.4" dy="2.4" layer="1"/>
<smd name="B" x="4.7" y="0" dx="2.4" dy="2.4" layer="1"/>
<text x="2.589" y="-1.44" size="1.016" layer="21" rot="R90">&gt;NAME</text>
</package>
<package name="HIROSE-MQ172-4POS">
<wire x1="3.6" y1="0" x2="-3.6" y2="0" width="0.127" layer="21"/>
<wire x1="-3.6" y1="1.7" x2="-3.6" y2="0" width="0.127" layer="21"/>
<wire x1="3.6" y1="1.7" x2="3.6" y2="0" width="0.127" layer="21"/>
<wire x1="-3.6" y1="1.7" x2="-3.2" y2="1.7" width="0.127" layer="21"/>
<wire x1="3.6" y1="1.7" x2="3.2" y2="1.7" width="0.127" layer="21"/>
<smd name="P$1" x="-3.85" y="4.45" dx="2.3" dy="3.8" layer="1"/>
<smd name="P$2" x="3.85" y="4.45" dx="2.3" dy="3.8" layer="1"/>
<smd name="3" x="0.4" y="6.75" dx="0.55" dy="2.5" layer="1"/>
<smd name="2" x="-0.4" y="6.75" dx="0.55" dy="2.5" layer="1"/>
<smd name="1" x="-1.2" y="6.75" dx="0.55" dy="2.5" layer="1"/>
<smd name="4" x="1.2" y="6.75" dx="0.55" dy="2.5" layer="1"/>
<text x="-2.7918" y="0.2997" size="1.27" layer="21">&gt;NAME</text>
<hole x="-1.5" y="3.6" drill="1"/>
<hole x="1.5" y="3.6" drill="1"/>
</package>
<package name="2512">
<wire x1="-1.27" y1="1.905" x2="7.239" y2="1.905" width="0.127" layer="21"/>
<wire x1="7.239" y1="1.905" x2="7.239" y2="-1.905" width="0.127" layer="21"/>
<wire x1="7.239" y1="-1.905" x2="-1.27" y2="-1.905" width="0.127" layer="21"/>
<wire x1="-1.27" y1="-1.905" x2="-1.27" y2="1.905" width="0.127" layer="21"/>
<smd name="P$1" x="0" y="0" dx="2.15" dy="3.5" layer="1"/>
<smd name="P$2" x="5.95" y="0" dx="2.15" dy="3.5" layer="1"/>
<text x="-1.016" y="2.159" size="1.27" layer="21">&gt;NAME</text>
</package>
<package name="HEADER-MALE-10X2-0.100-SHROUDED">
<wire x1="-4.572" y1="16.51" x2="4.572" y2="16.51" width="0.254" layer="21"/>
<wire x1="-4.572" y1="-16.51" x2="4.572" y2="-16.51" width="0.254" layer="21"/>
<wire x1="-4.572" y1="16.51" x2="-4.572" y2="-16.51" width="0.254" layer="21"/>
<wire x1="4.572" y1="16.51" x2="4.572" y2="-16.51" width="0.254" layer="21"/>
<pad name="1" x="-1.27" y="11.43" drill="1.143" shape="square"/>
<pad name="2" x="1.27" y="11.43" drill="1.143"/>
<pad name="3" x="-1.27" y="8.89" drill="1.143"/>
<pad name="4" x="1.27" y="8.89" drill="1.143"/>
<pad name="5" x="-1.27" y="6.35" drill="1.143"/>
<pad name="6" x="1.27" y="6.35" drill="1.143"/>
<pad name="7" x="-1.27" y="3.81" drill="1.143"/>
<pad name="8" x="1.27" y="3.81" drill="1.143"/>
<pad name="9" x="-1.27" y="1.27" drill="1.143"/>
<pad name="10" x="1.27" y="1.27" drill="1.143"/>
<pad name="11" x="-1.27" y="-1.27" drill="1.143"/>
<pad name="12" x="1.27" y="-1.27" drill="1.143"/>
<pad name="13" x="-1.27" y="-3.81" drill="1.143"/>
<pad name="14" x="1.27" y="-3.81" drill="1.143"/>
<pad name="15" x="-1.27" y="-6.35" drill="1.143"/>
<pad name="16" x="1.27" y="-6.35" drill="1.143"/>
<pad name="17" x="-1.27" y="-8.89" drill="1.143"/>
<pad name="18" x="1.27" y="-8.89" drill="1.143"/>
<pad name="19" x="-1.27" y="-11.43" drill="1.143"/>
<pad name="20" x="1.27" y="-11.43" drill="1.143"/>
<text x="-3.175" y="14.605" size="1.27" layer="21" ratio="17">&gt;NAME</text>
</package>
<package name="TSSOP-24">
<wire x1="-3.3" y1="-2.2" x2="-3.05" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-3.05" y1="-2.2" x2="-2.8" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-2.8" y1="-2.2" x2="4.6" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-3.3" y1="2.2" x2="-3.05" y2="2.2" width="0.254" layer="21"/>
<wire x1="-3.05" y1="2.2" x2="-2.8" y2="2.2" width="0.254" layer="21"/>
<wire x1="-2.8" y1="2.2" x2="-2.55" y2="2.2" width="0.254" layer="21"/>
<wire x1="-2.55" y1="2.2" x2="4.6" y2="2.2" width="0.254" layer="21"/>
<wire x1="-3.3" y1="2.2" x2="-3.3" y2="-2.2" width="0.254" layer="21"/>
<wire x1="4.6" y1="2.2" x2="4.6" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-2.8" y1="2.2" x2="-2.8" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-3.05" y1="2.2" x2="-3.05" y2="-2.2" width="0.254" layer="21"/>
<wire x1="-2.55" y1="2.2" x2="-2.55" y2="-2.2" width="0.254" layer="21"/>
<smd name="1" x="-2.925" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="2" x="-2.275" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="3" x="-1.625" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="4" x="-0.975" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="5" x="-0.325" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="6" x="0.325" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="7" x="0.975" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="8" x="1.625" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="9" x="2.275" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="10" x="2.925" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="20" x="-0.325" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="19" x="0.325" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="18" x="0.975" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="17" x="1.625" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="16" x="2.275" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="15" x="2.925" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="14" x="3.575" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="13" x="4.225" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="11" x="3.575" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="12" x="4.225" y="-3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="21" x="-0.975" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="22" x="-1.625" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="23" x="-2.275" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<smd name="24" x="-2.925" y="3.2" dx="0.3" dy="1.5" layer="1"/>
<text x="-2" y="0.5" size="1.27" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="RLC_0603">
<wire x1="-1.4732" y1="0.6858" x2="1.4732" y2="0.6858" width="0.2032" layer="21"/>
<wire x1="1.4732" y1="0.6858" x2="1.4732" y2="-0.6858" width="0.2032" layer="21"/>
<wire x1="1.4732" y1="-0.6858" x2="-1.4732" y2="-0.6858" width="0.2032" layer="21"/>
<wire x1="-1.4732" y1="-0.6858" x2="-1.4732" y2="0.6858" width="0.2032" layer="21"/>
<smd name="1" x="-0.85" y="0" dx="1" dy="1.1" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1" dy="1.1" layer="1"/>
<text x="-1.1938" y="-0.4064" size="0.762" layer="51">&gt;NAME</text>
</package>
<package name="MSOP8">
<wire x1="2.4" y1="4.2" x2="-0.4" y2="4.2" width="0.127" layer="21"/>
<wire x1="-0.4" y1="4.2" x2="-0.6" y2="4" width="0.127" layer="21"/>
<wire x1="-0.6" y1="4" x2="-0.6" y2="0.8" width="0.127" layer="21"/>
<wire x1="-0.6" y1="0.8" x2="-0.4" y2="0.6" width="0.127" layer="21"/>
<wire x1="-0.4" y1="0.6" x2="2.4" y2="0.6" width="0.127" layer="21"/>
<wire x1="2.4" y1="0.6" x2="2.6" y2="0.8" width="0.127" layer="21"/>
<wire x1="2.6" y1="0.8" x2="2.6" y2="4" width="0.127" layer="21"/>
<wire x1="2.6" y1="4" x2="2.4" y2="4.2" width="0.127" layer="21"/>
<circle x="0" y="1.2" radius="0.2" width="0.127" layer="21"/>
<smd name="1" x="0" y="0" dx="0.41" dy="1.02" layer="1"/>
<smd name="2" x="0.65" y="0" dx="0.41" dy="1.02" layer="1"/>
<smd name="3" x="1.3" y="0" dx="0.41" dy="1.02" layer="1"/>
<smd name="4" x="1.95" y="0" dx="0.41" dy="1.02" layer="1"/>
<smd name="8" x="0" y="4.8" dx="0.41" dy="1.02" layer="1"/>
<smd name="7" x="0.65" y="4.8" dx="0.41" dy="1.02" layer="1"/>
<smd name="6" x="1.3" y="4.8" dx="0.41" dy="1.02" layer="1"/>
<smd name="5" x="1.95" y="4.8" dx="0.41" dy="1.02" layer="1"/>
<text x="2.8" y="4.6" size="1.27" layer="25" rot="R270">&gt;NAME</text>
</package>
<package name="TACTSWITCH">
<wire x1="-1.27" y1="3.175" x2="6.35" y2="3.175" width="0.127" layer="21"/>
<wire x1="6.35" y1="3.175" x2="6.35" y2="-3.175" width="0.127" layer="21"/>
<wire x1="6.35" y1="-3.175" x2="-1.27" y2="-3.175" width="0.127" layer="21"/>
<wire x1="-1.27" y1="-3.175" x2="-1.27" y2="3.175" width="0.127" layer="21"/>
<pad name="P$1" x="0" y="0" drill="0.8128" diameter="1.6764"/>
<pad name="P$2" x="5.08" y="0" drill="0.8128" diameter="1.6764"/>
</package>
<package name="TACTSWITCH-SMD-EVQQ1">
<wire x1="-4.25" y1="5" x2="-3.2" y2="5" width="0.3048" layer="21"/>
<wire x1="3.2" y1="5" x2="4.25" y2="5" width="0.3048" layer="21"/>
<wire x1="-4.25" y1="-5" x2="-3.2" y2="-5" width="0.3048" layer="21"/>
<wire x1="3.2" y1="-5" x2="4.25" y2="-5" width="0.3048" layer="21"/>
<wire x1="-4.25" y1="5" x2="-4.25" y2="-5" width="0.3048" layer="21"/>
<wire x1="4.25" y1="5" x2="4.25" y2="-5" width="0.3048" layer="21"/>
<wire x1="-1.34" y1="5" x2="1.31" y2="5" width="0.3048" layer="21"/>
<wire x1="-1.34" y1="-5" x2="1.31" y2="-5" width="0.3048" layer="21"/>
<smd name="A" x="-2.25" y="4.5" dx="1.5" dy="3" layer="1"/>
<smd name="P$2" x="2.25" y="4.5" dx="1.5" dy="3" layer="1"/>
<smd name="P$3" x="-2.25" y="-4.5" dx="1.5" dy="3" layer="1"/>
<smd name="B" x="2.25" y="-4.5" dx="1.5" dy="3" layer="1"/>
<text x="-3.81" y="1.27" size="1.27" layer="21" ratio="13">&gt;NAME</text>
</package>
<package name="CAPCAITOR-ELECTROLYTIC-ALCHIP-MZA-F80">
<wire x1="-3.6" y1="3.3" x2="2.4" y2="3.3" width="0.3048" layer="21"/>
<wire x1="-3.6" y1="-3.3" x2="2.4" y2="-3.3" width="0.3048" layer="21"/>
<wire x1="-3.6" y1="3.3" x2="-3.6" y2="1" width="0.3048" layer="21"/>
<wire x1="-3.6" y1="-1" x2="-3.6" y2="-3.3" width="0.3048" layer="21"/>
<wire x1="3.6" y1="2.5" x2="3.6" y2="1" width="0.3048" layer="21"/>
<wire x1="3.6" y1="-1" x2="3.6" y2="-2.5" width="0.3048" layer="21"/>
<wire x1="2.5" y1="-3.3" x2="3.6" y2="-2.5" width="0.3048" layer="21"/>
<wire x1="2.4" y1="3.3" x2="3.6" y2="2.5" width="0.3048" layer="21"/>
<smd name="+" x="2.75" y="0" dx="3.5" dy="1.5" layer="1"/>
<smd name="-" x="-2.75" y="0" dx="3.5" dy="1.5" layer="1"/>
<text x="1.84" y="1.905" size="1.27" layer="21" ratio="16">+</text>
<text x="-3" y="-2.5" size="1.27" layer="21" ratio="16">&gt;NAME</text>
</package>
<package name="LED_0603">
<wire x1="-1.4732" y1="0.6858" x2="1.4732" y2="0.6858" width="0.2032" layer="21"/>
<wire x1="1.4732" y1="0.6858" x2="1.4732" y2="-0.6858" width="0.2032" layer="21"/>
<wire x1="1.4732" y1="-0.6858" x2="-1.4732" y2="-0.6858" width="0.2032" layer="21"/>
<wire x1="-1.4732" y1="-0.6858" x2="-1.4732" y2="0.6858" width="0.2032" layer="21"/>
<smd name="+" x="-0.85" y="0" dx="1" dy="1.1" layer="1"/>
<smd name="-" x="0.85" y="0" dx="1" dy="1.1" layer="1"/>
<text x="-1.1938" y="-0.4064" size="0.762" layer="51">&gt;NAME</text>
<text x="-1.2779" y="0.7482" size="1.016" layer="21" ratio="18">+</text>
</package>
<package name="TESTPAD-PTH-0.7MM">
<circle x="0" y="0" radius="1.2" width="0.127" layer="21"/>
<pad name="1" x="0" y="0" drill="0.7" diameter="1.778"/>
</package>
<package name="SQUARE-PAD-0.200-INCH">
<smd name="1" x="0" y="0" dx="5.08" dy="5.08" layer="1"/>
</package>
<package name="KEYSTONE-SMD-TESTPOINT-5015">
<wire x1="-1.9685" y1="1.143" x2="-0.508" y2="1.143" width="0.254" layer="21"/>
<wire x1="-1.9685" y1="-1.143" x2="-0.508" y2="-1.143" width="0.254" layer="21"/>
<wire x1="-1.9685" y1="1.143" x2="-1.9685" y2="-1.143" width="0.254" layer="21"/>
<wire x1="1.9685" y1="1.143" x2="0.508" y2="1.143" width="0.254" layer="21"/>
<wire x1="1.9685" y1="-1.143" x2="0.508" y2="-1.143" width="0.254" layer="21"/>
<wire x1="1.9685" y1="1.143" x2="1.9685" y2="-1.143" width="0.254" layer="21"/>
<smd name="1" x="0" y="0" dx="3.429" dy="1.778" layer="1"/>
<text x="-1.8263" y="-0.5699" size="1.27" layer="51">&gt;NAME</text>
</package>
<package name="KEYSTONE-PTH-TESTPOINT-5011">
<wire x1="-1.524" y1="0" x2="-1.397" y2="0" width="0.254" layer="21"/>
<wire x1="1.524" y1="0" x2="1.397" y2="0" width="0.254" layer="21"/>
<circle x="0" y="0" radius="1.5875" width="0.254" layer="21"/>
<pad name="A" x="0" y="0" drill="1.6002"/>
<text x="-1.27" y="-0.508" size="0.889" layer="51">&gt;NAME</text>
</package>
<package name="SOT-23-8">
<wire x1="-1.5508" y1="1.5" x2="-1.5508" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.2968" y1="1.5" x2="-1.2968" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.3208" y1="-1.524" x2="-1.5748" y2="-1.524" width="0.254" layer="21"/>
<wire x1="-1.3208" y1="1.524" x2="-1.5748" y2="1.524" width="0.254" layer="21"/>
<wire x1="-1.5748" y1="1.524" x2="-1.5748" y2="-1.524" width="0.254" layer="21"/>
<wire x1="1.5" y1="1.5" x2="1.5" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.27" y1="0.508" x2="-1.016" y2="0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.508" x2="-1.016" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="-0.508" x2="-1.27" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-1.27" y1="-0.508" x2="-1.27" y2="0.254" width="0.254" layer="21"/>
<wire x1="-1.27" y1="0.254" x2="-1.016" y2="0.254" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.254" x2="-1.016" y2="0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.508" x2="-0.762" y2="0.508" width="0.254" layer="21"/>
<wire x1="-0.762" y1="0.508" x2="-0.762" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-0.762" y1="-0.508" x2="-1.016" y2="-0.508" width="0.254" layer="21"/>
<smd name="2" x="-0.325" y="-1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="1" x="-0.975" y="-1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="3" x="0.325" y="-1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="4" x="0.975" y="-1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="7" x="-0.325" y="1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="8" x="-0.975" y="1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="6" x="0.325" y="1.4" dx="0.325" dy="1.25" layer="1"/>
<smd name="5" x="0.975" y="1.4" dx="0.325" dy="1.25" layer="1"/>
<text x="-1.905" y="-1.27" size="1.016" layer="21" ratio="17" rot="R90">&gt;NAME</text>
</package>
<package name="SOT-23-8-OR-MSOP-8">
<wire x1="-1.5508" y1="1.5" x2="-1.5508" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.2968" y1="1.5" x2="-1.2968" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.3208" y1="-1.524" x2="-1.5748" y2="-1.524" width="0.254" layer="21"/>
<wire x1="-1.3208" y1="1.524" x2="-1.5748" y2="1.524" width="0.254" layer="21"/>
<wire x1="-1.5748" y1="1.524" x2="-1.5748" y2="-1.524" width="0.254" layer="21"/>
<wire x1="1.5" y1="1.5" x2="1.5" y2="-1.5" width="0.254" layer="21"/>
<wire x1="-1.27" y1="0.508" x2="-1.016" y2="0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.508" x2="-1.016" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="-0.508" x2="-1.27" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-1.27" y1="-0.508" x2="-1.27" y2="0.254" width="0.254" layer="21"/>
<wire x1="-1.27" y1="0.254" x2="-1.016" y2="0.254" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.254" x2="-1.016" y2="0.508" width="0.254" layer="21"/>
<wire x1="-1.016" y1="0.508" x2="-0.762" y2="0.508" width="0.254" layer="21"/>
<wire x1="-0.762" y1="0.508" x2="-0.762" y2="-0.508" width="0.254" layer="21"/>
<wire x1="-0.762" y1="-0.508" x2="-1.016" y2="-0.508" width="0.254" layer="21"/>
<smd name="2" x="-0.325" y="-1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="1" x="-0.975" y="-1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="3" x="0.325" y="-1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="4" x="0.975" y="-1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="7" x="-0.325" y="1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="8" x="-0.975" y="1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="6" x="0.325" y="1.65" dx="0.325" dy="1.8" layer="1"/>
<smd name="5" x="0.975" y="1.65" dx="0.325" dy="1.8" layer="1"/>
<text x="-1.905" y="-1.27" size="1.016" layer="21" ratio="17" rot="R90">&gt;NAME</text>
</package>
<package name="SOIC-8">
<wire x1="4.445" y1="3.81" x2="0.381" y2="3.81" width="0.254" layer="21"/>
<wire x1="0.381" y1="3.81" x2="0.127" y2="3.81" width="0.254" layer="21"/>
<wire x1="0.127" y1="3.81" x2="-0.127" y2="3.81" width="0.254" layer="21"/>
<wire x1="-0.127" y1="3.81" x2="-0.381" y2="3.81" width="0.254" layer="21"/>
<wire x1="-0.381" y1="3.81" x2="-0.635" y2="3.81" width="0.254" layer="21"/>
<wire x1="-0.635" y1="3.81" x2="-0.635" y2="1.27" width="0.254" layer="21"/>
<wire x1="-0.635" y1="1.27" x2="-0.381" y2="1.27" width="0.254" layer="21"/>
<wire x1="-0.381" y1="1.27" x2="-0.127" y2="1.27" width="0.254" layer="21"/>
<wire x1="-0.127" y1="1.27" x2="0.127" y2="1.27" width="0.254" layer="21"/>
<wire x1="0.127" y1="1.27" x2="4.445" y2="1.27" width="0.254" layer="21"/>
<wire x1="4.445" y1="1.27" x2="4.445" y2="3.81" width="0.254" layer="21"/>
<wire x1="-0.381" y1="3.81" x2="-0.381" y2="1.27" width="0.254" layer="21"/>
<smd name="1" x="0" y="0" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="2" x="1.27" y="0" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="3" x="2.54" y="0" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="4" x="3.81" y="0" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="5" x="3.81" y="5.08" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="6" x="2.54" y="5.08" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="7" x="1.27" y="5.08" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<smd name="8" x="0" y="5.08" dx="2.032" dy="0.6604" layer="1" rot="R90"/>
<text x="0.6825" y="1.8742" size="1.27" layer="25" ratio="15">&gt;NAME</text>
</package>
<package name="TSSOP-14">
<wire x1="0.8" y1="0.3" x2="5.3" y2="0.3" width="0.127" layer="21"/>
<wire x1="5.3" y1="0.3" x2="5.3" y2="0" width="0.127" layer="21"/>
<wire x1="5.3" y1="0" x2="5.3" y2="-0.3" width="0.127" layer="21"/>
<wire x1="5.3" y1="-0.3" x2="5.3" y2="-4.2" width="0.127" layer="21"/>
<wire x1="5.3" y1="-4.2" x2="0.8" y2="-4.2" width="0.127" layer="21"/>
<wire x1="0.8" y1="-4.2" x2="0.8" y2="-0.3" width="0.127" layer="21"/>
<wire x1="0.8" y1="-0.3" x2="0.8" y2="0" width="0.127" layer="21"/>
<wire x1="0.8" y1="0" x2="0.8" y2="0.3" width="0.127" layer="21"/>
<wire x1="0.8" y1="0" x2="5.3" y2="0" width="0.127" layer="21"/>
<wire x1="5.3" y1="-0.3" x2="0.8" y2="-0.3" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.2" x2="5.2" y2="-0.2" width="0.127" layer="21"/>
<wire x1="5.2" y1="-0.1" x2="0.9" y2="-0.1" width="0.127" layer="21"/>
<wire x1="0.9" y1="0.1" x2="5.2" y2="0.1" width="0.127" layer="21"/>
<wire x1="5.2" y1="0.2" x2="0.9" y2="0.2" width="0.127" layer="21"/>
<smd name="1" x="0" y="0" dx="1.3" dy="0.36" layer="1"/>
<smd name="2" x="0" y="-0.65" dx="1.3" dy="0.36" layer="1"/>
<smd name="3" x="0" y="-1.3" dx="1.3" dy="0.36" layer="1"/>
<smd name="4" x="0" y="-1.95" dx="1.3" dy="0.36" layer="1"/>
<smd name="5" x="0" y="-2.6" dx="1.3" dy="0.36" layer="1"/>
<smd name="6" x="0" y="-3.25" dx="1.3" dy="0.36" layer="1"/>
<smd name="7" x="0" y="-3.9" dx="1.3" dy="0.36" layer="1"/>
<smd name="14" x="6.095" y="0" dx="1.3" dy="0.36" layer="1"/>
<smd name="13" x="6.095" y="-0.65" dx="1.3" dy="0.36" layer="1"/>
<smd name="12" x="6.095" y="-1.3" dx="1.3" dy="0.36" layer="1"/>
<smd name="11" x="6.095" y="-1.95" dx="1.3" dy="0.36" layer="1"/>
<smd name="10" x="6.095" y="-2.6" dx="1.3" dy="0.36" layer="1"/>
<smd name="9" x="6.095" y="-3.25" dx="1.3" dy="0.36" layer="1"/>
<smd name="8" x="6.095" y="-3.9" dx="1.3" dy="0.36" layer="1"/>
<text x="1" y="-2.1" size="1.27" layer="21" ratio="15">&gt;NAME</text>
</package>
<package name="TSSOP-20">
<wire x1="0.9" y1="0.2" x2="5.2" y2="0.2" width="0.127" layer="21"/>
<wire x1="5.2" y1="0.2" x2="5.2" y2="0" width="0.127" layer="21"/>
<wire x1="5.2" y1="0" x2="5.2" y2="-0.2" width="0.127" layer="21"/>
<wire x1="5.2" y1="-0.2" x2="5.2" y2="-0.4" width="0.127" layer="21"/>
<wire x1="5.2" y1="-0.4" x2="5.2" y2="-0.6" width="0.127" layer="21"/>
<wire x1="5.2" y1="-0.6" x2="5.2" y2="-6" width="0.127" layer="21"/>
<wire x1="5.2" y1="-6" x2="0.9" y2="-6" width="0.127" layer="21"/>
<wire x1="0.9" y1="-6" x2="0.9" y2="-0.6" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.6" x2="0.9" y2="-0.4" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.4" x2="0.9" y2="-0.2" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.2" x2="0.9" y2="0" width="0.127" layer="21"/>
<wire x1="0.9" y1="0" x2="0.9" y2="0.2" width="0.127" layer="21"/>
<wire x1="0.9" y1="0" x2="5.2" y2="0" width="0.127" layer="21"/>
<wire x1="5.2" y1="-0.2" x2="0.9" y2="-0.2" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.4" x2="5.2" y2="-0.4" width="0.127" layer="21"/>
<wire x1="0.9" y1="-0.6" x2="5.2" y2="-0.6" width="0.127" layer="21"/>
<smd name="1" x="0" y="0" dx="1.4" dy="0.36" layer="1"/>
<smd name="2" x="0" y="-0.65" dx="1.4" dy="0.36" layer="1"/>
<smd name="3" x="0" y="-1.3" dx="1.4" dy="0.36" layer="1"/>
<smd name="4" x="0" y="-1.95" dx="1.4" dy="0.36" layer="1"/>
<smd name="5" x="0" y="-2.6" dx="1.4" dy="0.36" layer="1"/>
<smd name="6" x="0" y="-3.25" dx="1.4" dy="0.36" layer="1"/>
<smd name="7" x="0" y="-3.9" dx="1.4" dy="0.36" layer="1"/>
<smd name="8" x="0" y="-4.55" dx="1.4" dy="0.36" layer="1"/>
<smd name="9" x="0" y="-5.2" dx="1.4" dy="0.36" layer="1"/>
<smd name="10" x="0" y="-5.85" dx="1.4" dy="0.36" layer="1"/>
<smd name="20" x="6.095" y="0" dx="1.4" dy="0.36" layer="1"/>
<smd name="19" x="6.095" y="-0.65" dx="1.4" dy="0.36" layer="1"/>
<smd name="18" x="6.095" y="-1.3" dx="1.4" dy="0.36" layer="1"/>
<smd name="17" x="6.095" y="-1.95" dx="1.4" dy="0.36" layer="1"/>
<smd name="16" x="6.095" y="-2.6" dx="1.4" dy="0.36" layer="1"/>
<smd name="15" x="6.095" y="-3.25" dx="1.4" dy="0.36" layer="1"/>
<smd name="14" x="6.095" y="-3.9" dx="1.4" dy="0.36" layer="1"/>
<smd name="13" x="6.095" y="-4.55" dx="1.4" dy="0.36" layer="1"/>
<smd name="12" x="6.095" y="-5.2" dx="1.4" dy="0.36" layer="1"/>
<smd name="11" x="6.095" y="-5.85" dx="1.4" dy="0.36" layer="1"/>
<text x="2" y="-0.8" size="1.27" layer="21" ratio="15" rot="R270">&gt;NAME</text>
</package>
<package name="SMV-5">
<wire x1="0.2" y1="0.5" x2="1.8" y2="0.5" width="0.127" layer="21"/>
<wire x1="0.2" y1="-2.5" x2="1.8" y2="-2.5" width="0.127" layer="21"/>
<wire x1="0.2" y1="-2.5" x2="0.2" y2="-2.3" width="0.127" layer="21"/>
<wire x1="1.8" y1="-2.5" x2="1.8" y2="-2.3" width="0.127" layer="21"/>
<wire x1="0.2" y1="0.5" x2="0.2" y2="0.4" width="0.127" layer="21"/>
<wire x1="1.8" y1="0.5" x2="1.8" y2="0.4" width="0.127" layer="21"/>
<wire x1="1.8" y1="-0.6" x2="1.8" y2="-0.4" width="0.127" layer="21"/>
<wire x1="1.8" y1="-0.6" x2="1.8" y2="-1.4" width="0.127" layer="21"/>
<smd name="1" x="0" y="0" dx="1.5" dy="0.5" layer="1"/>
<smd name="2" x="0" y="-0.95" dx="1.5" dy="0.5" layer="1"/>
<smd name="3" x="0" y="-1.9" dx="1.5" dy="0.5" layer="1"/>
<smd name="5" x="2.2" y="0" dx="1.5" dy="0.5" layer="1"/>
<smd name="4" x="2.2" y="-1.9" dx="1.5" dy="0.5" layer="1"/>
<text x="0" y="0.8" size="1.016" layer="21" ratio="18">&gt;NAME</text>
</package>
<package name="USB-MINIB-SMD">
<wire x1="-3.05" y1="-4.25" x2="-3.05" y2="1.85" width="0.3" layer="21"/>
<wire x1="-6.05" y1="-7.85" x2="-6.05" y2="-4.25" width="0.127" layer="21"/>
<wire x1="-6.05" y1="-4.25" x2="-6.05" y2="5.45" width="0.127" layer="21"/>
<wire x1="-6.05" y1="-4.25" x2="-6.05" y2="1.85" width="0.3" layer="21"/>
<smd name="1" x="2" y="0.4" dx="2.3" dy="0.5" layer="1"/>
<smd name="2" x="2" y="-0.4" dx="2.3" dy="0.5" layer="1"/>
<smd name="3" x="2" y="-1.2" dx="2.3" dy="0.5" layer="1"/>
<smd name="4" x="2" y="-2" dx="2.3" dy="0.5" layer="1"/>
<smd name="5" x="2" y="-2.8" dx="2.3" dy="0.5" layer="1"/>
<smd name="TAB1" x="2.2" y="3.9" dx="3.5" dy="3.5" layer="1"/>
<smd name="TAB0" x="-2.3" y="3.9" dx="3.5" dy="3.5" layer="1"/>
<smd name="TAB2" x="2.2" y="-6.3" dx="3.5" dy="3.5" layer="1"/>
<smd name="TAB3" x="-2.3" y="-6.3" dx="3.5" dy="3.5" layer="1"/>
<text x="-3.9" y="-3.9" size="1.27" layer="21" ratio="18" rot="R90">&gt;NAME</text>
<hole x="-0.05" y="0.55" drill="0.9"/>
<hole x="-0.05" y="-2.95" drill="0.9"/>
</package>
</packages>
<symbols>
<symbol name="AT91SAM7S64">
<wire x1="-55.88" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="-81.28" width="0.254" layer="94"/>
<wire x1="0" y1="-81.28" x2="-55.88" y2="-81.28" width="0.254" layer="94"/>
<wire x1="-55.88" y1="-81.28" x2="-55.88" y2="2.54" width="0.254" layer="94"/>
<text x="-34.8341" y="-71.9348" size="3.81" layer="95" rot="R90">AT91SAM7S64</text>
<text x="-34.1356" y="-23.2111" size="3.81" layer="95">&gt;NAME</text>
<pin name="ADVREF" x="-60.96" y="-63.5" length="middle"/>
<pin name="GND0" x="-20.32" y="-86.36" length="middle" rot="R90"/>
<pin name="AD4" x="-60.96" y="-66.04" length="middle"/>
<pin name="AD5" x="-60.96" y="-68.58" length="middle"/>
<pin name="AD6" x="-60.96" y="-71.12" length="middle"/>
<pin name="AD7" x="-60.96" y="-73.66" length="middle"/>
<pin name="VDDIN" x="-30.48" y="7.62" length="middle" rot="R270"/>
<pin name="VDDOUT" x="-12.7" y="7.62" length="middle" rot="R270"/>
<pin name="PA17/AD0" x="5.08" y="-43.18" length="middle" rot="R180"/>
<pin name="PA18/AD1" x="5.08" y="-45.72" length="middle" rot="R180"/>
<pin name="PA21" x="5.08" y="-53.34" length="middle" rot="R180"/>
<pin name="VDDCORE0" x="-20.32" y="7.62" length="middle" rot="R270"/>
<pin name="PA19/AD2" x="5.08" y="-48.26" length="middle" rot="R180"/>
<pin name="PA22" x="5.08" y="-55.88" length="middle" rot="R180"/>
<pin name="PA23" x="5.08" y="-58.42" length="middle" rot="R180"/>
<pin name="PA20/AD3" x="5.08" y="-50.8" length="middle" rot="R180"/>
<pin name="GND1" x="-22.86" y="-86.36" length="middle" rot="R90"/>
<pin name="VDDIO0" x="-40.64" y="7.62" length="middle" rot="R270"/>
<pin name="PA16" x="5.08" y="-40.64" length="middle" rot="R180"/>
<pin name="PA15" x="5.08" y="-38.1" length="middle" rot="R180"/>
<pin name="PA14" x="5.08" y="-35.56" length="middle" rot="R180"/>
<pin name="PA13" x="5.08" y="-33.02" length="middle" rot="R180"/>
<pin name="PA24" x="5.08" y="-60.96" length="middle" rot="R180"/>
<pin name="VDDCORE1" x="-17.78" y="7.62" length="middle" rot="R270"/>
<pin name="PA25" x="5.08" y="-63.5" length="middle" rot="R180"/>
<pin name="PA26" x="5.08" y="-66.04" length="middle" rot="R180"/>
<pin name="PA12" x="5.08" y="-30.48" length="middle" rot="R180"/>
<pin name="PA11" x="5.08" y="-27.94" length="middle" rot="R180"/>
<pin name="PA10" x="5.08" y="-25.4" length="middle" rot="R180"/>
<pin name="PA9" x="5.08" y="-22.86" length="middle" rot="R180"/>
<pin name="PA8" x="5.08" y="-20.32" length="middle" rot="R180"/>
<pin name="PA7" x="5.08" y="-17.78" length="middle" rot="R180"/>
<pin name="PA6" x="5.08" y="-15.24" length="middle" rot="R180"/>
<pin name="PA5" x="5.08" y="-12.7" length="middle" rot="R180"/>
<pin name="PA4" x="5.08" y="-10.16" length="middle" rot="R180"/>
<pin name="PA27" x="5.08" y="-68.58" length="middle" rot="R180"/>
<pin name="PA28" x="5.08" y="-71.12" length="middle" rot="R180"/>
<pin name="NRST" x="-60.96" y="-17.78" length="middle"/>
<pin name="TST" x="-60.96" y="-12.7" length="middle"/>
<pin name="PA29" x="5.08" y="-73.66" length="middle" rot="R180"/>
<pin name="PA30" x="5.08" y="-76.2" length="middle" rot="R180"/>
<pin name="PA3" x="5.08" y="-7.62" length="middle" rot="R180"/>
<pin name="PA2" x="5.08" y="-5.08" length="middle" rot="R180"/>
<pin name="VDDIO1" x="-38.1" y="7.62" length="middle" rot="R270"/>
<pin name="GND2" x="-25.4" y="-86.36" length="middle" rot="R90"/>
<pin name="PA1" x="5.08" y="-2.54" length="middle" rot="R180"/>
<pin name="PA0" x="5.08" y="0" length="middle" rot="R180"/>
<pin name="TDO" x="-60.96" y="-25.4" length="middle"/>
<pin name="JTAGSEL" x="-60.96" y="-15.24" length="middle"/>
<pin name="TMS" x="-60.96" y="-20.32" length="middle"/>
<pin name="TCK" x="-60.96" y="-22.86" length="middle"/>
<pin name="ERASE" x="-60.96" y="-35.56" length="middle"/>
<pin name="VDDCORE2" x="-15.24" y="7.62" length="middle" rot="R270"/>
<pin name="DDM" x="-60.96" y="-55.88" length="middle"/>
<pin name="DDP" x="-60.96" y="-58.42" length="middle"/>
<pin name="VDDFLASH" x="-33.02" y="7.62" length="middle" rot="R270"/>
<pin name="VDDIO2" x="-35.56" y="7.62" length="middle" rot="R270"/>
<pin name="XOUT" x="-60.96" y="-40.64" length="middle"/>
<pin name="GND3" x="-27.94" y="-86.36" length="middle" rot="R90"/>
<pin name="XIN/PGMCK" x="-60.96" y="-50.8" length="middle"/>
<pin name="PLLRC" x="-60.96" y="-5.08" length="middle"/>
<pin name="VDDPLL" x="-22.86" y="7.62" length="middle" rot="R270"/>
<pin name="TDI" x="-60.96" y="-27.94" length="middle"/>
<pin name="PA31" x="5.08" y="-78.74" length="middle" rot="R180"/>
</symbol>
<symbol name="CAPACITOR">
<wire x1="0" y1="0" x2="0" y2="0.508" width="0.1524" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="2.032" width="0.1524" layer="94"/>
<text x="1.524" y="2.921" size="1.778" layer="95">&gt;NAME</text>
<text x="1.27" y="-1.905" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-2.032" y1="1.524" x2="2.032" y2="2.032" layer="94"/>
<rectangle x1="-2.032" y1="0.508" x2="2.032" y2="1.016" layer="94"/>
<pin name="1" x="0" y="5.08" visible="off" length="short" direction="pas" swaplevel="1" rot="R270"/>
<pin name="2" x="0" y="-2.54" visible="off" length="short" direction="pas" swaplevel="1" rot="R90"/>
</symbol>
<symbol name="SUPPLY_+2V5">
<wire x1="0" y1="0.635" x2="-1.27" y2="-0.635" width="0.1524" layer="94"/>
<wire x1="0" y1="0.635" x2="1.27" y2="-0.635" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="0.635" width="0.1524" layer="94"/>
<text x="-2.032" y="-3.937" size="1.778" layer="95" rot="R90">+2v5</text>
<pin name="+2V5" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
<symbol name="CRYSTAL">
<wire x1="2.54" y1="0" x2="2.54" y2="-5.08" width="0.254" layer="94"/>
<wire x1="3.81" y1="-1.27" x2="3.81" y2="-3.81" width="0.254" layer="94"/>
<wire x1="3.81" y1="-3.81" x2="6.35" y2="-3.81" width="0.254" layer="94"/>
<wire x1="6.35" y1="-3.81" x2="6.35" y2="-1.27" width="0.254" layer="94"/>
<wire x1="6.35" y1="-1.27" x2="3.81" y2="-1.27" width="0.254" layer="94"/>
<wire x1="7.62" y1="0" x2="7.62" y2="-5.08" width="0.254" layer="94"/>
<text x="0.0161" y="0.6764" size="1.778" layer="95">&gt;NAME</text>
<pin name="A" x="0" y="-2.54" visible="off" length="short"/>
<pin name="B" x="10.16" y="-2.54" visible="off" length="short" rot="R180"/>
</symbol>
<symbol name="INVERTER">
<wire x1="0" y1="2.54" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="2.54" y2="-1.27" width="0.254" layer="94"/>
<wire x1="2.54" y1="-1.27" x2="5.08" y2="0" width="0.254" layer="94"/>
<wire x1="5.08" y1="0" x2="2.54" y2="1.27" width="0.254" layer="94"/>
<wire x1="2.54" y1="1.27" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="7.62" y1="0" x2="6.35" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="2.54" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="2.54" y1="1.27" x2="2.54" y2="2.54" width="0.1524" layer="94"/>
<circle x="5.715" y="0" radius="0.635" width="0.1524" layer="94"/>
<text x="3.1661" y="2.7576" size="1.016" layer="95">Vdd</text>
<text x="3.1048" y="-3.6768" size="1.016" layer="95">Vss</text>
<text x="-3.8786" y="3.7099" size="1.4224" layer="95">&gt;NAME</text>
<pin name="A" x="-2.54" y="0" visible="pad" length="short"/>
<pin name="Y" x="7.62" y="0" visible="pad" length="point" rot="R180"/>
<pin name="VDD" x="2.54" y="5.08" visible="pad" length="short" rot="R270"/>
<pin name="VSS" x="2.54" y="-5.08" visible="pad" length="short" rot="R90"/>
</symbol>
<symbol name="4TERMSTRIP">
<wire x1="0" y1="2.54" x2="0" y2="-10.16" width="0.254" layer="94"/>
<wire x1="0" y1="-10.16" x2="10.16" y2="-10.16" width="0.254" layer="94"/>
<wire x1="10.16" y1="-10.16" x2="10.16" y2="2.54" width="0.254" layer="94"/>
<wire x1="10.16" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<text x="9.8506" y="-10.8263" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<pin name="PIN1" x="-5.08" y="0" visible="pin" length="middle" direction="pas"/>
<pin name="PIN2" x="-5.08" y="-2.54" visible="pin" length="middle" direction="pas"/>
<pin name="PIN3" x="-5.08" y="-5.08" visible="pin" length="middle" direction="pas"/>
<pin name="PIN4" x="-5.08" y="-7.62" visible="pin" length="middle" direction="pas"/>
</symbol>
<symbol name="OPAMP">
<wire x1="0" y1="5.08" x2="0" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="0" y1="-5.08" x2="10.16" y2="0" width="0.4064" layer="94"/>
<wire x1="10.16" y1="0" x2="0" y2="5.08" width="0.4064" layer="94"/>
<wire x1="1.27" y1="3.175" x2="1.27" y2="1.905" width="0.1524" layer="94"/>
<wire x1="0.635" y1="2.54" x2="1.905" y2="2.54" width="0.1524" layer="94"/>
<wire x1="0.635" y1="-2.54" x2="1.905" y2="-2.54" width="0.1524" layer="94"/>
<text x="7.62" y="3.175" size="1.778" layer="95">&gt;NAME</text>
<text x="7.62" y="-5.08" size="1.778" layer="96">&gt;VALUE</text>
<pin name="-IN" x="-2.54" y="-2.54" visible="pad" length="short" direction="in"/>
<pin name="+IN" x="-2.54" y="2.54" visible="pad" length="short" direction="in"/>
<pin name="OUT" x="12.7" y="0" visible="pad" length="short" direction="out" rot="R180"/>
</symbol>
<symbol name="POWER-PINS">
<text x="2.54" y="0" size="1.778" layer="95">&gt;NAME</text>
<pin name="V+" x="0" y="5.08" length="middle" rot="R270"/>
<pin name="V-" x="0" y="-17.78" length="middle" rot="R90"/>
</symbol>
<symbol name="RESISTOR">
<wire x1="-2.54" y1="0" x2="-2.159" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-2.159" y1="1.016" x2="-1.524" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-1.524" y1="-1.016" x2="-0.889" y2="1.016" width="0.2032" layer="94"/>
<wire x1="-0.889" y1="1.016" x2="-0.254" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="-0.254" y1="-1.016" x2="0.381" y2="1.016" width="0.2032" layer="94"/>
<wire x1="0.381" y1="1.016" x2="1.016" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="1.016" y1="-1.016" x2="1.651" y2="1.016" width="0.2032" layer="94"/>
<wire x1="1.651" y1="1.016" x2="2.286" y2="-1.016" width="0.2032" layer="94"/>
<wire x1="2.286" y1="-1.016" x2="2.54" y2="0" width="0.2032" layer="94"/>
<text x="-3.81" y="1.4986" size="1.778" layer="95">&gt;NAME</text>
<text x="-3.81" y="-3.175" size="1.778" layer="96">&gt;VALUE</text>
<pin name="2" x="5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1" rot="R180"/>
<pin name="1" x="-5.08" y="0" visible="off" length="short" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="HEADER-MALE-10X2">
<wire x1="1.27" y1="-12.7" x2="-6.35" y2="-12.7" width="0.4064" layer="94"/>
<wire x1="-1.27" y1="-5.08" x2="0" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="-7.62" x2="0" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="-10.16" x2="0" y2="-10.16" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="-3.81" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-3.81" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="-10.16" x2="-3.81" y2="-10.16" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="0" x2="0" y2="0" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="-2.54" x2="0" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="0" x2="-3.81" y2="0" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="-2.54" x2="-3.81" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="7.62" x2="0" y2="7.62" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="5.08" x2="0" y2="5.08" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="2.54" x2="0" y2="2.54" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="7.62" x2="-3.81" y2="7.62" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="5.08" x2="-3.81" y2="5.08" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="2.54" x2="-3.81" y2="2.54" width="0.6096" layer="94"/>
<wire x1="-6.35" y1="15.24" x2="-6.35" y2="-12.7" width="0.4064" layer="94"/>
<wire x1="1.27" y1="-12.7" x2="1.27" y2="15.24" width="0.4064" layer="94"/>
<wire x1="-6.35" y1="15.24" x2="1.27" y2="15.24" width="0.4064" layer="94"/>
<wire x1="-1.27" y1="12.7" x2="0" y2="12.7" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="10.16" x2="0" y2="10.16" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="12.7" x2="-3.81" y2="12.7" width="0.6096" layer="94"/>
<wire x1="-5.08" y1="10.16" x2="-3.81" y2="10.16" width="0.6096" layer="94"/>
<text x="-6.35" y="-15.24" size="1.778" layer="96">&gt;VALUE</text>
<text x="-6.35" y="16.002" size="1.778" layer="95">&gt;NAME</text>
<pin name="1" x="5.08" y="-10.16" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="3" x="5.08" y="-7.62" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="5" x="5.08" y="-5.08" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="2" x="-10.16" y="-10.16" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="4" x="-10.16" y="-7.62" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="6" x="-10.16" y="-5.08" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="7" x="5.08" y="-2.54" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="9" x="5.08" y="0" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="8" x="-10.16" y="-2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="10" x="-10.16" y="0" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="11" x="5.08" y="2.54" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="13" x="5.08" y="5.08" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="15" x="5.08" y="7.62" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="12" x="-10.16" y="2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="14" x="-10.16" y="5.08" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="16" x="-10.16" y="7.62" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="17" x="5.08" y="10.16" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="19" x="5.08" y="12.7" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="18" x="-10.16" y="10.16" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="20" x="-10.16" y="12.7" visible="pad" length="middle" direction="pas" swaplevel="1"/>
</symbol>
<symbol name="SUPPLY_VMID">
<wire x1="-1.27" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-0.635" y2="0.635" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-0.635" y2="-0.635" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0.635" y2="0.635" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0.635" y2="-0.635" width="0.254" layer="94"/>
<text x="-2.032" y="-2.794" size="1.27" layer="95" rot="R90">Vmid</text>
<pin name="VMID" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
<symbol name="TLC5540">
<wire x1="0" y1="0" x2="-27.94" y2="0" width="0.254" layer="94"/>
<wire x1="-27.94" y1="0" x2="-27.94" y2="-45.72" width="0.254" layer="94"/>
<wire x1="-27.94" y1="-45.72" x2="0" y2="-45.72" width="0.254" layer="94"/>
<wire x1="0" y1="-45.72" x2="0" y2="0" width="0.254" layer="94"/>
<text x="-0.508" y="0.762" size="2.1844" layer="95" rot="MR0">&gt;NAME</text>
<text x="-27.686" y="-46.482" size="2.1844" layer="95" rot="MR180">TLC5540</text>
<pin name="D1" x="5.08" y="-2.54" length="middle" rot="R180"/>
<pin name="D2" x="5.08" y="-5.08" length="middle" rot="R180"/>
<pin name="D3" x="5.08" y="-7.62" length="middle" rot="R180"/>
<pin name="D4" x="5.08" y="-10.16" length="middle" rot="R180"/>
<pin name="D5" x="5.08" y="-12.7" length="middle" rot="R180"/>
<pin name="D6" x="5.08" y="-15.24" length="middle" rot="R180"/>
<pin name="D7" x="5.08" y="-17.78" length="middle" rot="R180"/>
<pin name="MSB-D8" x="5.08" y="-20.32" length="middle" rot="R180"/>
<pin name="VDDD1" x="-12.7" y="5.08" length="middle" rot="R270"/>
<pin name="VDDD0" x="-15.24" y="5.08" length="middle" rot="R270"/>
<pin name="VDDA2" x="-20.32" y="5.08" length="middle" rot="R270"/>
<pin name="VDDA1" x="-22.86" y="5.08" length="middle" rot="R270"/>
<pin name="CLK" x="5.08" y="-30.48" length="middle" rot="R180"/>
<pin name="REFB" x="-33.02" y="-30.48" length="middle"/>
<pin name="REFBS" x="-33.02" y="-33.02" length="middle"/>
<pin name="VDDA0" x="-25.4" y="5.08" length="middle" rot="R270"/>
<pin name="ANALOGIN" x="-33.02" y="-15.24" length="middle"/>
<pin name="NOE" x="5.08" y="-25.4" length="middle" rot="R180"/>
<pin name="DGND0" x="-2.54" y="-50.8" length="middle" rot="R90"/>
<pin name="DGND1" x="-5.08" y="-50.8" length="middle" rot="R90"/>
<pin name="AGND0" x="-10.16" y="-50.8" length="middle" rot="R90"/>
<pin name="AGND1" x="-12.7" y="-50.8" length="middle" rot="R90"/>
<pin name="REFT" x="-33.02" y="-25.4" length="middle"/>
<pin name="REFTS" x="-33.02" y="-22.86" length="middle"/>
</symbol>
<symbol name="74XX244-OCTAL-TRISTATE-BUFFERS">
<wire x1="0" y1="0" x2="0" y2="5.08" width="0.254" layer="94"/>
<wire x1="0" y1="5.08" x2="5.08" y2="2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="-7.62" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="5.08" y2="-5.08" width="0.254" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="0" y2="-7.62" width="0.254" layer="94"/>
<wire x1="0" y1="-15.24" x2="0" y2="-10.16" width="0.254" layer="94"/>
<wire x1="0" y1="-10.16" x2="5.08" y2="-12.7" width="0.254" layer="94"/>
<wire x1="5.08" y1="-12.7" x2="0" y2="-15.24" width="0.254" layer="94"/>
<wire x1="0" y1="-22.86" x2="0" y2="-17.78" width="0.254" layer="94"/>
<wire x1="0" y1="-17.78" x2="5.08" y2="-20.32" width="0.254" layer="94"/>
<wire x1="5.08" y1="-20.32" x2="0" y2="-22.86" width="0.254" layer="94"/>
<wire x1="0" y1="7.62" x2="2.54" y2="7.62" width="0.1524" layer="94"/>
<wire x1="2.54" y1="7.62" x2="2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="2.54" y1="5.08" x2="6.35" y2="5.08" width="0.1524" layer="94"/>
<wire x1="6.35" y1="5.08" x2="6.35" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-2.54" x2="6.35" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-10.16" x2="6.35" y2="-17.78" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-17.78" x2="2.54" y2="-17.78" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-2.54" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-10.16" x2="2.54" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="0" y1="-35.56" x2="0" y2="-30.48" width="0.254" layer="94"/>
<wire x1="0" y1="-30.48" x2="5.08" y2="-33.02" width="0.254" layer="94"/>
<wire x1="5.08" y1="-33.02" x2="0" y2="-35.56" width="0.254" layer="94"/>
<wire x1="0" y1="-43.18" x2="0" y2="-38.1" width="0.254" layer="94"/>
<wire x1="0" y1="-38.1" x2="5.08" y2="-40.64" width="0.254" layer="94"/>
<wire x1="5.08" y1="-40.64" x2="0" y2="-43.18" width="0.254" layer="94"/>
<wire x1="0" y1="-50.8" x2="0" y2="-45.72" width="0.254" layer="94"/>
<wire x1="0" y1="-45.72" x2="5.08" y2="-48.26" width="0.254" layer="94"/>
<wire x1="5.08" y1="-48.26" x2="0" y2="-50.8" width="0.254" layer="94"/>
<wire x1="0" y1="-58.42" x2="0" y2="-53.34" width="0.254" layer="94"/>
<wire x1="0" y1="-53.34" x2="5.08" y2="-55.88" width="0.254" layer="94"/>
<wire x1="5.08" y1="-55.88" x2="0" y2="-58.42" width="0.254" layer="94"/>
<wire x1="0" y1="-27.94" x2="2.54" y2="-27.94" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-27.94" x2="2.54" y2="-30.48" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-30.48" x2="6.35" y2="-30.48" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-30.48" x2="6.35" y2="-38.1" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-38.1" x2="6.35" y2="-45.72" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-45.72" x2="6.35" y2="-53.34" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-53.34" x2="2.54" y2="-53.34" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-38.1" x2="2.54" y2="-38.1" width="0.1524" layer="94"/>
<wire x1="6.35" y1="-45.72" x2="2.54" y2="-45.72" width="0.1524" layer="94"/>
<wire x1="-6.35" y1="10.16" x2="11.43" y2="10.16" width="0.1524" layer="94" style="shortdash"/>
<wire x1="11.43" y1="10.16" x2="11.43" y2="-60.96" width="0.1524" layer="94" style="shortdash"/>
<wire x1="11.43" y1="-60.96" x2="-6.35" y2="-60.96" width="0.1524" layer="94" style="shortdash"/>
<wire x1="-6.35" y1="-60.96" x2="-6.35" y2="10.16" width="0.1524" layer="94" style="shortdash"/>
<wire x1="2.54" y1="4.445" x2="2.54" y2="5.08" width="0.1524" layer="94" style="shortdash"/>
<wire x1="2.54" y1="-53.975" x2="2.54" y2="-53.34" width="0.1524" layer="94" style="shortdash"/>
<wire x1="2.54" y1="-46.355" x2="2.54" y2="-45.72" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-38.735" x2="2.54" y2="-38.1" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-31.115" x2="2.54" y2="-30.48" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-18.415" x2="2.54" y2="-17.78" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-10.795" x2="2.54" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-3.175" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<circle x="6.35" y="-2.54" radius="0.1796" width="0.254" layer="94"/>
<circle x="2.54" y="5.08" radius="0.1796" width="0.254" layer="94"/>
<circle x="6.35" y="-10.16" radius="0.1796" width="0.254" layer="94"/>
<circle x="6.35" y="-38.1" radius="0.1796" width="0.254" layer="94"/>
<circle x="2.54" y="-30.48" radius="0.1796" width="0.254" layer="94"/>
<circle x="6.35" y="-45.72" radius="0.1796" width="0.254" layer="94"/>
<circle x="2.54" y="4.1275" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-3.4925" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-11.1125" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-18.7325" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-31.4325" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-39.0525" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-46.6725" radius="0.3175" width="0.1524" layer="94"/>
<circle x="2.54" y="-54.2925" radius="0.3175" width="0.1524" layer="94"/>
<text x="-4.395" y="10.8211" size="1.778" layer="95">&gt;NAME</text>
<text x="9.8628" y="-61.7992" size="1.778" layer="95" rot="R180">'244</text>
<text x="9.1153" y="9.4136" size="1.27" layer="95" rot="R180">Vdd</text>
<text x="-4.0182" y="-60.1844" size="1.27" layer="95">Vss</text>
<pin name="1A1" x="-7.62" y="2.54" visible="pad"/>
<pin name="1Y1" x="12.7" y="2.54" visible="pad" rot="R180"/>
<pin name="1A2" x="-7.62" y="-5.08" visible="pad"/>
<pin name="1Y2" x="12.7" y="-5.08" visible="pad" rot="R180"/>
<pin name="1A3" x="-7.62" y="-12.7" visible="pad"/>
<pin name="1Y3" x="12.7" y="-12.7" visible="pad" rot="R180"/>
<pin name="1A4" x="-7.62" y="-20.32" visible="pad"/>
<pin name="1Y4" x="12.7" y="-20.32" visible="pad" rot="R180"/>
<pin name="1NOE" x="-7.62" y="7.62" visible="pad"/>
<pin name="2A1" x="-7.62" y="-33.02" visible="pad"/>
<pin name="2Y1" x="12.7" y="-33.02" visible="pad" rot="R180"/>
<pin name="2A2" x="-7.62" y="-40.64" visible="pad"/>
<pin name="2Y2" x="12.7" y="-40.64" visible="pad" rot="R180"/>
<pin name="2A3" x="-7.62" y="-48.26" visible="pad"/>
<pin name="2Y3" x="12.7" y="-48.26" visible="pad" rot="R180"/>
<pin name="2A4" x="-7.62" y="-55.88" visible="pad"/>
<pin name="2Y4" x="12.7" y="-55.88" visible="pad" rot="R180"/>
<pin name="2NOE" x="-7.62" y="-27.94" visible="pad"/>
<pin name="VSS" x="-2.54" y="-63.5" visible="pad" length="short" rot="R90"/>
<pin name="VDD" x="7.62" y="12.7" visible="pad" length="short" rot="R270"/>
</symbol>
<symbol name="SPARTAN-II-XC2S30-100-VQFP">
<wire x1="48.26" y1="0" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="-119.38" width="0.254" layer="94"/>
<wire x1="0" y1="-119.38" x2="48.26" y2="-119.38" width="0.254" layer="94"/>
<wire x1="48.26" y1="-119.38" x2="48.26" y2="0" width="0.254" layer="94"/>
<text x="41.148" y="-1.016" size="1.9304" layer="95" rot="R180">VccInt</text>
<text x="12.192" y="-117.856" size="1.9304" layer="95">GND</text>
<text x="47.244" y="-120.396" size="2.1844" layer="95" rot="R180">&gt;NAME</text>
<text x="27.178" y="-94.996" size="5.08" layer="95" rot="R90">XC2330 100-VQFP</text>
<text x="27.686" y="-117.856" size="1.9304" layer="95">NC</text>
<pin name="GND1" x="5.08" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND2" x="7.62" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND3" x="10.16" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND4" x="12.7" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND5" x="15.24" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND6" x="17.78" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND7" x="20.32" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="GND8" x="22.86" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="NC0" x="27.94" y="-124.46" visible="pad" length="middle" rot="R90"/>
<pin name="VCCO_B76" x="5.08" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B65" x="7.62" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B54" x="10.16" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B43" x="12.7" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B32" x="15.24" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B21" x="17.78" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B10" x="20.32" y="5.08" length="middle" rot="R270"/>
<pin name="VCCO_B07" x="22.86" y="5.08" length="middle" rot="R270"/>
<pin name="TDI" x="-5.08" y="-17.78" length="middle"/>
<pin name="TDO" x="-5.08" y="-20.32" length="middle"/>
<pin name="TMS" x="-5.08" y="-22.86" length="middle"/>
<pin name="TCK" x="-5.08" y="-25.4" length="middle"/>
<pin name="VCCINT1" x="27.94" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT2" x="30.48" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT3" x="33.02" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT4" x="35.56" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT5" x="38.1" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT6" x="40.64" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT7" x="43.18" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="VCCINT8" x="45.72" y="5.08" visible="pad" length="middle" rot="R270"/>
<pin name="P3_IO7" x="53.34" y="-7.62" length="middle" rot="R180"/>
<pin name="P4_IOV7" x="53.34" y="-10.16" length="middle" rot="R180"/>
<pin name="P5_IO7" x="53.34" y="-12.7" length="middle" rot="R180"/>
<pin name="P6_IO7" x="53.34" y="-15.24" length="middle" rot="R180"/>
<pin name="P7_IO7" x="53.34" y="-17.78" length="middle" rot="R180"/>
<pin name="P8_IOV7" x="53.34" y="-20.32" length="middle" rot="R180"/>
<pin name="P9_IO7" x="53.34" y="-22.86" length="middle" rot="R180"/>
<pin name="P10_IO7" x="53.34" y="-25.4" length="middle" rot="R180"/>
<pin name="P13_IO6" x="53.34" y="-30.48" length="middle" rot="R180"/>
<pin name="P15_IO6" x="53.34" y="-33.02" length="middle" rot="R180"/>
<pin name="P16_IOV6" x="53.34" y="-35.56" length="middle" rot="R180"/>
<pin name="P17_IO6" x="53.34" y="-38.1" length="middle" rot="R180"/>
<pin name="P18_IO6" x="53.34" y="-40.64" length="middle" rot="R180"/>
<pin name="P19_IO6" x="53.34" y="-43.18" length="middle" rot="R180"/>
<pin name="P20_IOV6" x="53.34" y="-45.72" length="middle" rot="R180"/>
<pin name="P21_IO6" x="53.34" y="-48.26" length="middle" rot="R180"/>
<pin name="P22_IO6" x="53.34" y="-50.8" length="middle" rot="R180"/>
<pin name="M0" x="-5.08" y="-30.48" length="middle"/>
<pin name="M1" x="-5.08" y="-33.02" length="middle"/>
<pin name="M2" x="-5.08" y="-35.56" length="middle"/>
<pin name="DONE" x="-5.08" y="-40.64" length="middle"/>
<pin name="/PROGRAM" x="-5.08" y="-43.18" length="middle"/>
<pin name="CCLK" x="-5.08" y="-45.72" length="middle"/>
<pin name="P30_IOV5" x="53.34" y="-55.88" length="middle" rot="R180"/>
<pin name="P31_IO5" x="53.34" y="-58.42" length="middle" rot="R180"/>
<pin name="P32_IO5" x="53.34" y="-60.96" length="middle" rot="R180"/>
<pin name="P34_IOV5" x="53.34" y="-63.5" length="middle" rot="R180"/>
<pin name="P36_IGCK5" x="53.34" y="-66.04" length="middle" rot="R180"/>
<pin name="P39_IGCK4" x="53.34" y="-71.12" length="middle" rot="R180"/>
<pin name="P40_IO4" x="53.34" y="-73.66" length="middle" rot="R180"/>
<pin name="P41_IOV4" x="53.34" y="-76.2" length="middle" rot="R180"/>
<pin name="P43_IO4" x="53.34" y="-78.74" length="middle" rot="R180"/>
<pin name="P44_IO4" x="53.34" y="-81.28" length="middle" rot="R180"/>
<pin name="P45_IOV4" x="53.34" y="-83.82" length="middle" rot="R180"/>
<pin name="P46_IO4" x="53.34" y="-86.36" length="middle" rot="R180"/>
<pin name="P47_IO4" x="53.34" y="-88.9" length="middle" rot="R180"/>
<pin name="P52_IO3" x="53.34" y="-93.98" length="middle" rot="R180"/>
<pin name="P53_IO3" x="53.34" y="-96.52" length="middle" rot="R180"/>
<pin name="P54_IOV3" x="53.34" y="-99.06" length="middle" rot="R180"/>
<pin name="P55_IO3" x="53.34" y="-101.6" length="middle" rot="R180"/>
<pin name="P56_IO3" x="53.34" y="-104.14" length="middle" rot="R180"/>
<pin name="P57_IO3" x="53.34" y="-106.68" length="middle" rot="R180"/>
<pin name="P58_IO3" x="53.34" y="-109.22" length="middle" rot="R180"/>
<pin name="P59_IOV3" x="53.34" y="-111.76" length="middle" rot="R180"/>
<pin name="P60_IO3" x="53.34" y="-114.3" length="middle" rot="R180"/>
<pin name="P62_IO3" x="53.34" y="-116.84" length="middle" rot="R180"/>
<pin name="P65_IO2" x="-5.08" y="-55.88" length="middle"/>
<pin name="P66_IO2" x="-5.08" y="-58.42" length="middle"/>
<pin name="P67_IOV2" x="-5.08" y="-60.96" length="middle"/>
<pin name="P68_IO2" x="-5.08" y="-63.5" length="middle"/>
<pin name="P69_IO2" x="-5.08" y="-66.04" length="middle"/>
<pin name="P70_IO2" x="-5.08" y="-68.58" length="middle"/>
<pin name="P71_IO2" x="-5.08" y="-71.12" length="middle"/>
<pin name="P72_IOV2" x="-5.08" y="-73.66" length="middle"/>
<pin name="DIN" x="-5.08" y="-48.26" length="middle"/>
<pin name="DOUT" x="-5.08" y="-50.8" length="middle"/>
<pin name="P80_IO1" x="-5.08" y="-78.74" length="middle"/>
<pin name="P81_IO1" x="-5.08" y="-81.28" length="middle"/>
<pin name="P82_IOV1" x="-5.08" y="-83.82" length="middle"/>
<pin name="P83_IO1" x="-5.08" y="-86.36" length="middle"/>
<pin name="P84_IO1" x="-5.08" y="-88.9" length="middle"/>
<pin name="P86_IOV1" x="-5.08" y="-91.44" length="middle"/>
<pin name="P87_IO1" x="-5.08" y="-93.98" length="middle"/>
<pin name="P88_IGCK1" x="-5.08" y="-96.52" length="middle"/>
<pin name="P91_IGCK0" x="-5.08" y="-101.6" length="middle"/>
<pin name="P93_IOV0" x="-5.08" y="-104.14" length="middle"/>
<pin name="P95_IO0" x="-5.08" y="-106.68" length="middle"/>
<pin name="P96_IO0" x="-5.08" y="-109.22" length="middle"/>
<pin name="P97_IOV0" x="-5.08" y="-111.76" length="middle"/>
<pin name="P98_IO0" x="-5.08" y="-114.3" length="middle"/>
<pin name="NC1" x="30.48" y="-124.46" visible="pad" length="middle" rot="R90"/>
</symbol>
<symbol name="FERRITE">
<wire x1="0" y1="1.27" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="7.62" y2="-1.27" width="0.254" layer="94"/>
<wire x1="7.62" y1="-1.27" x2="7.62" y2="1.27" width="0.254" layer="94"/>
<wire x1="7.62" y1="1.27" x2="0" y2="1.27" width="0.254" layer="94"/>
<text x="0.8509" y="-0.5065" size="1.016" layer="94" font="vector">ferrite</text>
<text x="0.189" y="1.6198" size="1.6764" layer="95" font="vector">&gt;NAME</text>
<pin name="A" x="-2.54" y="0" visible="off" length="short"/>
<pin name="B" x="10.16" y="0" visible="off" length="short" rot="R180"/>
</symbol>
<symbol name="CD4066-ANALOG-SWITCH">
<wire x1="15.24" y1="5.08" x2="0" y2="5.08" width="0.254" layer="94"/>
<wire x1="0" y1="5.08" x2="0" y2="-27.94" width="0.254" layer="94"/>
<wire x1="0" y1="-27.94" x2="15.24" y2="-27.94" width="0.254" layer="94"/>
<wire x1="15.24" y1="-27.94" x2="15.24" y2="5.08" width="0.254" layer="94"/>
<text x="9.906" y="-26.924" size="1.524" layer="95">Vss</text>
<text x="5.334" y="4.064" size="1.524" layer="95" rot="R180">Vdd</text>
<text x="14.732" y="5.842" size="1.524" layer="95" rot="MR0">CD4066</text>
<text x="1.016" y="-28.956" size="1.524" layer="95" rot="MR180">&gt;NAME</text>
<pin name="A1" x="-5.08" y="0" length="middle" direction="pas"/>
<pin name="C1" x="-5.08" y="-2.54" length="middle" direction="in"/>
<pin name="A2" x="-5.08" y="-7.62" length="middle" direction="pas"/>
<pin name="C2" x="-5.08" y="-10.16" length="middle" direction="in"/>
<pin name="A3" x="-5.08" y="-15.24" length="middle" direction="pas"/>
<pin name="C3" x="-5.08" y="-17.78" length="middle" direction="in"/>
<pin name="A4" x="-5.08" y="-22.86" length="middle" direction="pas"/>
<pin name="C4" x="-5.08" y="-25.4" length="middle" direction="in"/>
<pin name="B1" x="20.32" y="0" length="middle" direction="pas" rot="R180"/>
<pin name="B2" x="20.32" y="-7.62" length="middle" direction="pas" rot="R180"/>
<pin name="B3" x="20.32" y="-15.24" length="middle" direction="pas" rot="R180"/>
<pin name="B4" x="20.32" y="-22.86" length="middle" direction="pas" rot="R180"/>
<pin name="VDD" x="2.54" y="10.16" visible="pad" length="middle" rot="R270"/>
<pin name="VSS" x="12.7" y="-33.02" visible="pad" length="middle" rot="R90"/>
</symbol>
<symbol name="LP2989-LDO">
<wire x1="0" y1="2.54" x2="-22.86" y2="2.54" width="0.254" layer="94"/>
<wire x1="-22.86" y1="2.54" x2="-22.86" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-22.86" y1="-15.24" x2="0" y2="-15.24" width="0.254" layer="94"/>
<wire x1="0" y1="-15.24" x2="0" y2="2.54" width="0.254" layer="94"/>
<text x="-22.098" y="3.048" size="1.778" layer="95">&gt;NAME</text>
<text x="-0.508" y="-15.748" size="1.778" layer="95" rot="R180">&gt;VALUE</text>
<pin name="OUT" x="5.08" y="0" length="middle" direction="pas" rot="R180"/>
<pin name="/ERROR" x="5.08" y="-10.16" length="middle" direction="pas" rot="R180"/>
<pin name="SENSE" x="5.08" y="-2.54" length="middle" direction="pas" rot="R180"/>
<pin name="IN" x="-27.94" y="0" length="middle" direction="pas"/>
<pin name="GND" x="-27.94" y="-12.7" length="middle" direction="pas"/>
<pin name="/SHDN" x="-27.94" y="-5.08" length="middle" direction="pas"/>
<pin name="BYP" x="-27.94" y="-10.16" length="middle" direction="pas"/>
</symbol>
<symbol name="LED">
<wire x1="1.27" y1="2.54" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="-1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="0" width="0.1524" layer="94"/>
<wire x1="-2.032" y1="1.778" x2="-3.429" y2="0.381" width="0.1524" layer="94"/>
<wire x1="-3.429" y1="0.381" x2="-2.54" y2="0.762" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="0.762" x2="-3.048" y2="1.27" width="0.1524" layer="94"/>
<wire x1="-3.048" y1="1.27" x2="-3.429" y2="0.381" width="0.1524" layer="94"/>
<wire x1="-3.302" y1="-0.762" x2="-2.413" y2="-0.381" width="0.1524" layer="94"/>
<wire x1="-2.413" y1="-0.381" x2="-2.921" y2="0.127" width="0.1524" layer="94"/>
<wire x1="-2.921" y1="0.127" x2="-3.302" y2="-0.762" width="0.1524" layer="94"/>
<wire x1="-1.905" y1="0.635" x2="-3.302" y2="-0.762" width="0.1524" layer="94"/>
<text x="3.556" y="-2.032" size="1.778" layer="95" rot="R90">&gt;NAME</text>
<pin name="C" x="0" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
<pin name="A" x="0" y="5.08" visible="off" length="short" direction="pas" rot="R270"/>
</symbol>
<symbol name="CAPACITOR-POLARIZED">
<wire x1="-2.54" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="-1.016" x2="0" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="-1" x2="2.4892" y2="-1.8542" width="0.254" layer="94" curve="-37.878202" cap="flat"/>
<wire x1="-2.4669" y1="-1.8504" x2="0" y2="-1.0161" width="0.254" layer="94" curve="-37.376341" cap="flat"/>
<text x="1.016" y="0.635" size="1.778" layer="95">&gt;NAME</text>
<text x="1.016" y="-4.191" size="1.778" layer="96">&gt;VALUE</text>
<rectangle x1="-2.253" y1="0.668" x2="-1.364" y2="0.795" layer="94"/>
<rectangle x1="-1.872" y1="0.287" x2="-1.745" y2="1.176" layer="94"/>
<pin name="+" x="0" y="2.54" visible="off" length="short" direction="pas" swaplevel="1" rot="R270"/>
<pin name="-" x="0" y="-5.08" visible="off" length="short" direction="pas" swaplevel="1" rot="R90"/>
</symbol>
<symbol name="TESTPAD">
<wire x1="0" y1="0" x2="1.27" y2="0.635" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0.635" x2="0.635" y2="0.635" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0.635" x2="-0.635" y2="0.635" width="0.1524" layer="94"/>
<wire x1="-0.635" y1="0.635" x2="-1.27" y2="0.635" width="0.1524" layer="94"/>
<wire x1="-1.27" y1="0.635" x2="0" y2="0" width="0.1524" layer="94"/>
<wire x1="-0.635" y1="0.635" x2="-0.635" y2="3.175" width="0.1524" layer="94"/>
<wire x1="-0.635" y1="3.175" x2="0.635" y2="3.175" width="0.1524" layer="94"/>
<wire x1="0.635" y1="3.175" x2="0.635" y2="0.635" width="0.1524" layer="94"/>
<wire x1="0" y1="2.8575" x2="0" y2="0.9525" width="0.1524" layer="94"/>
<wire x1="0" y1="0.9525" x2="0.3175" y2="1.27" width="0.1524" layer="94"/>
<wire x1="0.3175" y1="1.27" x2="-0.3175" y2="1.27" width="0.1524" layer="94"/>
<wire x1="-0.3175" y1="1.27" x2="0" y2="0.9525" width="0.1524" layer="94"/>
<text x="-1.27" y="3.4925" size="1.27" layer="95">&gt;NAME</text>
<pin name="P$1" x="0" y="-2.54" visible="off" length="short" rot="R90"/>
</symbol>
<symbol name="TACTSWITCH">
<wire x1="-1.27" y1="1.27" x2="0.635" y2="1.27" width="0.254" layer="94"/>
<wire x1="0.635" y1="1.27" x2="4.445" y2="1.27" width="0.254" layer="94"/>
<wire x1="4.445" y1="1.27" x2="6.35" y2="1.27" width="0.254" layer="94"/>
<wire x1="0.635" y1="1.27" x2="0.635" y2="5.715" width="0.254" layer="94"/>
<wire x1="0.635" y1="5.715" x2="4.445" y2="5.715" width="0.254" layer="94"/>
<wire x1="4.445" y1="5.715" x2="4.445" y2="1.27" width="0.254" layer="94"/>
<text x="-0.508" y="4.826" size="1.778" layer="95" rot="MR0">&gt;NAME</text>
<pin name="P$1" x="-5.08" y="0" visible="off" length="middle"/>
<pin name="P$2" x="10.16" y="0" visible="off" length="middle" rot="R180"/>
</symbol>
<symbol name="USB-MINIB">
<wire x1="0" y1="2.54" x2="0" y2="-25.4" width="0.254" layer="94"/>
<wire x1="0" y1="-25.4" x2="-12.7" y2="-25.4" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-25.4" x2="-12.7" y2="2.54" width="0.254" layer="94"/>
<wire x1="-12.7" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<text x="-0.4082" y="-26.0474" size="1.6764" layer="95" rot="R180">&gt;NAME</text>
<pin name="VBUS/1" x="5.08" y="0" length="middle" rot="R180"/>
<pin name="D-/2" x="5.08" y="-2.54" length="middle" rot="R180"/>
<pin name="D+/3" x="5.08" y="-5.08" length="middle" rot="R180"/>
<pin name="ID/4" x="5.08" y="-7.62" length="middle" rot="R180"/>
<pin name="GND/5" x="5.08" y="-10.16" length="middle" rot="R180"/>
<pin name="SHLD0" x="5.08" y="-15.24" visible="pin" length="middle" rot="R180"/>
<pin name="SHLD1" x="5.08" y="-17.78" visible="pin" length="middle" rot="R180"/>
<pin name="SHLD2" x="5.08" y="-20.32" visible="pin" length="middle" rot="R180"/>
<pin name="SHLD3" x="5.08" y="-22.86" visible="pin" length="middle" rot="R180"/>
</symbol>
<symbol name="NFET">
<wire x1="-2.54" y1="1.27" x2="-2.54" y2="0" width="0.3048" layer="94"/>
<wire x1="-2.54" y1="0" x2="-2.54" y2="-1.27" width="0.3048" layer="94"/>
<wire x1="-2.54" y1="-1.27" x2="-2.54" y2="-2.54" width="0.3048" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="-2.54" y2="-3.81" width="0.3048" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="-2.2225" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="-2.2225" y1="-1.27" x2="-2.54" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="-1.27" y1="-1.27" x2="-1.27" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="-2.54" x2="-1.27" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="-2.54" y2="0" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-2.54" x2="-3.81" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="-1.27" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="-3.81" y1="-2.54" x2="-3.81" y2="0" width="0.1524" layer="94"/>
<wire x1="-1.5875" y1="-1.905" x2="-1.5875" y2="-0.635" width="0.1016" layer="94"/>
<wire x1="-1.5875" y1="-0.635" x2="-2.2225" y2="-1.27" width="0.1016" layer="94"/>
<wire x1="-2.2225" y1="-1.27" x2="-1.5875" y2="-1.905" width="0.1016" layer="94"/>
<text x="2.54" y="3.556" size="1.778" layer="95" font="vector" rot="R180">&gt;NAME</text>
<pin name="G" x="-5.08" y="-2.54" visible="off" length="point" direction="pas"/>
<pin name="D" x="0" y="0" visible="off" length="point" direction="pas" rot="R180"/>
<pin name="S" x="0" y="-2.54" visible="off" length="point" direction="pas" rot="R180"/>
</symbol>
<symbol name="PFET">
<wire x1="5.08" y1="-1.27" x2="5.08" y2="1.27" width="0.254" layer="94"/>
<wire x1="5.08" y1="1.27" x2="5.08" y2="3.81" width="0.254" layer="94"/>
<wire x1="6.35" y1="2.54" x2="6.35" y2="0" width="0.1524" layer="94"/>
<wire x1="5.08" y1="1.27" x2="4.1275" y2="1.27" width="0.1524" layer="94"/>
<wire x1="4.1275" y1="1.27" x2="3.81" y2="1.27" width="0.1524" layer="94"/>
<wire x1="3.81" y1="1.27" x2="3.81" y2="0" width="0.1524" layer="94"/>
<wire x1="7.62" y1="0" x2="6.35" y2="0" width="0.1524" layer="94"/>
<wire x1="4.7625" y1="1.905" x2="4.7625" y2="0.635" width="0.1016" layer="94"/>
<wire x1="4.7625" y1="0.635" x2="4.1275" y2="1.27" width="0.1016" layer="94"/>
<wire x1="4.1275" y1="1.27" x2="4.7625" y2="1.905" width="0.1016" layer="94"/>
<text x="0" y="4.064" size="1.778" layer="95" font="vector">&gt;NAME</text>
<pin name="G" x="7.62" y="0" visible="off" length="point" direction="pas" rot="R180"/>
<pin name="S" x="2.54" y="0" visible="off" length="short" direction="pas"/>
<pin name="D" x="2.54" y="2.54" visible="off" length="short" direction="pas"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AT91SAM7S64" prefix="IC">
<gates>
<gate name="G$1" symbol="AT91SAM7S64" x="30.48" y="2.54"/>
</gates>
<devices>
<device name="" package="LQFP-64">
<connects>
<connect gate="G$1" pin="AD4" pad="3"/>
<connect gate="G$1" pin="AD5" pad="4"/>
<connect gate="G$1" pin="AD6" pad="5"/>
<connect gate="G$1" pin="AD7" pad="6"/>
<connect gate="G$1" pin="ADVREF" pad="1"/>
<connect gate="G$1" pin="DDM" pad="56"/>
<connect gate="G$1" pin="DDP" pad="57"/>
<connect gate="G$1" pin="ERASE" pad="55"/>
<connect gate="G$1" pin="GND0" pad="2"/>
<connect gate="G$1" pin="GND1" pad="17"/>
<connect gate="G$1" pin="GND2" pad="46"/>
<connect gate="G$1" pin="GND3" pad="60"/>
<connect gate="G$1" pin="JTAGSEL" pad="50"/>
<connect gate="G$1" pin="NRST" pad="39"/>
<connect gate="G$1" pin="PA0" pad="48"/>
<connect gate="G$1" pin="PA1" pad="47"/>
<connect gate="G$1" pin="PA10" pad="29"/>
<connect gate="G$1" pin="PA11" pad="28"/>
<connect gate="G$1" pin="PA12" pad="27"/>
<connect gate="G$1" pin="PA13" pad="22"/>
<connect gate="G$1" pin="PA14" pad="21"/>
<connect gate="G$1" pin="PA15" pad="20"/>
<connect gate="G$1" pin="PA16" pad="19"/>
<connect gate="G$1" pin="PA17/AD0" pad="9"/>
<connect gate="G$1" pin="PA18/AD1" pad="10"/>
<connect gate="G$1" pin="PA19/AD2" pad="13"/>
<connect gate="G$1" pin="PA2" pad="44"/>
<connect gate="G$1" pin="PA20/AD3" pad="16"/>
<connect gate="G$1" pin="PA21" pad="11"/>
<connect gate="G$1" pin="PA22" pad="14"/>
<connect gate="G$1" pin="PA23" pad="15"/>
<connect gate="G$1" pin="PA24" pad="23"/>
<connect gate="G$1" pin="PA25" pad="25"/>
<connect gate="G$1" pin="PA26" pad="26"/>
<connect gate="G$1" pin="PA27" pad="37"/>
<connect gate="G$1" pin="PA28" pad="38"/>
<connect gate="G$1" pin="PA29" pad="41"/>
<connect gate="G$1" pin="PA3" pad="43"/>
<connect gate="G$1" pin="PA30" pad="42"/>
<connect gate="G$1" pin="PA31" pad="52"/>
<connect gate="G$1" pin="PA4" pad="36"/>
<connect gate="G$1" pin="PA5" pad="35"/>
<connect gate="G$1" pin="PA6" pad="34"/>
<connect gate="G$1" pin="PA7" pad="32"/>
<connect gate="G$1" pin="PA8" pad="31"/>
<connect gate="G$1" pin="PA9" pad="30"/>
<connect gate="G$1" pin="PLLRC" pad="63"/>
<connect gate="G$1" pin="TCK" pad="53"/>
<connect gate="G$1" pin="TDI" pad="33"/>
<connect gate="G$1" pin="TDO" pad="49"/>
<connect gate="G$1" pin="TMS" pad="51"/>
<connect gate="G$1" pin="TST" pad="40"/>
<connect gate="G$1" pin="VDDCORE0" pad="12"/>
<connect gate="G$1" pin="VDDCORE1" pad="24"/>
<connect gate="G$1" pin="VDDCORE2" pad="54"/>
<connect gate="G$1" pin="VDDFLASH" pad="59"/>
<connect gate="G$1" pin="VDDIN" pad="7"/>
<connect gate="G$1" pin="VDDIO0" pad="18"/>
<connect gate="G$1" pin="VDDIO1" pad="45"/>
<connect gate="G$1" pin="VDDIO2" pad="58"/>
<connect gate="G$1" pin="VDDOUT" pad="8"/>
<connect gate="G$1" pin="VDDPLL" pad="64"/>
<connect gate="G$1" pin="XIN/PGMCK" pad="62"/>
<connect gate="G$1" pin="XOUT" pad="61"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SUPPLY_2V5" prefix="V">
<gates>
<gate name="G$1" symbol="SUPPLY_+2V5" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="XTAL-SMD-CITIZEN-CS10" prefix="XT">
<gates>
<gate name="G$1" symbol="CRYSTAL" x="-10.16" y="2.54"/>
</gates>
<devices>
<device name="" package="XTAL-SMD-CITIZEN-CS10">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="B" pad="B"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="HIROSE-MQ172-4POS" prefix="SV">
<gates>
<gate name="G$1" symbol="4TERMSTRIP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="HIROSE-MQ172-4POS">
<connects>
<connect gate="G$1" pin="PIN1" pad="1"/>
<connect gate="G$1" pin="PIN2" pad="2"/>
<connect gate="G$1" pin="PIN3" pad="3"/>
<connect gate="G$1" pin="PIN4" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="HEADER-MALE-10X2" prefix="SV" uservalue="yes">
<gates>
<gate name="G$1" symbol="HEADER-MALE-10X2" x="2.54" y="-2.54"/>
</gates>
<devices>
<device name="" package="HEADER-MALE-10X2-0.100-SHROUDED">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="10" pad="10"/>
<connect gate="G$1" pin="11" pad="11"/>
<connect gate="G$1" pin="12" pad="12"/>
<connect gate="G$1" pin="13" pad="13"/>
<connect gate="G$1" pin="14" pad="14"/>
<connect gate="G$1" pin="15" pad="15"/>
<connect gate="G$1" pin="16" pad="16"/>
<connect gate="G$1" pin="17" pad="17"/>
<connect gate="G$1" pin="18" pad="18"/>
<connect gate="G$1" pin="19" pad="19"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="20" pad="20"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
<connect gate="G$1" pin="8" pad="8"/>
<connect gate="G$1" pin="9" pad="9"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SUPPLY_VMID" prefix="V">
<gates>
<gate name="G$1" symbol="SUPPLY_VMID" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TLC5540" prefix="IC">
<gates>
<gate name="G$1" symbol="TLC5540" x="0" y="0"/>
</gates>
<devices>
<device name="" package="TSSOP-24">
<connects>
<connect gate="G$1" pin="AGND0" pad="20"/>
<connect gate="G$1" pin="AGND1" pad="21"/>
<connect gate="G$1" pin="ANALOGIN" pad="19"/>
<connect gate="G$1" pin="CLK" pad="12"/>
<connect gate="G$1" pin="D1" pad="3"/>
<connect gate="G$1" pin="D2" pad="4"/>
<connect gate="G$1" pin="D3" pad="5"/>
<connect gate="G$1" pin="D4" pad="6"/>
<connect gate="G$1" pin="D5" pad="7"/>
<connect gate="G$1" pin="D6" pad="8"/>
<connect gate="G$1" pin="D7" pad="9"/>
<connect gate="G$1" pin="DGND0" pad="2"/>
<connect gate="G$1" pin="DGND1" pad="24"/>
<connect gate="G$1" pin="MSB-D8" pad="10"/>
<connect gate="G$1" pin="NOE" pad="1"/>
<connect gate="G$1" pin="REFB" pad="23"/>
<connect gate="G$1" pin="REFBS" pad="22"/>
<connect gate="G$1" pin="REFT" pad="17"/>
<connect gate="G$1" pin="REFTS" pad="16"/>
<connect gate="G$1" pin="VDDA0" pad="14"/>
<connect gate="G$1" pin="VDDA1" pad="15"/>
<connect gate="G$1" pin="VDDA2" pad="18"/>
<connect gate="G$1" pin="VDDD0" pad="11"/>
<connect gate="G$1" pin="VDDD1" pad="13"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="SPARTAN-II-XC2S30-100-VQFP" prefix="IC">
<gates>
<gate name="G$1" symbol="SPARTAN-II-XC2S30-100-VQFP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="VQFP-100">
<connects>
<connect gate="G$1" pin="/PROGRAM" pad="51"/>
<connect gate="G$1" pin="CCLK" pad="75"/>
<connect gate="G$1" pin="DIN" pad="73"/>
<connect gate="G$1" pin="DONE" pad="49"/>
<connect gate="G$1" pin="DOUT" pad="74"/>
<connect gate="G$1" pin="GND1" pad="1"/>
<connect gate="G$1" pin="GND2" pad="11"/>
<connect gate="G$1" pin="GND3" pad="38"/>
<connect gate="G$1" pin="GND4" pad="78"/>
<connect gate="G$1" pin="GND5" pad="89"/>
<connect gate="G$1" pin="GND6" pad="48"/>
<connect gate="G$1" pin="GND7" pad="64"/>
<connect gate="G$1" pin="GND8" pad="24"/>
<connect gate="G$1" pin="M0" pad="25"/>
<connect gate="G$1" pin="M1" pad="23"/>
<connect gate="G$1" pin="M2" pad="27"/>
<connect gate="G$1" pin="NC0" pad="28"/>
<connect gate="G$1" pin="NC1" pad="29"/>
<connect gate="G$1" pin="P10_IO7" pad="10"/>
<connect gate="G$1" pin="P13_IO6" pad="13"/>
<connect gate="G$1" pin="P15_IO6" pad="15"/>
<connect gate="G$1" pin="P16_IOV6" pad="16"/>
<connect gate="G$1" pin="P17_IO6" pad="17"/>
<connect gate="G$1" pin="P18_IO6" pad="18"/>
<connect gate="G$1" pin="P19_IO6" pad="19"/>
<connect gate="G$1" pin="P20_IOV6" pad="20"/>
<connect gate="G$1" pin="P21_IO6" pad="21"/>
<connect gate="G$1" pin="P22_IO6" pad="22"/>
<connect gate="G$1" pin="P30_IOV5" pad="30"/>
<connect gate="G$1" pin="P31_IO5" pad="31"/>
<connect gate="G$1" pin="P32_IO5" pad="32"/>
<connect gate="G$1" pin="P34_IOV5" pad="34"/>
<connect gate="G$1" pin="P36_IGCK5" pad="36"/>
<connect gate="G$1" pin="P39_IGCK4" pad="39"/>
<connect gate="G$1" pin="P3_IO7" pad="3"/>
<connect gate="G$1" pin="P40_IO4" pad="40"/>
<connect gate="G$1" pin="P41_IOV4" pad="41"/>
<connect gate="G$1" pin="P43_IO4" pad="43"/>
<connect gate="G$1" pin="P44_IO4" pad="44"/>
<connect gate="G$1" pin="P45_IOV4" pad="45"/>
<connect gate="G$1" pin="P46_IO4" pad="46"/>
<connect gate="G$1" pin="P47_IO4" pad="47"/>
<connect gate="G$1" pin="P4_IOV7" pad="4"/>
<connect gate="G$1" pin="P52_IO3" pad="52"/>
<connect gate="G$1" pin="P53_IO3" pad="53"/>
<connect gate="G$1" pin="P54_IOV3" pad="54"/>
<connect gate="G$1" pin="P55_IO3" pad="55"/>
<connect gate="G$1" pin="P56_IO3" pad="56"/>
<connect gate="G$1" pin="P57_IO3" pad="57"/>
<connect gate="G$1" pin="P58_IO3" pad="58"/>
<connect gate="G$1" pin="P59_IOV3" pad="59"/>
<connect gate="G$1" pin="P5_IO7" pad="5"/>
<connect gate="G$1" pin="P60_IO3" pad="60"/>
<connect gate="G$1" pin="P62_IO3" pad="62"/>
<connect gate="G$1" pin="P65_IO2" pad="65"/>
<connect gate="G$1" pin="P66_IO2" pad="66"/>
<connect gate="G$1" pin="P67_IOV2" pad="67"/>
<connect gate="G$1" pin="P68_IO2" pad="68"/>
<connect gate="G$1" pin="P69_IO2" pad="69"/>
<connect gate="G$1" pin="P6_IO7" pad="6"/>
<connect gate="G$1" pin="P70_IO2" pad="70"/>
<connect gate="G$1" pin="P71_IO2" pad="71"/>
<connect gate="G$1" pin="P72_IOV2" pad="72"/>
<connect gate="G$1" pin="P7_IO7" pad="7"/>
<connect gate="G$1" pin="P80_IO1" pad="80"/>
<connect gate="G$1" pin="P81_IO1" pad="81"/>
<connect gate="G$1" pin="P82_IOV1" pad="82"/>
<connect gate="G$1" pin="P83_IO1" pad="83"/>
<connect gate="G$1" pin="P84_IO1" pad="84"/>
<connect gate="G$1" pin="P86_IOV1" pad="86"/>
<connect gate="G$1" pin="P87_IO1" pad="87"/>
<connect gate="G$1" pin="P88_IGCK1" pad="88"/>
<connect gate="G$1" pin="P8_IOV7" pad="8"/>
<connect gate="G$1" pin="P91_IGCK0" pad="91"/>
<connect gate="G$1" pin="P93_IOV0" pad="93"/>
<connect gate="G$1" pin="P95_IO0" pad="95"/>
<connect gate="G$1" pin="P96_IO0" pad="96"/>
<connect gate="G$1" pin="P97_IOV0" pad="97"/>
<connect gate="G$1" pin="P98_IO0" pad="98"/>
<connect gate="G$1" pin="P9_IO7" pad="9"/>
<connect gate="G$1" pin="TCK" pad="99"/>
<connect gate="G$1" pin="TDI" pad="79"/>
<connect gate="G$1" pin="TDO" pad="77"/>
<connect gate="G$1" pin="TMS" pad="2"/>
<connect gate="G$1" pin="VCCINT1" pad="61"/>
<connect gate="G$1" pin="VCCINT2" pad="92"/>
<connect gate="G$1" pin="VCCINT3" pad="85"/>
<connect gate="G$1" pin="VCCINT4" pad="94"/>
<connect gate="G$1" pin="VCCINT5" pad="14"/>
<connect gate="G$1" pin="VCCINT6" pad="33"/>
<connect gate="G$1" pin="VCCINT7" pad="35"/>
<connect gate="G$1" pin="VCCINT8" pad="42"/>
<connect gate="G$1" pin="VCCO_B07" pad="100"/>
<connect gate="G$1" pin="VCCO_B10" pad="90"/>
<connect gate="G$1" pin="VCCO_B21" pad="76"/>
<connect gate="G$1" pin="VCCO_B32" pad="63"/>
<connect gate="G$1" pin="VCCO_B43" pad="50"/>
<connect gate="G$1" pin="VCCO_B54" pad="37"/>
<connect gate="G$1" pin="VCCO_B65" pad="26"/>
<connect gate="G$1" pin="VCCO_B76" pad="12"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="RESISTOR" prefix="R" uservalue="yes">
<gates>
<gate name="G$1" symbol="RESISTOR" x="-5.08" y="0"/>
</gates>
<devices>
<device name="2512" package="2512">
<connects>
<connect gate="G$1" pin="1" pad="P$1"/>
<connect gate="G$1" pin="2" pad="P$2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0603" package="RLC_0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="RLC_0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="1210" package="RLC_1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="FERRITE" prefix="U">
<gates>
<gate name="G$1" symbol="FERRITE" x="-7.62" y="0"/>
</gates>
<devices>
<device name="0603" package="RLC_0603">
<connects>
<connect gate="G$1" pin="A" pad="1"/>
<connect gate="G$1" pin="B" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="RLC_0805">
<connects>
<connect gate="G$1" pin="A" pad="1"/>
<connect gate="G$1" pin="B" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="1210" package="RLC_1210">
<connects>
<connect gate="G$1" pin="A" pad="1"/>
<connect gate="G$1" pin="B" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAPACITOR" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAPACITOR" x="2.54" y="0"/>
</gates>
<devices>
<device name="0603" package="RLC_0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="RLC_0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="1210" package="RLC_1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="OPAMP-AD8052" prefix="IC">
<gates>
<gate name="A" symbol="OPAMP" x="0" y="0"/>
<gate name="B" symbol="OPAMP" x="0" y="-12.7"/>
<gate name="P" symbol="POWER-PINS" x="-20.32" y="2.54"/>
</gates>
<devices>
<device name="" package="MSOP8">
<connects>
<connect gate="A" pin="+IN" pad="3"/>
<connect gate="A" pin="-IN" pad="2"/>
<connect gate="A" pin="OUT" pad="1"/>
<connect gate="B" pin="+IN" pad="5"/>
<connect gate="B" pin="-IN" pad="6"/>
<connect gate="B" pin="OUT" pad="7"/>
<connect gate="P" pin="V+" pad="8"/>
<connect gate="P" pin="V-" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CAPACITOR-POLARIZED" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="CAPACITOR-POLARIZED" x="0" y="0"/>
</gates>
<devices>
<device name="ALCHIP-MZA-F80" package="CAPCAITOR-ELECTROLYTIC-ALCHIP-MZA-F80">
<connects>
<connect gate="G$1" pin="+" pad="+"/>
<connect gate="G$1" pin="-" pad="-"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LED" prefix="D">
<gates>
<gate name="G$1" symbol="LED" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LED_0603">
<connects>
<connect gate="G$1" pin="A" pad="+"/>
<connect gate="G$1" pin="C" pad="-"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TESTPAD" prefix="TP">
<gates>
<gate name="G$1" symbol="TESTPAD" x="0" y="2.54"/>
</gates>
<devices>
<device name="0.7MM-ROUND-DRILLED" package="TESTPAD-PTH-0.7MM">
<connects>
<connect gate="G$1" pin="P$1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0.200&quot;-SQUARE" package="SQUARE-PAD-0.200-INCH">
<connects>
<connect gate="G$1" pin="P$1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="KEYSTONE-SMD" package="KEYSTONE-SMD-TESTPOINT-5015">
<connects>
<connect gate="G$1" pin="P$1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="KEYSTONE-PTH" package="KEYSTONE-PTH-TESTPOINT-5011">
<connects>
<connect gate="G$1" pin="P$1" pad="A"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="TACTSWITCH" prefix="SW">
<gates>
<gate name="G$1" symbol="TACTSWITCH" x="-10.16" y="5.08"/>
</gates>
<devices>
<device name="THROUGHHOLE" package="TACTSWITCH">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="" package="TACTSWITCH-SMD-EVQQ1">
<connects>
<connect gate="G$1" pin="P$1" pad="A"/>
<connect gate="G$1" pin="P$2" pad="B"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="COMP-TLV3502" prefix="IC">
<gates>
<gate name="A" symbol="OPAMP" x="0" y="12.7"/>
<gate name="B" symbol="OPAMP" x="0" y="0"/>
<gate name="P" symbol="POWER-PINS" x="-15.24" y="12.7"/>
</gates>
<devices>
<device name="SOT-23-8" package="SOT-23-8">
<connects>
<connect gate="A" pin="+IN" pad="1"/>
<connect gate="A" pin="-IN" pad="2"/>
<connect gate="A" pin="OUT" pad="7"/>
<connect gate="B" pin="+IN" pad="3"/>
<connect gate="B" pin="-IN" pad="4"/>
<connect gate="B" pin="OUT" pad="6"/>
<connect gate="P" pin="V+" pad="8"/>
<connect gate="P" pin="V-" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="SOT-23-8-OR-MSOP-8" package="SOT-23-8-OR-MSOP-8">
<connects>
<connect gate="A" pin="+IN" pad="1"/>
<connect gate="A" pin="-IN" pad="2"/>
<connect gate="A" pin="OUT" pad="7"/>
<connect gate="B" pin="+IN" pad="3"/>
<connect gate="B" pin="-IN" pad="4"/>
<connect gate="B" pin="OUT" pad="6"/>
<connect gate="P" pin="V+" pad="8"/>
<connect gate="P" pin="V-" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="IRF7307" prefix="Q">
<gates>
<gate name="N" symbol="NFET" x="0" y="2.54"/>
<gate name="P" symbol="PFET" x="10.16" y="0"/>
</gates>
<devices>
<device name="" package="SOIC-8">
<connects>
<connect gate="N" pin="D" pad="8"/>
<connect gate="N" pin="G" pad="2"/>
<connect gate="N" pin="S" pad="1"/>
<connect gate="P" pin="D" pad="6"/>
<connect gate="P" pin="G" pad="4"/>
<connect gate="P" pin="S" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LP2989-LDO" prefix="IC" uservalue="yes">
<gates>
<gate name="G$1" symbol="LP2989-LDO" x="0" y="-2.54"/>
</gates>
<devices>
<device name="SOIC" package="SOIC-8">
<connects>
<connect gate="G$1" pin="/ERROR" pad="7"/>
<connect gate="G$1" pin="/SHDN" pad="8"/>
<connect gate="G$1" pin="BYP" pad="1"/>
<connect gate="G$1" pin="GND" pad="3"/>
<connect gate="G$1" pin="IN" pad="4"/>
<connect gate="G$1" pin="OUT" pad="5"/>
<connect gate="G$1" pin="SENSE" pad="6"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="CD4066" prefix="IC">
<gates>
<gate name="G$1" symbol="CD4066-ANALOG-SWITCH" x="-15.24" y="0"/>
</gates>
<devices>
<device name="" package="TSSOP-14">
<connects>
<connect gate="G$1" pin="A1" pad="1"/>
<connect gate="G$1" pin="A2" pad="4"/>
<connect gate="G$1" pin="A3" pad="8"/>
<connect gate="G$1" pin="A4" pad="11"/>
<connect gate="G$1" pin="B1" pad="2"/>
<connect gate="G$1" pin="B2" pad="3"/>
<connect gate="G$1" pin="B3" pad="9"/>
<connect gate="G$1" pin="B4" pad="10"/>
<connect gate="G$1" pin="C1" pad="13"/>
<connect gate="G$1" pin="C2" pad="5"/>
<connect gate="G$1" pin="C3" pad="6"/>
<connect gate="G$1" pin="C4" pad="12"/>
<connect gate="G$1" pin="VDD" pad="14"/>
<connect gate="G$1" pin="VSS" pad="7"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="OPAMP-TLC2274" prefix="IC">
<gates>
<gate name="A" symbol="OPAMP" x="0" y="0"/>
<gate name="B" symbol="OPAMP" x="0" y="-12.7"/>
<gate name="C" symbol="OPAMP" x="0" y="-25.4"/>
<gate name="D" symbol="OPAMP" x="0" y="-38.1"/>
<gate name="P" symbol="POWER-PINS" x="-30.48" y="-2.54"/>
</gates>
<devices>
<device name="" package="TSSOP-14">
<connects>
<connect gate="A" pin="+IN" pad="3"/>
<connect gate="A" pin="-IN" pad="2"/>
<connect gate="A" pin="OUT" pad="1"/>
<connect gate="B" pin="+IN" pad="5"/>
<connect gate="B" pin="-IN" pad="6"/>
<connect gate="B" pin="OUT" pad="7"/>
<connect gate="C" pin="+IN" pad="10"/>
<connect gate="C" pin="-IN" pad="9"/>
<connect gate="C" pin="OUT" pad="8"/>
<connect gate="D" pin="+IN" pad="12"/>
<connect gate="D" pin="-IN" pad="13"/>
<connect gate="D" pin="OUT" pad="14"/>
<connect gate="P" pin="V+" pad="4"/>
<connect gate="P" pin="V-" pad="11"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="74XX244-OCTAL-TRISTATE-BUFFERS" prefix="IC">
<gates>
<gate name="G$1" symbol="74XX244-OCTAL-TRISTATE-BUFFERS" x="-10.16" y="-10.16"/>
</gates>
<devices>
<device name="" package="TSSOP-20">
<connects>
<connect gate="G$1" pin="1A1" pad="2"/>
<connect gate="G$1" pin="1A2" pad="4"/>
<connect gate="G$1" pin="1A3" pad="6"/>
<connect gate="G$1" pin="1A4" pad="8"/>
<connect gate="G$1" pin="1NOE" pad="1"/>
<connect gate="G$1" pin="1Y1" pad="18"/>
<connect gate="G$1" pin="1Y2" pad="16"/>
<connect gate="G$1" pin="1Y3" pad="14"/>
<connect gate="G$1" pin="1Y4" pad="12"/>
<connect gate="G$1" pin="2A1" pad="11"/>
<connect gate="G$1" pin="2A2" pad="13"/>
<connect gate="G$1" pin="2A3" pad="15"/>
<connect gate="G$1" pin="2A4" pad="17"/>
<connect gate="G$1" pin="2NOE" pad="19"/>
<connect gate="G$1" pin="2Y1" pad="9"/>
<connect gate="G$1" pin="2Y2" pad="7"/>
<connect gate="G$1" pin="2Y3" pad="5"/>
<connect gate="G$1" pin="2Y4" pad="3"/>
<connect gate="G$1" pin="VDD" pad="20"/>
<connect gate="G$1" pin="VSS" pad="10"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="HCU-INVERTER" prefix="IC">
<gates>
<gate name="G$1" symbol="INVERTER" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SMV-5">
<connects>
<connect gate="G$1" pin="A" pad="2"/>
<connect gate="G$1" pin="VDD" pad="5"/>
<connect gate="G$1" pin="VSS" pad="3"/>
<connect gate="G$1" pin="Y" pad="4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="USB-MINIB" prefix="SV">
<gates>
<gate name="G$1" symbol="USB-MINIB" x="0" y="-2.54"/>
</gates>
<devices>
<device name="" package="USB-MINIB-SMD">
<connects>
<connect gate="G$1" pin="D+/3" pad="3"/>
<connect gate="G$1" pin="D-/2" pad="2"/>
<connect gate="G$1" pin="GND/5" pad="5"/>
<connect gate="G$1" pin="ID/4" pad="4"/>
<connect gate="G$1" pin="SHLD0" pad="TAB0"/>
<connect gate="G$1" pin="SHLD1" pad="TAB1"/>
<connect gate="G$1" pin="SHLD2" pad="TAB2"/>
<connect gate="G$1" pin="SHLD3" pad="TAB3"/>
<connect gate="G$1" pin="VBUS/1" pad="1"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply2">
<packages>
</packages>
<symbols>
<symbol name="GND">
<wire x1="-1.27" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="0" y2="-1.27" width="0.254" layer="94"/>
<wire x1="0" y1="-1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<text x="-1.905" y="-3.175" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="SUPPLY">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="GND" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply1">
<packages>
</packages>
<symbols>
<symbol name="+3V3">
<wire x1="1.27" y1="-1.905" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<text x="-2.54" y="-5.08" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="+3V3" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
<symbol name="VDD">
<wire x1="1.27" y1="-1.905" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<wire x1="0" y1="1.27" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<wire x1="1.27" y1="-1.905" x2="0" y2="1.27" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="VDD" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="+3V3" prefix="+3V3">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="+3V3" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="VDD" prefix="VDD">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="VDD" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="proxmark3">
<description>Generated from &lt;b&gt;proxmark3.sch&lt;/b&gt;&lt;p&gt;
by exp-lbrs.ulp</description>
<packages>
<package name="--MERGED_TQ-SMD-RELAY">
<wire x1="-7" y1="-6.6" x2="7" y2="-6.6" width="0.254" layer="21"/>
<wire x1="-7" y1="6.6" x2="7" y2="6.6" width="0.254" layer="21"/>
<wire x1="-7" y1="6.6" x2="-7" y2="-6.6" width="0.254" layer="21"/>
<wire x1="7" y1="6.6" x2="7" y2="-6.6" width="0.254" layer="21"/>
<wire x1="-4" y1="2" x2="-6" y2="2" width="0.254" layer="21"/>
<wire x1="-6" y1="2" x2="-6" y2="-2" width="0.254" layer="21"/>
<wire x1="-6" y1="-2" x2="-4" y2="-2" width="0.254" layer="21"/>
<wire x1="-4" y1="-2" x2="-4" y2="2" width="0.254" layer="21"/>
<smd name="1" x="-5.08" y="-4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="2" x="-2.54" y="-4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="3" x="0" y="-4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="4" x="2.54" y="-4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="5" x="5.08" y="-4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="6" x="5.08" y="4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="7" x="2.54" y="4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="8" x="0" y="4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="9" x="-2.54" y="4.78" dx="1" dy="3" layer="1" rot="R180"/>
<smd name="10" x="-5.08" y="4.78" dx="1" dy="3" layer="1" rot="R180"/>
<text x="6.35" y="-2.54" size="1.27" layer="21" ratio="17" rot="R90">&gt;NAME</text>
</package>
<package name="--MERGED_SOT23-3LEAD">
<wire x1="-1" y1="1" x2="-1" y2="-0.1" width="0.127" layer="21"/>
<wire x1="-0.3" y1="-0.7" x2="0.3" y2="-0.7" width="0.127" layer="21"/>
<wire x1="1" y1="-0.1" x2="1" y2="1" width="0.127" layer="21"/>
<wire x1="1" y1="1" x2="0.6" y2="1" width="0.127" layer="21"/>
<wire x1="-1" y1="1" x2="-0.6" y2="1" width="0.127" layer="21"/>
<smd name="1" x="-0.9398" y="-0.762" dx="1.016" dy="1.016" layer="1"/>
<smd name="2" x="0.9652" y="-0.762" dx="1.016" dy="1.016" layer="1"/>
<smd name="3" x="0.0127" y="1.143" dx="1.016" dy="1.778" layer="1"/>
<text x="2.4326" y="-0.1281" size="1.016" layer="25" font="vector" ratio="18" rot="R90">&gt;NAME</text>
</package>
<package name="--MERGED_SOT-23-5">
<wire x1="-1.3398" y1="-0.4224" x2="1.2602" y2="-0.4224" width="0.127" layer="21"/>
<wire x1="1.2602" y1="-0.4224" x2="1.2602" y2="0.4776" width="0.127" layer="21"/>
<wire x1="1.2602" y1="0.4776" x2="-1.3398" y2="0.4776" width="0.127" layer="21"/>
<wire x1="-1.3398" y1="0.4776" x2="-1.3398" y2="-0.4224" width="0.127" layer="21"/>
<smd name="1" x="-0.9398" y="-1.4224" dx="0.4" dy="1.5" layer="1"/>
<smd name="2" x="0.0102" y="-1.4224" dx="0.4" dy="1.5" layer="1"/>
<smd name="3" x="0.9602" y="-1.4224" dx="0.4" dy="1.5" layer="1"/>
<smd name="4" x="0.9602" y="1.4276" dx="0.4" dy="1.5" layer="1"/>
<smd name="5" x="-0.9398" y="1.4276" dx="0.4" dy="1.5" layer="1"/>
<text x="1.4602" y="2.1776" size="1.27" layer="21" font="vector" rot="R270">&gt;NAME</text>
</package>
<package name="--MERGED_HEADER-MALE-6X1-0.100-UNKEYED">
<wire x1="-1.17" y1="7.62" x2="1.17" y2="7.62" width="0.254" layer="21"/>
<wire x1="1.17" y1="7.62" x2="1.17" y2="-7.62" width="0.254" layer="21"/>
<wire x1="1.17" y1="-7.62" x2="-1.17" y2="-7.62" width="0.254" layer="21"/>
<wire x1="-1.17" y1="-7.62" x2="-1.17" y2="7.62" width="0.254" layer="21"/>
<pad name="1" x="0" y="6.35" drill="1.143" shape="square"/>
<pad name="2" x="0" y="3.81" drill="1.143"/>
<pad name="3" x="0" y="1.27" drill="1.143"/>
<pad name="4" x="0" y="-1.27" drill="1.143"/>
<pad name="5" x="0" y="-3.81" drill="1.143"/>
<pad name="6" x="0" y="-6.35" drill="1.143"/>
<text x="1.905" y="6.985" size="1.27" layer="21" ratio="22" rot="R270">&gt;NAME</text>
</package>
<package name="--MERGED_RLC_0603">
<wire x1="-1.4732" y1="0.6858" x2="1.4732" y2="0.6858" width="0.15" layer="21"/>
<wire x1="1.4732" y1="0.6858" x2="1.4732" y2="-0.6858" width="0.15" layer="21"/>
<wire x1="1.4732" y1="-0.6858" x2="-1.4732" y2="-0.6858" width="0.15" layer="21"/>
<wire x1="-1.4732" y1="-0.6858" x2="-1.4732" y2="0.6858" width="0.15" layer="21"/>
<smd name="1" x="-0.85" y="0" dx="1" dy="1.1" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1" dy="1.1" layer="1"/>
<text x="-1.1938" y="-0.4064" size="0.762" layer="51">&gt;NAME</text>
</package>
<package name="--MERGED_RLC_0805">
<wire x1="-1.651" y1="0.9144" x2="1.651" y2="0.9144" width="0.254" layer="21"/>
<wire x1="1.651" y1="0.9144" x2="1.651" y2="-0.9144" width="0.254" layer="21"/>
<wire x1="1.651" y1="-0.9144" x2="-1.651" y2="-0.9144" width="0.254" layer="21"/>
<wire x1="-1.651" y1="-0.9144" x2="-1.651" y2="0.9144" width="0.254" layer="21"/>
<smd name="1" x="-0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<smd name="2" x="0.85" y="0" dx="1.3" dy="1.5" layer="1"/>
<text x="-1.4732" y="-0.635" size="1.27" layer="51">&gt;NAME</text>
</package>
<package name="--MERGED_RLC_1210">
<wire x1="-2.4638" y1="1.6764" x2="2.4638" y2="1.6764" width="0.3048" layer="21"/>
<wire x1="2.4638" y1="1.6764" x2="2.4638" y2="-1.651" width="0.3048" layer="21"/>
<wire x1="2.4638" y1="-1.651" x2="-2.4638" y2="-1.651" width="0.3048" layer="21"/>
<wire x1="-2.4638" y1="-1.651" x2="-2.4638" y2="1.6764" width="0.3048" layer="21"/>
<smd name="1" x="-1.5" y="0" dx="1.5" dy="2.9" layer="1"/>
<smd name="2" x="1.5" y="0" dx="1.5" dy="2.9" layer="1"/>
<text x="-2.2096" y="-0.736" size="1.524" layer="51">&gt;NAME</text>
</package>
</packages>
<symbols>
<symbol name="--MERGED_RELAY-COIL">
<wire x1="0" y1="0" x2="1.27" y2="0" width="0.1524" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="-1.27" width="0.1524" layer="94" curve="-90"/>
<wire x1="2.54" y1="-1.27" x2="1.27" y2="-2.54" width="0.1524" layer="94" curve="-90"/>
<wire x1="1.27" y1="-2.54" x2="2.54" y2="-3.81" width="0.1524" layer="94" curve="-90"/>
<wire x1="2.54" y1="-3.81" x2="1.27" y2="-5.08" width="0.1524" layer="94" curve="-90"/>
<wire x1="1.27" y1="-7.62" x2="2.54" y2="-6.35" width="0.1524" layer="94" curve="90"/>
<wire x1="2.54" y1="-6.35" x2="1.27" y2="-5.08" width="0.1524" layer="94" curve="90"/>
<wire x1="1.27" y1="-7.62" x2="0" y2="-7.62" width="0.1524" layer="94"/>
<pin name="P$1" x="-2.54" y="0" visible="pad" length="short" direction="pas"/>
<pin name="P$2" x="-2.54" y="-7.62" visible="pad" length="short" direction="pas"/>
<text x="3.048" y="0" size="1.6764" layer="95" rot="R270">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_RELAY-NO-NC">
<wire x1="0" y1="0" x2="-0.635" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0.635" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="10.16" y1="0" x2="9.525" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="10.16" y1="0" x2="10.795" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="5.08" y1="0.635" x2="5.08" y2="0" width="0.1524" layer="94"/>
<wire x1="-1.778" y1="-0.254" x2="11.684" y2="1.524" width="0.254" layer="94"/>
<pin name="COM" x="5.08" y="-5.08" visible="pad" length="middle" direction="pas" rot="R90"/>
<pin name="NC" x="0" y="-5.08" visible="pad" length="middle" direction="pas" rot="R90"/>
<pin name="NO" x="10.16" y="-5.08" visible="pad" length="middle" direction="pas" rot="R90"/>
<text x="-0.508" y="1.27" size="1.6764" layer="95">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_DIODE">
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<pin name="A" x="-2.54" y="0" visible="off" length="short" direction="pas"/>
<pin name="C" x="2.54" y="0" visible="off" length="short" direction="pas" rot="R180"/>
<text x="2.54" y="0.4826" size="1.778" layer="95">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_NPN-THIN-SYMBOL">
<wire x1="0" y1="1.905" x2="0" y2="0.635" width="0.3048" layer="94"/>
<wire x1="0" y1="0.635" x2="0" y2="-0.635" width="0.3048" layer="94"/>
<wire x1="0" y1="-0.635" x2="0" y2="-1.905" width="0.3048" layer="94"/>
<wire x1="0" y1="0.635" x2="2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="0" y2="-0.635" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="2.54" y2="-5.08" width="0.1524" layer="94"/>
<wire x1="2.54" y1="2.54" x2="2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-2.54" x2="1.905" y2="-1.27" width="0.1524" layer="94"/>
<wire x1="1.905" y1="-1.27" x2="1.27" y2="-1.905" width="0.1524" layer="94"/>
<wire x1="1.27" y1="-1.905" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<pin name="B" x="-2.54" y="0" visible="pad" length="short"/>
<pin name="C" x="2.54" y="5.08" visible="pad" length="point" rot="R180"/>
<pin name="E" x="2.54" y="-5.08" visible="pad" length="point" rot="R180"/>
<text x="-2.8525" y="3.2215" size="1.4224" layer="95">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_PQ1X331M2ZP-3V3-REG">
<wire x1="0" y1="2.54" x2="0" y2="-7.62" width="0.254" layer="94"/>
<wire x1="0" y1="-7.62" x2="17.78" y2="-7.62" width="0.254" layer="94"/>
<wire x1="17.78" y1="-7.62" x2="17.78" y2="2.54" width="0.254" layer="94"/>
<wire x1="17.78" y1="2.54" x2="0" y2="2.54" width="0.254" layer="94"/>
<pin name="GND" x="-5.08" y="-2.54" length="middle"/>
<pin name="NR" x="22.86" y="-5.08" length="middle" rot="R180"/>
<pin name="VC" x="-5.08" y="-5.08" length="middle"/>
<pin name="VIN" x="-5.08" y="0" length="middle"/>
<pin name="VO" x="22.86" y="0" length="middle" rot="R180"/>
<text x="17.6045" y="-8.0371" size="1.778" layer="95" font="vector" rot="R180">&gt;NAME</text>
<text x="0.2195" y="3.0304" size="1.778" layer="95" font="vector">PQ1X331M2ZP 3v3 LDO</text>
</symbol>
<symbol name="--MERGED_HEADER-MALE-6X1">
<wire x1="-3.81" y1="12.7" x2="1.27" y2="12.7" width="0.4064" layer="94"/>
<wire x1="-1.27" y1="5.08" x2="-2.54" y2="5.08" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="7.62" x2="-2.54" y2="7.62" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="10.16" x2="-2.54" y2="10.16" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="0" x2="-2.54" y2="0" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="2.54" x2="-2.54" y2="2.54" width="0.6096" layer="94"/>
<wire x1="-1.27" y1="-2.54" x2="-2.54" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-5.08" x2="1.27" y2="12.7" width="0.4064" layer="94"/>
<wire x1="-3.81" y1="12.7" x2="-3.81" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="1.27" y1="-5.08" x2="-3.81" y2="-5.08" width="0.4064" layer="94"/>
<pin name="1" x="-7.62" y="10.16" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="2" x="-7.62" y="7.62" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="3" x="-7.62" y="5.08" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="4" x="-7.62" y="2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="5" x="-7.62" y="0" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="6" x="-7.62" y="-2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<text x="1.27" y="-5.842" size="1.778" layer="95" rot="R180">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_RESET-GENERATOR-3TERM">
<wire x1="-2.54" y1="0" x2="-12.7" y2="0" width="0.254" layer="94"/>
<wire x1="-12.7" y1="0" x2="-12.7" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-15.24" x2="-2.54" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-15.24" x2="-2.54" y2="0" width="0.254" layer="94"/>
<pin name="OUT" x="2.54" y="-7.62" length="middle" rot="R180"/>
<pin name="V+" x="-10.16" y="5.08" length="middle" rot="R270"/>
<pin name="V-" x="-10.16" y="-20.32" length="middle" rot="R90"/>
<text x="-8.6526" y="0.6687" size="1.778" layer="95">&gt;NAME</text>
<text x="-13.4709" y="-14.4837" size="1.778" layer="95" rot="R90">&gt;VALUE</text>
</symbol>
<symbol name="--MERGED_DUAL-TVS-COMMON-ANODE">
<circle x="0" y="1.27" radius="0.127" width="0.254" layer="94"/>
<wire x1="-3.81" y1="5.08" x2="-2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="-1.27" y2="5.08" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="-3.81" y2="2.54" width="0.1524" layer="94"/>
<wire x1="-3.81" y1="2.54" x2="-2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="-1.27" y2="2.54" width="0.1524" layer="94"/>
<wire x1="-1.27" y1="2.54" x2="-2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="1.27" y1="2.54" x2="2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="2.54" y1="2.54" x2="3.81" y2="2.54" width="0.1524" layer="94"/>
<wire x1="3.81" y1="2.54" x2="2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="2.54" y1="5.08" x2="1.27" y2="2.54" width="0.1524" layer="94"/>
<wire x1="1.27" y1="5.08" x2="2.54" y2="5.08" width="0.1524" layer="94"/>
<wire x1="2.54" y1="5.08" x2="3.81" y2="5.08" width="0.1524" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="1.27" width="0.1524" layer="94"/>
<wire x1="0" y1="1.27" x2="2.54" y2="1.27" width="0.1524" layer="94"/>
<wire x1="2.54" y1="1.27" x2="2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="1.27" x2="-2.54" y2="1.27" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="1.27" x2="-2.54" y2="2.54" width="0.1524" layer="94"/>
<wire x1="-1.27" y1="5.08" x2="-0.635" y2="5.715" width="0.1524" layer="94"/>
<wire x1="-3.81" y1="5.08" x2="-4.445" y2="4.445" width="0.1524" layer="94"/>
<wire x1="1.27" y1="5.08" x2="0.635" y2="4.445" width="0.1524" layer="94"/>
<wire x1="4.445" y1="5.715" x2="3.81" y2="5.08" width="0.1524" layer="94"/>
<pin name="A" x="0" y="0" visible="pad" length="point" rot="R90"/>
<pin name="K1" x="-2.54" y="7.62" visible="pad" length="short" rot="R270"/>
<pin name="K2" x="2.54" y="7.62" visible="pad" length="short" rot="R270"/>
<text x="4.8316" y="7.183" size="1.778" layer="95">&gt;NAME</text>
</symbol>
<symbol name="--MERGED_CAPACITOR">
<rectangle x1="-2.032" y1="1.524" x2="2.032" y2="2.032" layer="94"/>
<rectangle x1="-2.032" y1="0.508" x2="2.032" y2="1.016" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="0.508" width="0.1524" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="2.032" width="0.1524" layer="94"/>
<pin name="1" x="0" y="5.08" visible="off" length="short" direction="pas" swaplevel="1" rot="R270"/>
<pin name="2" x="0" y="-2.54" visible="off" length="short" direction="pas" swaplevel="1" rot="R90"/>
<text x="1.524" y="2.921" size="1.778" layer="95">&gt;NAME</text>
<text x="1.27" y="-1.905" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="--MERGED_RELAY-DPDT" prefix="RLY">
<gates>
<gate name="A" symbol="--MERGED_RELAY-NO-NC" x="-7.62" y="2.54"/>
<gate name="B" symbol="--MERGED_RELAY-NO-NC" x="10.16" y="2.54"/>
<gate name="L" symbol="--MERGED_RELAY-COIL" x="-2.54" y="17.78"/>
</gates>
<devices>
<device name="" package="--MERGED_TQ-SMD-RELAY">
<connects>
<connect gate="A" pin="COM" pad="8"/>
<connect gate="A" pin="NC" pad="9"/>
<connect gate="A" pin="NO" pad="7"/>
<connect gate="B" pin="COM" pad="3"/>
<connect gate="B" pin="NC" pad="2"/>
<connect gate="B" pin="NO" pad="4"/>
<connect gate="L" pin="P$1" pad="1"/>
<connect gate="L" pin="P$2" pad="10"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_DIODE-SMD-SOT23" prefix="D">
<gates>
<gate name="G$1" symbol="--MERGED_DIODE" x="2.54" y="0"/>
</gates>
<devices>
<device name="" package="--MERGED_SOT23-3LEAD">
<connects>
<connect gate="G$1" pin="A" pad="1"/>
<connect gate="G$1" pin="C" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_NPN-SOT23-2N3904" prefix="Q">
<gates>
<gate name="G$1" symbol="--MERGED_NPN-THIN-SYMBOL" x="0" y="0"/>
</gates>
<devices>
<device name="" package="--MERGED_SOT23-3LEAD">
<connects>
<connect gate="G$1" pin="B" pad="1"/>
<connect gate="G$1" pin="C" pad="3"/>
<connect gate="G$1" pin="E" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_PQ1X331M2ZP-3V3-LDO-SOT-23-5" prefix="IC">
<gates>
<gate name="G$1" symbol="--MERGED_PQ1X331M2ZP-3V3-REG" x="-17.78" y="-2.54"/>
</gates>
<devices>
<device name="" package="--MERGED_SOT-23-5">
<connects>
<connect gate="G$1" pin="GND" pad="2"/>
<connect gate="G$1" pin="NR" pad="4"/>
<connect gate="G$1" pin="VC" pad="3"/>
<connect gate="G$1" pin="VIN" pad="1"/>
<connect gate="G$1" pin="VO" pad="5"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_HEADER-MALE-6X1" prefix="SV">
<gates>
<gate name="G$1" symbol="--MERGED_HEADER-MALE-6X1" x="0" y="-10.16"/>
</gates>
<devices>
<device name="" package="--MERGED_HEADER-MALE-6X1-0.100-UNKEYED">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_MCP100" prefix="IC">
<gates>
<gate name="G$1" symbol="--MERGED_RESET-GENERATOR-3TERM" x="2.54" y="7.62"/>
</gates>
<devices>
<device name="" package="--MERGED_SOT23-3LEAD">
<connects>
<connect gate="G$1" pin="OUT" pad="1"/>
<connect gate="G$1" pin="V+" pad="2"/>
<connect gate="G$1" pin="V-" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_DUAL-TVS-COMMON-ANODE" prefix="D">
<gates>
<gate name="G$1" symbol="--MERGED_DUAL-TVS-COMMON-ANODE" x="0" y="0"/>
</gates>
<devices>
<device name="" package="--MERGED_SOT23-3LEAD">
<connects>
<connect gate="G$1" pin="A" pad="3"/>
<connect gate="G$1" pin="K1" pad="1"/>
<connect gate="G$1" pin="K2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="--MERGED_CAPACITOR" prefix="C" uservalue="yes">
<gates>
<gate name="G$1" symbol="--MERGED_CAPACITOR" x="2.54" y="0"/>
</gates>
<devices>
<device name="0603" package="--MERGED_RLC_0603">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="0805" package="--MERGED_RLC_0805">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="1210" package="--MERGED_RLC_1210">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="FRAME1" library="frames" deviceset="A4L-LOC" device=""/>
<part name="IC3" library="proxmark3" deviceset="--MERGED_PQ1X331M2ZP-3V3-LDO-SOT-23-5" device=""/>
<part name="SV1" library="  merged" deviceset="USB-MINIB" device=""/>
<part name="V1" library="supply2" deviceset="GND" device=""/>
<part name="C1" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="V2" library="supply2" deviceset="GND" device=""/>
<part name="V6" library="supply2" deviceset="GND" device=""/>
<part name="U$3" library="supply1" deviceset="+3V3" device=""/>
<part name="VDD1" library="supply1" deviceset="VDD" device=""/>
<part name="U1" library="  merged" deviceset="FERRITE" device="0805"/>
<part name="C21" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C22" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C23" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C24" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="VDD7" library="supply1" deviceset="VDD" device=""/>
<part name="V29" library="supply2" deviceset="GND" device=""/>
<part name="C25" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C26" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="U$8" library="supply1" deviceset="+3V3" device=""/>
<part name="V30" library="supply2" deviceset="GND" device=""/>
<part name="V31" library="  merged" deviceset="SUPPLY_2V5" device=""/>
<part name="C29" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C30" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C31" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="V32" library="supply2" deviceset="GND" device=""/>
<part name="IC12" library="  merged" deviceset="LP2989-LDO" device="SOIC" value="LP2989AIM-2.5"/>
<part name="V35" library="supply2" deviceset="GND" device=""/>
<part name="R23" library="  merged" deviceset="RESISTOR" device="0603" value="1k"/>
<part name="V36" library="supply2" deviceset="GND" device=""/>
<part name="V37" library="  merged" deviceset="SUPPLY_2V5" device=""/>
<part name="C33" library="  merged" deviceset="CAPACITOR" device="1210" value="4u7"/>
<part name="IC13" library="proxmark3" deviceset="--MERGED_PQ1X331M2ZP-3V3-LDO-SOT-23-5" device=""/>
<part name="C27" library="  merged" deviceset="CAPACITOR" device="1210" value="4u7"/>
<part name="C28" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C32" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="V38" library="supply2" deviceset="GND" device=""/>
<part name="C34" library="  merged" deviceset="CAPACITOR" device="1210" value="4u7"/>
<part name="V39" library="supply2" deviceset="GND" device=""/>
<part name="C37" library="  merged" deviceset="CAPACITOR-POLARIZED" device="ALCHIP-MZA-F80" value="100u"/>
<part name="C38" library="  merged" deviceset="CAPACITOR-POLARIZED" device="ALCHIP-MZA-F80" value="100u"/>
<part name="TP6" library="  merged" deviceset="TESTPAD" device="KEYSTONE-PTH"/>
<part name="V51" library="supply2" deviceset="GND" device=""/>
<part name="C44" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="D8" library="proxmark3" deviceset="--MERGED_DUAL-TVS-COMMON-ANODE" device=""/>
<part name="V54" library="supply2" deviceset="GND" device=""/>
<part name="C46" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C47" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="Q2" library="  merged" deviceset="IRF7307" device=""/>
<part name="V25" library="supply2" deviceset="GND" device=""/>
<part name="R56" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="R57" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="R58" library="  merged" deviceset="RESISTOR" device="0603" value="dnp"/>
<part name="FRAME2" library="frames" deviceset="A4L-LOC" device=""/>
<part name="IC1" library="  merged" deviceset="SPARTAN-II-XC2S30-100-VQFP" device=""/>
<part name="V3" library="supply2" deviceset="GND" device=""/>
<part name="V4" library="  merged" deviceset="SUPPLY_2V5" device=""/>
<part name="XT1" library="  merged" deviceset="XTAL-SMD-CITIZEN-CS10" device=""/>
<part name="IC4" library="  merged" deviceset="HCU-INVERTER" device=""/>
<part name="R1" library="  merged" deviceset="RESISTOR" device="0603" value="1meg"/>
<part name="C2" library="  merged" deviceset="CAPACITOR" device="0603" value="22p"/>
<part name="C3" library="  merged" deviceset="CAPACITOR" device="0603" value="22p"/>
<part name="R2" library="  merged" deviceset="RESISTOR" device="0603" value="100"/>
<part name="V7" library="supply2" deviceset="GND" device=""/>
<part name="IC8" library="  merged" deviceset="TLC5540" device=""/>
<part name="VDD4" library="supply1" deviceset="VDD" device=""/>
<part name="V24" library="supply2" deviceset="GND" device=""/>
<part name="C18" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C19" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="SV5" library="proxmark3" deviceset="--MERGED_HEADER-MALE-6X1" device=""/>
<part name="V28" library="supply2" deviceset="GND" device=""/>
<part name="U$5" library="supply1" deviceset="+3V3" device=""/>
<part name="R39" library="  merged" deviceset="RESISTOR" device="0603" value="1k"/>
<part name="R44" library="  merged" deviceset="RESISTOR" device="0603" value="1k"/>
<part name="R47" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="VDD10" library="supply1" deviceset="VDD" device=""/>
<part name="TP7" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="R52" library="  merged" deviceset="RESISTOR" device="0603" value="100"/>
<part name="VDD11" library="supply1" deviceset="VDD" device=""/>
<part name="R53" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="R54" library="  merged" deviceset="RESISTOR" device="0603" value="100"/>
<part name="V55" library="supply2" deviceset="GND" device=""/>
<part name="R59" library="  merged" deviceset="RESISTOR" device="0603" value="3k3"/>
<part name="FRAME3" library="frames" deviceset="A4L-LOC" device=""/>
<part name="IC2" library="  merged" deviceset="AT91SAM7S64" device=""/>
<part name="V5" library="supply2" deviceset="GND" device=""/>
<part name="U$2" library="supply1" deviceset="+3V3" device=""/>
<part name="R3" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="C4" library="  merged" deviceset="CAPACITOR" device="0603" value="33n"/>
<part name="C5" library="  merged" deviceset="CAPACITOR" device="0603" value="2n"/>
<part name="V8" library="supply2" deviceset="GND" device=""/>
<part name="XT2" library="  merged" deviceset="XTAL-SMD-CITIZEN-CS10" device=""/>
<part name="C6" library="proxmark3" deviceset="--MERGED_CAPACITOR" device="0603" value="22p"/>
<part name="C7" library="proxmark3" deviceset="--MERGED_CAPACITOR" device="0603" value="22p"/>
<part name="V9" library="supply2" deviceset="GND" device=""/>
<part name="R4" library="  merged" deviceset="RESISTOR" device="0603" value="27"/>
<part name="R5" library="  merged" deviceset="RESISTOR" device="0603" value="27"/>
<part name="R6" library="  merged" deviceset="RESISTOR" device="0603" value="1k5"/>
<part name="C8" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="C9" library="  merged" deviceset="CAPACITOR" device="1210" value="2u2"/>
<part name="V10" library="supply2" deviceset="GND" device=""/>
<part name="SV3" library="  merged" deviceset="HEADER-MALE-10X2" device=""/>
<part name="V11" library="supply2" deviceset="GND" device=""/>
<part name="U$6" library="supply1" deviceset="+3V3" device=""/>
<part name="R7" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="V12" library="supply2" deviceset="GND" device=""/>
<part name="R8" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="U$7" library="supply1" deviceset="+3V3" device=""/>
<part name="D4" library="  merged" deviceset="LED" device=""/>
<part name="D5" library="  merged" deviceset="LED" device=""/>
<part name="D6" library="  merged" deviceset="LED" device=""/>
<part name="R24" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="R25" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="R26" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="V40" library="supply2" deviceset="GND" device=""/>
<part name="R43" library="  merged" deviceset="RESISTOR" device="0603" value="1k"/>
<part name="U$9" library="supply1" deviceset="+3V3" device=""/>
<part name="SW1" library="  merged" deviceset="TACTSWITCH" device=""/>
<part name="V50" library="supply2" deviceset="GND" device=""/>
<part name="D9" library="  merged" deviceset="LED" device=""/>
<part name="R55" library="  merged" deviceset="RESISTOR" device="0603" value="330"/>
<part name="TP8" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="IC7" library="proxmark3" deviceset="--MERGED_MCP100" device=""/>
<part name="FRAME4" library="frames" deviceset="A4L-LOC" device=""/>
<part name="IC6" library="  merged" deviceset="OPAMP-TLC2274" device="" value="MCP6294"/>
<part name="R9" library="  merged" deviceset="RESISTOR" device="0603" value="100k"/>
<part name="R10" library="  merged" deviceset="RESISTOR" device="0603" value="100k"/>
<part name="V13" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="V14" library="supply2" deviceset="GND" device=""/>
<part name="C10" library="  merged" deviceset="CAPACITOR" device="0603" value="100n"/>
<part name="V15" library="supply2" deviceset="GND" device=""/>
<part name="D1" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="R11" library="  merged" deviceset="RESISTOR" device="0603" value="510k"/>
<part name="C11" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="C12" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="R12" library="  merged" deviceset="RESISTOR" device="0603" value="100k"/>
<part name="V16" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="R13" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="V17" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="R14" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="C13" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="V18" library="supply2" deviceset="GND" device=""/>
<part name="D2" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="C14" library="  merged" deviceset="CAPACITOR" device="0603" value="22p"/>
<part name="D3" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="V19" library="supply2" deviceset="GND" device=""/>
<part name="C15" library="  merged" deviceset="CAPACITOR" device="0603" value="130p"/>
<part name="R15" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="R16" library="  merged" deviceset="RESISTOR" device="0603" value="zerohm"/>
<part name="C16" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="R17" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="V20" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="R18" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="C17" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="R19" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="V21" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="V22" library="supply2" deviceset="GND" device=""/>
<part name="R20" library="  merged" deviceset="RESISTOR" device="0603" value="240k"/>
<part name="V23" library="supply2" deviceset="GND" device=""/>
<part name="VDD2" library="supply1" deviceset="VDD" device=""/>
<part name="VDD3" library="supply1" deviceset="VDD" device=""/>
<part name="IC11" library="  merged" deviceset="CD4066" device=""/>
<part name="V33" library="supply2" deviceset="GND" device=""/>
<part name="VDD8" library="supply1" deviceset="VDD" device=""/>
<part name="IC5" library="  merged" deviceset="COMP-TLV3502" device="SOT-23-8-OR-MSOP-8" value="TLV3502"/>
<part name="V34" library="supply2" deviceset="GND" device=""/>
<part name="R21" library="  merged" deviceset="RESISTOR" device="0603" value="24k"/>
<part name="R30" library="  merged" deviceset="RESISTOR" device="0603" value="10meg"/>
<part name="R31" library="  merged" deviceset="RESISTOR" device="0603" value="240k"/>
<part name="V42" library="supply2" deviceset="GND" device=""/>
<part name="V43" library="supply2" deviceset="GND" device=""/>
<part name="IC14" library="  merged" deviceset="OPAMP-AD8052" device="" value="AD8052"/>
<part name="R33" library="  merged" deviceset="RESISTOR" device="0603" value="2k4"/>
<part name="C40" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="R34" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="C41" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="V45" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="C42" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="C43" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="R32" library="  merged" deviceset="RESISTOR" device="0603" value="zerohm"/>
<part name="R35" library="  merged" deviceset="RESISTOR" device="0603" value="zerohm"/>
<part name="R36" library="  merged" deviceset="RESISTOR" device="0603" value="dnp"/>
<part name="R37" library="  merged" deviceset="RESISTOR" device="0603" value="dnp"/>
<part name="R38" library="  merged" deviceset="RESISTOR" device="0603" value="dnp"/>
<part name="V46" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="RLY1" library="proxmark3" deviceset="--MERGED_RELAY-DPDT" device=""/>
<part name="Q1" library="proxmark3" deviceset="--MERGED_NPN-SOT23-2N3904" device=""/>
<part name="D7" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="VDD9" library="supply1" deviceset="VDD" device=""/>
<part name="V48" library="supply2" deviceset="GND" device=""/>
<part name="R40" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="R41" library="  merged" deviceset="RESISTOR" device="0603" value="10meg"/>
<part name="R42" library="  merged" deviceset="RESISTOR" device="0603" value="1meg"/>
<part name="V49" library="supply2" deviceset="GND" device=""/>
<part name="TP1" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="R48" library="  merged" deviceset="RESISTOR" device="0603" value="dnp"/>
<part name="R49" library="  merged" deviceset="RESISTOR" device="0603" value="2k4"/>
<part name="R50" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="V52" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="V44" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="R51" library="  merged" deviceset="RESISTOR" device="0603" value="2k4"/>
<part name="V53" library="  merged" deviceset="SUPPLY_VMID" device=""/>
<part name="C45" library="  merged" deviceset="CAPACITOR" device="0603" value="100n"/>
<part name="D10" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="D11" library="proxmark3" deviceset="--MERGED_DIODE-SMD-SOT23" device=""/>
<part name="V56" library="supply2" deviceset="GND" device=""/>
<part name="FRAME5" library="frames" deviceset="A4L-LOC" device=""/>
<part name="SV2" library="  merged" deviceset="HIROSE-MQ172-4POS" device=""/>
<part name="IC9" library="  merged" deviceset="74XX244-OCTAL-TRISTATE-BUFFERS" device=""/>
<part name="V26" library="supply2" deviceset="GND" device=""/>
<part name="VDD5" library="supply1" deviceset="VDD" device=""/>
<part name="IC10" library="  merged" deviceset="74XX244-OCTAL-TRISTATE-BUFFERS" device=""/>
<part name="V27" library="supply2" deviceset="GND" device=""/>
<part name="VDD6" library="supply1" deviceset="VDD" device=""/>
<part name="R22" library="  merged" deviceset="RESISTOR" device="0603" value="33"/>
<part name="R27" library="  merged" deviceset="RESISTOR" device="0603" value="33"/>
<part name="R28" library="  merged" deviceset="RESISTOR" device="0603" value="33"/>
<part name="R29" library="  merged" deviceset="RESISTOR" device="0603" value="33"/>
<part name="C39" library="  merged" deviceset="CAPACITOR" device="0603" value="1n"/>
<part name="V47" library="supply2" deviceset="GND" device=""/>
<part name="C20" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="C35" library="  merged" deviceset="CAPACITOR" device="0603" value="47p"/>
<part name="C36" library="  merged" deviceset="CAPACITOR" device="0603" value="dnp"/>
<part name="V41" library="supply2" deviceset="GND" device=""/>
<part name="R45" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="R46" library="  merged" deviceset="RESISTOR" device="0603" value="10k"/>
<part name="TP2" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="TP3" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="TP4" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
<part name="TP5" library="  merged" deviceset="TESTPAD" device="KEYSTONE-SMD"/>
</parts>
<sheets>
<sheet>
<plain>
<text x="165.1" y="17.78" size="2.54" layer="95">power and bypass</text>
</plain>
<instances>
<instance part="FRAME1" gate="G$1" x="0" y="0"/>
<instance part="IC3" gate="G$1" x="162.56" y="111.76"/>
<instance part="SV1" gate="G$1" x="43.18" y="111.76"/>
<instance part="V1" gate="GND" x="50.8" y="83.82"/>
<instance part="C1" gate="G$1" x="81.28" y="104.14"/>
<instance part="V2" gate="GND" x="81.28" y="96.52"/>
<instance part="V6" gate="GND" x="154.94" y="73.66" rot="MR0"/>
<instance part="U$3" gate="G$1" x="200.66" y="121.92" rot="MR0"/>
<instance part="VDD1" gate="G$1" x="147.32" y="93.98" rot="MR0"/>
<instance part="U1" gate="G$1" x="88.9" y="111.76"/>
<instance part="C21" gate="G$1" x="30.48" y="144.78"/>
<instance part="C22" gate="G$1" x="40.64" y="144.78"/>
<instance part="C23" gate="G$1" x="50.8" y="144.78"/>
<instance part="C24" gate="G$1" x="60.96" y="144.78"/>
<instance part="VDD7" gate="G$1" x="30.48" y="157.48"/>
<instance part="V29" gate="GND" x="101.6" y="134.62"/>
<instance part="C25" gate="G$1" x="116.84" y="144.78"/>
<instance part="C26" gate="G$1" x="127" y="144.78"/>
<instance part="U$8" gate="G$1" x="116.84" y="157.48"/>
<instance part="V30" gate="GND" x="137.16" y="134.62"/>
<instance part="V31" gate="G$1" x="198.12" y="157.48"/>
<instance part="C29" gate="G$1" x="198.12" y="144.78"/>
<instance part="C30" gate="G$1" x="208.28" y="144.78"/>
<instance part="C31" gate="G$1" x="218.44" y="144.78"/>
<instance part="V32" gate="GND" x="228.6" y="134.62"/>
<instance part="IC12" gate="G$1" x="182.88" y="63.5"/>
<instance part="V35" gate="GND" x="152.4" y="45.72" rot="MR0"/>
<instance part="R23" gate="G$1" x="137.16" y="50.8" rot="R90"/>
<instance part="V36" gate="GND" x="137.16" y="40.64" rot="MR0"/>
<instance part="V37" gate="G$1" x="195.58" y="68.58" rot="MR0"/>
<instance part="C33" gate="G$1" x="228.6" y="144.78"/>
<instance part="IC13" gate="G$1" x="162.56" y="83.82"/>
<instance part="C27" gate="G$1" x="137.16" y="144.78"/>
<instance part="C28" gate="G$1" x="152.4" y="144.78"/>
<instance part="C32" gate="G$1" x="162.56" y="144.78"/>
<instance part="V38" gate="GND" x="182.88" y="134.62"/>
<instance part="C34" gate="G$1" x="182.88" y="144.78"/>
<instance part="V39" gate="GND" x="154.94" y="101.6" rot="MR0"/>
<instance part="C37" gate="G$1" x="91.44" y="147.32"/>
<instance part="C38" gate="G$1" x="101.6" y="147.32"/>
<instance part="TP6" gate="G$1" x="55.88" y="63.5"/>
<instance part="V51" gate="GND" x="55.88" y="53.34"/>
<instance part="C44" gate="G$1" x="71.12" y="144.78"/>
<instance part="D8" gate="G$1" x="60.96" y="93.98"/>
<instance part="V54" gate="GND" x="60.96" y="88.9"/>
<instance part="C46" gate="G$1" x="81.28" y="144.78"/>
<instance part="C47" gate="G$1" x="172.72" y="144.78"/>
<instance part="Q2" gate="N" x="73.66" y="63.5"/>
<instance part="Q2" gate="P" x="129.54" y="93.98" rot="R180"/>
<instance part="V25" gate="GND" x="76.2" y="53.34"/>
<instance part="R56" gate="G$1" x="119.38" y="101.6" rot="R90"/>
<instance part="R57" gate="G$1" x="111.76" y="93.98"/>
<instance part="R58" gate="G$1" x="137.16" y="93.98" rot="R90"/>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<wire x1="48.26" y1="101.6" x2="50.8" y2="101.6" width="0.1524" layer="91"/>
<wire x1="50.8" y1="101.6" x2="50.8" y2="96.52" width="0.1524" layer="91"/>
<wire x1="50.8" y1="96.52" x2="50.8" y2="93.98" width="0.1524" layer="91"/>
<wire x1="50.8" y1="93.98" x2="50.8" y2="91.44" width="0.1524" layer="91"/>
<wire x1="50.8" y1="91.44" x2="50.8" y2="88.9" width="0.1524" layer="91"/>
<wire x1="50.8" y1="88.9" x2="50.8" y2="86.36" width="0.1524" layer="91"/>
<wire x1="48.26" y1="96.52" x2="50.8" y2="96.52" width="0.1524" layer="91"/>
<wire x1="48.26" y1="93.98" x2="50.8" y2="93.98" width="0.1524" layer="91"/>
<wire x1="48.26" y1="91.44" x2="50.8" y2="91.44" width="0.1524" layer="91"/>
<wire x1="48.26" y1="88.9" x2="50.8" y2="88.9" width="0.1524" layer="91"/>
<junction x="50.8" y="96.52"/>
<junction x="50.8" y="93.98"/>
<junction x="50.8" y="91.44"/>
<junction x="50.8" y="88.9"/>
<pinref part="SV1" gate="G$1" pin="GND/5"/>
<pinref part="V1" gate="GND" pin="GND"/>
<pinref part="SV1" gate="G$1" pin="SHLD0"/>
<pinref part="SV1" gate="G$1" pin="SHLD1"/>
<pinref part="SV1" gate="G$1" pin="SHLD2"/>
<pinref part="SV1" gate="G$1" pin="SHLD3"/>
</segment>
<segment>
<wire x1="81.28" y1="101.6" x2="81.28" y2="99.06" width="0.1524" layer="91"/>
<pinref part="C1" gate="G$1" pin="2"/>
<pinref part="V2" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="137.16" y1="142.24" x2="137.16" y2="139.7" width="0.1524" layer="91"/>
<wire x1="137.16" y1="139.7" x2="137.16" y2="137.16" width="0.1524" layer="91"/>
<wire x1="137.16" y1="139.7" x2="127" y2="139.7" width="0.1524" layer="91"/>
<wire x1="127" y1="139.7" x2="127" y2="142.24" width="0.1524" layer="91"/>
<wire x1="127" y1="139.7" x2="116.84" y2="139.7" width="0.1524" layer="91"/>
<wire x1="116.84" y1="139.7" x2="116.84" y2="142.24" width="0.1524" layer="91"/>
<junction x="137.16" y="139.7"/>
<junction x="127" y="139.7"/>
<pinref part="V30" gate="GND" pin="GND"/>
<pinref part="C26" gate="G$1" pin="2"/>
<pinref part="C25" gate="G$1" pin="2"/>
<pinref part="C27" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="228.6" y1="137.16" x2="228.6" y2="139.7" width="0.1524" layer="91"/>
<wire x1="228.6" y1="139.7" x2="228.6" y2="142.24" width="0.1524" layer="91"/>
<wire x1="228.6" y1="139.7" x2="218.44" y2="139.7" width="0.1524" layer="91"/>
<wire x1="218.44" y1="139.7" x2="218.44" y2="142.24" width="0.1524" layer="91"/>
<wire x1="218.44" y1="139.7" x2="208.28" y2="139.7" width="0.1524" layer="91"/>
<wire x1="208.28" y1="139.7" x2="208.28" y2="142.24" width="0.1524" layer="91"/>
<wire x1="208.28" y1="139.7" x2="198.12" y2="139.7" width="0.1524" layer="91"/>
<wire x1="198.12" y1="139.7" x2="198.12" y2="142.24" width="0.1524" layer="91"/>
<junction x="228.6" y="139.7"/>
<junction x="218.44" y="139.7"/>
<junction x="208.28" y="139.7"/>
<pinref part="V32" gate="GND" pin="GND"/>
<pinref part="C31" gate="G$1" pin="2"/>
<pinref part="C30" gate="G$1" pin="2"/>
<pinref part="C29" gate="G$1" pin="2"/>
<pinref part="C33" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="154.94" y1="50.8" x2="152.4" y2="50.8" width="0.1524" layer="91"/>
<wire x1="152.4" y1="50.8" x2="152.4" y2="48.26" width="0.1524" layer="91"/>
<pinref part="IC12" gate="G$1" pin="GND"/>
<pinref part="V35" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="137.16" y1="43.18" x2="137.16" y2="45.72" width="0.1524" layer="91"/>
<pinref part="V36" gate="GND" pin="GND"/>
<pinref part="R23" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="182.88" y1="142.24" x2="182.88" y2="139.7" width="0.1524" layer="91"/>
<wire x1="182.88" y1="139.7" x2="182.88" y2="137.16" width="0.1524" layer="91"/>
<wire x1="182.88" y1="139.7" x2="172.72" y2="139.7" width="0.1524" layer="91"/>
<wire x1="172.72" y1="139.7" x2="162.56" y2="139.7" width="0.1524" layer="91"/>
<wire x1="162.56" y1="139.7" x2="162.56" y2="142.24" width="0.1524" layer="91"/>
<wire x1="162.56" y1="139.7" x2="152.4" y2="139.7" width="0.1524" layer="91"/>
<wire x1="152.4" y1="139.7" x2="152.4" y2="142.24" width="0.1524" layer="91"/>
<wire x1="172.72" y1="142.24" x2="172.72" y2="139.7" width="0.1524" layer="91"/>
<junction x="182.88" y="139.7"/>
<junction x="162.56" y="139.7"/>
<junction x="172.72" y="139.7"/>
<pinref part="V38" gate="GND" pin="GND"/>
<pinref part="C32" gate="G$1" pin="2"/>
<pinref part="C28" gate="G$1" pin="2"/>
<pinref part="C34" gate="G$1" pin="2"/>
<pinref part="C47" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="157.48" y1="81.28" x2="154.94" y2="81.28" width="0.1524" layer="91"/>
<wire x1="154.94" y1="81.28" x2="154.94" y2="76.2" width="0.1524" layer="91"/>
<pinref part="IC13" gate="G$1" pin="GND"/>
<pinref part="V6" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="157.48" y1="109.22" x2="154.94" y2="109.22" width="0.1524" layer="91"/>
<wire x1="154.94" y1="109.22" x2="154.94" y2="104.14" width="0.1524" layer="91"/>
<pinref part="IC3" gate="G$1" pin="GND"/>
<pinref part="V39" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="101.6" y1="142.24" x2="101.6" y2="139.7" width="0.1524" layer="91"/>
<wire x1="101.6" y1="139.7" x2="101.6" y2="137.16" width="0.1524" layer="91"/>
<wire x1="60.96" y1="139.7" x2="60.96" y2="142.24" width="0.1524" layer="91"/>
<wire x1="60.96" y1="139.7" x2="50.8" y2="139.7" width="0.1524" layer="91"/>
<wire x1="50.8" y1="139.7" x2="40.64" y2="139.7" width="0.1524" layer="91"/>
<wire x1="40.64" y1="139.7" x2="30.48" y2="139.7" width="0.1524" layer="91"/>
<wire x1="30.48" y1="139.7" x2="30.48" y2="142.24" width="0.1524" layer="91"/>
<wire x1="40.64" y1="142.24" x2="40.64" y2="139.7" width="0.1524" layer="91"/>
<wire x1="50.8" y1="142.24" x2="50.8" y2="139.7" width="0.1524" layer="91"/>
<wire x1="101.6" y1="139.7" x2="91.44" y2="139.7" width="0.1524" layer="91"/>
<wire x1="91.44" y1="139.7" x2="81.28" y2="139.7" width="0.1524" layer="91"/>
<wire x1="81.28" y1="139.7" x2="71.12" y2="139.7" width="0.1524" layer="91"/>
<wire x1="71.12" y1="139.7" x2="60.96" y2="139.7" width="0.1524" layer="91"/>
<wire x1="91.44" y1="142.24" x2="91.44" y2="139.7" width="0.1524" layer="91"/>
<wire x1="71.12" y1="142.24" x2="71.12" y2="139.7" width="0.1524" layer="91"/>
<wire x1="81.28" y1="142.24" x2="81.28" y2="139.7" width="0.1524" layer="91"/>
<junction x="40.64" y="139.7"/>
<junction x="50.8" y="139.7"/>
<junction x="101.6" y="139.7"/>
<junction x="60.96" y="139.7"/>
<junction x="91.44" y="139.7"/>
<junction x="71.12" y="139.7"/>
<junction x="81.28" y="139.7"/>
<pinref part="C38" gate="G$1" pin="-"/>
<pinref part="V29" gate="GND" pin="GND"/>
<pinref part="C24" gate="G$1" pin="2"/>
<pinref part="C21" gate="G$1" pin="2"/>
<pinref part="C22" gate="G$1" pin="2"/>
<pinref part="C23" gate="G$1" pin="2"/>
<pinref part="C37" gate="G$1" pin="-"/>
<pinref part="C44" gate="G$1" pin="2"/>
<pinref part="C46" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="55.88" y1="60.96" x2="55.88" y2="55.88" width="0.1524" layer="91"/>
<pinref part="TP6" gate="G$1" pin="P$1"/>
<pinref part="V51" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="60.96" y1="93.98" x2="60.96" y2="91.44" width="0.1524" layer="91"/>
<pinref part="D8" gate="G$1" pin="A"/>
<pinref part="V54" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="68.58" y1="60.96" x2="66.04" y2="60.96" width="0.1524" layer="91"/>
<wire x1="66.04" y1="60.96" x2="66.04" y2="58.42" width="0.1524" layer="91"/>
<wire x1="73.66" y1="63.5" x2="76.2" y2="63.5" width="0.1524" layer="91"/>
<wire x1="76.2" y1="63.5" x2="76.2" y2="60.96" width="0.1524" layer="91"/>
<wire x1="76.2" y1="60.96" x2="76.2" y2="58.42" width="0.1524" layer="91"/>
<wire x1="76.2" y1="58.42" x2="76.2" y2="55.88" width="0.1524" layer="91"/>
<wire x1="73.66" y1="60.96" x2="76.2" y2="60.96" width="0.1524" layer="91"/>
<wire x1="66.04" y1="58.42" x2="76.2" y2="58.42" width="0.1524" layer="91"/>
<junction x="76.2" y="60.96"/>
<junction x="76.2" y="58.42"/>
<pinref part="Q2" gate="N" pin="G"/>
<pinref part="Q2" gate="N" pin="D"/>
<pinref part="V25" gate="GND" pin="GND"/>
<pinref part="Q2" gate="N" pin="S"/>
</segment>
</net>
<net name="N$1" class="0">
<segment>
<wire x1="48.26" y1="111.76" x2="81.28" y2="111.76" width="0.1524" layer="91"/>
<wire x1="81.28" y1="111.76" x2="86.36" y2="111.76" width="0.1524" layer="91"/>
<wire x1="81.28" y1="109.22" x2="81.28" y2="111.76" width="0.1524" layer="91"/>
<junction x="81.28" y="111.76"/>
<pinref part="SV1" gate="G$1" pin="VBUS/1"/>
<pinref part="C1" gate="G$1" pin="1"/>
<pinref part="U1" gate="G$1" pin="A"/>
</segment>
</net>
<net name="USB_D-" class="0">
<segment>
<wire x1="66.04" y1="109.22" x2="63.5" y2="109.22" width="0.1524" layer="91"/>
<wire x1="63.5" y1="109.22" x2="48.26" y2="109.22" width="0.1524" layer="91"/>
<wire x1="63.5" y1="109.22" x2="63.5" y2="101.6" width="0.1524" layer="91"/>
<junction x="63.5" y="109.22"/>
<label x="66.04" y="109.22" size="1.778" layer="95"/>
<pinref part="SV1" gate="G$1" pin="D-/2"/>
<pinref part="D8" gate="G$1" pin="K2"/>
</segment>
</net>
<net name="USB_D+" class="0">
<segment>
<wire x1="66.04" y1="106.68" x2="58.42" y2="106.68" width="0.1524" layer="91"/>
<wire x1="58.42" y1="106.68" x2="48.26" y2="106.68" width="0.1524" layer="91"/>
<wire x1="58.42" y1="106.68" x2="58.42" y2="101.6" width="0.1524" layer="91"/>
<junction x="58.42" y="106.68"/>
<label x="66.04" y="106.68" size="1.778" layer="95"/>
<pinref part="SV1" gate="G$1" pin="D+/3"/>
<pinref part="D8" gate="G$1" pin="K1"/>
</segment>
</net>
<net name="+3V3" class="0">
<segment>
<wire x1="185.42" y1="111.76" x2="200.66" y2="111.76" width="0.1524" layer="91"/>
<wire x1="200.66" y1="111.76" x2="200.66" y2="119.38" width="0.1524" layer="91"/>
<pinref part="IC3" gate="G$1" pin="VO"/>
<pinref part="U$3" gate="G$1" pin="+3V3"/>
</segment>
<segment>
<wire x1="116.84" y1="149.86" x2="116.84" y2="152.4" width="0.1524" layer="91"/>
<wire x1="116.84" y1="152.4" x2="116.84" y2="154.94" width="0.1524" layer="91"/>
<wire x1="116.84" y1="152.4" x2="127" y2="152.4" width="0.1524" layer="91"/>
<wire x1="127" y1="152.4" x2="127" y2="149.86" width="0.1524" layer="91"/>
<wire x1="127" y1="152.4" x2="137.16" y2="152.4" width="0.1524" layer="91"/>
<wire x1="137.16" y1="152.4" x2="137.16" y2="149.86" width="0.1524" layer="91"/>
<junction x="116.84" y="152.4"/>
<junction x="127" y="152.4"/>
<pinref part="C25" gate="G$1" pin="1"/>
<pinref part="U$8" gate="G$1" pin="+3V3"/>
<pinref part="C26" gate="G$1" pin="1"/>
<pinref part="C27" gate="G$1" pin="1"/>
</segment>
</net>
<net name="VDD" class="0">
<segment>
<wire x1="30.48" y1="154.94" x2="30.48" y2="152.4" width="0.1524" layer="91"/>
<wire x1="30.48" y1="152.4" x2="30.48" y2="149.86" width="0.1524" layer="91"/>
<wire x1="30.48" y1="152.4" x2="40.64" y2="152.4" width="0.1524" layer="91"/>
<wire x1="40.64" y1="152.4" x2="50.8" y2="152.4" width="0.1524" layer="91"/>
<wire x1="50.8" y1="152.4" x2="60.96" y2="152.4" width="0.1524" layer="91"/>
<wire x1="60.96" y1="152.4" x2="60.96" y2="149.86" width="0.1524" layer="91"/>
<wire x1="50.8" y1="152.4" x2="50.8" y2="149.86" width="0.1524" layer="91"/>
<wire x1="40.64" y1="149.86" x2="40.64" y2="152.4" width="0.1524" layer="91"/>
<wire x1="60.96" y1="152.4" x2="71.12" y2="152.4" width="0.1524" layer="91"/>
<wire x1="71.12" y1="152.4" x2="81.28" y2="152.4" width="0.1524" layer="91"/>
<wire x1="81.28" y1="152.4" x2="91.44" y2="152.4" width="0.1524" layer="91"/>
<wire x1="91.44" y1="152.4" x2="101.6" y2="152.4" width="0.1524" layer="91"/>
<wire x1="101.6" y1="152.4" x2="101.6" y2="149.86" width="0.1524" layer="91"/>
<wire x1="91.44" y1="149.86" x2="91.44" y2="152.4" width="0.1524" layer="91"/>
<wire x1="71.12" y1="149.86" x2="71.12" y2="152.4" width="0.1524" layer="91"/>
<wire x1="81.28" y1="149.86" x2="81.28" y2="152.4" width="0.1524" layer="91"/>
<junction x="30.48" y="152.4"/>
<junction x="50.8" y="152.4"/>
<junction x="40.64" y="152.4"/>
<junction x="60.96" y="152.4"/>
<junction x="91.44" y="152.4"/>
<junction x="71.12" y="152.4"/>
<junction x="81.28" y="152.4"/>
<pinref part="VDD7" gate="G$1" pin="VDD"/>
<pinref part="C21" gate="G$1" pin="1"/>
<pinref part="C24" gate="G$1" pin="1"/>
<pinref part="C23" gate="G$1" pin="1"/>
<pinref part="C22" gate="G$1" pin="1"/>
<pinref part="C38" gate="G$1" pin="+"/>
<pinref part="C37" gate="G$1" pin="+"/>
<pinref part="C44" gate="G$1" pin="1"/>
<pinref part="C46" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="132.08" y1="91.44" x2="132.08" y2="86.36" width="0.1524" layer="91"/>
<wire x1="147.32" y1="83.82" x2="147.32" y2="86.36" width="0.1524" layer="91"/>
<wire x1="147.32" y1="86.36" x2="147.32" y2="91.44" width="0.1524" layer="91"/>
<wire x1="157.48" y1="83.82" x2="147.32" y2="83.82" width="0.1524" layer="91"/>
<wire x1="154.94" y1="63.5" x2="147.32" y2="63.5" width="0.1524" layer="91"/>
<wire x1="147.32" y1="63.5" x2="147.32" y2="83.82" width="0.1524" layer="91"/>
<wire x1="132.08" y1="86.36" x2="137.16" y2="86.36" width="0.1524" layer="91"/>
<wire x1="137.16" y1="86.36" x2="147.32" y2="86.36" width="0.1524" layer="91"/>
<wire x1="127" y1="91.44" x2="132.08" y2="91.44" width="0.1524" layer="91"/>
<wire x1="137.16" y1="88.9" x2="137.16" y2="86.36" width="0.1524" layer="91"/>
<junction x="147.32" y="83.82"/>
<junction x="147.32" y="86.36"/>
<junction x="137.16" y="86.36"/>
<pinref part="VDD1" gate="G$1" pin="VDD"/>
<pinref part="IC13" gate="G$1" pin="VIN"/>
<pinref part="IC12" gate="G$1" pin="IN"/>
<pinref part="Q2" gate="P" pin="D"/>
<pinref part="R58" gate="G$1" pin="1"/>
</segment>
</net>
<net name="+2V5" class="0">
<segment>
<wire x1="198.12" y1="154.94" x2="198.12" y2="152.4" width="0.1524" layer="91"/>
<wire x1="198.12" y1="152.4" x2="198.12" y2="149.86" width="0.1524" layer="91"/>
<wire x1="198.12" y1="152.4" x2="208.28" y2="152.4" width="0.1524" layer="91"/>
<wire x1="208.28" y1="152.4" x2="208.28" y2="149.86" width="0.1524" layer="91"/>
<wire x1="208.28" y1="152.4" x2="218.44" y2="152.4" width="0.1524" layer="91"/>
<wire x1="218.44" y1="152.4" x2="218.44" y2="149.86" width="0.1524" layer="91"/>
<wire x1="218.44" y1="152.4" x2="228.6" y2="152.4" width="0.1524" layer="91"/>
<wire x1="228.6" y1="152.4" x2="228.6" y2="149.86" width="0.1524" layer="91"/>
<junction x="198.12" y="152.4"/>
<junction x="208.28" y="152.4"/>
<junction x="218.44" y="152.4"/>
<pinref part="V31" gate="G$1" pin="+2V5"/>
<pinref part="C29" gate="G$1" pin="1"/>
<pinref part="C30" gate="G$1" pin="1"/>
<pinref part="C31" gate="G$1" pin="1"/>
<pinref part="C33" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="187.96" y1="60.96" x2="195.58" y2="60.96" width="0.1524" layer="91"/>
<wire x1="195.58" y1="60.96" x2="195.58" y2="63.5" width="0.1524" layer="91"/>
<wire x1="195.58" y1="63.5" x2="195.58" y2="66.04" width="0.1524" layer="91"/>
<wire x1="187.96" y1="63.5" x2="195.58" y2="63.5" width="0.1524" layer="91"/>
<junction x="195.58" y="63.5"/>
<pinref part="IC12" gate="G$1" pin="SENSE"/>
<pinref part="V37" gate="G$1" pin="+2V5"/>
<pinref part="IC12" gate="G$1" pin="OUT"/>
</segment>
</net>
<net name="FPGA_ON" class="0">
<segment>
<wire x1="157.48" y1="78.74" x2="139.7" y2="78.74" width="0.1524" layer="91"/>
<wire x1="137.16" y1="55.88" x2="137.16" y2="58.42" width="0.1524" layer="91"/>
<wire x1="137.16" y1="58.42" x2="139.7" y2="58.42" width="0.1524" layer="91"/>
<wire x1="139.7" y1="58.42" x2="154.94" y2="58.42" width="0.1524" layer="91"/>
<wire x1="129.54" y1="58.42" x2="137.16" y2="58.42" width="0.1524" layer="91"/>
<wire x1="139.7" y1="78.74" x2="139.7" y2="58.42" width="0.1524" layer="91"/>
<junction x="137.16" y="58.42"/>
<junction x="139.7" y="58.42"/>
<label x="129.54" y="58.42" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC13" gate="G$1" pin="VC"/>
<pinref part="R23" gate="G$1" pin="2"/>
<pinref part="IC12" gate="G$1" pin="/SHDN"/>
</segment>
</net>
<net name="+3V3-FPGA" class="0">
<segment>
<wire x1="185.42" y1="83.82" x2="200.66" y2="83.82" width="0.1524" layer="91"/>
<label x="200.66" y="83.82" size="1.778" layer="95"/>
<pinref part="IC13" gate="G$1" pin="VO"/>
</segment>
<segment>
<wire x1="152.4" y1="149.86" x2="152.4" y2="152.4" width="0.1524" layer="91"/>
<wire x1="152.4" y1="152.4" x2="162.56" y2="152.4" width="0.1524" layer="91"/>
<wire x1="162.56" y1="152.4" x2="162.56" y2="149.86" width="0.1524" layer="91"/>
<wire x1="162.56" y1="152.4" x2="172.72" y2="152.4" width="0.1524" layer="91"/>
<wire x1="172.72" y1="152.4" x2="182.88" y2="152.4" width="0.1524" layer="91"/>
<wire x1="182.88" y1="152.4" x2="182.88" y2="149.86" width="0.1524" layer="91"/>
<wire x1="152.4" y1="152.4" x2="152.4" y2="157.48" width="0.1524" layer="91"/>
<wire x1="152.4" y1="157.48" x2="154.94" y2="157.48" width="0.1524" layer="91"/>
<wire x1="172.72" y1="149.86" x2="172.72" y2="152.4" width="0.1524" layer="91"/>
<junction x="152.4" y="152.4"/>
<junction x="162.56" y="152.4"/>
<junction x="172.72" y="152.4"/>
<label x="154.94" y="157.48" size="1.778" layer="95"/>
<pinref part="C28" gate="G$1" pin="1"/>
<pinref part="C32" gate="G$1" pin="1"/>
<pinref part="C34" gate="G$1" pin="1"/>
<pinref part="C47" gate="G$1" pin="1"/>
</segment>
</net>
<net name="PFETGATE" class="0">
<segment>
<wire x1="121.92" y1="93.98" x2="119.38" y2="93.98" width="0.1524" layer="91"/>
<wire x1="119.38" y1="93.98" x2="119.38" y2="96.52" width="0.1524" layer="91"/>
<wire x1="116.84" y1="93.98" x2="119.38" y2="93.98" width="0.1524" layer="91"/>
<junction x="119.38" y="93.98"/>
<pinref part="Q2" gate="P" pin="G"/>
<pinref part="R56" gate="G$1" pin="1"/>
<pinref part="R57" gate="G$1" pin="2"/>
</segment>
</net>
<net name="NVDD_ON" class="0">
<segment>
<wire x1="104.14" y1="93.98" x2="106.68" y2="93.98" width="0.1524" layer="91"/>
<label x="104.14" y="93.98" size="1.778" layer="95" rot="MR0"/>
<pinref part="R57" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$49" class="0">
<segment>
<wire x1="137.16" y1="99.06" x2="137.16" y2="101.6" width="0.1524" layer="91"/>
<wire x1="152.4" y1="111.76" x2="157.48" y2="111.76" width="0.1524" layer="91"/>
<wire x1="157.48" y1="106.68" x2="152.4" y2="106.68" width="0.1524" layer="91"/>
<wire x1="152.4" y1="106.68" x2="152.4" y2="111.76" width="0.1524" layer="91"/>
<wire x1="99.06" y1="111.76" x2="119.38" y2="111.76" width="0.1524" layer="91"/>
<wire x1="119.38" y1="111.76" x2="132.08" y2="111.76" width="0.1524" layer="91"/>
<wire x1="132.08" y1="111.76" x2="152.4" y2="111.76" width="0.1524" layer="91"/>
<wire x1="132.08" y1="93.98" x2="132.08" y2="101.6" width="0.1524" layer="91"/>
<wire x1="132.08" y1="101.6" x2="132.08" y2="111.76" width="0.1524" layer="91"/>
<wire x1="132.08" y1="93.98" x2="127" y2="93.98" width="0.1524" layer="91"/>
<wire x1="119.38" y1="106.68" x2="119.38" y2="111.76" width="0.1524" layer="91"/>
<wire x1="137.16" y1="101.6" x2="132.08" y2="101.6" width="0.1524" layer="91"/>
<junction x="152.4" y="111.76"/>
<junction x="132.08" y="111.76"/>
<junction x="119.38" y="111.76"/>
<junction x="132.08" y="101.6"/>
<pinref part="R58" gate="G$1" pin="2"/>
<pinref part="IC3" gate="G$1" pin="VIN"/>
<pinref part="IC3" gate="G$1" pin="VC"/>
<pinref part="U1" gate="G$1" pin="B"/>
<pinref part="Q2" gate="P" pin="S"/>
<pinref part="R56" gate="G$1" pin="2"/>
</segment>
</net>
</nets>
</sheet>
<sheet>
<plain>
<text x="165.1" y="17.78" size="2.54" layer="95">FPGA and ADC</text>
<text x="50.8" y="123.19" size="1.778" layer="95">M2:0=111 means slave</text>
<text x="50.8" y="120.65" size="1.778" layer="95">serial (with pull-ups)</text>
</plain>
<instances>
<instance part="FRAME2" gate="G$1" x="0" y="0"/>
<instance part="IC1" gate="G$1" x="88.9" y="154.94"/>
<instance part="V3" gate="GND" x="93.98" y="22.86" rot="MR0"/>
<instance part="V4" gate="G$1" x="134.62" y="167.64" rot="MR0"/>
<instance part="XT1" gate="G$1" x="38.1" y="66.04"/>
<instance part="IC4" gate="G$1" x="27.94" y="55.88"/>
<instance part="R1" gate="G$1" x="43.18" y="73.66"/>
<instance part="C2" gate="G$1" x="20.32" y="45.72" rot="MR0"/>
<instance part="C3" gate="G$1" x="53.34" y="45.72" rot="MR0"/>
<instance part="R2" gate="G$1" x="43.18" y="55.88"/>
<instance part="V7" gate="GND" x="53.34" y="35.56"/>
<instance part="IC8" gate="G$1" x="187.96" y="154.94" rot="MR0"/>
<instance part="VDD4" gate="G$1" x="213.36" y="167.64" rot="MR0"/>
<instance part="V24" gate="GND" x="190.5" y="96.52" rot="MR0"/>
<instance part="C18" gate="G$1" x="223.52" y="99.06"/>
<instance part="C19" gate="G$1" x="233.68" y="99.06"/>
<instance part="SV5" gate="G$1" x="210.82" y="53.34" rot="MR0"/>
<instance part="V28" gate="GND" x="223.52" y="45.72" rot="MR0"/>
<instance part="U$5" gate="G$1" x="220.98" y="76.2"/>
<instance part="R39" gate="G$1" x="55.88" y="93.98"/>
<instance part="R44" gate="G$1" x="55.88" y="86.36"/>
<instance part="R47" gate="G$1" x="180.34" y="114.3" rot="R90"/>
<instance part="VDD10" gate="G$1" x="170.18" y="111.76"/>
<instance part="TP7" gate="G$1" x="149.86" y="109.22"/>
<instance part="R52" gate="G$1" x="243.84" y="137.16" rot="R90"/>
<instance part="VDD11" gate="G$1" x="243.84" y="147.32" rot="MR0"/>
<instance part="R53" gate="G$1" x="243.84" y="121.92" rot="R90"/>
<instance part="R54" gate="G$1" x="243.84" y="93.98" rot="R90"/>
<instance part="V55" gate="GND" x="243.84" y="81.28"/>
<instance part="R59" gate="G$1" x="170.18" y="50.8"/>
</instances>
<busses>
<bus name="NCS,SPCK,MISO,MOSI">
<segment>
<wire x1="157.48" y1="81.28" x2="157.48" y2="71.12" width="0.762" layer="92"/>
<wire x1="157.48" y1="71.12" x2="160.02" y2="68.58" width="0.762" layer="92"/>
<label x="160.02" y="68.58" size="1.778" layer="95"/>
</segment>
</bus>
<bus name="ADC[1..8]">
<segment>
<wire x1="170.18" y1="132.08" x2="170.18" y2="149.86" width="0.762" layer="92"/>
<wire x1="170.18" y1="149.86" x2="167.64" y2="152.4" width="0.762" layer="92"/>
<wire x1="157.48" y1="33.02" x2="154.94" y2="35.56" width="0.762" layer="92"/>
<wire x1="154.94" y1="35.56" x2="154.94" y2="55.88" width="0.762" layer="92"/>
<label x="167.64" y="152.4" size="1.778" layer="95" rot="MR0"/>
<label x="157.48" y="33.02" size="1.778" layer="95"/>
</segment>
</bus>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<wire x1="53.34" y1="43.18" x2="53.34" y2="40.64" width="0.1524" layer="91"/>
<wire x1="53.34" y1="40.64" x2="53.34" y2="38.1" width="0.1524" layer="91"/>
<wire x1="53.34" y1="40.64" x2="30.48" y2="40.64" width="0.1524" layer="91"/>
<wire x1="30.48" y1="40.64" x2="30.48" y2="50.8" width="0.1524" layer="91"/>
<wire x1="30.48" y1="40.64" x2="20.32" y2="40.64" width="0.1524" layer="91"/>
<wire x1="20.32" y1="40.64" x2="20.32" y2="43.18" width="0.1524" layer="91"/>
<junction x="53.34" y="40.64"/>
<junction x="30.48" y="40.64"/>
<pinref part="C3" gate="G$1" pin="2"/>
<pinref part="V7" gate="GND" pin="GND"/>
<pinref part="IC4" gate="G$1" pin="VSS"/>
<pinref part="C2" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="200.66" y1="104.14" x2="200.66" y2="101.6" width="0.1524" layer="91"/>
<wire x1="200.66" y1="101.6" x2="198.12" y2="101.6" width="0.1524" layer="91"/>
<wire x1="198.12" y1="101.6" x2="198.12" y2="104.14" width="0.1524" layer="91"/>
<wire x1="198.12" y1="101.6" x2="193.04" y2="101.6" width="0.1524" layer="91"/>
<wire x1="193.04" y1="101.6" x2="193.04" y2="104.14" width="0.1524" layer="91"/>
<wire x1="193.04" y1="101.6" x2="190.5" y2="101.6" width="0.1524" layer="91"/>
<wire x1="190.5" y1="101.6" x2="190.5" y2="104.14" width="0.1524" layer="91"/>
<wire x1="190.5" y1="99.06" x2="190.5" y2="101.6" width="0.1524" layer="91"/>
<junction x="198.12" y="101.6"/>
<junction x="193.04" y="101.6"/>
<junction x="190.5" y="101.6"/>
<pinref part="IC8" gate="G$1" pin="AGND1"/>
<pinref part="IC8" gate="G$1" pin="AGND0"/>
<pinref part="IC8" gate="G$1" pin="DGND1"/>
<pinref part="IC8" gate="G$1" pin="DGND0"/>
<pinref part="V24" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="119.38" y1="30.48" x2="119.38" y2="27.94" width="0.1524" layer="91"/>
<wire x1="119.38" y1="27.94" x2="116.84" y2="27.94" width="0.1524" layer="91"/>
<wire x1="116.84" y1="27.94" x2="111.76" y2="27.94" width="0.1524" layer="91"/>
<wire x1="111.76" y1="27.94" x2="109.22" y2="27.94" width="0.1524" layer="91"/>
<wire x1="109.22" y1="27.94" x2="106.68" y2="27.94" width="0.1524" layer="91"/>
<wire x1="106.68" y1="27.94" x2="104.14" y2="27.94" width="0.1524" layer="91"/>
<wire x1="104.14" y1="27.94" x2="101.6" y2="27.94" width="0.1524" layer="91"/>
<wire x1="101.6" y1="27.94" x2="99.06" y2="27.94" width="0.1524" layer="91"/>
<wire x1="99.06" y1="27.94" x2="96.52" y2="27.94" width="0.1524" layer="91"/>
<wire x1="96.52" y1="27.94" x2="93.98" y2="27.94" width="0.1524" layer="91"/>
<wire x1="93.98" y1="27.94" x2="93.98" y2="30.48" width="0.1524" layer="91"/>
<wire x1="96.52" y1="30.48" x2="96.52" y2="27.94" width="0.1524" layer="91"/>
<wire x1="99.06" y1="30.48" x2="99.06" y2="27.94" width="0.1524" layer="91"/>
<wire x1="101.6" y1="30.48" x2="101.6" y2="27.94" width="0.1524" layer="91"/>
<wire x1="104.14" y1="30.48" x2="104.14" y2="27.94" width="0.1524" layer="91"/>
<wire x1="106.68" y1="30.48" x2="106.68" y2="27.94" width="0.1524" layer="91"/>
<wire x1="109.22" y1="30.48" x2="109.22" y2="27.94" width="0.1524" layer="91"/>
<wire x1="111.76" y1="30.48" x2="111.76" y2="27.94" width="0.1524" layer="91"/>
<wire x1="116.84" y1="30.48" x2="116.84" y2="27.94" width="0.1524" layer="91"/>
<wire x1="93.98" y1="25.4" x2="93.98" y2="27.94" width="0.1524" layer="91"/>
<junction x="96.52" y="27.94"/>
<junction x="99.06" y="27.94"/>
<junction x="101.6" y="27.94"/>
<junction x="104.14" y="27.94"/>
<junction x="106.68" y="27.94"/>
<junction x="109.22" y="27.94"/>
<junction x="111.76" y="27.94"/>
<junction x="116.84" y="27.94"/>
<junction x="93.98" y="27.94"/>
<pinref part="IC1" gate="G$1" pin="NC1"/>
<pinref part="IC1" gate="G$1" pin="GND1"/>
<pinref part="IC1" gate="G$1" pin="GND2"/>
<pinref part="IC1" gate="G$1" pin="GND3"/>
<pinref part="IC1" gate="G$1" pin="GND4"/>
<pinref part="IC1" gate="G$1" pin="GND5"/>
<pinref part="IC1" gate="G$1" pin="GND6"/>
<pinref part="IC1" gate="G$1" pin="GND7"/>
<pinref part="IC1" gate="G$1" pin="GND8"/>
<pinref part="IC1" gate="G$1" pin="NC0"/>
<pinref part="V3" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="218.44" y1="53.34" x2="223.52" y2="53.34" width="0.1524" layer="91"/>
<wire x1="223.52" y1="53.34" x2="223.52" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SV5" gate="G$1" pin="5"/>
<pinref part="V28" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="243.84" y1="88.9" x2="243.84" y2="86.36" width="0.1524" layer="91"/>
<wire x1="243.84" y1="86.36" x2="243.84" y2="83.82" width="0.1524" layer="91"/>
<wire x1="233.68" y1="96.52" x2="233.68" y2="86.36" width="0.1524" layer="91"/>
<wire x1="233.68" y1="86.36" x2="223.52" y2="86.36" width="0.1524" layer="91"/>
<wire x1="223.52" y1="86.36" x2="223.52" y2="96.52" width="0.1524" layer="91"/>
<wire x1="243.84" y1="86.36" x2="233.68" y2="86.36" width="0.1524" layer="91"/>
<junction x="243.84" y="86.36"/>
<junction x="233.68" y="86.36"/>
<pinref part="R54" gate="G$1" pin="1"/>
<pinref part="V55" gate="GND" pin="GND"/>
<pinref part="C19" gate="G$1" pin="2"/>
<pinref part="C18" gate="G$1" pin="2"/>
</segment>
</net>
<net name="+3V3" class="0">
<segment>
<wire x1="218.44" y1="50.8" x2="220.98" y2="50.8" width="0.1524" layer="91"/>
<wire x1="220.98" y1="50.8" x2="220.98" y2="73.66" width="0.1524" layer="91"/>
<pinref part="SV5" gate="G$1" pin="6"/>
<pinref part="U$5" gate="G$1" pin="+3V3"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<wire x1="25.4" y1="55.88" x2="20.32" y2="55.88" width="0.1524" layer="91"/>
<wire x1="20.32" y1="55.88" x2="20.32" y2="63.5" width="0.1524" layer="91"/>
<wire x1="20.32" y1="63.5" x2="38.1" y2="63.5" width="0.1524" layer="91"/>
<wire x1="38.1" y1="73.66" x2="20.32" y2="73.66" width="0.1524" layer="91"/>
<wire x1="20.32" y1="73.66" x2="20.32" y2="63.5" width="0.1524" layer="91"/>
<wire x1="20.32" y1="55.88" x2="20.32" y2="50.8" width="0.1524" layer="91"/>
<junction x="20.32" y="63.5"/>
<junction x="20.32" y="55.88"/>
<pinref part="IC4" gate="G$1" pin="A"/>
<pinref part="XT1" gate="G$1" pin="A"/>
<pinref part="R1" gate="G$1" pin="1"/>
<pinref part="C2" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$5" class="0">
<segment>
<wire x1="48.26" y1="63.5" x2="53.34" y2="63.5" width="0.1524" layer="91"/>
<wire x1="53.34" y1="63.5" x2="53.34" y2="55.88" width="0.1524" layer="91"/>
<wire x1="53.34" y1="55.88" x2="53.34" y2="53.34" width="0.1524" layer="91"/>
<wire x1="83.82" y1="53.34" x2="76.2" y2="53.34" width="0.1524" layer="91"/>
<wire x1="76.2" y1="53.34" x2="53.34" y2="53.34" width="0.1524" layer="91"/>
<wire x1="48.26" y1="73.66" x2="53.34" y2="73.66" width="0.1524" layer="91"/>
<wire x1="53.34" y1="73.66" x2="53.34" y2="63.5" width="0.1524" layer="91"/>
<wire x1="53.34" y1="50.8" x2="53.34" y2="53.34" width="0.1524" layer="91"/>
<wire x1="48.26" y1="55.88" x2="53.34" y2="55.88" width="0.1524" layer="91"/>
<wire x1="83.82" y1="50.8" x2="76.2" y2="50.8" width="0.1524" layer="91"/>
<wire x1="76.2" y1="50.8" x2="76.2" y2="53.34" width="0.1524" layer="91"/>
<junction x="53.34" y="63.5"/>
<junction x="53.34" y="53.34"/>
<junction x="53.34" y="55.88"/>
<junction x="76.2" y="53.34"/>
<pinref part="XT1" gate="G$1" pin="B"/>
<pinref part="IC1" gate="G$1" pin="P91_IGCK0"/>
<pinref part="R1" gate="G$1" pin="2"/>
<pinref part="C3" gate="G$1" pin="1"/>
<pinref part="R2" gate="G$1" pin="2"/>
<pinref part="IC1" gate="G$1" pin="P93_IOV0"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<wire x1="38.1" y1="55.88" x2="35.56" y2="55.88" width="0.1524" layer="91"/>
<pinref part="R2" gate="G$1" pin="1"/>
<pinref part="IC4" gate="G$1" pin="Y"/>
</segment>
</net>
<net name="SPCK" class="0">
<segment>
<wire x1="157.48" y1="81.28" x2="154.94" y2="83.82" width="0.1524" layer="91"/>
<wire x1="154.94" y1="83.82" x2="142.24" y2="83.82" width="0.1524" layer="91"/>
<label x="144.78" y="83.82" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P39_IGCK4"/>
</segment>
</net>
<net name="MISO" class="0">
<segment>
<wire x1="157.48" y1="78.74" x2="154.94" y2="81.28" width="0.1524" layer="91"/>
<wire x1="154.94" y1="81.28" x2="142.24" y2="81.28" width="0.1524" layer="91"/>
<label x="144.78" y="81.28" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P40_IO4"/>
</segment>
</net>
<net name="MOSI" class="0">
<segment>
<wire x1="157.48" y1="73.66" x2="154.94" y2="76.2" width="0.1524" layer="91"/>
<wire x1="154.94" y1="76.2" x2="142.24" y2="76.2" width="0.1524" layer="91"/>
<label x="144.78" y="76.2" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P43_IO4"/>
</segment>
</net>
<net name="NCS" class="0">
<segment>
<wire x1="157.48" y1="71.12" x2="154.94" y2="73.66" width="0.1524" layer="91"/>
<wire x1="154.94" y1="73.66" x2="142.24" y2="73.66" width="0.1524" layer="91"/>
<label x="144.78" y="73.66" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P44_IO4"/>
</segment>
</net>
<net name="VDD" class="0">
<segment>
<wire x1="203.2" y1="160.02" x2="203.2" y2="162.56" width="0.1524" layer="91"/>
<wire x1="200.66" y1="162.56" x2="200.66" y2="160.02" width="0.1524" layer="91"/>
<wire x1="203.2" y1="162.56" x2="200.66" y2="162.56" width="0.1524" layer="91"/>
<wire x1="203.2" y1="162.56" x2="208.28" y2="162.56" width="0.1524" layer="91"/>
<wire x1="208.28" y1="162.56" x2="208.28" y2="160.02" width="0.1524" layer="91"/>
<wire x1="208.28" y1="162.56" x2="210.82" y2="162.56" width="0.1524" layer="91"/>
<wire x1="210.82" y1="162.56" x2="210.82" y2="160.02" width="0.1524" layer="91"/>
<wire x1="210.82" y1="162.56" x2="213.36" y2="162.56" width="0.1524" layer="91"/>
<wire x1="213.36" y1="162.56" x2="213.36" y2="160.02" width="0.1524" layer="91"/>
<wire x1="213.36" y1="165.1" x2="213.36" y2="162.56" width="0.1524" layer="91"/>
<junction x="203.2" y="162.56"/>
<junction x="208.28" y="162.56"/>
<junction x="210.82" y="162.56"/>
<junction x="213.36" y="162.56"/>
<pinref part="IC8" gate="G$1" pin="VDDD0"/>
<pinref part="IC8" gate="G$1" pin="VDDD1"/>
<pinref part="IC8" gate="G$1" pin="VDDA2"/>
<pinref part="IC8" gate="G$1" pin="VDDA1"/>
<pinref part="IC8" gate="G$1" pin="VDDA0"/>
<pinref part="VDD4" gate="G$1" pin="VDD"/>
</segment>
<segment>
<wire x1="180.34" y1="109.22" x2="180.34" y2="106.68" width="0.1524" layer="91"/>
<wire x1="180.34" y1="106.68" x2="170.18" y2="106.68" width="0.1524" layer="91"/>
<wire x1="170.18" y1="106.68" x2="170.18" y2="109.22" width="0.1524" layer="91"/>
<pinref part="R47" gate="G$1" pin="1"/>
<pinref part="VDD10" gate="G$1" pin="VDD"/>
</segment>
<segment>
<wire x1="243.84" y1="144.78" x2="243.84" y2="142.24" width="0.1524" layer="91"/>
<pinref part="VDD11" gate="G$1" pin="VDD"/>
<pinref part="R52" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$24" class="0">
<segment>
<wire x1="220.98" y1="124.46" x2="223.52" y2="124.46" width="0.1524" layer="91"/>
<wire x1="223.52" y1="124.46" x2="223.52" y2="114.3" width="0.1524" layer="91"/>
<wire x1="223.52" y1="114.3" x2="223.52" y2="104.14" width="0.1524" layer="91"/>
<wire x1="243.84" y1="116.84" x2="243.84" y2="114.3" width="0.1524" layer="91"/>
<wire x1="243.84" y1="114.3" x2="243.84" y2="99.06" width="0.1524" layer="91"/>
<wire x1="223.52" y1="114.3" x2="243.84" y2="114.3" width="0.1524" layer="91"/>
<junction x="223.52" y="114.3"/>
<junction x="243.84" y="114.3"/>
<pinref part="IC8" gate="G$1" pin="REFB"/>
<pinref part="C18" gate="G$1" pin="1"/>
<pinref part="R53" gate="G$1" pin="1"/>
<pinref part="R54" gate="G$1" pin="2"/>
</segment>
</net>
<net name="+2V5" class="0">
<segment>
<wire x1="116.84" y1="162.56" x2="116.84" y2="160.02" width="0.1524" layer="91"/>
<wire x1="116.84" y1="162.56" x2="119.38" y2="162.56" width="0.1524" layer="91"/>
<wire x1="119.38" y1="162.56" x2="119.38" y2="160.02" width="0.1524" layer="91"/>
<wire x1="119.38" y1="162.56" x2="121.92" y2="162.56" width="0.1524" layer="91"/>
<wire x1="121.92" y1="162.56" x2="121.92" y2="160.02" width="0.1524" layer="91"/>
<wire x1="121.92" y1="162.56" x2="124.46" y2="162.56" width="0.1524" layer="91"/>
<wire x1="124.46" y1="162.56" x2="124.46" y2="160.02" width="0.1524" layer="91"/>
<wire x1="124.46" y1="162.56" x2="127" y2="162.56" width="0.1524" layer="91"/>
<wire x1="127" y1="162.56" x2="127" y2="160.02" width="0.1524" layer="91"/>
<wire x1="127" y1="162.56" x2="129.54" y2="162.56" width="0.1524" layer="91"/>
<wire x1="129.54" y1="162.56" x2="129.54" y2="160.02" width="0.1524" layer="91"/>
<wire x1="129.54" y1="162.56" x2="132.08" y2="162.56" width="0.1524" layer="91"/>
<wire x1="132.08" y1="162.56" x2="132.08" y2="160.02" width="0.1524" layer="91"/>
<wire x1="132.08" y1="162.56" x2="134.62" y2="162.56" width="0.1524" layer="91"/>
<wire x1="134.62" y1="162.56" x2="134.62" y2="160.02" width="0.1524" layer="91"/>
<wire x1="134.62" y1="162.56" x2="134.62" y2="165.1" width="0.1524" layer="91"/>
<junction x="119.38" y="162.56"/>
<junction x="121.92" y="162.56"/>
<junction x="124.46" y="162.56"/>
<junction x="127" y="162.56"/>
<junction x="129.54" y="162.56"/>
<junction x="132.08" y="162.56"/>
<junction x="134.62" y="162.56"/>
<pinref part="IC1" gate="G$1" pin="VCCINT1"/>
<pinref part="IC1" gate="G$1" pin="VCCINT2"/>
<pinref part="IC1" gate="G$1" pin="VCCINT3"/>
<pinref part="IC1" gate="G$1" pin="VCCINT4"/>
<pinref part="IC1" gate="G$1" pin="VCCINT5"/>
<pinref part="IC1" gate="G$1" pin="VCCINT6"/>
<pinref part="IC1" gate="G$1" pin="VCCINT7"/>
<pinref part="IC1" gate="G$1" pin="VCCINT8"/>
<pinref part="V4" gate="G$1" pin="+2V5"/>
</segment>
</net>
<net name="FPGA_TDI" class="0">
<segment>
<wire x1="78.74" y1="137.16" x2="83.82" y2="137.16" width="0.1524" layer="91"/>
<label x="78.74" y="137.16" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="TDI"/>
</segment>
<segment>
<wire x1="226.06" y1="60.96" x2="218.44" y2="60.96" width="0.1524" layer="91"/>
<label x="226.06" y="60.96" size="1.778" layer="95"/>
<pinref part="SV5" gate="G$1" pin="2"/>
</segment>
</net>
<net name="FPGA_TDO" class="0">
<segment>
<wire x1="78.74" y1="134.62" x2="83.82" y2="134.62" width="0.1524" layer="91"/>
<label x="78.74" y="134.62" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="TDO"/>
</segment>
<segment>
<wire x1="226.06" y1="58.42" x2="218.44" y2="58.42" width="0.1524" layer="91"/>
<label x="226.06" y="58.42" size="1.778" layer="95"/>
<pinref part="SV5" gate="G$1" pin="3"/>
</segment>
</net>
<net name="FPGA_TMS" class="0">
<segment>
<wire x1="78.74" y1="132.08" x2="83.82" y2="132.08" width="0.1524" layer="91"/>
<label x="78.74" y="132.08" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="TMS"/>
</segment>
<segment>
<wire x1="226.06" y1="63.5" x2="218.44" y2="63.5" width="0.1524" layer="91"/>
<label x="226.06" y="63.5" size="1.778" layer="95"/>
<pinref part="SV5" gate="G$1" pin="1"/>
</segment>
</net>
<net name="FPGA_TCK" class="0">
<segment>
<wire x1="78.74" y1="129.54" x2="83.82" y2="129.54" width="0.1524" layer="91"/>
<label x="78.74" y="129.54" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="TCK"/>
</segment>
<segment>
<wire x1="226.06" y1="55.88" x2="218.44" y2="55.88" width="0.1524" layer="91"/>
<label x="226.06" y="55.88" size="1.778" layer="95"/>
<pinref part="SV5" gate="G$1" pin="4"/>
</segment>
</net>
<net name="FPGA_DONE" class="0">
<segment>
<wire x1="78.74" y1="114.3" x2="83.82" y2="114.3" width="0.1524" layer="91"/>
<label x="78.74" y="114.3" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="DONE"/>
</segment>
</net>
<net name="FPGA_NPROGRAM" class="0">
<segment>
<wire x1="78.74" y1="111.76" x2="83.82" y2="111.76" width="0.1524" layer="91"/>
<label x="78.74" y="111.76" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="/PROGRAM"/>
</segment>
</net>
<net name="FPGA_CCLK" class="0">
<segment>
<wire x1="78.74" y1="109.22" x2="83.82" y2="109.22" width="0.1524" layer="91"/>
<label x="78.74" y="109.22" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="CCLK"/>
</segment>
</net>
<net name="FPGA_DIN" class="0">
<segment>
<wire x1="78.74" y1="106.68" x2="83.82" y2="106.68" width="0.1524" layer="91"/>
<label x="78.74" y="106.68" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="DIN"/>
</segment>
</net>
<net name="FPGA_DOUT" class="0">
<segment>
<wire x1="78.74" y1="104.14" x2="83.82" y2="104.14" width="0.1524" layer="91"/>
<label x="78.74" y="104.14" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="DOUT"/>
</segment>
</net>
<net name="+3V3-FPGA" class="0">
<segment>
<wire x1="30.48" y1="60.96" x2="30.48" y2="81.28" width="0.1524" layer="91"/>
<wire x1="30.48" y1="81.28" x2="27.94" y2="81.28" width="0.1524" layer="91"/>
<label x="27.94" y="81.28" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC4" gate="G$1" pin="VDD"/>
</segment>
<segment>
<wire x1="93.98" y1="162.56" x2="93.98" y2="160.02" width="0.1524" layer="91"/>
<wire x1="93.98" y1="162.56" x2="96.52" y2="162.56" width="0.1524" layer="91"/>
<wire x1="96.52" y1="162.56" x2="96.52" y2="160.02" width="0.1524" layer="91"/>
<wire x1="96.52" y1="162.56" x2="99.06" y2="162.56" width="0.1524" layer="91"/>
<wire x1="99.06" y1="162.56" x2="99.06" y2="160.02" width="0.1524" layer="91"/>
<wire x1="99.06" y1="162.56" x2="101.6" y2="162.56" width="0.1524" layer="91"/>
<wire x1="101.6" y1="162.56" x2="101.6" y2="160.02" width="0.1524" layer="91"/>
<wire x1="101.6" y1="162.56" x2="104.14" y2="162.56" width="0.1524" layer="91"/>
<wire x1="104.14" y1="162.56" x2="104.14" y2="160.02" width="0.1524" layer="91"/>
<wire x1="104.14" y1="162.56" x2="106.68" y2="162.56" width="0.1524" layer="91"/>
<wire x1="106.68" y1="162.56" x2="106.68" y2="160.02" width="0.1524" layer="91"/>
<wire x1="106.68" y1="162.56" x2="109.22" y2="162.56" width="0.1524" layer="91"/>
<wire x1="109.22" y1="162.56" x2="109.22" y2="160.02" width="0.1524" layer="91"/>
<wire x1="109.22" y1="162.56" x2="111.76" y2="162.56" width="0.1524" layer="91"/>
<wire x1="111.76" y1="162.56" x2="111.76" y2="160.02" width="0.1524" layer="91"/>
<wire x1="111.76" y1="162.56" x2="111.76" y2="167.64" width="0.1524" layer="91"/>
<wire x1="111.76" y1="167.64" x2="109.22" y2="167.64" width="0.1524" layer="91"/>
<wire x1="177.8" y1="50.8" x2="175.26" y2="50.8" width="0.1524" layer="91"/>
<wire x1="177.8" y1="50.8" x2="177.8" y2="53.34" width="0.1524" layer="91"/>
<wire x1="177.8" y1="53.34" x2="180.34" y2="53.34" width="0.1524" layer="91"/>
<junction x="96.52" y="162.56"/>
<junction x="99.06" y="162.56"/>
<junction x="101.6" y="162.56"/>
<junction x="104.14" y="162.56"/>
<junction x="106.68" y="162.56"/>
<junction x="109.22" y="162.56"/>
<junction x="111.76" y="162.56"/>
<label x="109.22" y="167.64" size="1.778" layer="95" rot="MR0"/>
<label x="180.34" y="53.34" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B76"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B65"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B54"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B43"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B32"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B21"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B10"/>
<pinref part="IC1" gate="G$1" pin="VCCO_B07"/>
<pinref part="R59" gate="G$1" pin="2"/>
</segment>
</net>
<net name="ADC1" class="0">
<segment>
<wire x1="170.18" y1="149.86" x2="172.72" y2="152.4" width="0.1524" layer="91"/>
<wire x1="172.72" y1="152.4" x2="182.88" y2="152.4" width="0.1524" layer="91"/>
<label x="175.26" y="152.4" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D1"/>
</segment>
<segment>
<wire x1="154.94" y1="35.56" x2="152.4" y2="38.1" width="0.1524" layer="91"/>
<wire x1="152.4" y1="38.1" x2="142.24" y2="38.1" width="0.1524" layer="91"/>
<label x="144.78" y="38.1" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P62_IO3"/>
</segment>
</net>
<net name="ADC2" class="0">
<segment>
<wire x1="170.18" y1="147.32" x2="172.72" y2="149.86" width="0.1524" layer="91"/>
<wire x1="172.72" y1="149.86" x2="182.88" y2="149.86" width="0.1524" layer="91"/>
<label x="175.26" y="149.86" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D2"/>
</segment>
<segment>
<wire x1="154.94" y1="38.1" x2="152.4" y2="40.64" width="0.1524" layer="91"/>
<wire x1="152.4" y1="40.64" x2="142.24" y2="40.64" width="0.1524" layer="91"/>
<label x="144.78" y="40.64" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P60_IO3"/>
</segment>
</net>
<net name="ADC3" class="0">
<segment>
<wire x1="170.18" y1="144.78" x2="172.72" y2="147.32" width="0.1524" layer="91"/>
<wire x1="172.72" y1="147.32" x2="182.88" y2="147.32" width="0.1524" layer="91"/>
<label x="175.26" y="147.32" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D3"/>
</segment>
<segment>
<wire x1="154.94" y1="43.18" x2="152.4" y2="45.72" width="0.1524" layer="91"/>
<wire x1="152.4" y1="45.72" x2="142.24" y2="45.72" width="0.1524" layer="91"/>
<label x="144.78" y="45.72" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P58_IO3"/>
</segment>
</net>
<net name="ADC4" class="0">
<segment>
<wire x1="170.18" y1="142.24" x2="172.72" y2="144.78" width="0.1524" layer="91"/>
<wire x1="172.72" y1="144.78" x2="182.88" y2="144.78" width="0.1524" layer="91"/>
<label x="175.26" y="144.78" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D4"/>
</segment>
<segment>
<wire x1="154.94" y1="45.72" x2="152.4" y2="48.26" width="0.1524" layer="91"/>
<wire x1="152.4" y1="48.26" x2="142.24" y2="48.26" width="0.1524" layer="91"/>
<label x="144.78" y="48.26" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P57_IO3"/>
</segment>
</net>
<net name="ADC5" class="0">
<segment>
<wire x1="170.18" y1="139.7" x2="172.72" y2="142.24" width="0.1524" layer="91"/>
<wire x1="172.72" y1="142.24" x2="182.88" y2="142.24" width="0.1524" layer="91"/>
<label x="175.26" y="142.24" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D5"/>
</segment>
<segment>
<wire x1="154.94" y1="48.26" x2="152.4" y2="50.8" width="0.1524" layer="91"/>
<wire x1="152.4" y1="50.8" x2="142.24" y2="50.8" width="0.1524" layer="91"/>
<label x="144.78" y="50.8" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P56_IO3"/>
</segment>
</net>
<net name="ADC6" class="0">
<segment>
<wire x1="170.18" y1="137.16" x2="172.72" y2="139.7" width="0.1524" layer="91"/>
<wire x1="172.72" y1="139.7" x2="182.88" y2="139.7" width="0.1524" layer="91"/>
<label x="175.26" y="139.7" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D6"/>
</segment>
<segment>
<wire x1="154.94" y1="50.8" x2="152.4" y2="53.34" width="0.1524" layer="91"/>
<wire x1="152.4" y1="53.34" x2="142.24" y2="53.34" width="0.1524" layer="91"/>
<label x="144.78" y="53.34" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P55_IO3"/>
</segment>
</net>
<net name="ADC7" class="0">
<segment>
<wire x1="170.18" y1="134.62" x2="172.72" y2="137.16" width="0.1524" layer="91"/>
<wire x1="172.72" y1="137.16" x2="182.88" y2="137.16" width="0.1524" layer="91"/>
<label x="175.26" y="137.16" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="D7"/>
</segment>
<segment>
<wire x1="154.94" y1="53.34" x2="152.4" y2="55.88" width="0.1524" layer="91"/>
<wire x1="152.4" y1="55.88" x2="142.24" y2="55.88" width="0.1524" layer="91"/>
<label x="144.78" y="55.88" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P54_IOV3"/>
</segment>
</net>
<net name="ADC8" class="0">
<segment>
<wire x1="170.18" y1="132.08" x2="172.72" y2="134.62" width="0.1524" layer="91"/>
<wire x1="172.72" y1="134.62" x2="182.88" y2="134.62" width="0.1524" layer="91"/>
<label x="175.26" y="134.62" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="MSB-D8"/>
</segment>
<segment>
<wire x1="154.94" y1="55.88" x2="152.4" y2="58.42" width="0.1524" layer="91"/>
<wire x1="152.4" y1="58.42" x2="142.24" y2="58.42" width="0.1524" layer="91"/>
<label x="144.78" y="58.42" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P53_IO3"/>
</segment>
</net>
<net name="ADC_NOE" class="0">
<segment>
<wire x1="175.26" y1="129.54" x2="180.34" y2="129.54" width="0.1524" layer="91"/>
<wire x1="180.34" y1="129.54" x2="182.88" y2="129.54" width="0.1524" layer="91"/>
<wire x1="180.34" y1="129.54" x2="180.34" y2="119.38" width="0.1524" layer="91"/>
<junction x="180.34" y="129.54"/>
<label x="175.26" y="129.54" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC8" gate="G$1" pin="NOE"/>
<pinref part="R47" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="144.78" y1="66.04" x2="142.24" y2="66.04" width="0.1524" layer="91"/>
<label x="144.78" y="66.04" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P47_IO4"/>
</segment>
</net>
<net name="ADC_CLK" class="0">
<segment>
<wire x1="175.26" y1="124.46" x2="182.88" y2="124.46" width="0.1524" layer="91"/>
<label x="175.26" y="124.46" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC8" gate="G$1" pin="CLK"/>
</segment>
<segment>
<wire x1="144.78" y1="68.58" x2="142.24" y2="68.58" width="0.1524" layer="91"/>
<label x="144.78" y="68.58" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P46_IO4"/>
</segment>
</net>
<net name="SSP_DIN" class="0">
<segment>
<wire x1="144.78" y1="93.98" x2="142.24" y2="93.98" width="0.1524" layer="91"/>
<label x="144.78" y="93.98" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P32_IO5"/>
</segment>
</net>
<net name="SSP_DOUT" class="0">
<segment>
<wire x1="144.78" y1="91.44" x2="142.24" y2="91.44" width="0.1524" layer="91"/>
<label x="144.78" y="91.44" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P34_IOV5"/>
</segment>
</net>
<net name="ADC_IN" class="0">
<segment>
<wire x1="223.52" y1="139.7" x2="220.98" y2="139.7" width="0.1524" layer="91"/>
<label x="223.52" y="139.7" size="1.778" layer="95"/>
<pinref part="IC8" gate="G$1" pin="ANALOGIN"/>
</segment>
</net>
<net name="PWR_HI" class="0">
<segment>
<wire x1="81.28" y1="76.2" x2="83.82" y2="76.2" width="0.1524" layer="91"/>
<label x="81.28" y="76.2" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P80_IO1"/>
</segment>
</net>
<net name="PWR_LO" class="0">
<segment>
<wire x1="81.28" y1="73.66" x2="83.82" y2="73.66" width="0.1524" layer="91"/>
<label x="81.28" y="73.66" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P81_IO1"/>
</segment>
</net>
<net name="PWR_OE1" class="0">
<segment>
<wire x1="81.28" y1="71.12" x2="83.82" y2="71.12" width="0.1524" layer="91"/>
<label x="81.28" y="71.12" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P82_IOV1"/>
</segment>
</net>
<net name="PWR_OE2" class="0">
<segment>
<wire x1="81.28" y1="68.58" x2="83.82" y2="68.58" width="0.1524" layer="91"/>
<label x="81.28" y="68.58" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P83_IO1"/>
</segment>
</net>
<net name="SSP_FRAME" class="0">
<segment>
<wire x1="144.78" y1="96.52" x2="142.24" y2="96.52" width="0.1524" layer="91"/>
<label x="144.78" y="96.52" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P31_IO5"/>
</segment>
</net>
<net name="N$27" class="0">
<segment>
<wire x1="60.96" y1="93.98" x2="66.04" y2="93.98" width="0.1524" layer="91"/>
<wire x1="66.04" y1="93.98" x2="66.04" y2="60.96" width="0.1524" layer="91"/>
<wire x1="66.04" y1="60.96" x2="83.82" y2="60.96" width="0.1524" layer="91"/>
<pinref part="R39" gate="G$1" pin="2"/>
<pinref part="IC1" gate="G$1" pin="P87_IO1"/>
</segment>
</net>
<net name="N$40" class="0">
<segment>
<wire x1="60.96" y1="86.36" x2="63.5" y2="86.36" width="0.1524" layer="91"/>
<wire x1="63.5" y1="86.36" x2="63.5" y2="58.42" width="0.1524" layer="91"/>
<wire x1="63.5" y1="58.42" x2="83.82" y2="58.42" width="0.1524" layer="91"/>
<pinref part="R44" gate="G$1" pin="2"/>
<pinref part="IC1" gate="G$1" pin="P88_IGCK1"/>
</segment>
</net>
<net name="CROSS_LO" class="0">
<segment>
<wire x1="48.26" y1="93.98" x2="50.8" y2="93.98" width="0.1524" layer="91"/>
<label x="48.26" y="93.98" size="1.778" layer="95" rot="MR0"/>
<pinref part="R39" gate="G$1" pin="1"/>
</segment>
</net>
<net name="CROSS_HI" class="0">
<segment>
<wire x1="48.26" y1="86.36" x2="50.8" y2="86.36" width="0.1524" layer="91"/>
<label x="48.26" y="86.36" size="1.778" layer="95" rot="MR0"/>
<pinref part="R44" gate="G$1" pin="1"/>
</segment>
</net>
<net name="PWR_OE3" class="0">
<segment>
<wire x1="81.28" y1="66.04" x2="83.82" y2="66.04" width="0.1524" layer="91"/>
<label x="81.28" y="66.04" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P84_IO1"/>
</segment>
</net>
<net name="PWR_OE4" class="0">
<segment>
<wire x1="81.28" y1="63.5" x2="83.82" y2="63.5" width="0.1524" layer="91"/>
<label x="81.28" y="63.5" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P86_IOV1"/>
</segment>
</net>
<net name="N$55" class="0">
<segment>
<wire x1="142.24" y1="104.14" x2="149.86" y2="104.14" width="0.1524" layer="91"/>
<wire x1="149.86" y1="104.14" x2="149.86" y2="106.68" width="0.1524" layer="91"/>
<pinref part="IC1" gate="G$1" pin="P22_IO6"/>
<pinref part="TP7" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="N$48" class="0">
<segment>
<wire x1="243.84" y1="132.08" x2="243.84" y2="129.54" width="0.1524" layer="91"/>
<wire x1="243.84" y1="129.54" x2="243.84" y2="127" width="0.1524" layer="91"/>
<wire x1="233.68" y1="129.54" x2="220.98" y2="129.54" width="0.1524" layer="91"/>
<wire x1="233.68" y1="129.54" x2="233.68" y2="104.14" width="0.1524" layer="91"/>
<wire x1="243.84" y1="129.54" x2="233.68" y2="129.54" width="0.1524" layer="91"/>
<junction x="243.84" y="129.54"/>
<junction x="233.68" y="129.54"/>
<pinref part="R52" gate="G$1" pin="1"/>
<pinref part="R53" gate="G$1" pin="2"/>
<pinref part="IC8" gate="G$1" pin="REFT"/>
<pinref part="C19" gate="G$1" pin="1"/>
</segment>
</net>
<net name="SSP_CLK" class="0">
<segment>
<wire x1="81.28" y1="83.82" x2="83.82" y2="83.82" width="0.1524" layer="91"/>
<label x="81.28" y="83.82" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC1" gate="G$1" pin="P71_IO2"/>
</segment>
</net>
<net name="PCK0" class="0">
<segment>
<wire x1="144.78" y1="88.9" x2="142.24" y2="88.9" width="0.1524" layer="91"/>
<label x="144.78" y="88.9" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P36_IGCK5"/>
</segment>
</net>
<net name="FPGA_NINIT" class="0">
<segment>
<wire x1="167.64" y1="60.96" x2="160.02" y2="60.96" width="0.1524" layer="91"/>
<wire x1="160.02" y1="60.96" x2="142.24" y2="60.96" width="0.1524" layer="91"/>
<wire x1="165.1" y1="50.8" x2="160.02" y2="50.8" width="0.1524" layer="91"/>
<wire x1="160.02" y1="50.8" x2="160.02" y2="60.96" width="0.1524" layer="91"/>
<junction x="160.02" y="60.96"/>
<label x="167.64" y="60.96" size="1.778" layer="95"/>
<pinref part="IC1" gate="G$1" pin="P52_IO3"/>
<pinref part="R59" gate="G$1" pin="1"/>
</segment>
</net>
</nets>
</sheet>
<sheet>
<plain>
<text x="165.1" y="17.78" size="2.54" layer="95">ARM micro and support</text>
</plain>
<instances>
<instance part="FRAME3" gate="G$1" x="0" y="0"/>
<instance part="IC2" gate="G$1" x="144.78" y="132.08"/>
<instance part="V5" gate="GND" x="124.46" y="38.1"/>
<instance part="U$2" gate="G$1" x="104.14" y="147.32"/>
<instance part="R3" gate="G$1" x="71.12" y="127"/>
<instance part="C4" gate="G$1" x="58.42" y="127" rot="R90"/>
<instance part="C5" gate="G$1" x="58.42" y="137.16" rot="R90"/>
<instance part="V8" gate="GND" x="48.26" y="121.92" rot="MR0"/>
<instance part="XT2" gate="G$1" x="76.2" y="81.28" smashed="yes" rot="R90">
<attribute name="NAME" x="75.0156" y="84.0901" size="1.778" layer="95" rot="R90"/>
</instance>
<instance part="C6" gate="G$1" x="66.04" y="91.44" rot="R90"/>
<instance part="C7" gate="G$1" x="66.04" y="81.28" rot="R90"/>
<instance part="V9" gate="GND" x="55.88" y="76.2" rot="MR0"/>
<instance part="R4" gate="G$1" x="38.1" y="66.04"/>
<instance part="R5" gate="G$1" x="38.1" y="58.42"/>
<instance part="R6" gate="G$1" x="45.72" y="73.66" rot="R90"/>
<instance part="C8" gate="G$1" x="144.78" y="149.86" rot="MR0"/>
<instance part="C9" gate="G$1" x="154.94" y="149.86" rot="MR0"/>
<instance part="V10" gate="GND" x="154.94" y="139.7"/>
<instance part="SV3" gate="G$1" x="215.9" y="137.16" rot="R180"/>
<instance part="V11" gate="GND" x="228.6" y="119.38"/>
<instance part="U$6" gate="G$1" x="203.2" y="162.56"/>
<instance part="R7" gate="G$1" x="203.2" y="149.86" rot="R90"/>
<instance part="V12" gate="GND" x="27.94" y="96.52" rot="MR0"/>
<instance part="R8" gate="G$1" x="58.42" y="114.3"/>
<instance part="U$7" gate="G$1" x="27.94" y="132.08"/>
<instance part="D4" gate="G$1" x="210.82" y="63.5" rot="R90"/>
<instance part="D5" gate="G$1" x="210.82" y="55.88" rot="R90"/>
<instance part="D6" gate="G$1" x="210.82" y="48.26" rot="R90"/>
<instance part="R24" gate="G$1" x="198.12" y="63.5"/>
<instance part="R25" gate="G$1" x="198.12" y="55.88"/>
<instance part="R26" gate="G$1" x="198.12" y="48.26"/>
<instance part="V40" gate="GND" x="218.44" y="33.02"/>
<instance part="R43" gate="G$1" x="236.22" y="81.28" rot="R90"/>
<instance part="U$9" gate="G$1" x="236.22" y="91.44" rot="MR0"/>
<instance part="SW1" gate="G$1" x="236.22" y="60.96" rot="R90"/>
<instance part="V50" gate="GND" x="236.22" y="50.8"/>
<instance part="D9" gate="G$1" x="210.82" y="40.64" rot="R90"/>
<instance part="R55" gate="G$1" x="198.12" y="40.64"/>
<instance part="TP8" gate="G$1" x="185.42" y="35.56" rot="R90"/>
<instance part="IC7" gate="G$1" x="38.1" y="121.92"/>
</instances>
<busses>
<bus name="NCS,SPCK,MISO,MOSI">
<segment>
<wire x1="172.72" y1="101.6" x2="172.72" y2="93.98" width="0.762" layer="92"/>
<wire x1="172.72" y1="93.98" x2="175.26" y2="91.44" width="0.762" layer="92"/>
<label x="175.26" y="91.44" size="1.778" layer="95"/>
</segment>
</bus>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<wire x1="124.46" y1="45.72" x2="124.46" y2="43.18" width="0.1524" layer="91"/>
<wire x1="124.46" y1="43.18" x2="124.46" y2="40.64" width="0.1524" layer="91"/>
<wire x1="124.46" y1="43.18" x2="121.92" y2="43.18" width="0.1524" layer="91"/>
<wire x1="121.92" y1="43.18" x2="121.92" y2="45.72" width="0.1524" layer="91"/>
<wire x1="121.92" y1="43.18" x2="119.38" y2="43.18" width="0.1524" layer="91"/>
<wire x1="119.38" y1="43.18" x2="119.38" y2="45.72" width="0.1524" layer="91"/>
<wire x1="119.38" y1="43.18" x2="116.84" y2="43.18" width="0.1524" layer="91"/>
<wire x1="116.84" y1="43.18" x2="116.84" y2="45.72" width="0.1524" layer="91"/>
<junction x="124.46" y="43.18"/>
<junction x="121.92" y="43.18"/>
<junction x="119.38" y="43.18"/>
<pinref part="IC2" gate="G$1" pin="GND0"/>
<pinref part="V5" gate="GND" pin="GND"/>
<pinref part="IC2" gate="G$1" pin="GND1"/>
<pinref part="IC2" gate="G$1" pin="GND2"/>
<pinref part="IC2" gate="G$1" pin="GND3"/>
</segment>
<segment>
<wire x1="53.34" y1="127" x2="48.26" y2="127" width="0.1524" layer="91"/>
<wire x1="48.26" y1="127" x2="48.26" y2="124.46" width="0.1524" layer="91"/>
<wire x1="53.34" y1="137.16" x2="48.26" y2="137.16" width="0.1524" layer="91"/>
<wire x1="48.26" y1="137.16" x2="48.26" y2="127" width="0.1524" layer="91"/>
<junction x="48.26" y="127"/>
<pinref part="C4" gate="G$1" pin="1"/>
<pinref part="V8" gate="GND" pin="GND"/>
<pinref part="C5" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="60.96" y1="91.44" x2="55.88" y2="91.44" width="0.1524" layer="91"/>
<wire x1="55.88" y1="91.44" x2="55.88" y2="81.28" width="0.1524" layer="91"/>
<wire x1="55.88" y1="81.28" x2="55.88" y2="78.74" width="0.1524" layer="91"/>
<wire x1="60.96" y1="81.28" x2="55.88" y2="81.28" width="0.1524" layer="91"/>
<junction x="55.88" y="81.28"/>
<pinref part="C6" gate="G$1" pin="1"/>
<pinref part="V9" gate="GND" pin="GND"/>
<pinref part="C7" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="154.94" y1="147.32" x2="154.94" y2="144.78" width="0.1524" layer="91"/>
<wire x1="154.94" y1="144.78" x2="154.94" y2="142.24" width="0.1524" layer="91"/>
<wire x1="154.94" y1="144.78" x2="144.78" y2="144.78" width="0.1524" layer="91"/>
<wire x1="144.78" y1="144.78" x2="144.78" y2="147.32" width="0.1524" layer="91"/>
<junction x="154.94" y="144.78"/>
<pinref part="C9" gate="G$1" pin="2"/>
<pinref part="V10" gate="GND" pin="GND"/>
<pinref part="C8" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="226.06" y1="144.78" x2="228.6" y2="144.78" width="0.1524" layer="91"/>
<wire x1="228.6" y1="144.78" x2="228.6" y2="142.24" width="0.1524" layer="91"/>
<wire x1="228.6" y1="142.24" x2="228.6" y2="139.7" width="0.1524" layer="91"/>
<wire x1="228.6" y1="139.7" x2="228.6" y2="137.16" width="0.1524" layer="91"/>
<wire x1="228.6" y1="137.16" x2="228.6" y2="134.62" width="0.1524" layer="91"/>
<wire x1="228.6" y1="134.62" x2="228.6" y2="132.08" width="0.1524" layer="91"/>
<wire x1="228.6" y1="132.08" x2="228.6" y2="129.54" width="0.1524" layer="91"/>
<wire x1="228.6" y1="129.54" x2="228.6" y2="127" width="0.1524" layer="91"/>
<wire x1="228.6" y1="127" x2="228.6" y2="124.46" width="0.1524" layer="91"/>
<wire x1="228.6" y1="124.46" x2="228.6" y2="121.92" width="0.1524" layer="91"/>
<wire x1="226.06" y1="124.46" x2="228.6" y2="124.46" width="0.1524" layer="91"/>
<wire x1="226.06" y1="127" x2="228.6" y2="127" width="0.1524" layer="91"/>
<wire x1="226.06" y1="129.54" x2="228.6" y2="129.54" width="0.1524" layer="91"/>
<wire x1="226.06" y1="132.08" x2="228.6" y2="132.08" width="0.1524" layer="91"/>
<wire x1="226.06" y1="134.62" x2="228.6" y2="134.62" width="0.1524" layer="91"/>
<wire x1="226.06" y1="137.16" x2="228.6" y2="137.16" width="0.1524" layer="91"/>
<wire x1="226.06" y1="139.7" x2="228.6" y2="139.7" width="0.1524" layer="91"/>
<wire x1="226.06" y1="142.24" x2="228.6" y2="142.24" width="0.1524" layer="91"/>
<junction x="228.6" y="124.46"/>
<junction x="228.6" y="127"/>
<junction x="228.6" y="129.54"/>
<junction x="228.6" y="132.08"/>
<junction x="228.6" y="134.62"/>
<junction x="228.6" y="137.16"/>
<junction x="228.6" y="139.7"/>
<junction x="228.6" y="142.24"/>
<pinref part="SV3" gate="G$1" pin="4"/>
<pinref part="V11" gate="GND" pin="GND"/>
<pinref part="SV3" gate="G$1" pin="20"/>
<pinref part="SV3" gate="G$1" pin="18"/>
<pinref part="SV3" gate="G$1" pin="16"/>
<pinref part="SV3" gate="G$1" pin="14"/>
<pinref part="SV3" gate="G$1" pin="12"/>
<pinref part="SV3" gate="G$1" pin="10"/>
<pinref part="SV3" gate="G$1" pin="8"/>
<pinref part="SV3" gate="G$1" pin="6"/>
</segment>
<segment>
<wire x1="27.94" y1="101.6" x2="27.94" y2="99.06" width="0.1524" layer="91"/>
<pinref part="V12" gate="GND" pin="GND"/>
<pinref part="IC7" gate="G$1" pin="V-"/>
</segment>
<segment>
<wire x1="213.36" y1="63.5" x2="218.44" y2="63.5" width="0.1524" layer="91"/>
<wire x1="218.44" y1="63.5" x2="218.44" y2="55.88" width="0.1524" layer="91"/>
<wire x1="218.44" y1="55.88" x2="218.44" y2="48.26" width="0.1524" layer="91"/>
<wire x1="218.44" y1="48.26" x2="218.44" y2="40.64" width="0.1524" layer="91"/>
<wire x1="218.44" y1="40.64" x2="218.44" y2="35.56" width="0.1524" layer="91"/>
<wire x1="213.36" y1="55.88" x2="218.44" y2="55.88" width="0.1524" layer="91"/>
<wire x1="213.36" y1="48.26" x2="218.44" y2="48.26" width="0.1524" layer="91"/>
<wire x1="213.36" y1="40.64" x2="218.44" y2="40.64" width="0.1524" layer="91"/>
<junction x="218.44" y="55.88"/>
<junction x="218.44" y="48.26"/>
<junction x="218.44" y="40.64"/>
<pinref part="D4" gate="G$1" pin="C"/>
<pinref part="V40" gate="GND" pin="GND"/>
<pinref part="D5" gate="G$1" pin="C"/>
<pinref part="D6" gate="G$1" pin="C"/>
<pinref part="D9" gate="G$1" pin="C"/>
</segment>
<segment>
<wire x1="236.22" y1="55.88" x2="236.22" y2="53.34" width="0.1524" layer="91"/>
<pinref part="SW1" gate="G$1" pin="P$1"/>
<pinref part="V50" gate="GND" pin="GND"/>
</segment>
</net>
<net name="+3V3" class="0">
<segment>
<wire x1="104.14" y1="144.78" x2="104.14" y2="142.24" width="0.1524" layer="91"/>
<wire x1="104.14" y1="142.24" x2="104.14" y2="139.7" width="0.1524" layer="91"/>
<wire x1="104.14" y1="142.24" x2="106.68" y2="142.24" width="0.1524" layer="91"/>
<wire x1="106.68" y1="142.24" x2="106.68" y2="139.7" width="0.1524" layer="91"/>
<wire x1="106.68" y1="142.24" x2="109.22" y2="142.24" width="0.1524" layer="91"/>
<wire x1="109.22" y1="142.24" x2="109.22" y2="139.7" width="0.1524" layer="91"/>
<wire x1="109.22" y1="142.24" x2="111.76" y2="142.24" width="0.1524" layer="91"/>
<wire x1="111.76" y1="142.24" x2="111.76" y2="139.7" width="0.1524" layer="91"/>
<wire x1="111.76" y1="142.24" x2="114.3" y2="142.24" width="0.1524" layer="91"/>
<wire x1="114.3" y1="142.24" x2="114.3" y2="139.7" width="0.1524" layer="91"/>
<junction x="104.14" y="142.24"/>
<junction x="106.68" y="142.24"/>
<junction x="109.22" y="142.24"/>
<junction x="111.76" y="142.24"/>
<pinref part="U$2" gate="G$1" pin="+3V3"/>
<pinref part="IC2" gate="G$1" pin="VDDIO0"/>
<pinref part="IC2" gate="G$1" pin="VDDIO1"/>
<pinref part="IC2" gate="G$1" pin="VDDIO2"/>
<pinref part="IC2" gate="G$1" pin="VDDFLASH"/>
<pinref part="IC2" gate="G$1" pin="VDDIN"/>
</segment>
<segment>
<wire x1="81.28" y1="68.58" x2="83.82" y2="68.58" width="0.1524" layer="91"/>
<label x="81.28" y="68.58" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="ADVREF"/>
</segment>
<segment>
<wire x1="210.82" y1="147.32" x2="208.28" y2="147.32" width="0.1524" layer="91"/>
<wire x1="208.28" y1="147.32" x2="208.28" y2="157.48" width="0.1524" layer="91"/>
<wire x1="203.2" y1="160.02" x2="203.2" y2="157.48" width="0.1524" layer="91"/>
<wire x1="203.2" y1="157.48" x2="203.2" y2="154.94" width="0.1524" layer="91"/>
<wire x1="208.28" y1="157.48" x2="203.2" y2="157.48" width="0.1524" layer="91"/>
<junction x="203.2" y="157.48"/>
<pinref part="SV3" gate="G$1" pin="1"/>
<pinref part="U$6" gate="G$1" pin="+3V3"/>
<pinref part="R7" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="27.94" y1="129.54" x2="27.94" y2="127" width="0.1524" layer="91"/>
<pinref part="U$7" gate="G$1" pin="+3V3"/>
<pinref part="IC7" gate="G$1" pin="V+"/>
</segment>
<segment>
<wire x1="236.22" y1="88.9" x2="236.22" y2="86.36" width="0.1524" layer="91"/>
<pinref part="U$9" gate="G$1" pin="+3V3"/>
<pinref part="R43" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<wire x1="60.96" y1="137.16" x2="78.74" y2="137.16" width="0.1524" layer="91"/>
<wire x1="78.74" y1="137.16" x2="78.74" y2="127" width="0.1524" layer="91"/>
<wire x1="78.74" y1="127" x2="83.82" y2="127" width="0.1524" layer="91"/>
<wire x1="78.74" y1="127" x2="76.2" y2="127" width="0.1524" layer="91"/>
<junction x="78.74" y="127"/>
<pinref part="C5" gate="G$1" pin="2"/>
<pinref part="IC2" gate="G$1" pin="PLLRC"/>
<pinref part="R3" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<wire x1="66.04" y1="127" x2="60.96" y2="127" width="0.1524" layer="91"/>
<pinref part="R3" gate="G$1" pin="1"/>
<pinref part="C4" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$7" class="0">
<segment>
<wire x1="83.82" y1="91.44" x2="78.74" y2="91.44" width="0.1524" layer="91"/>
<wire x1="78.74" y1="91.44" x2="68.58" y2="91.44" width="0.1524" layer="91"/>
<junction x="78.74" y="91.44"/>
<pinref part="IC2" gate="G$1" pin="XOUT"/>
<pinref part="XT2" gate="G$1" pin="B"/>
<pinref part="C6" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<wire x1="68.58" y1="81.28" x2="78.74" y2="81.28" width="0.1524" layer="91"/>
<wire x1="78.74" y1="81.28" x2="83.82" y2="81.28" width="0.1524" layer="91"/>
<junction x="78.74" y="81.28"/>
<pinref part="C7" gate="G$1" pin="2"/>
<pinref part="XT2" gate="G$1" pin="A"/>
<pinref part="IC2" gate="G$1" pin="XIN/PGMCK"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<wire x1="43.18" y1="66.04" x2="63.5" y2="66.04" width="0.1524" layer="91"/>
<wire x1="63.5" y1="66.04" x2="63.5" y2="76.2" width="0.1524" layer="91"/>
<wire x1="63.5" y1="76.2" x2="83.82" y2="76.2" width="0.1524" layer="91"/>
<pinref part="R4" gate="G$1" pin="2"/>
<pinref part="IC2" gate="G$1" pin="DDM"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<wire x1="83.82" y1="73.66" x2="66.04" y2="73.66" width="0.1524" layer="91"/>
<wire x1="66.04" y1="73.66" x2="66.04" y2="58.42" width="0.1524" layer="91"/>
<wire x1="66.04" y1="58.42" x2="45.72" y2="58.42" width="0.1524" layer="91"/>
<wire x1="45.72" y1="58.42" x2="43.18" y2="58.42" width="0.1524" layer="91"/>
<wire x1="45.72" y1="68.58" x2="45.72" y2="58.42" width="0.1524" layer="91"/>
<junction x="45.72" y="58.42"/>
<pinref part="IC2" gate="G$1" pin="DDP"/>
<pinref part="R5" gate="G$1" pin="2"/>
<pinref part="R6" gate="G$1" pin="1"/>
</segment>
</net>
<net name="USB_PU" class="0">
<segment>
<wire x1="45.72" y1="78.74" x2="45.72" y2="81.28" width="0.1524" layer="91"/>
<wire x1="45.72" y1="81.28" x2="43.18" y2="81.28" width="0.1524" layer="91"/>
<label x="43.18" y="81.28" size="1.778" layer="95" rot="MR0"/>
<pinref part="R6" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="152.4" y1="71.12" x2="149.86" y2="71.12" width="0.1524" layer="91"/>
<label x="152.4" y="71.12" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA24"/>
</segment>
</net>
<net name="USB_D-" class="0">
<segment>
<wire x1="30.48" y1="66.04" x2="33.02" y2="66.04" width="0.1524" layer="91"/>
<label x="30.48" y="66.04" size="1.778" layer="95" rot="MR0"/>
<pinref part="R4" gate="G$1" pin="1"/>
</segment>
</net>
<net name="USB_D+" class="0">
<segment>
<wire x1="30.48" y1="58.42" x2="33.02" y2="58.42" width="0.1524" layer="91"/>
<label x="30.48" y="58.42" size="1.778" layer="95" rot="MR0"/>
<pinref part="R5" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<wire x1="132.08" y1="142.24" x2="132.08" y2="139.7" width="0.1524" layer="91"/>
<wire x1="129.54" y1="142.24" x2="132.08" y2="142.24" width="0.1524" layer="91"/>
<wire x1="129.54" y1="139.7" x2="129.54" y2="142.24" width="0.1524" layer="91"/>
<wire x1="127" y1="142.24" x2="129.54" y2="142.24" width="0.1524" layer="91"/>
<wire x1="127" y1="139.7" x2="127" y2="142.24" width="0.1524" layer="91"/>
<wire x1="124.46" y1="142.24" x2="127" y2="142.24" width="0.1524" layer="91"/>
<wire x1="124.46" y1="139.7" x2="124.46" y2="142.24" width="0.1524" layer="91"/>
<wire x1="121.92" y1="142.24" x2="124.46" y2="142.24" width="0.1524" layer="91"/>
<wire x1="121.92" y1="142.24" x2="121.92" y2="139.7" width="0.1524" layer="91"/>
<wire x1="132.08" y1="142.24" x2="132.08" y2="157.48" width="0.1524" layer="91"/>
<wire x1="154.94" y1="154.94" x2="154.94" y2="157.48" width="0.1524" layer="91"/>
<wire x1="154.94" y1="157.48" x2="144.78" y2="157.48" width="0.1524" layer="91"/>
<wire x1="144.78" y1="157.48" x2="144.78" y2="154.94" width="0.1524" layer="91"/>
<wire x1="132.08" y1="157.48" x2="144.78" y2="157.48" width="0.1524" layer="91"/>
<junction x="129.54" y="142.24"/>
<junction x="127" y="142.24"/>
<junction x="124.46" y="142.24"/>
<junction x="132.08" y="142.24"/>
<junction x="144.78" y="157.48"/>
<pinref part="IC2" gate="G$1" pin="VDDOUT"/>
<pinref part="IC2" gate="G$1" pin="VDDCORE2"/>
<pinref part="IC2" gate="G$1" pin="VDDCORE1"/>
<pinref part="IC2" gate="G$1" pin="VDDCORE0"/>
<pinref part="IC2" gate="G$1" pin="VDDPLL"/>
<pinref part="C8" gate="G$1" pin="1"/>
<pinref part="C9" gate="G$1" pin="1"/>
</segment>
</net>
<net name="ARM_TMS" class="0">
<segment>
<wire x1="81.28" y1="111.76" x2="83.82" y2="111.76" width="0.1524" layer="91"/>
<label x="81.28" y="111.76" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="TMS"/>
</segment>
<segment>
<wire x1="198.12" y1="139.7" x2="210.82" y2="139.7" width="0.1524" layer="91"/>
<label x="198.12" y="139.7" size="1.778" layer="95" rot="MR0"/>
<pinref part="SV3" gate="G$1" pin="7"/>
</segment>
</net>
<net name="ARM_TCK" class="0">
<segment>
<wire x1="81.28" y1="109.22" x2="83.82" y2="109.22" width="0.1524" layer="91"/>
<label x="81.28" y="109.22" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="TCK"/>
</segment>
<segment>
<wire x1="198.12" y1="137.16" x2="203.2" y2="137.16" width="0.1524" layer="91"/>
<wire x1="203.2" y1="137.16" x2="210.82" y2="137.16" width="0.1524" layer="91"/>
<wire x1="203.2" y1="144.78" x2="203.2" y2="137.16" width="0.1524" layer="91"/>
<junction x="203.2" y="137.16"/>
<label x="198.12" y="137.16" size="1.778" layer="95" rot="MR0"/>
<pinref part="SV3" gate="G$1" pin="9"/>
<pinref part="R7" gate="G$1" pin="1"/>
</segment>
</net>
<net name="ARM_TDO" class="0">
<segment>
<wire x1="81.28" y1="106.68" x2="83.82" y2="106.68" width="0.1524" layer="91"/>
<label x="81.28" y="106.68" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="TDO"/>
</segment>
<segment>
<wire x1="198.12" y1="132.08" x2="210.82" y2="132.08" width="0.1524" layer="91"/>
<label x="198.12" y="132.08" size="1.778" layer="95" rot="MR0"/>
<pinref part="SV3" gate="G$1" pin="13"/>
</segment>
</net>
<net name="ARM_TDI" class="0">
<segment>
<wire x1="81.28" y1="104.14" x2="83.82" y2="104.14" width="0.1524" layer="91"/>
<label x="81.28" y="104.14" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="TDI"/>
</segment>
<segment>
<wire x1="198.12" y1="142.24" x2="210.82" y2="142.24" width="0.1524" layer="91"/>
<label x="198.12" y="142.24" size="1.778" layer="95" rot="MR0"/>
<pinref part="SV3" gate="G$1" pin="5"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<wire x1="63.5" y1="114.3" x2="83.82" y2="114.3" width="0.1524" layer="91"/>
<pinref part="R8" gate="G$1" pin="2"/>
<pinref part="IC2" gate="G$1" pin="NRST"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<wire x1="53.34" y1="114.3" x2="40.64" y2="114.3" width="0.1524" layer="91"/>
<pinref part="R8" gate="G$1" pin="1"/>
<pinref part="IC7" gate="G$1" pin="OUT"/>
</segment>
</net>
<net name="NCS" class="0">
<segment>
<wire x1="172.72" y1="101.6" x2="170.18" y2="104.14" width="0.1524" layer="91"/>
<wire x1="170.18" y1="104.14" x2="149.86" y2="104.14" width="0.1524" layer="91"/>
<label x="152.4" y="104.14" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA11"/>
</segment>
</net>
<net name="MISO" class="0">
<segment>
<wire x1="172.72" y1="99.06" x2="170.18" y2="101.6" width="0.1524" layer="91"/>
<wire x1="170.18" y1="101.6" x2="149.86" y2="101.6" width="0.1524" layer="91"/>
<label x="152.4" y="101.6" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA12"/>
</segment>
</net>
<net name="MOSI" class="0">
<segment>
<wire x1="172.72" y1="96.52" x2="170.18" y2="99.06" width="0.1524" layer="91"/>
<wire x1="170.18" y1="99.06" x2="149.86" y2="99.06" width="0.1524" layer="91"/>
<label x="152.4" y="99.06" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA13"/>
</segment>
</net>
<net name="SPCK" class="0">
<segment>
<wire x1="172.72" y1="93.98" x2="170.18" y2="96.52" width="0.1524" layer="91"/>
<wire x1="170.18" y1="96.52" x2="149.86" y2="96.52" width="0.1524" layer="91"/>
<label x="152.4" y="96.52" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA14"/>
</segment>
</net>
<net name="N$28" class="0">
<segment>
<wire x1="203.2" y1="48.26" x2="205.74" y2="48.26" width="0.1524" layer="91"/>
<pinref part="R26" gate="G$1" pin="2"/>
<pinref part="D6" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$29" class="0">
<segment>
<wire x1="203.2" y1="55.88" x2="205.74" y2="55.88" width="0.1524" layer="91"/>
<pinref part="R25" gate="G$1" pin="2"/>
<pinref part="D5" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$30" class="0">
<segment>
<wire x1="203.2" y1="63.5" x2="205.74" y2="63.5" width="0.1524" layer="91"/>
<pinref part="R24" gate="G$1" pin="2"/>
<pinref part="D4" gate="G$1" pin="A"/>
</segment>
</net>
<net name="AMPL_LO" class="0">
<segment>
<wire x1="81.28" y1="66.04" x2="83.82" y2="66.04" width="0.1524" layer="91"/>
<label x="81.28" y="66.04" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="AD4"/>
</segment>
</net>
<net name="AMPL_HI" class="0">
<segment>
<wire x1="81.28" y1="63.5" x2="83.82" y2="63.5" width="0.1524" layer="91"/>
<label x="81.28" y="63.5" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC2" gate="G$1" pin="AD5"/>
</segment>
</net>
<net name="BUTTON_A" class="0">
<segment>
<wire x1="149.86" y1="73.66" x2="152.4" y2="73.66" width="0.1524" layer="91"/>
<label x="152.4" y="73.66" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA23"/>
</segment>
<segment>
<wire x1="236.22" y1="71.12" x2="236.22" y2="73.66" width="0.1524" layer="91"/>
<wire x1="236.22" y1="73.66" x2="236.22" y2="76.2" width="0.1524" layer="91"/>
<wire x1="236.22" y1="73.66" x2="228.6" y2="73.66" width="0.1524" layer="91"/>
<junction x="236.22" y="73.66"/>
<label x="228.6" y="73.66" size="1.778" layer="95" rot="MR0"/>
<pinref part="SW1" gate="G$1" pin="P$2"/>
<pinref part="R43" gate="G$1" pin="1"/>
</segment>
</net>
<net name="LED_A" class="0">
<segment>
<wire x1="152.4" y1="132.08" x2="149.86" y2="132.08" width="0.1524" layer="91"/>
<label x="152.4" y="132.08" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA0"/>
</segment>
<segment>
<wire x1="190.5" y1="63.5" x2="193.04" y2="63.5" width="0.1524" layer="91"/>
<label x="190.5" y="63.5" size="1.778" layer="95" rot="MR0"/>
<pinref part="R24" gate="G$1" pin="1"/>
</segment>
</net>
<net name="LED_B" class="0">
<segment>
<wire x1="152.4" y1="111.76" x2="149.86" y2="111.76" width="0.1524" layer="91"/>
<label x="152.4" y="111.76" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA8"/>
</segment>
<segment>
<wire x1="190.5" y1="55.88" x2="193.04" y2="55.88" width="0.1524" layer="91"/>
<label x="190.5" y="55.88" size="1.778" layer="95" rot="MR0"/>
<pinref part="R25" gate="G$1" pin="1"/>
</segment>
</net>
<net name="LED_C" class="0">
<segment>
<wire x1="152.4" y1="109.22" x2="149.86" y2="109.22" width="0.1524" layer="91"/>
<label x="152.4" y="109.22" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA9"/>
</segment>
<segment>
<wire x1="190.5" y1="48.26" x2="193.04" y2="48.26" width="0.1524" layer="91"/>
<label x="190.5" y="48.26" size="1.778" layer="95" rot="MR0"/>
<pinref part="R26" gate="G$1" pin="1"/>
</segment>
</net>
<net name="SSP_DIN" class="0">
<segment>
<wire x1="152.4" y1="86.36" x2="149.86" y2="86.36" width="0.1524" layer="91"/>
<label x="152.4" y="86.36" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA18/AD1"/>
</segment>
</net>
<net name="SSP_DOUT" class="0">
<segment>
<wire x1="152.4" y1="88.9" x2="149.86" y2="88.9" width="0.1524" layer="91"/>
<label x="152.4" y="88.9" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA17/AD0"/>
</segment>
</net>
<net name="FPGA_DONE" class="0">
<segment>
<wire x1="152.4" y1="63.5" x2="149.86" y2="63.5" width="0.1524" layer="91"/>
<label x="152.4" y="63.5" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA27"/>
</segment>
</net>
<net name="FPGA_NPROGRAM" class="0">
<segment>
<wire x1="152.4" y1="60.96" x2="149.86" y2="60.96" width="0.1524" layer="91"/>
<label x="152.4" y="60.96" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA28"/>
</segment>
</net>
<net name="FPGA_CCLK" class="0">
<segment>
<wire x1="152.4" y1="58.42" x2="149.86" y2="58.42" width="0.1524" layer="91"/>
<label x="152.4" y="58.42" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA29"/>
</segment>
</net>
<net name="FPGA_DIN" class="0">
<segment>
<wire x1="152.4" y1="55.88" x2="149.86" y2="55.88" width="0.1524" layer="91"/>
<label x="152.4" y="55.88" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA30"/>
</segment>
</net>
<net name="FPGA_DOUT" class="0">
<segment>
<wire x1="152.4" y1="53.34" x2="149.86" y2="53.34" width="0.1524" layer="91"/>
<label x="152.4" y="53.34" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA31"/>
</segment>
</net>
<net name="FPGA_ON" class="0">
<segment>
<wire x1="152.4" y1="66.04" x2="149.86" y2="66.04" width="0.1524" layer="91"/>
<label x="152.4" y="66.04" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA26"/>
</segment>
</net>
<net name="RELAY_ON" class="0">
<segment>
<wire x1="152.4" y1="68.58" x2="149.86" y2="68.58" width="0.1524" layer="91"/>
<label x="152.4" y="68.58" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA25"/>
</segment>
</net>
<net name="SSP_CLK" class="0">
<segment>
<wire x1="152.4" y1="91.44" x2="149.86" y2="91.44" width="0.1524" layer="91"/>
<label x="152.4" y="91.44" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA16"/>
</segment>
</net>
<net name="SSP_FRAME" class="0">
<segment>
<wire x1="152.4" y1="93.98" x2="149.86" y2="93.98" width="0.1524" layer="91"/>
<label x="152.4" y="93.98" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA15"/>
</segment>
</net>
<net name="MUXSEL_HIPKD" class="0">
<segment>
<wire x1="152.4" y1="83.82" x2="149.86" y2="83.82" width="0.1524" layer="91"/>
<label x="152.4" y="83.82" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA19/AD2"/>
</segment>
</net>
<net name="MUXSEL_LOPKD" class="0">
<segment>
<wire x1="152.4" y1="81.28" x2="149.86" y2="81.28" width="0.1524" layer="91"/>
<label x="152.4" y="81.28" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA20/AD3"/>
</segment>
</net>
<net name="MUXSEL_HIRAW" class="0">
<segment>
<wire x1="152.4" y1="78.74" x2="149.86" y2="78.74" width="0.1524" layer="91"/>
<label x="152.4" y="78.74" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA21"/>
</segment>
</net>
<net name="MUXSEL_LORAW" class="0">
<segment>
<wire x1="152.4" y1="76.2" x2="149.86" y2="76.2" width="0.1524" layer="91"/>
<label x="152.4" y="76.2" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA22"/>
</segment>
</net>
<net name="PCK0" class="0">
<segment>
<wire x1="152.4" y1="116.84" x2="149.86" y2="116.84" width="0.1524" layer="91"/>
<label x="152.4" y="116.84" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA6"/>
</segment>
</net>
<net name="LED_D" class="0">
<segment>
<wire x1="152.4" y1="127" x2="149.86" y2="127" width="0.1524" layer="91"/>
<label x="152.4" y="127" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA2"/>
</segment>
<segment>
<wire x1="187.96" y1="35.56" x2="190.5" y2="35.56" width="0.1524" layer="91"/>
<wire x1="193.04" y1="40.64" x2="190.5" y2="40.64" width="0.1524" layer="91"/>
<wire x1="190.5" y1="40.64" x2="187.96" y2="40.64" width="0.1524" layer="91"/>
<wire x1="190.5" y1="35.56" x2="190.5" y2="40.64" width="0.1524" layer="91"/>
<junction x="190.5" y="40.64"/>
<label x="187.96" y="40.64" size="1.778" layer="95" rot="MR0"/>
<pinref part="TP8" gate="G$1" pin="P$1"/>
<pinref part="R55" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$25" class="0">
<segment>
<wire x1="203.2" y1="40.64" x2="205.74" y2="40.64" width="0.1524" layer="91"/>
<pinref part="R55" gate="G$1" pin="2"/>
<pinref part="D9" gate="G$1" pin="A"/>
</segment>
</net>
<net name="NVDD_ON" class="0">
<segment>
<wire x1="152.4" y1="124.46" x2="149.86" y2="124.46" width="0.1524" layer="91"/>
<label x="152.4" y="124.46" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA3"/>
</segment>
</net>
<net name="FPGA_NINIT" class="0">
<segment>
<wire x1="152.4" y1="121.92" x2="149.86" y2="121.92" width="0.1524" layer="91"/>
<label x="152.4" y="121.92" size="1.778" layer="95"/>
<pinref part="IC2" gate="G$1" pin="PA4"/>
</segment>
</net>
</nets>
</sheet>
<sheet>
<plain>
<text x="165.1" y="17.78" size="2.54" layer="95">analog rx path</text>
<text x="35.56" y="33.02" size="1.778" layer="95" rot="R180">D2, D3 are BAR18</text>
</plain>
<instances>
<instance part="FRAME4" gate="G$1" x="0" y="0"/>
<instance part="IC6" gate="A" x="208.28" y="152.4" rot="MR180"/>
<instance part="IC6" gate="B" x="101.6" y="129.54" rot="MR180"/>
<instance part="IC6" gate="C" x="132.08" y="43.18" rot="MR180"/>
<instance part="IC6" gate="D" x="177.8" y="43.18" rot="MR180"/>
<instance part="IC6" gate="P" x="238.76" y="116.84"/>
<instance part="R9" gate="G$1" x="195.58" y="157.48" rot="R90"/>
<instance part="R10" gate="G$1" x="195.58" y="142.24" rot="R90"/>
<instance part="V13" gate="G$1" x="238.76" y="157.48" rot="R270"/>
<instance part="V14" gate="GND" x="195.58" y="132.08"/>
<instance part="C10" gate="G$1" x="231.14" y="142.24"/>
<instance part="V15" gate="GND" x="241.3" y="132.08"/>
<instance part="D1" gate="G$1" x="38.1" y="127"/>
<instance part="R11" gate="G$1" x="60.96" y="119.38" rot="R90"/>
<instance part="C11" gate="G$1" x="48.26" y="119.38"/>
<instance part="C12" gate="G$1" x="83.82" y="127" rot="R90"/>
<instance part="R12" gate="G$1" x="88.9" y="119.38" rot="R90"/>
<instance part="V16" gate="G$1" x="93.98" y="111.76" rot="MR90"/>
<instance part="R13" gate="G$1" x="86.36" y="142.24"/>
<instance part="V17" gate="G$1" x="76.2" y="142.24" rot="R90"/>
<instance part="R14" gate="G$1" x="104.14" y="142.24"/>
<instance part="C13" gate="G$1" x="109.22" y="149.86" rot="R90"/>
<instance part="V18" gate="GND" x="60.96" y="106.68"/>
<instance part="D2" gate="G$1" x="50.8" y="40.64"/>
<instance part="C14" gate="G$1" x="38.1" y="40.64" rot="R90"/>
<instance part="D3" gate="G$1" x="43.18" y="30.48" rot="R90"/>
<instance part="V19" gate="GND" x="78.74" y="17.78"/>
<instance part="C15" gate="G$1" x="66.04" y="30.48"/>
<instance part="R15" gate="G$1" x="78.74" y="30.48" rot="R90"/>
<instance part="R16" gate="G$1" x="88.9" y="40.64"/>
<instance part="C16" gate="G$1" x="101.6" y="40.64" rot="R90"/>
<instance part="R17" gate="G$1" x="119.38" y="30.48" rot="R90"/>
<instance part="V20" gate="G$1" x="134.62" y="22.86" rot="MR90"/>
<instance part="R18" gate="G$1" x="132.08" y="55.88"/>
<instance part="C17" gate="G$1" x="137.16" y="63.5" rot="R90"/>
<instance part="R19" gate="G$1" x="116.84" y="53.34"/>
<instance part="V21" gate="G$1" x="106.68" y="53.34" rot="R90"/>
<instance part="V22" gate="GND" x="68.58" y="152.4" rot="MR0"/>
<instance part="R20" gate="G$1" x="60.96" y="162.56"/>
<instance part="V23" gate="GND" x="238.76" y="91.44"/>
<instance part="VDD2" gate="G$1" x="215.9" y="129.54"/>
<instance part="VDD3" gate="G$1" x="195.58" y="167.64"/>
<instance part="IC11" gate="G$1" x="203.2" y="81.28"/>
<instance part="V33" gate="GND" x="215.9" y="43.18"/>
<instance part="VDD8" gate="G$1" x="205.74" y="96.52"/>
<instance part="IC5" gate="A" x="73.66" y="160.02"/>
<instance part="IC5" gate="B" x="119.38" y="78.74"/>
<instance part="IC5" gate="P" x="215.9" y="116.84"/>
<instance part="V34" gate="GND" x="111.76" y="71.12" rot="MR0"/>
<instance part="R21" gate="G$1" x="106.68" y="81.28"/>
<instance part="R30" gate="G$1" x="45.72" y="142.24" rot="R90"/>
<instance part="R31" gate="G$1" x="55.88" y="142.24" rot="R90"/>
<instance part="V42" gate="GND" x="55.88" y="134.62"/>
<instance part="V43" gate="GND" x="170.18" y="35.56" rot="MR0"/>
<instance part="IC14" gate="A" x="71.12" y="76.2" rot="MR180"/>
<instance part="IC14" gate="B" x="165.1" y="132.08" rot="MR180"/>
<instance part="IC14" gate="P" x="226.06" y="116.84"/>
<instance part="R33" gate="G$1" x="73.66" y="86.36"/>
<instance part="C40" gate="G$1" x="73.66" y="93.98" rot="R90"/>
<instance part="R34" gate="G$1" x="170.18" y="142.24"/>
<instance part="C41" gate="G$1" x="170.18" y="149.86" rot="R90"/>
<instance part="V45" gate="G$1" x="152.4" y="114.3" rot="R90"/>
<instance part="C42" gate="G$1" x="35.56" y="83.82" rot="R90"/>
<instance part="C43" gate="G$1" x="137.16" y="129.54" rot="R90"/>
<instance part="R32" gate="G$1" x="147.32" y="129.54"/>
<instance part="R35" gate="G$1" x="45.72" y="83.82"/>
<instance part="R36" gate="G$1" x="127" y="30.48" rot="R90"/>
<instance part="R37" gate="G$1" x="109.22" y="33.02" rot="R90"/>
<instance part="R38" gate="G$1" x="109.22" y="20.32" rot="R90"/>
<instance part="V46" gate="G$1" x="114.3" y="12.7" rot="MR90"/>
<instance part="RLY1" gate="L" x="27.94" y="119.38"/>
<instance part="RLY1" gate="A" x="15.24" y="48.26" rot="R90"/>
<instance part="RLY1" gate="B" x="17.78" y="144.78" rot="R90"/>
<instance part="Q1" gate="G$1" x="20.32" y="101.6"/>
<instance part="D7" gate="G$1" x="15.24" y="116.84" rot="R90"/>
<instance part="VDD9" gate="G$1" x="15.24" y="129.54"/>
<instance part="V48" gate="GND" x="22.86" y="91.44"/>
<instance part="R40" gate="G$1" x="12.7" y="93.98" rot="R90"/>
<instance part="R41" gate="G$1" x="68.58" y="50.8" rot="R90"/>
<instance part="R42" gate="G$1" x="78.74" y="58.42" rot="R180"/>
<instance part="V49" gate="GND" x="86.36" y="53.34"/>
<instance part="TP1" gate="G$1" x="231.14" y="78.74" rot="R270"/>
<instance part="R48" gate="G$1" x="157.48" y="121.92" rot="R90"/>
<instance part="R49" gate="G$1" x="53.34" y="76.2" rot="R90"/>
<instance part="R50" gate="G$1" x="149.86" y="157.48"/>
<instance part="V52" gate="G$1" x="139.7" y="157.48" rot="R90"/>
<instance part="V44" gate="G$1" x="48.26" y="68.58" rot="R90"/>
<instance part="R51" gate="G$1" x="58.42" y="93.98"/>
<instance part="V53" gate="G$1" x="48.26" y="93.98" rot="R90"/>
<instance part="C45" gate="G$1" x="241.3" y="142.24"/>
<instance part="D10" gate="G$1" x="58.42" y="30.48" rot="R90"/>
<instance part="D11" gate="G$1" x="73.66" y="116.84" rot="R90"/>
<instance part="V56" gate="GND" x="73.66" y="106.68"/>
</instances>
<busses>
</busses>
<nets>
<net name="N$14" class="0">
<segment>
<wire x1="205.74" y1="149.86" x2="195.58" y2="149.86" width="0.1524" layer="91"/>
<wire x1="195.58" y1="149.86" x2="195.58" y2="152.4" width="0.1524" layer="91"/>
<wire x1="195.58" y1="149.86" x2="195.58" y2="147.32" width="0.1524" layer="91"/>
<junction x="195.58" y="149.86"/>
<pinref part="IC6" gate="A" pin="+IN"/>
<pinref part="R9" gate="G$1" pin="1"/>
<pinref part="R10" gate="G$1" pin="2"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<wire x1="195.58" y1="137.16" x2="195.58" y2="134.62" width="0.1524" layer="91"/>
<pinref part="R10" gate="G$1" pin="1"/>
<pinref part="V14" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="48.26" y1="116.84" x2="48.26" y2="111.76" width="0.1524" layer="91"/>
<wire x1="60.96" y1="114.3" x2="60.96" y2="111.76" width="0.1524" layer="91"/>
<wire x1="60.96" y1="111.76" x2="60.96" y2="109.22" width="0.1524" layer="91"/>
<wire x1="48.26" y1="111.76" x2="60.96" y2="111.76" width="0.1524" layer="91"/>
<junction x="60.96" y="111.76"/>
<pinref part="C11" gate="G$1" pin="2"/>
<pinref part="R11" gate="G$1" pin="1"/>
<pinref part="V18" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="71.12" y1="157.48" x2="68.58" y2="157.48" width="0.1524" layer="91"/>
<wire x1="68.58" y1="157.48" x2="68.58" y2="154.94" width="0.1524" layer="91"/>
<pinref part="V22" gate="GND" pin="GND"/>
<pinref part="IC5" gate="A" pin="-IN"/>
</segment>
<segment>
<wire x1="238.76" y1="93.98" x2="238.76" y2="96.52" width="0.1524" layer="91"/>
<wire x1="238.76" y1="96.52" x2="238.76" y2="99.06" width="0.1524" layer="91"/>
<wire x1="238.76" y1="96.52" x2="226.06" y2="96.52" width="0.1524" layer="91"/>
<wire x1="226.06" y1="96.52" x2="215.9" y2="96.52" width="0.1524" layer="91"/>
<wire x1="215.9" y1="96.52" x2="215.9" y2="99.06" width="0.1524" layer="91"/>
<wire x1="226.06" y1="99.06" x2="226.06" y2="96.52" width="0.1524" layer="91"/>
<junction x="238.76" y="96.52"/>
<junction x="226.06" y="96.52"/>
<pinref part="V23" gate="GND" pin="GND"/>
<pinref part="IC6" gate="P" pin="V-"/>
<pinref part="IC5" gate="P" pin="V-"/>
<pinref part="IC14" gate="P" pin="V-"/>
</segment>
<segment>
<wire x1="66.04" y1="27.94" x2="66.04" y2="22.86" width="0.1524" layer="91"/>
<wire x1="78.74" y1="25.4" x2="78.74" y2="22.86" width="0.1524" layer="91"/>
<wire x1="78.74" y1="22.86" x2="78.74" y2="20.32" width="0.1524" layer="91"/>
<wire x1="66.04" y1="22.86" x2="78.74" y2="22.86" width="0.1524" layer="91"/>
<wire x1="66.04" y1="22.86" x2="58.42" y2="22.86" width="0.1524" layer="91"/>
<wire x1="58.42" y1="22.86" x2="43.18" y2="22.86" width="0.1524" layer="91"/>
<wire x1="43.18" y1="22.86" x2="43.18" y2="27.94" width="0.1524" layer="91"/>
<wire x1="58.42" y1="27.94" x2="58.42" y2="22.86" width="0.1524" layer="91"/>
<junction x="78.74" y="22.86"/>
<junction x="66.04" y="22.86"/>
<junction x="58.42" y="22.86"/>
<pinref part="C15" gate="G$1" pin="2"/>
<pinref part="R15" gate="G$1" pin="1"/>
<pinref part="V19" gate="GND" pin="GND"/>
<pinref part="D3" gate="G$1" pin="A"/>
<pinref part="D10" gate="G$1" pin="A"/>
</segment>
<segment>
<wire x1="215.9" y1="48.26" x2="215.9" y2="45.72" width="0.1524" layer="91"/>
<pinref part="IC11" gate="G$1" pin="VSS"/>
<pinref part="V33" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="116.84" y1="76.2" x2="111.76" y2="76.2" width="0.1524" layer="91"/>
<wire x1="111.76" y1="76.2" x2="111.76" y2="73.66" width="0.1524" layer="91"/>
<pinref part="IC5" gate="B" pin="-IN"/>
<pinref part="V34" gate="GND" pin="GND"/>
</segment>
<segment>
<pinref part="R31" gate="G$1" pin="1"/>
<pinref part="V42" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="175.26" y1="45.72" x2="170.18" y2="45.72" width="0.1524" layer="91"/>
<wire x1="170.18" y1="45.72" x2="170.18" y2="40.64" width="0.1524" layer="91"/>
<wire x1="170.18" y1="40.64" x2="170.18" y2="38.1" width="0.1524" layer="91"/>
<wire x1="175.26" y1="40.64" x2="170.18" y2="40.64" width="0.1524" layer="91"/>
<junction x="170.18" y="40.64"/>
<pinref part="IC6" gate="D" pin="-IN"/>
<pinref part="V43" gate="GND" pin="GND"/>
<pinref part="IC6" gate="D" pin="+IN"/>
</segment>
<segment>
<wire x1="22.86" y1="96.52" x2="22.86" y2="93.98" width="0.1524" layer="91"/>
<pinref part="Q1" gate="G$1" pin="E"/>
<pinref part="V48" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="83.82" y1="58.42" x2="86.36" y2="58.42" width="0.1524" layer="91"/>
<wire x1="86.36" y1="58.42" x2="86.36" y2="55.88" width="0.1524" layer="91"/>
<pinref part="R42" gate="G$1" pin="1"/>
<pinref part="V49" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="241.3" y1="139.7" x2="241.3" y2="137.16" width="0.1524" layer="91"/>
<wire x1="241.3" y1="137.16" x2="241.3" y2="134.62" width="0.1524" layer="91"/>
<wire x1="241.3" y1="137.16" x2="231.14" y2="137.16" width="0.1524" layer="91"/>
<wire x1="231.14" y1="137.16" x2="231.14" y2="139.7" width="0.1524" layer="91"/>
<junction x="241.3" y="137.16"/>
<pinref part="C45" gate="G$1" pin="2"/>
<pinref part="V15" gate="GND" pin="GND"/>
<pinref part="C10" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="73.66" y1="114.3" x2="73.66" y2="109.22" width="0.1524" layer="91"/>
<pinref part="D11" gate="G$1" pin="A"/>
<pinref part="V56" gate="GND" pin="GND"/>
</segment>
</net>
<net name="VMID" class="0">
<segment>
<wire x1="205.74" y1="154.94" x2="203.2" y2="154.94" width="0.1524" layer="91"/>
<wire x1="203.2" y1="154.94" x2="203.2" y2="162.56" width="0.1524" layer="91"/>
<wire x1="203.2" y1="162.56" x2="231.14" y2="162.56" width="0.1524" layer="91"/>
<wire x1="231.14" y1="162.56" x2="231.14" y2="157.48" width="0.1524" layer="91"/>
<wire x1="231.14" y1="157.48" x2="231.14" y2="152.4" width="0.1524" layer="91"/>
<wire x1="231.14" y1="152.4" x2="220.98" y2="152.4" width="0.1524" layer="91"/>
<wire x1="236.22" y1="157.48" x2="231.14" y2="157.48" width="0.1524" layer="91"/>
<wire x1="231.14" y1="147.32" x2="231.14" y2="149.86" width="0.1524" layer="91"/>
<wire x1="231.14" y1="149.86" x2="231.14" y2="152.4" width="0.1524" layer="91"/>
<wire x1="231.14" y1="149.86" x2="241.3" y2="149.86" width="0.1524" layer="91"/>
<wire x1="241.3" y1="149.86" x2="241.3" y2="147.32" width="0.1524" layer="91"/>
<junction x="231.14" y="157.48"/>
<junction x="231.14" y="152.4"/>
<junction x="231.14" y="149.86"/>
<pinref part="IC6" gate="A" pin="-IN"/>
<pinref part="IC6" gate="A" pin="OUT"/>
<pinref part="V13" gate="G$1" pin="VMID"/>
<pinref part="C10" gate="G$1" pin="1"/>
<pinref part="C45" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="91.44" y1="111.76" x2="88.9" y2="111.76" width="0.1524" layer="91"/>
<wire x1="88.9" y1="111.76" x2="88.9" y2="114.3" width="0.1524" layer="91"/>
<pinref part="V16" gate="G$1" pin="VMID"/>
<pinref part="R12" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="81.28" y1="142.24" x2="78.74" y2="142.24" width="0.1524" layer="91"/>
<pinref part="R13" gate="G$1" pin="1"/>
<pinref part="V17" gate="G$1" pin="VMID"/>
</segment>
<segment>
<wire x1="132.08" y1="22.86" x2="127" y2="22.86" width="0.1524" layer="91"/>
<wire x1="127" y1="22.86" x2="119.38" y2="22.86" width="0.1524" layer="91"/>
<wire x1="119.38" y1="22.86" x2="119.38" y2="25.4" width="0.1524" layer="91"/>
<wire x1="127" y1="25.4" x2="127" y2="22.86" width="0.1524" layer="91"/>
<junction x="127" y="22.86"/>
<pinref part="V20" gate="G$1" pin="VMID"/>
<pinref part="R17" gate="G$1" pin="1"/>
<pinref part="R36" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="111.76" y1="12.7" x2="109.22" y2="12.7" width="0.1524" layer="91"/>
<wire x1="109.22" y1="12.7" x2="109.22" y2="15.24" width="0.1524" layer="91"/>
<pinref part="V46" gate="G$1" pin="VMID"/>
<pinref part="R38" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="111.76" y1="53.34" x2="109.22" y2="53.34" width="0.1524" layer="91"/>
<pinref part="R19" gate="G$1" pin="1"/>
<pinref part="V21" gate="G$1" pin="VMID"/>
</segment>
<segment>
<wire x1="154.94" y1="114.3" x2="157.48" y2="114.3" width="0.1524" layer="91"/>
<wire x1="157.48" y1="114.3" x2="157.48" y2="116.84" width="0.1524" layer="91"/>
<pinref part="V45" gate="G$1" pin="VMID"/>
<pinref part="R48" gate="G$1" pin="1"/>
</segment>
<segment>
<wire x1="144.78" y1="157.48" x2="142.24" y2="157.48" width="0.1524" layer="91"/>
<pinref part="R50" gate="G$1" pin="1"/>
<pinref part="V52" gate="G$1" pin="VMID"/>
</segment>
<segment>
<wire x1="50.8" y1="68.58" x2="53.34" y2="68.58" width="0.1524" layer="91"/>
<wire x1="53.34" y1="68.58" x2="53.34" y2="71.12" width="0.1524" layer="91"/>
<pinref part="R49" gate="G$1" pin="1"/>
<pinref part="V44" gate="G$1" pin="VMID"/>
</segment>
<segment>
<wire x1="53.34" y1="93.98" x2="50.8" y2="93.98" width="0.1524" layer="91"/>
<pinref part="R51" gate="G$1" pin="1"/>
<pinref part="V53" gate="G$1" pin="VMID"/>
</segment>
</net>
<net name="N$16" class="0">
<segment>
<wire x1="88.9" y1="124.46" x2="88.9" y2="127" width="0.1524" layer="91"/>
<wire x1="88.9" y1="127" x2="86.36" y2="127" width="0.1524" layer="91"/>
<wire x1="88.9" y1="127" x2="99.06" y2="127" width="0.1524" layer="91"/>
<junction x="88.9" y="127"/>
<pinref part="R12" gate="G$1" pin="2"/>
<pinref part="C12" gate="G$1" pin="2"/>
<pinref part="IC6" gate="B" pin="+IN"/>
</segment>
</net>
<net name="N$17" class="0">
<segment>
<wire x1="99.06" y1="142.24" x2="96.52" y2="142.24" width="0.1524" layer="91"/>
<wire x1="96.52" y1="142.24" x2="93.98" y2="142.24" width="0.1524" layer="91"/>
<wire x1="93.98" y1="142.24" x2="91.44" y2="142.24" width="0.1524" layer="91"/>
<wire x1="99.06" y1="132.08" x2="93.98" y2="132.08" width="0.1524" layer="91"/>
<wire x1="93.98" y1="132.08" x2="93.98" y2="142.24" width="0.1524" layer="91"/>
<wire x1="96.52" y1="149.86" x2="96.52" y2="142.24" width="0.1524" layer="91"/>
<wire x1="104.14" y1="149.86" x2="96.52" y2="149.86" width="0.1524" layer="91"/>
<junction x="93.98" y="142.24"/>
<junction x="96.52" y="142.24"/>
<pinref part="R14" gate="G$1" pin="1"/>
<pinref part="R13" gate="G$1" pin="2"/>
<pinref part="IC6" gate="B" pin="-IN"/>
<pinref part="C13" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$19" class="0">
<segment>
<wire x1="40.64" y1="127" x2="45.72" y2="127" width="0.1524" layer="91"/>
<wire x1="45.72" y1="127" x2="48.26" y2="127" width="0.1524" layer="91"/>
<wire x1="48.26" y1="127" x2="48.26" y2="124.46" width="0.1524" layer="91"/>
<wire x1="48.26" y1="127" x2="60.96" y2="127" width="0.1524" layer="91"/>
<wire x1="60.96" y1="127" x2="60.96" y2="124.46" width="0.1524" layer="91"/>
<wire x1="60.96" y1="127" x2="73.66" y2="127" width="0.1524" layer="91"/>
<wire x1="73.66" y1="127" x2="78.74" y2="127" width="0.1524" layer="91"/>
<wire x1="45.72" y1="137.16" x2="45.72" y2="127" width="0.1524" layer="91"/>
<wire x1="73.66" y1="119.38" x2="73.66" y2="127" width="0.1524" layer="91"/>
<junction x="48.26" y="127"/>
<junction x="60.96" y="127"/>
<junction x="45.72" y="127"/>
<junction x="73.66" y="127"/>
<pinref part="D1" gate="G$1" pin="C"/>
<pinref part="C11" gate="G$1" pin="1"/>
<pinref part="R11" gate="G$1" pin="2"/>
<pinref part="C12" gate="G$1" pin="1"/>
<pinref part="R30" gate="G$1" pin="1"/>
<pinref part="D11" gate="G$1" pin="C"/>
</segment>
</net>
<net name="ADCDR_LO" class="0">
<segment>
<wire x1="109.22" y1="142.24" x2="124.46" y2="142.24" width="0.1524" layer="91"/>
<wire x1="124.46" y1="142.24" x2="124.46" y2="129.54" width="0.1524" layer="91"/>
<wire x1="124.46" y1="129.54" x2="114.3" y2="129.54" width="0.1524" layer="91"/>
<wire x1="124.46" y1="149.86" x2="124.46" y2="142.24" width="0.1524" layer="91"/>
<wire x1="124.46" y1="149.86" x2="111.76" y2="149.86" width="0.1524" layer="91"/>
<wire x1="124.46" y1="129.54" x2="124.46" y2="106.68" width="0.1524" layer="91"/>
<wire x1="124.46" y1="106.68" x2="170.18" y2="106.68" width="0.1524" layer="91"/>
<wire x1="170.18" y1="106.68" x2="170.18" y2="73.66" width="0.1524" layer="91"/>
<wire x1="170.18" y1="73.66" x2="198.12" y2="73.66" width="0.1524" layer="91"/>
<junction x="124.46" y="142.24"/>
<junction x="124.46" y="129.54"/>
<pinref part="R14" gate="G$1" pin="2"/>
<pinref part="IC6" gate="B" pin="OUT"/>
<pinref part="C13" gate="G$1" pin="2"/>
<pinref part="IC11" gate="G$1" pin="A2"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<wire x1="127" y1="55.88" x2="124.46" y2="55.88" width="0.1524" layer="91"/>
<wire x1="129.54" y1="45.72" x2="124.46" y2="45.72" width="0.1524" layer="91"/>
<wire x1="124.46" y1="45.72" x2="124.46" y2="53.34" width="0.1524" layer="91"/>
<wire x1="124.46" y1="53.34" x2="124.46" y2="55.88" width="0.1524" layer="91"/>
<wire x1="124.46" y1="55.88" x2="124.46" y2="63.5" width="0.1524" layer="91"/>
<wire x1="124.46" y1="63.5" x2="132.08" y2="63.5" width="0.1524" layer="91"/>
<wire x1="121.92" y1="53.34" x2="124.46" y2="53.34" width="0.1524" layer="91"/>
<junction x="124.46" y="55.88"/>
<junction x="124.46" y="53.34"/>
<pinref part="R18" gate="G$1" pin="1"/>
<pinref part="IC6" gate="C" pin="-IN"/>
<pinref part="C17" gate="G$1" pin="1"/>
<pinref part="R19" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$20" class="0">
<segment>
<wire x1="40.64" y1="40.64" x2="43.18" y2="40.64" width="0.1524" layer="91"/>
<wire x1="43.18" y1="40.64" x2="43.18" y2="33.02" width="0.1524" layer="91"/>
<wire x1="43.18" y1="40.64" x2="48.26" y2="40.64" width="0.1524" layer="91"/>
<junction x="43.18" y="40.64"/>
<pinref part="C14" gate="G$1" pin="2"/>
<pinref part="D3" gate="G$1" pin="C"/>
<pinref part="D2" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$18" class="0">
<segment>
<wire x1="93.98" y1="40.64" x2="96.52" y2="40.64" width="0.1524" layer="91"/>
<pinref part="R16" gate="G$1" pin="2"/>
<pinref part="C16" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$22" class="0">
<segment>
<wire x1="129.54" y1="40.64" x2="127" y2="40.64" width="0.1524" layer="91"/>
<wire x1="127" y1="40.64" x2="119.38" y2="40.64" width="0.1524" layer="91"/>
<wire x1="119.38" y1="40.64" x2="109.22" y2="40.64" width="0.1524" layer="91"/>
<wire x1="109.22" y1="40.64" x2="104.14" y2="40.64" width="0.1524" layer="91"/>
<wire x1="119.38" y1="35.56" x2="119.38" y2="40.64" width="0.1524" layer="91"/>
<wire x1="127" y1="35.56" x2="127" y2="40.64" width="0.1524" layer="91"/>
<wire x1="109.22" y1="38.1" x2="109.22" y2="40.64" width="0.1524" layer="91"/>
<junction x="119.38" y="40.64"/>
<junction x="127" y="40.64"/>
<junction x="109.22" y="40.64"/>
<pinref part="IC6" gate="C" pin="+IN"/>
<pinref part="C16" gate="G$1" pin="2"/>
<pinref part="R17" gate="G$1" pin="2"/>
<pinref part="R36" gate="G$1" pin="2"/>
<pinref part="R37" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$23" class="0">
<segment>
<wire x1="66.04" y1="162.56" x2="71.12" y2="162.56" width="0.1524" layer="91"/>
<pinref part="R20" gate="G$1" pin="2"/>
<pinref part="IC5" gate="A" pin="+IN"/>
</segment>
</net>
<net name="CROSS_LO" class="0">
<segment>
<wire x1="86.36" y1="160.02" x2="99.06" y2="160.02" width="0.1524" layer="91"/>
<label x="99.06" y="160.02" size="1.778" layer="95"/>
<pinref part="IC5" gate="A" pin="OUT"/>
</segment>
</net>
<net name="VDD" class="0">
<segment>
<wire x1="195.58" y1="165.1" x2="195.58" y2="162.56" width="0.1524" layer="91"/>
<pinref part="VDD3" gate="G$1" pin="VDD"/>
<pinref part="R9" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="215.9" y1="127" x2="215.9" y2="124.46" width="0.1524" layer="91"/>
<wire x1="215.9" y1="124.46" x2="215.9" y2="121.92" width="0.1524" layer="91"/>
<wire x1="215.9" y1="124.46" x2="226.06" y2="124.46" width="0.1524" layer="91"/>
<wire x1="226.06" y1="124.46" x2="238.76" y2="124.46" width="0.1524" layer="91"/>
<wire x1="238.76" y1="124.46" x2="238.76" y2="121.92" width="0.1524" layer="91"/>
<wire x1="226.06" y1="121.92" x2="226.06" y2="124.46" width="0.1524" layer="91"/>
<junction x="215.9" y="124.46"/>
<junction x="226.06" y="124.46"/>
<pinref part="VDD2" gate="G$1" pin="VDD"/>
<pinref part="IC6" gate="P" pin="V+"/>
<pinref part="IC5" gate="P" pin="V+"/>
<pinref part="IC14" gate="P" pin="V+"/>
</segment>
<segment>
<wire x1="205.74" y1="93.98" x2="205.74" y2="91.44" width="0.1524" layer="91"/>
<pinref part="VDD8" gate="G$1" pin="VDD"/>
<pinref part="IC11" gate="G$1" pin="VDD"/>
</segment>
<segment>
<wire x1="25.4" y1="119.38" x2="22.86" y2="119.38" width="0.1524" layer="91"/>
<wire x1="22.86" y1="119.38" x2="22.86" y2="124.46" width="0.1524" layer="91"/>
<wire x1="15.24" y1="119.38" x2="15.24" y2="124.46" width="0.1524" layer="91"/>
<wire x1="15.24" y1="124.46" x2="15.24" y2="127" width="0.1524" layer="91"/>
<wire x1="22.86" y1="124.46" x2="15.24" y2="124.46" width="0.1524" layer="91"/>
<junction x="15.24" y="124.46"/>
<pinref part="RLY1" gate="L" pin="P$1"/>
<pinref part="D7" gate="G$1" pin="C"/>
<pinref part="VDD9" gate="G$1" pin="VDD"/>
</segment>
</net>
<net name="ADC_IN" class="0">
<segment>
<wire x1="223.52" y1="58.42" x2="226.06" y2="58.42" width="0.1524" layer="91"/>
<wire x1="223.52" y1="81.28" x2="226.06" y2="81.28" width="0.1524" layer="91"/>
<wire x1="226.06" y1="81.28" x2="226.06" y2="78.74" width="0.1524" layer="91"/>
<wire x1="226.06" y1="78.74" x2="226.06" y2="73.66" width="0.1524" layer="91"/>
<wire x1="226.06" y1="73.66" x2="223.52" y2="73.66" width="0.1524" layer="91"/>
<wire x1="223.52" y1="66.04" x2="226.06" y2="66.04" width="0.1524" layer="91"/>
<wire x1="226.06" y1="66.04" x2="226.06" y2="68.58" width="0.1524" layer="91"/>
<wire x1="226.06" y1="68.58" x2="226.06" y2="73.66" width="0.1524" layer="91"/>
<wire x1="226.06" y1="58.42" x2="226.06" y2="66.04" width="0.1524" layer="91"/>
<wire x1="233.68" y1="68.58" x2="226.06" y2="68.58" width="0.1524" layer="91"/>
<wire x1="228.6" y1="78.74" x2="226.06" y2="78.74" width="0.1524" layer="91"/>
<junction x="226.06" y="73.66"/>
<junction x="226.06" y="66.04"/>
<junction x="226.06" y="68.58"/>
<junction x="226.06" y="78.74"/>
<label x="233.68" y="68.58" size="1.778" layer="95"/>
<pinref part="IC11" gate="G$1" pin="B4"/>
<pinref part="IC11" gate="G$1" pin="B1"/>
<pinref part="IC11" gate="G$1" pin="B2"/>
<pinref part="IC11" gate="G$1" pin="B3"/>
<pinref part="TP1" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="N$26" class="0">
<segment>
<wire x1="116.84" y1="81.28" x2="111.76" y2="81.28" width="0.1524" layer="91"/>
<pinref part="IC5" gate="B" pin="+IN"/>
<pinref part="R21" gate="G$1" pin="2"/>
</segment>
</net>
<net name="CROSS_HI" class="0">
<segment>
<wire x1="139.7" y1="78.74" x2="132.08" y2="78.74" width="0.1524" layer="91"/>
<label x="139.7" y="78.74" size="1.778" layer="95"/>
<pinref part="IC5" gate="B" pin="OUT"/>
</segment>
</net>
<net name="ADCDR_HI" class="0">
<segment>
<wire x1="137.16" y1="55.88" x2="152.4" y2="55.88" width="0.1524" layer="91"/>
<wire x1="139.7" y1="63.5" x2="152.4" y2="63.5" width="0.1524" layer="91"/>
<wire x1="152.4" y1="63.5" x2="152.4" y2="58.42" width="0.1524" layer="91"/>
<wire x1="152.4" y1="58.42" x2="152.4" y2="55.88" width="0.1524" layer="91"/>
<wire x1="152.4" y1="55.88" x2="152.4" y2="43.18" width="0.1524" layer="91"/>
<wire x1="152.4" y1="43.18" x2="144.78" y2="43.18" width="0.1524" layer="91"/>
<wire x1="198.12" y1="58.42" x2="152.4" y2="58.42" width="0.1524" layer="91"/>
<junction x="152.4" y="55.88"/>
<junction x="152.4" y="58.42"/>
<pinref part="R18" gate="G$1" pin="2"/>
<pinref part="C17" gate="G$1" pin="2"/>
<pinref part="IC6" gate="C" pin="OUT"/>
<pinref part="IC11" gate="G$1" pin="A4"/>
</segment>
</net>
<net name="AMPL_LO" class="0">
<segment>
<wire x1="45.72" y1="147.32" x2="45.72" y2="149.86" width="0.1524" layer="91"/>
<wire x1="45.72" y1="149.86" x2="55.88" y2="149.86" width="0.1524" layer="91"/>
<wire x1="55.88" y1="149.86" x2="55.88" y2="147.32" width="0.1524" layer="91"/>
<wire x1="45.72" y1="149.86" x2="45.72" y2="152.4" width="0.1524" layer="91"/>
<wire x1="45.72" y1="152.4" x2="48.26" y2="152.4" width="0.1524" layer="91"/>
<junction x="45.72" y="149.86"/>
<label x="48.26" y="152.4" size="1.778" layer="95"/>
<pinref part="R30" gate="G$1" pin="2"/>
<pinref part="R31" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$52" class="0">
<segment>
<wire x1="68.58" y1="93.98" x2="66.04" y2="93.98" width="0.1524" layer="91"/>
<wire x1="68.58" y1="86.36" x2="66.04" y2="86.36" width="0.1524" layer="91"/>
<wire x1="68.58" y1="78.74" x2="66.04" y2="78.74" width="0.1524" layer="91"/>
<wire x1="66.04" y1="86.36" x2="66.04" y2="78.74" width="0.1524" layer="91"/>
<wire x1="66.04" y1="93.98" x2="66.04" y2="86.36" width="0.1524" layer="91"/>
<wire x1="63.5" y1="93.98" x2="66.04" y2="93.98" width="0.1524" layer="91"/>
<junction x="66.04" y="86.36"/>
<junction x="66.04" y="93.98"/>
<pinref part="C40" gate="G$1" pin="1"/>
<pinref part="R33" gate="G$1" pin="1"/>
<pinref part="IC14" gate="A" pin="-IN"/>
<pinref part="R51" gate="G$1" pin="2"/>
</segment>
</net>
<net name="RAW_HI" class="0">
<segment>
<wire x1="76.2" y1="93.98" x2="91.44" y2="93.98" width="0.1524" layer="91"/>
<wire x1="91.44" y1="93.98" x2="91.44" y2="91.44" width="0.1524" layer="91"/>
<wire x1="91.44" y1="91.44" x2="91.44" y2="86.36" width="0.1524" layer="91"/>
<wire x1="91.44" y1="86.36" x2="91.44" y2="76.2" width="0.1524" layer="91"/>
<wire x1="91.44" y1="76.2" x2="83.82" y2="76.2" width="0.1524" layer="91"/>
<wire x1="78.74" y1="86.36" x2="91.44" y2="86.36" width="0.1524" layer="91"/>
<wire x1="91.44" y1="91.44" x2="160.02" y2="91.44" width="0.1524" layer="91"/>
<wire x1="160.02" y1="91.44" x2="160.02" y2="66.04" width="0.1524" layer="91"/>
<wire x1="160.02" y1="66.04" x2="198.12" y2="66.04" width="0.1524" layer="91"/>
<junction x="91.44" y="86.36"/>
<junction x="91.44" y="91.44"/>
<pinref part="C40" gate="G$1" pin="2"/>
<pinref part="IC14" gate="A" pin="OUT"/>
<pinref part="R33" gate="G$1" pin="2"/>
<pinref part="IC11" gate="G$1" pin="A3"/>
</segment>
</net>
<net name="MUXSEL_LOPKD" class="0">
<segment>
<wire x1="195.58" y1="71.12" x2="198.12" y2="71.12" width="0.1524" layer="91"/>
<label x="195.58" y="71.12" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC11" gate="G$1" pin="C2"/>
</segment>
</net>
<net name="MUXSEL_HIPKD" class="0">
<segment>
<wire x1="195.58" y1="55.88" x2="198.12" y2="55.88" width="0.1524" layer="91"/>
<label x="195.58" y="55.88" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC11" gate="G$1" pin="C4"/>
</segment>
</net>
<net name="MUXSEL_HIRAW" class="0">
<segment>
<wire x1="195.58" y1="63.5" x2="198.12" y2="63.5" width="0.1524" layer="91"/>
<label x="195.58" y="63.5" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC11" gate="G$1" pin="C3"/>
</segment>
</net>
<net name="MUXSEL_LORAW" class="0">
<segment>
<wire x1="195.58" y1="78.74" x2="198.12" y2="78.74" width="0.1524" layer="91"/>
<label x="195.58" y="78.74" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC11" gate="G$1" pin="C1"/>
</segment>
</net>
<net name="N$43" class="0">
<segment>
<wire x1="198.12" y1="81.28" x2="185.42" y2="81.28" width="0.1524" layer="91"/>
<wire x1="185.42" y1="81.28" x2="185.42" y2="132.08" width="0.1524" layer="91"/>
<wire x1="185.42" y1="132.08" x2="177.8" y2="132.08" width="0.1524" layer="91"/>
<wire x1="175.26" y1="142.24" x2="185.42" y2="142.24" width="0.1524" layer="91"/>
<wire x1="185.42" y1="142.24" x2="185.42" y2="132.08" width="0.1524" layer="91"/>
<wire x1="172.72" y1="149.86" x2="185.42" y2="149.86" width="0.1524" layer="91"/>
<wire x1="185.42" y1="149.86" x2="185.42" y2="142.24" width="0.1524" layer="91"/>
<junction x="185.42" y="132.08"/>
<junction x="185.42" y="142.24"/>
<pinref part="IC11" gate="G$1" pin="A1"/>
<pinref part="IC14" gate="B" pin="OUT"/>
<pinref part="R34" gate="G$1" pin="2"/>
<pinref part="C41" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$53" class="0">
<segment>
<wire x1="165.1" y1="149.86" x2="160.02" y2="149.86" width="0.1524" layer="91"/>
<wire x1="160.02" y1="149.86" x2="160.02" y2="142.24" width="0.1524" layer="91"/>
<wire x1="160.02" y1="142.24" x2="160.02" y2="134.62" width="0.1524" layer="91"/>
<wire x1="160.02" y1="134.62" x2="162.56" y2="134.62" width="0.1524" layer="91"/>
<wire x1="165.1" y1="142.24" x2="160.02" y2="142.24" width="0.1524" layer="91"/>
<wire x1="154.94" y1="157.48" x2="160.02" y2="157.48" width="0.1524" layer="91"/>
<wire x1="160.02" y1="157.48" x2="160.02" y2="149.86" width="0.1524" layer="91"/>
<junction x="160.02" y="142.24"/>
<junction x="160.02" y="149.86"/>
<pinref part="C41" gate="G$1" pin="1"/>
<pinref part="IC14" gate="B" pin="-IN"/>
<pinref part="R34" gate="G$1" pin="1"/>
<pinref part="R50" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$54" class="0">
<segment>
<wire x1="152.4" y1="129.54" x2="157.48" y2="129.54" width="0.1524" layer="91"/>
<wire x1="157.48" y1="127" x2="157.48" y2="129.54" width="0.1524" layer="91"/>
<wire x1="157.48" y1="129.54" x2="162.56" y2="129.54" width="0.1524" layer="91"/>
<junction x="157.48" y="129.54"/>
<pinref part="R32" gate="G$1" pin="2"/>
<pinref part="R48" gate="G$1" pin="2"/>
<pinref part="IC14" gate="B" pin="+IN"/>
</segment>
</net>
<net name="N$56" class="0">
<segment>
<wire x1="109.22" y1="27.94" x2="109.22" y2="25.4" width="0.1524" layer="91"/>
<pinref part="R37" gate="G$1" pin="1"/>
<pinref part="R38" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$35" class="0">
<segment>
<wire x1="25.4" y1="111.76" x2="22.86" y2="111.76" width="0.1524" layer="91"/>
<wire x1="22.86" y1="111.76" x2="22.86" y2="109.22" width="0.1524" layer="91"/>
<wire x1="22.86" y1="109.22" x2="15.24" y2="109.22" width="0.1524" layer="91"/>
<wire x1="15.24" y1="109.22" x2="15.24" y2="114.3" width="0.1524" layer="91"/>
<wire x1="22.86" y1="106.68" x2="22.86" y2="109.22" width="0.1524" layer="91"/>
<junction x="22.86" y="109.22"/>
<pinref part="RLY1" gate="L" pin="P$2"/>
<pinref part="D7" gate="G$1" pin="A"/>
<pinref part="Q1" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$38" class="0">
<segment>
<wire x1="17.78" y1="101.6" x2="12.7" y2="101.6" width="0.1524" layer="91"/>
<wire x1="12.7" y1="101.6" x2="12.7" y2="99.06" width="0.1524" layer="91"/>
<pinref part="Q1" gate="G$1" pin="B"/>
<pinref part="R40" gate="G$1" pin="2"/>
</segment>
</net>
<net name="RELAY_ON" class="0">
<segment>
<wire x1="12.7" y1="86.36" x2="12.7" y2="88.9" width="0.1524" layer="91"/>
<label x="12.7" y="86.36" size="1.778" layer="95" rot="MR270"/>
<pinref part="R40" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$36" class="0">
<segment>
<wire x1="20.32" y1="48.26" x2="22.86" y2="48.26" width="0.1524" layer="91"/>
<wire x1="22.86" y1="48.26" x2="22.86" y2="40.64" width="0.1524" layer="91"/>
<wire x1="22.86" y1="40.64" x2="30.48" y2="40.64" width="0.1524" layer="91"/>
<wire x1="30.48" y1="40.64" x2="33.02" y2="40.64" width="0.1524" layer="91"/>
<wire x1="30.48" y1="63.5" x2="30.48" y2="40.64" width="0.1524" layer="91"/>
<wire x1="30.48" y1="63.5" x2="96.52" y2="63.5" width="0.1524" layer="91"/>
<wire x1="96.52" y1="63.5" x2="96.52" y2="81.28" width="0.1524" layer="91"/>
<wire x1="96.52" y1="81.28" x2="101.6" y2="81.28" width="0.1524" layer="91"/>
<junction x="30.48" y="40.64"/>
<pinref part="RLY1" gate="A" pin="NC"/>
<pinref part="C14" gate="G$1" pin="1"/>
<pinref part="R21" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$37" class="0">
<segment>
<wire x1="22.86" y1="58.42" x2="20.32" y2="58.42" width="0.1524" layer="91"/>
<wire x1="22.86" y1="83.82" x2="22.86" y2="58.42" width="0.1524" layer="91"/>
<wire x1="30.48" y1="83.82" x2="22.86" y2="83.82" width="0.1524" layer="91"/>
<pinref part="RLY1" gate="A" pin="NO"/>
<pinref part="C42" gate="G$1" pin="1"/>
</segment>
</net>
<net name="ANT_HI" class="0">
<segment>
<wire x1="20.32" y1="53.34" x2="27.94" y2="53.34" width="0.1524" layer="91"/>
<wire x1="27.94" y1="53.34" x2="27.94" y2="55.88" width="0.1524" layer="91"/>
<label x="27.94" y="55.88" size="1.778" layer="95" rot="R90"/>
<pinref part="RLY1" gate="A" pin="COM"/>
</segment>
</net>
<net name="ANT_LO" class="0">
<segment>
<wire x1="22.86" y1="149.86" x2="30.48" y2="149.86" width="0.1524" layer="91"/>
<wire x1="30.48" y1="149.86" x2="30.48" y2="152.4" width="0.1524" layer="91"/>
<label x="30.48" y="152.4" size="1.778" layer="95" rot="R90"/>
<pinref part="RLY1" gate="B" pin="COM"/>
</segment>
</net>
<net name="N$39" class="0">
<segment>
<wire x1="55.88" y1="162.56" x2="33.02" y2="162.56" width="0.1524" layer="91"/>
<wire x1="33.02" y1="162.56" x2="33.02" y2="144.78" width="0.1524" layer="91"/>
<wire x1="35.56" y1="127" x2="27.94" y2="127" width="0.1524" layer="91"/>
<wire x1="27.94" y1="127" x2="27.94" y2="144.78" width="0.1524" layer="91"/>
<wire x1="27.94" y1="144.78" x2="22.86" y2="144.78" width="0.1524" layer="91"/>
<wire x1="33.02" y1="144.78" x2="27.94" y2="144.78" width="0.1524" layer="91"/>
<junction x="27.94" y="144.78"/>
<pinref part="R20" gate="G$1" pin="1"/>
<pinref part="D1" gate="G$1" pin="A"/>
<pinref part="RLY1" gate="B" pin="NC"/>
</segment>
</net>
<net name="N$46" class="0">
<segment>
<wire x1="22.86" y1="154.94" x2="25.4" y2="154.94" width="0.1524" layer="91"/>
<wire x1="25.4" y1="154.94" x2="25.4" y2="170.18" width="0.1524" layer="91"/>
<wire x1="25.4" y1="170.18" x2="129.54" y2="170.18" width="0.1524" layer="91"/>
<wire x1="129.54" y1="170.18" x2="129.54" y2="129.54" width="0.1524" layer="91"/>
<wire x1="129.54" y1="129.54" x2="132.08" y2="129.54" width="0.1524" layer="91"/>
<pinref part="RLY1" gate="B" pin="NO"/>
<pinref part="C43" gate="G$1" pin="1"/>
</segment>
</net>
<net name="AMPL_HI" class="0">
<segment>
<wire x1="68.58" y1="55.88" x2="68.58" y2="58.42" width="0.1524" layer="91"/>
<wire x1="68.58" y1="58.42" x2="73.66" y2="58.42" width="0.1524" layer="91"/>
<wire x1="63.5" y1="58.42" x2="68.58" y2="58.42" width="0.1524" layer="91"/>
<junction x="68.58" y="58.42"/>
<label x="63.5" y="58.42" size="1.778" layer="95" rot="MR0"/>
<pinref part="R41" gate="G$1" pin="2"/>
<pinref part="R42" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$50" class="0">
<segment>
<wire x1="142.24" y1="129.54" x2="139.7" y2="129.54" width="0.1524" layer="91"/>
<pinref part="R32" gate="G$1" pin="1"/>
<pinref part="C43" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$51" class="0">
<segment>
<wire x1="40.64" y1="83.82" x2="38.1" y2="83.82" width="0.1524" layer="91"/>
<pinref part="R35" gate="G$1" pin="1"/>
<pinref part="C42" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$47" class="0">
<segment>
<wire x1="68.58" y1="73.66" x2="60.96" y2="73.66" width="0.1524" layer="91"/>
<wire x1="60.96" y1="73.66" x2="60.96" y2="83.82" width="0.1524" layer="91"/>
<wire x1="50.8" y1="83.82" x2="53.34" y2="83.82" width="0.1524" layer="91"/>
<wire x1="53.34" y1="81.28" x2="53.34" y2="83.82" width="0.1524" layer="91"/>
<wire x1="60.96" y1="83.82" x2="53.34" y2="83.82" width="0.1524" layer="91"/>
<junction x="53.34" y="83.82"/>
<pinref part="IC14" gate="A" pin="+IN"/>
<pinref part="R35" gate="G$1" pin="2"/>
<pinref part="R49" gate="G$1" pin="2"/>
</segment>
</net>
<net name="N$21" class="0">
<segment>
<wire x1="53.34" y1="40.64" x2="58.42" y2="40.64" width="0.1524" layer="91"/>
<wire x1="58.42" y1="40.64" x2="66.04" y2="40.64" width="0.1524" layer="91"/>
<wire x1="66.04" y1="40.64" x2="66.04" y2="35.56" width="0.1524" layer="91"/>
<wire x1="66.04" y1="40.64" x2="68.58" y2="40.64" width="0.1524" layer="91"/>
<wire x1="68.58" y1="40.64" x2="78.74" y2="40.64" width="0.1524" layer="91"/>
<wire x1="78.74" y1="40.64" x2="78.74" y2="35.56" width="0.1524" layer="91"/>
<wire x1="78.74" y1="40.64" x2="83.82" y2="40.64" width="0.1524" layer="91"/>
<wire x1="68.58" y1="45.72" x2="68.58" y2="40.64" width="0.1524" layer="91"/>
<wire x1="58.42" y1="33.02" x2="58.42" y2="40.64" width="0.1524" layer="91"/>
<junction x="66.04" y="40.64"/>
<junction x="78.74" y="40.64"/>
<junction x="68.58" y="40.64"/>
<junction x="58.42" y="40.64"/>
<pinref part="D10" gate="G$1" pin="C"/>
<pinref part="D2" gate="G$1" pin="C"/>
<pinref part="C15" gate="G$1" pin="1"/>
<pinref part="R15" gate="G$1" pin="2"/>
<pinref part="R16" gate="G$1" pin="1"/>
<pinref part="R41" gate="G$1" pin="1"/>
</segment>
</net>
</nets>
</sheet>
<sheet>
<plain>
<text x="165.1" y="17.78" size="2.54" layer="95">analog tx path</text>
<text x="165.1" y="12.7" size="2.54" layer="95">(incl. coil drivers)</text>
</plain>
<instances>
<instance part="FRAME5" gate="G$1" x="0" y="0"/>
<instance part="SV2" gate="G$1" x="226.06" y="119.38"/>
<instance part="IC9" gate="G$1" x="55.88" y="124.46"/>
<instance part="V26" gate="GND" x="53.34" y="55.88" rot="MR0"/>
<instance part="VDD5" gate="G$1" x="63.5" y="142.24" rot="MR0"/>
<instance part="IC10" gate="G$1" x="127" y="124.46"/>
<instance part="V27" gate="GND" x="124.46" y="55.88" rot="MR0"/>
<instance part="VDD6" gate="G$1" x="134.62" y="142.24" rot="MR0"/>
<instance part="R22" gate="G$1" x="78.74" y="78.74"/>
<instance part="R27" gate="G$1" x="78.74" y="99.06"/>
<instance part="R28" gate="G$1" x="149.86" y="124.46"/>
<instance part="R29" gate="G$1" x="149.86" y="73.66"/>
<instance part="C39" gate="G$1" x="167.64" y="109.22" rot="MR0"/>
<instance part="V47" gate="GND" x="167.64" y="99.06" rot="MR0"/>
<instance part="C20" gate="G$1" x="175.26" y="109.22" rot="MR0"/>
<instance part="C35" gate="G$1" x="200.66" y="104.14" rot="MR0"/>
<instance part="C36" gate="G$1" x="208.28" y="104.14" rot="MR0"/>
<instance part="V41" gate="GND" x="200.66" y="93.98" rot="MR0"/>
<instance part="R45" gate="G$1" x="78.74" y="91.44"/>
<instance part="R46" gate="G$1" x="149.86" y="91.44"/>
<instance part="TP2" gate="G$1" x="208.28" y="124.46"/>
<instance part="TP3" gate="G$1" x="218.44" y="124.46"/>
<instance part="TP4" gate="G$1" x="213.36" y="124.46"/>
<instance part="TP5" gate="G$1" x="203.2" y="124.46"/>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<wire x1="53.34" y1="60.96" x2="53.34" y2="58.42" width="0.1524" layer="91"/>
<pinref part="IC9" gate="G$1" pin="VSS"/>
<pinref part="V26" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="124.46" y1="60.96" x2="124.46" y2="58.42" width="0.1524" layer="91"/>
<pinref part="IC10" gate="G$1" pin="VSS"/>
<pinref part="V27" gate="GND" pin="GND"/>
</segment>
<segment>
<wire x1="167.64" y1="106.68" x2="167.64" y2="104.14" width="0.1524" layer="91"/>
<wire x1="167.64" y1="104.14" x2="167.64" y2="101.6" width="0.1524" layer="91"/>
<wire x1="167.64" y1="104.14" x2="175.26" y2="104.14" width="0.1524" layer="91"/>
<wire x1="175.26" y1="104.14" x2="175.26" y2="106.68" width="0.1524" layer="91"/>
<junction x="167.64" y="104.14"/>
<pinref part="C20" gate="G$1" pin="2"/>
<pinref part="V47" gate="GND" pin="GND"/>
<pinref part="C39" gate="G$1" pin="2"/>
</segment>
<segment>
<wire x1="200.66" y1="101.6" x2="200.66" y2="99.06" width="0.1524" layer="91"/>
<wire x1="200.66" y1="99.06" x2="200.66" y2="96.52" width="0.1524" layer="91"/>
<wire x1="200.66" y1="99.06" x2="208.28" y2="99.06" width="0.1524" layer="91"/>
<wire x1="208.28" y1="99.06" x2="208.28" y2="101.6" width="0.1524" layer="91"/>
<junction x="200.66" y="99.06"/>
<pinref part="C35" gate="G$1" pin="2"/>
<pinref part="V41" gate="GND" pin="GND"/>
<pinref part="C36" gate="G$1" pin="2"/>
</segment>
</net>
<net name="VDD" class="0">
<segment>
<wire x1="63.5" y1="139.7" x2="63.5" y2="137.16" width="0.1524" layer="91"/>
<pinref part="VDD5" gate="G$1" pin="VDD"/>
<pinref part="IC9" gate="G$1" pin="VDD"/>
</segment>
<segment>
<wire x1="134.62" y1="139.7" x2="134.62" y2="137.16" width="0.1524" layer="91"/>
<pinref part="VDD6" gate="G$1" pin="VDD"/>
<pinref part="IC10" gate="G$1" pin="VDD"/>
</segment>
</net>
<net name="N$31" class="0">
<segment>
<wire x1="93.98" y1="99.06" x2="93.98" y2="48.26" width="0.1524" layer="91"/>
<wire x1="83.82" y1="99.06" x2="86.36" y2="99.06" width="0.1524" layer="91"/>
<wire x1="86.36" y1="99.06" x2="93.98" y2="99.06" width="0.1524" layer="91"/>
<wire x1="93.98" y1="48.26" x2="157.48" y2="48.26" width="0.1524" layer="91"/>
<wire x1="157.48" y1="48.26" x2="157.48" y2="73.66" width="0.1524" layer="91"/>
<wire x1="157.48" y1="73.66" x2="154.94" y2="73.66" width="0.1524" layer="91"/>
<wire x1="157.48" y1="73.66" x2="182.88" y2="73.66" width="0.1524" layer="91"/>
<wire x1="182.88" y1="73.66" x2="182.88" y2="114.3" width="0.1524" layer="91"/>
<wire x1="182.88" y1="114.3" x2="213.36" y2="114.3" width="0.1524" layer="91"/>
<wire x1="213.36" y1="114.3" x2="220.98" y2="114.3" width="0.1524" layer="91"/>
<wire x1="83.82" y1="91.44" x2="86.36" y2="91.44" width="0.1524" layer="91"/>
<wire x1="86.36" y1="91.44" x2="86.36" y2="99.06" width="0.1524" layer="91"/>
<wire x1="213.36" y1="121.92" x2="213.36" y2="114.3" width="0.1524" layer="91"/>
<junction x="157.48" y="73.66"/>
<junction x="86.36" y="99.06"/>
<junction x="213.36" y="114.3"/>
<pinref part="R27" gate="G$1" pin="2"/>
<pinref part="R29" gate="G$1" pin="2"/>
<pinref part="SV2" gate="G$1" pin="PIN3"/>
<pinref part="R45" gate="G$1" pin="2"/>
<pinref part="TP4" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="N$41" class="0">
<segment>
<wire x1="68.58" y1="83.82" x2="71.12" y2="83.82" width="0.1524" layer="91"/>
<wire x1="71.12" y1="76.2" x2="71.12" y2="78.74" width="0.1524" layer="91"/>
<wire x1="71.12" y1="78.74" x2="71.12" y2="83.82" width="0.1524" layer="91"/>
<wire x1="71.12" y1="76.2" x2="68.58" y2="76.2" width="0.1524" layer="91"/>
<wire x1="71.12" y1="68.58" x2="71.12" y2="76.2" width="0.1524" layer="91"/>
<wire x1="68.58" y1="68.58" x2="71.12" y2="68.58" width="0.1524" layer="91"/>
<wire x1="73.66" y1="78.74" x2="71.12" y2="78.74" width="0.1524" layer="91"/>
<junction x="71.12" y="76.2"/>
<junction x="71.12" y="78.74"/>
<pinref part="IC9" gate="G$1" pin="2Y3"/>
<pinref part="IC9" gate="G$1" pin="2Y2"/>
<pinref part="IC9" gate="G$1" pin="2Y4"/>
<pinref part="R22" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$33" class="0">
<segment>
<wire x1="68.58" y1="127" x2="71.12" y2="127" width="0.1524" layer="91"/>
<wire x1="71.12" y1="127" x2="71.12" y2="119.38" width="0.1524" layer="91"/>
<wire x1="71.12" y1="119.38" x2="68.58" y2="119.38" width="0.1524" layer="91"/>
<wire x1="68.58" y1="111.76" x2="71.12" y2="111.76" width="0.1524" layer="91"/>
<wire x1="71.12" y1="111.76" x2="71.12" y2="119.38" width="0.1524" layer="91"/>
<wire x1="68.58" y1="104.14" x2="71.12" y2="104.14" width="0.1524" layer="91"/>
<wire x1="71.12" y1="104.14" x2="71.12" y2="111.76" width="0.1524" layer="91"/>
<wire x1="71.12" y1="99.06" x2="71.12" y2="104.14" width="0.1524" layer="91"/>
<wire x1="73.66" y1="99.06" x2="71.12" y2="99.06" width="0.1524" layer="91"/>
<junction x="71.12" y="119.38"/>
<junction x="71.12" y="111.76"/>
<junction x="71.12" y="104.14"/>
<pinref part="IC9" gate="G$1" pin="1Y2"/>
<pinref part="IC9" gate="G$1" pin="1Y1"/>
<pinref part="IC9" gate="G$1" pin="1Y3"/>
<pinref part="IC9" gate="G$1" pin="1Y4"/>
<pinref part="R27" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$34" class="0">
<segment>
<wire x1="142.24" y1="104.14" x2="139.7" y2="104.14" width="0.1524" layer="91"/>
<wire x1="142.24" y1="111.76" x2="142.24" y2="104.14" width="0.1524" layer="91"/>
<wire x1="139.7" y1="111.76" x2="142.24" y2="111.76" width="0.1524" layer="91"/>
<wire x1="139.7" y1="127" x2="142.24" y2="127" width="0.1524" layer="91"/>
<wire x1="142.24" y1="127" x2="142.24" y2="124.46" width="0.1524" layer="91"/>
<wire x1="144.78" y1="124.46" x2="142.24" y2="124.46" width="0.1524" layer="91"/>
<wire x1="142.24" y1="124.46" x2="142.24" y2="119.38" width="0.1524" layer="91"/>
<wire x1="142.24" y1="119.38" x2="142.24" y2="111.76" width="0.1524" layer="91"/>
<wire x1="139.7" y1="119.38" x2="142.24" y2="119.38" width="0.1524" layer="91"/>
<junction x="142.24" y="111.76"/>
<junction x="142.24" y="124.46"/>
<junction x="142.24" y="119.38"/>
<pinref part="IC10" gate="G$1" pin="1Y4"/>
<pinref part="IC10" gate="G$1" pin="1Y3"/>
<pinref part="IC10" gate="G$1" pin="1Y1"/>
<pinref part="R28" gate="G$1" pin="1"/>
<pinref part="IC10" gate="G$1" pin="1Y2"/>
</segment>
</net>
<net name="N$42" class="0">
<segment>
<wire x1="139.7" y1="76.2" x2="142.24" y2="76.2" width="0.1524" layer="91"/>
<wire x1="139.7" y1="83.82" x2="142.24" y2="83.82" width="0.1524" layer="91"/>
<wire x1="142.24" y1="83.82" x2="142.24" y2="81.28" width="0.1524" layer="91"/>
<wire x1="142.24" y1="78.74" x2="142.24" y2="81.28" width="0.1524" layer="91"/>
<wire x1="142.24" y1="81.28" x2="142.24" y2="76.2" width="0.1524" layer="91"/>
<wire x1="142.24" y1="76.2" x2="142.24" y2="73.66" width="0.1524" layer="91"/>
<wire x1="142.24" y1="73.66" x2="142.24" y2="68.58" width="0.1524" layer="91"/>
<wire x1="142.24" y1="68.58" x2="139.7" y2="68.58" width="0.1524" layer="91"/>
<wire x1="144.78" y1="73.66" x2="142.24" y2="73.66" width="0.1524" layer="91"/>
<junction x="142.24" y="81.28"/>
<junction x="142.24" y="76.2"/>
<junction x="142.24" y="73.66"/>
<pinref part="IC10" gate="G$1" pin="2Y3"/>
<pinref part="IC10" gate="G$1" pin="2Y2"/>
<pinref part="IC10" gate="G$1" pin="2Y4"/>
<pinref part="R29" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$32" class="0">
<segment>
<wire x1="220.98" y1="119.38" x2="203.2" y2="119.38" width="0.1524" layer="91"/>
<wire x1="203.2" y1="119.38" x2="185.42" y2="119.38" width="0.1524" layer="91"/>
<wire x1="185.42" y1="119.38" x2="185.42" y2="132.08" width="0.1524" layer="91"/>
<wire x1="88.9" y1="78.74" x2="88.9" y2="147.32" width="0.1524" layer="91"/>
<wire x1="88.9" y1="147.32" x2="157.48" y2="147.32" width="0.1524" layer="91"/>
<wire x1="157.48" y1="147.32" x2="157.48" y2="132.08" width="0.1524" layer="91"/>
<wire x1="157.48" y1="132.08" x2="157.48" y2="124.46" width="0.1524" layer="91"/>
<wire x1="83.82" y1="78.74" x2="88.9" y2="78.74" width="0.1524" layer="91"/>
<wire x1="154.94" y1="124.46" x2="157.48" y2="124.46" width="0.1524" layer="91"/>
<wire x1="185.42" y1="132.08" x2="157.48" y2="132.08" width="0.1524" layer="91"/>
<wire x1="154.94" y1="91.44" x2="157.48" y2="91.44" width="0.1524" layer="91"/>
<wire x1="157.48" y1="91.44" x2="157.48" y2="124.46" width="0.1524" layer="91"/>
<wire x1="203.2" y1="121.92" x2="203.2" y2="119.38" width="0.1524" layer="91"/>
<junction x="157.48" y="132.08"/>
<junction x="157.48" y="124.46"/>
<junction x="203.2" y="119.38"/>
<pinref part="SV2" gate="G$1" pin="PIN1"/>
<pinref part="R22" gate="G$1" pin="2"/>
<pinref part="R28" gate="G$1" pin="2"/>
<pinref part="R46" gate="G$1" pin="2"/>
<pinref part="TP5" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="PWR_OE1" class="0">
<segment>
<wire x1="48.26" y1="132.08" x2="38.1" y2="132.08" width="0.1524" layer="91"/>
<label x="38.1" y="132.08" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC9" gate="G$1" pin="1NOE"/>
</segment>
</net>
<net name="PWR_OE2" class="0">
<segment>
<wire x1="119.38" y1="132.08" x2="109.22" y2="132.08" width="0.1524" layer="91"/>
<label x="109.22" y="132.08" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC10" gate="G$1" pin="1NOE"/>
</segment>
</net>
<net name="PWR_LO" class="0">
<segment>
<wire x1="119.38" y1="127" x2="116.84" y2="127" width="0.1524" layer="91"/>
<wire x1="116.84" y1="127" x2="116.84" y2="121.92" width="0.1524" layer="91"/>
<wire x1="116.84" y1="121.92" x2="116.84" y2="119.38" width="0.1524" layer="91"/>
<wire x1="116.84" y1="119.38" x2="119.38" y2="119.38" width="0.1524" layer="91"/>
<wire x1="119.38" y1="111.76" x2="116.84" y2="111.76" width="0.1524" layer="91"/>
<wire x1="116.84" y1="111.76" x2="116.84" y2="119.38" width="0.1524" layer="91"/>
<wire x1="119.38" y1="104.14" x2="116.84" y2="104.14" width="0.1524" layer="91"/>
<wire x1="116.84" y1="104.14" x2="116.84" y2="111.76" width="0.1524" layer="91"/>
<wire x1="119.38" y1="91.44" x2="116.84" y2="91.44" width="0.1524" layer="91"/>
<wire x1="116.84" y1="91.44" x2="116.84" y2="104.14" width="0.1524" layer="91"/>
<wire x1="116.84" y1="121.92" x2="109.22" y2="121.92" width="0.1524" layer="91"/>
<junction x="116.84" y="119.38"/>
<junction x="116.84" y="111.76"/>
<junction x="116.84" y="104.14"/>
<junction x="116.84" y="121.92"/>
<label x="109.22" y="121.92" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC10" gate="G$1" pin="1A1"/>
<pinref part="IC10" gate="G$1" pin="1A2"/>
<pinref part="IC10" gate="G$1" pin="1A3"/>
<pinref part="IC10" gate="G$1" pin="1A4"/>
<pinref part="IC10" gate="G$1" pin="2A1"/>
</segment>
<segment>
<wire x1="48.26" y1="83.82" x2="45.72" y2="83.82" width="0.1524" layer="91"/>
<wire x1="45.72" y1="83.82" x2="45.72" y2="81.28" width="0.1524" layer="91"/>
<wire x1="45.72" y1="81.28" x2="45.72" y2="76.2" width="0.1524" layer="91"/>
<wire x1="45.72" y1="76.2" x2="45.72" y2="68.58" width="0.1524" layer="91"/>
<wire x1="45.72" y1="68.58" x2="48.26" y2="68.58" width="0.1524" layer="91"/>
<wire x1="48.26" y1="76.2" x2="45.72" y2="76.2" width="0.1524" layer="91"/>
<wire x1="45.72" y1="81.28" x2="38.1" y2="81.28" width="0.1524" layer="91"/>
<junction x="45.72" y="76.2"/>
<junction x="45.72" y="81.28"/>
<label x="38.1" y="81.28" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC9" gate="G$1" pin="2A2"/>
<pinref part="IC9" gate="G$1" pin="2A4"/>
<pinref part="IC9" gate="G$1" pin="2A3"/>
</segment>
</net>
<net name="PWR_HI" class="0">
<segment>
<wire x1="45.72" y1="104.14" x2="48.26" y2="104.14" width="0.1524" layer="91"/>
<wire x1="48.26" y1="111.76" x2="45.72" y2="111.76" width="0.1524" layer="91"/>
<wire x1="48.26" y1="119.38" x2="45.72" y2="119.38" width="0.1524" layer="91"/>
<wire x1="48.26" y1="127" x2="45.72" y2="127" width="0.1524" layer="91"/>
<wire x1="45.72" y1="127" x2="45.72" y2="121.92" width="0.1524" layer="91"/>
<wire x1="45.72" y1="121.92" x2="45.72" y2="119.38" width="0.1524" layer="91"/>
<wire x1="45.72" y1="119.38" x2="45.72" y2="111.76" width="0.1524" layer="91"/>
<wire x1="45.72" y1="111.76" x2="45.72" y2="104.14" width="0.1524" layer="91"/>
<wire x1="48.26" y1="91.44" x2="45.72" y2="91.44" width="0.1524" layer="91"/>
<wire x1="45.72" y1="91.44" x2="45.72" y2="104.14" width="0.1524" layer="91"/>
<wire x1="45.72" y1="121.92" x2="38.1" y2="121.92" width="0.1524" layer="91"/>
<junction x="45.72" y="119.38"/>
<junction x="45.72" y="111.76"/>
<junction x="45.72" y="104.14"/>
<junction x="45.72" y="121.92"/>
<label x="38.1" y="121.92" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC9" gate="G$1" pin="1A4"/>
<pinref part="IC9" gate="G$1" pin="1A3"/>
<pinref part="IC9" gate="G$1" pin="1A2"/>
<pinref part="IC9" gate="G$1" pin="1A1"/>
<pinref part="IC9" gate="G$1" pin="2A1"/>
</segment>
<segment>
<wire x1="119.38" y1="83.82" x2="116.84" y2="83.82" width="0.1524" layer="91"/>
<wire x1="116.84" y1="83.82" x2="116.84" y2="81.28" width="0.1524" layer="91"/>
<wire x1="116.84" y1="81.28" x2="116.84" y2="76.2" width="0.1524" layer="91"/>
<wire x1="116.84" y1="76.2" x2="116.84" y2="68.58" width="0.1524" layer="91"/>
<wire x1="116.84" y1="68.58" x2="119.38" y2="68.58" width="0.1524" layer="91"/>
<wire x1="119.38" y1="76.2" x2="116.84" y2="76.2" width="0.1524" layer="91"/>
<wire x1="116.84" y1="81.28" x2="109.22" y2="81.28" width="0.1524" layer="91"/>
<junction x="116.84" y="76.2"/>
<junction x="116.84" y="81.28"/>
<label x="109.22" y="81.28" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC10" gate="G$1" pin="2A2"/>
<pinref part="IC10" gate="G$1" pin="2A4"/>
<pinref part="IC10" gate="G$1" pin="2A3"/>
</segment>
</net>
<net name="ANT_HI" class="0">
<segment>
<wire x1="220.98" y1="111.76" x2="218.44" y2="111.76" width="0.1524" layer="91"/>
<wire x1="218.44" y1="111.76" x2="208.28" y2="111.76" width="0.1524" layer="91"/>
<wire x1="208.28" y1="111.76" x2="208.28" y2="109.22" width="0.1524" layer="91"/>
<wire x1="208.28" y1="111.76" x2="200.66" y2="111.76" width="0.1524" layer="91"/>
<wire x1="200.66" y1="111.76" x2="200.66" y2="109.22" width="0.1524" layer="91"/>
<wire x1="200.66" y1="111.76" x2="198.12" y2="111.76" width="0.1524" layer="91"/>
<wire x1="218.44" y1="121.92" x2="218.44" y2="111.76" width="0.1524" layer="91"/>
<junction x="208.28" y="111.76"/>
<junction x="200.66" y="111.76"/>
<junction x="218.44" y="111.76"/>
<label x="198.12" y="111.76" size="1.778" layer="95" rot="MR0"/>
<pinref part="SV2" gate="G$1" pin="PIN4"/>
<pinref part="C36" gate="G$1" pin="1"/>
<pinref part="C35" gate="G$1" pin="1"/>
<pinref part="TP3" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="ANT_LO" class="0">
<segment>
<wire x1="220.98" y1="116.84" x2="208.28" y2="116.84" width="0.1524" layer="91"/>
<wire x1="208.28" y1="116.84" x2="175.26" y2="116.84" width="0.1524" layer="91"/>
<wire x1="175.26" y1="116.84" x2="170.18" y2="116.84" width="0.1524" layer="91"/>
<wire x1="170.18" y1="116.84" x2="167.64" y2="116.84" width="0.1524" layer="91"/>
<wire x1="167.64" y1="116.84" x2="167.64" y2="114.3" width="0.1524" layer="91"/>
<wire x1="175.26" y1="114.3" x2="175.26" y2="116.84" width="0.1524" layer="91"/>
<wire x1="170.18" y1="116.84" x2="170.18" y2="119.38" width="0.1524" layer="91"/>
<wire x1="170.18" y1="119.38" x2="172.72" y2="119.38" width="0.1524" layer="91"/>
<wire x1="208.28" y1="121.92" x2="208.28" y2="116.84" width="0.1524" layer="91"/>
<junction x="175.26" y="116.84"/>
<junction x="170.18" y="116.84"/>
<junction x="208.28" y="116.84"/>
<label x="172.72" y="119.38" size="1.778" layer="95"/>
<pinref part="SV2" gate="G$1" pin="PIN2"/>
<pinref part="C39" gate="G$1" pin="1"/>
<pinref part="C20" gate="G$1" pin="1"/>
<pinref part="TP2" gate="G$1" pin="P$1"/>
</segment>
</net>
<net name="PWR_OE3" class="0">
<segment>
<wire x1="38.1" y1="96.52" x2="48.26" y2="96.52" width="0.1524" layer="91"/>
<label x="38.1" y="96.52" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC9" gate="G$1" pin="2NOE"/>
</segment>
</net>
<net name="PWR_OE4" class="0">
<segment>
<wire x1="109.22" y1="96.52" x2="119.38" y2="96.52" width="0.1524" layer="91"/>
<label x="109.22" y="96.52" size="1.778" layer="95" rot="MR0"/>
<pinref part="IC10" gate="G$1" pin="2NOE"/>
</segment>
</net>
<net name="N$44" class="0">
<segment>
<wire x1="73.66" y1="91.44" x2="68.58" y2="91.44" width="0.1524" layer="91"/>
<pinref part="IC9" gate="G$1" pin="2Y1"/>
<pinref part="R45" gate="G$1" pin="1"/>
</segment>
</net>
<net name="N$45" class="0">
<segment>
<wire x1="144.78" y1="91.44" x2="139.7" y2="91.44" width="0.1524" layer="91"/>
<pinref part="IC10" gate="G$1" pin="2Y1"/>
<pinref part="R46" gate="G$1" pin="1"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
