Thank you for looking into translating OSM Scout Server. In Sailfish,
only harbour-osmscout-server.ts version is used and it should contain
all required strings.

To simplify translations, there is a project registered at
https://www.transifex.com/rinigus/osm-scout-server . Please use it for
working on translations of the strings within application.

Note that Valhalla routing instructions are translated as a part of
Valhalla's project. Please see
https://github.com/valhalla/valhalla/tree/master/locales for current
translations and how to contribute.

User's Guide is located in `docs` subfolder of the project. For new
translations, fork the repository in github, add new language
subfolder by making a copy of `docs/en`, and translate HTML files into
your language. In addition, you would have to make screenshots on
Sailfish OS for the tutorials using translated application(s). These
screenshots would have to be named as they are in English version to
keep the same structure of the tutorial. Notice that tutorials
information text covers part of the screenshots. You would either have
to make screenshots which are very close to the English version or
start changing location of the information text to accommodate your
screenshots.

Some of the strings would contain macros in the form %macro%. These
macros mark the positions in the phrase that would be filled in with
corresponding data. For example, in string

Leave roundabout (%num% exit) into street %street%

keep %num% and %street% untranslated. These macros should be placed in
the sentence where they are expected to be replaced by the exit number
(%num%) and the street name %street%. 

For road types, as requested in routing speed table, you may find it
useful to see http://wiki.openstreetmap.org/wiki/Map_Features#Highway
(link found by @eson57). Similar content in other languages:

http://wiki.openstreetmap.org/wiki/FR:France_roads_tagging

