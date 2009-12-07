This is a Winamp plugin that reports the tracks you are listening to to the
Digital Point Forum. I wrote it in the summer of 2006 but never published.
Now I finally did. Enjoy! :)

It was written by Peteris Krumins (peter@catonmat.net).
His blog is at http://www.catonmat.net  --  good coders code, great reuse.

The code is licensed under the GNU GPL license.

The project is located on my website. I wrote a detailed description of why I
created this plugin and added some details on how I created it. The latest
version of the plugin is also always available on my website at this address:

    http://www.catonmat.net/projects/dpf-winamp-music-reporter/

------------------------------------------------------------------------------

The plugin itself is a DLL file. You can either download it from my website or
compile it yourself. See compile.txt for instructions on how to compile it
yourself (you'll need Visual Studio and LibCurl).

The latest version can be always downloaded here:

    http://www.catonmat.net/download/gen_digitalpoint.dll

Download this file and put it in C:\Program Files\Winamp\Plugins directory.

Now start (or restart) the Winamp and navigate to General Purpose plugins
preferences. Take these steps:

    1. Right click on Winamp.
    2. Select Options -> Preferences.
    3. Find Plug-ins section.
    4. Click on General Purpose label.

You will see all the installed general purpose plugins. Notice our shiny new
plugin "Winamp DPF Music Reporter v1.0". (See winamp-preferences.png image in
screenshots directory image if you can't find it.)

Now double click on "Winamp DPF Music Reporter v1.0". The configuration
dialog will apear. (See plugin-configuration.png image.)

!!!!!!!!!!!!!!!!!!!!!!!!!!

    If you don't see the plugin in the list, you may be missing some of the
    latest Windows DLLs that it depends on. Download the latest DLL pack for
    Windows here:
    http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2&displaylang=en

!!!!!!!!!!!!!!!!!!!!!!!!!!

You have to fill "DPF User Name," which is your nickname on DPF. And you also
have to fill in the "DPF Hash," which is your unique identifier on DPF. You
can find it at the "Edit Profile" page here:

    http://forums.digitalpoint.com/profile.php?do=editprofile

The hash is in the "Authentication Hash" box, and it looks like
"a3f2823fa9281abd9382cd09182fe198". Copy this hash string to "DPF Hash" field
in the plugin.

You may also change the format of the title. The default is "%A - %T," which
means "Artist - Title". See below for all possible format strings (they are
also listed in the plugin).

Finally click "Enable Digital Point Forums Winamp Music Reporter".

That's it, from now on everyone on the forums will know what you are listening
to! :) Each time a new song is played, the status will update.

------------------------------------------------------------------------------

Here are all the possible format strings:

    %A - Artist
    %T - Title
    %L - Album
    %Y - Year
    %B - Bitrate (kbps)
    %E - Length (m:ss)
    %P - Path
    %% - Character "%"

You may combine them any way you wish. For example:

  "%A - %T [%B kb/s, %L]" would be "Artist - Title [bitrate kb/s, length m:ss]

You can also put arbitrary test in it. For example:

  "coolest song: %T" would be "coolest song: Title"

For some Internet radio streams the info can't be extracted. In those cases
the plugin uses information from the playlist as you see it. (Usually it's
something like "Radio Station: Artist - Title (DJ's Name)".)

------------------------------------------------------------------------------

The plugin in opensourced, you may use any way you wish, just remember that
it's licensed under GNU GPL license -- any changes that you make to it must be
made freely available.

You can read more about what this music reporter is on Digital Point Forums
itself. Go to this address to read about it:

    http://forums.digitalpoint.com/faq.php?faq=dp_faq#faq_music_reporter

------------------------------------------------------------------------------

Have fun on DPF! Btw, my nickname on DPF is "pkrumins". Give me some rep. ;)


Sincerely,
Peteris Krumins
http://www.catonmat.net


