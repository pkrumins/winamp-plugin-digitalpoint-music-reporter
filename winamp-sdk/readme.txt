Winamp 5 SDK	Current version: 5.04 (no changes were made for 5.05)
------------

This installer contains various SDKs for creating new plugins for Winamp 5.
As well you can create plugins for the older 2.x series of Winamp since the
5.x SDK is a continuation of the 2.x SDK (have a look in wa_api.txt).

Note: this is a customised version of the Official 5.04 SDK but contains
      some extras useful for making more advanced plugins as well as better
      documentation of the APIs (this documentation is a work in progress
      so there will be updates made as the process goes on)

[Installer Revision: #3 28/08/2004]


Contents
--------

Here's a listing of the various folders that can be installed
(you can chose what to or not to install on the next page)


/winamp - Headers and IPC calls for controlling Winamp and API information

/gen_ml - Headers and IPC calls for accessing/controlling/querying the Media 
          Library (includes extended gay_string.c\.h for ml_www <3 )

/gen_ml/ml_ex - Sample code for a Media Library plugin

/ml_www - Code for ml_www 0.41a as a more advanced Media Library plugin

/jnetlib - A portable C++ asynchronous network abstraction layer (v0.41)
           (http://www.nullsoft.com/free/jnetlib/)

/gen_tray - Sample code for a General Purpose plugin

/in_raw - Sample code for an Input plugin

/dsp_test - Sample code for a DSP test plugin

/out_raw - Sample code for an Output plugin

/vis - Includes vis plugin headers and wa5vis.txt which describes how to embed
       your visualisation plugin in the Winamp 5 drawer

/vis/vis_test - Sample code for a Visualisation plugin

/vis/vis_avs/apesdk - Sample code for an AVS APE plugin

/vis/vis_avs/ns-eel - Nullsoft Expression Evaluator Library (NS-EEL)
                      This is what powers AVS's expression evaluators, feel free
                      to include/modify the code in your own AVS APE plugins

/lang_b - Sample code for a Language Pack plugin (always but in release mode)

/maki - Compiler for building Maki binaries