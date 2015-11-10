# playback #
These patches depend on a hardcoded file path, relative to the execution of the 
main <code>en.pd<code> patch. They were developed for the *Klangbewegung 
Maschine* project where the (((20Hz))) team designed motion and light to 
accompany three Einsturzende Neubauten tracks; *Feurio*, *Prologue* and *Merle*.   

You'll need to put your tracks in folders named <code>feurio</code>, 
<code>prologue</code> or <code>merle</code> to get this working out of the box. 
Otherwise you can patch around this and dump your files wherever. Read the AkE 
manual for an idea of the naming conventions. 

It's designed to be driven by a TouchOSC interface, there are plans to migrate 
to an open-source alternative, most likely a local web server, patches welcome.
