#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script generates Makefile that can be used to import countries
# into libosmscout and generate geocoder-nlp database

import pycountry, os, json

Name2Code = {
    "ireland-and-northern-ireland": "GB-IE",
    "north-america/us": "US",
    "haiti-and-domrep": "HT-DO",
    "senegal-and-gambia": "SN-GM",
    "gcc-states": "BH-KW-OM-QA-SA-AE",
    "israel-and-palestine": "IL-PS",
    "malaysia-singapore-brunei": "MY-SG-BN",

    "azores": "PT",
    "bosnia-herzegovina": "BA",
    "great-britain": "GB",
    "kosovo": "RS", # in libpostal dataset
    "lesotho": "ZA", # in libpostal dataset
    "macedonia": "MK",
    "russia": "RU",
    "canary-islands": "ES",
    "cape-verde": "CV",
    "comores": "KM",
    "congo-brazzaville": "CG",
    "congo-democratic-republic": "CD",
    "guinea-bissau": "GW",
    "ivory-coast": "CI",
    "saint-helena-ascension-and-tristan-da-cunha": "SH",
    "iran": "IR",
    "north-korea": "KP",
    "south-korea": "KR",
    "syria": "SY",
    "vietnam": "VN",

    "europe/germany": "DE",
    "europe/france": "FR",
    "europe/great-britain": "GB",
    "europe/italy": "IT",
}

Name2Pretty = {
    "canary-islands": "Canary Islands",
    "ireland-and-northern-ireland": "Ireland and Northern Ireland",
    "kosovo": "Kosovo",
    "lesotho": "Lesotho",
    "australia-oceania": "Australia and Oceania",
    "north-america/us": "North America/US",
    "haiti-and-domrep": "Haiti and Dominican Republic",
    "senegal-and-gambia": "Senegal and Gambia",
    "gcc-states": "GCC States",
    "israel-and-palestine": "Israel and Palestine",
    "malaysia-singapore-brunei": "Malaysia, Singapore, and Brunei",    

    "us-midwest": "US/Region/Midwest",
    "us-northeast": "US/Region/Northeast",
    "us-pacific": "US/Region/Pacific",
    "us-south": "US/Region/South",
    "us-west": "US/Region/West",

    "europe/france": "Europe/France",
    "basse-normandie": "Basse-Normandie",
    "provence-alpes-cote-d-azur": "Provence Alpes-Cote-d'Azur",

    "europe/germany": "Europe/Germany",
    "baden-wuerttemberg": "Baden-Württemberg",
    "mecklenburg-vorpommern": "Mecklenburg-Vorpommern",
    "nordrhein-westfalen": "Nordrhein-Westfalen",
    "rheinland-pfalz": "Rheinland-Pfalz",
    "sachsen-anhalt": "Sachsen-Anhalt",
    "schleswig-holstein": "Schleswig-Holstein",
    "thueringen": "Thüringen",

    "europe/great-britain": "Europe/United Kingdom",
    "europe/italy": "Europe/Italy",
    
    "russia": "Europe/Russian Federation",
}

NameCapitalize = [
    "azores",
    "north-america/us",

    "alsace", "aquitaine", "auvergne", "bourgogne", "bretagne",
    "centre", "champagne-ardenne", "corse", "franche-comte", "guadeloupe",
    "guyane", "haute-normandie", "ile-de-france", "languedoc-roussillon",
    "limousin", "lorraine", "martinique", "mayotte", "midi-pyrenees",
    "nord-pas-de-calais", "pays-de-la-loire", "picardie", "poitou-charentes",
    "reunion", "rhone-alpes", 
    
    "bayern", "berlin", "brandenburg", "bremen",
    "hamburg", "hessen",
    "niedersachsen", "saarland", "sachsen",

    "england", "scotland", "wales",

    "centro", "isole", "nord-est", "nord-ovest", "sud",

    "central-fed-district",
    "far-eastern-fed-district",
    "north-caucasus-fed-district",
    "northwestern-fed-district",
    "siberian-fed-district",
    "south-fed-district", "ural-fed-district",
    "volga-fed-district",
]

SpecialURL = {
    "russia": "http://download.geofabrik.de/russia-latest.osm.pbf"
}

Countries = {
    "europe": [ "albania",
                "andorra",
                "austria",
                "azores",
                "belarus",
                "belgium",
                "bosnia-herzegovina",
                "bulgaria",
                "croatia",
                "cyprus",
                "czech-republic",
                "denmark",
                "estonia",
                "faroe-islands",
                "finland",
                "france",
                "georgia",
                "germany",
                "great-britain",
                "greece",
                "hungary",
                "iceland",
                "ireland-and-northern-ireland",
                "isle-of-man",
                "italy",
                "kosovo", 
                "latvia",
                "liechtenstein",
                "lithuania",
                "luxembourg",
                "macedonia",
                "malta",
                "moldova",
                "monaco",
                "montenegro",
                "netherlands",
                "norway",
                "poland",
                "portugal",
                "romania",
                "russia",
                "serbia",
                "slovakia",
                "slovenia",
                "spain",
                "sweden",
                "switzerland",
                "turkey",
                "ukraine"
    ],

    "europe/france": [
        "alsace", "aquitaine", "auvergne",
        "basse-normandie", "bourgogne", "bretagne", "centre", "champagne-ardenne", 
        "corse", "franche-comte", "guadeloupe", "guyane", "haute-normandie",
        "ile-de-france", "languedoc-roussillon", "limousin", "lorraine",
        "martinique", "mayotte", "midi-pyrenees", "nord-pas-de-calais",
        "pays-de-la-loire", "picardie", "poitou-charentes",
        "provence-alpes-cote-d-azur", "reunion", "rhone-alpes", 
    ],

    "europe/germany": [
        "baden-wuerttemberg",
        "bayern",
        "berlin",
        "brandenburg",
        "bremen",
        "hamburg",
        "hessen",
        "mecklenburg-vorpommern",
        "niedersachsen",
        "nordrhein-westfalen",
        "rheinland-pfalz",
        "saarland",
        "sachsen",
        "sachsen-anhalt",
        "schleswig-holstein",
        "thueringen",
    ],

    "europe/great-britain": [
        "england", "scotland", "wales"
        ],

    "europe/italy": [
        "centro", "isole", "nord-est", "nord-ovest", "sud",
        ],
    
    "russia": [
        "central-fed-district",
        "far-eastern-fed-district",
        "north-caucasus-fed-district",
        "northwestern-fed-district",
        "siberian-fed-district",
        "south-fed-district", "ural-fed-district",
        "volga-fed-district",
        ],
    
    "africa": [ "algeria",
                "angola",
                "benin",
                "botswana",
                "burkina-faso",
                "cameroon",
                "canary-islands",
                "cape-verde",
                "central-african-republic",
                "chad",
                "comores",
                "congo-brazzaville",
                "congo-democratic-republic",
                "djibouti",
                "egypt",
                "equatorial-guinea",
                "eritrea",
                "ethiopia",
                "gabon",
                "ghana",
                "guinea",
                "guinea-bissau",
                "ivory-coast",
                "kenya",
                "lesotho",
                "liberia",
                "libya",
                "madagascar",
                "malawi",
                "mali",
                "mauritania",
                "mauritius",
                "morocco",
                "mozambique",
                "namibia",
                "niger",
                "nigeria",
                "rwanda",
                "saint-helena-ascension-and-tristan-da-cunha",
                "sao-tome-and-principe",
                "senegal-and-gambia",
                "seychelles",
                "sierra-leone",
                "somalia",
                "south-africa",
                "south-sudan",
                "sudan",
                "swaziland",
                "tanzania",
                "togo",
                "tunisia",
                "uganda",
                "zambia",
                "zimbabwe" ],

    "asia": [ "afghanistan",
              "azerbaijan",
              "bangladesh",
              "cambodia",
              "gcc-states",
              "china",
              "india",
              "indonesia",
              "japan",
              "iran",
              "iraq",
              "israel-and-palestine",
              "jordan",
              "kazakhstan",
              "kyrgyzstan",
              "lebanon",
              "malaysia-singapore-brunei",
              "maldives",
              "mongolia",
              "nepal",
              "north-korea",
              "pakistan",
              "philippines",
              "south-korea",
              "sri-lanka",
              "syria",
              "taiwan",
              "tajikistan",
              "thailand",
              "turkmenistan",
              "uzbekistan",
              "vietnam",
              "yemen" ], 

    "australia-oceania": [ "australia",
                           "fiji",
                           "new-caledonia",
                           "new-zealand" ],

    "central-america": [ "belize",
                         "cuba",
                         "guatemala",
                         "haiti-and-domrep",
                         "nicaragua" ],

    "north-america": [ "canada",
                       "greenland",
                       "mexico",
                       "us-midwest",
                       "us-northeast",
                       "us-pacific",
                       "us-south",
                       "us-west" ],

    "north-america/us": [ "alaska",
                          "alabama",
                          "arizona",
                          "arkansas",
                          "california",
                          "colorado",
                          "connecticut",
                          "delaware",
                          "district-of-columbia",
                          "florida",
                          "georgia",
                          "hawaii",
                          "idaho",
                          "illinois",
                          "indiana",
                          "iowa",
                          "kansas",
                          "kentucky",
                          "louisiana",
                          "maine",
                          "maryland",
                          "massachusetts",
                          "michigan",
                          "minnesota",
                          "mississippi",
                          "missouri",
                          "montana",
                          "nebraska",
                          "nevada",
                          "new-hampshire",
                          "new-jersey",
                          "new-mexico",
                          "new-york",
                          "north-carolina",
                          "north-dakota",
                          "ohio",
                          "oklahoma",
                          "oregon",
                          "pennsylvania",
                          "rhode-island",
                          "south-carolina",
                          "south-dakota",
                          "tennessee",
                          "texas",
                          "utah",
                          "vermont",
                          "virginia",
                          "washington",
                          "west-virginia",
                          "wisconsin",
                          "wyoming" ],

    "south-america": [ "argentina",
                       "bolivia",
                       "brazil",
                       "chile",
                       "colombia",
                       "ecuador",
                       "paraguay",
                       "peru",
                       "suriname",
                       "uruguay", 
    ],
}

# Countries = {
#     "europe": [ "albania",
#                 "andorra",
#                 "estonia",
#                 "spain",
#     ],
# }

fmake = open("Makefile", "w")
fmake.write("# This Makefile is generated by script\n\n")
fmake.write("BUILDER=./build.sh\n")
fmake.write("BASE_DIR=distribution\n")
fmake.write("DOWNLOADS_DIR=downloads\n")
fmake.write("\nall: $(DOWNLOADS_DIR)/.directory $(BASE_DIR)/all_countries_done\n\techo All Done\n\n")
fmake.write("$(DOWNLOADS_DIR)/.directory:\n\tmkdir -p $(DOWNLOADS_DIR)\n\ttouch $(DOWNLOADS_DIR)/.directory\n\n")

all_countries = ""
all_downloads = ""

def pbfname(continent, country):
    cc = continent.replace("/", "-")
    return cc + "-" + country + ".pbf"

def pbfurl(continent, country):
    if country in SpecialURL: return SpecialURL[country]
    return "http://download.geofabrik.de/%s/%s-latest.osm.pbf" % (continent, country)

def namecode(continent, country):
    pretty_name = None
    country_spaces = country.replace('-', ' ')

    if continent in Name2Code: code = Name2Code[continent]
    elif country in Name2Code: code = Name2Code[country]
    elif country.find("us-")==0: code = "US"
    else:
        c = pycountry.countries.lookup(country_spaces)
        code = c.alpha_2
        pretty_name = c.name
    
    if continent in Name2Pretty: pretty_continent = Name2Pretty[continent]
    else:
        pretty_continent = ""
        for c in continent.split('-'):
            pretty_continent += c.capitalize() + " "
        pretty_continent = pretty_continent.strip()
        
    if pretty_name is None:
        if country!="russia" and country in Name2Pretty: pretty_name = Name2Pretty[country]
        elif country in NameCapitalize or continent in NameCapitalize or country.find("us-")==0:
            pretty_name = ""
            for c in country_spaces.split():
                pretty_name += c.capitalize() + " "
            pretty_name = pretty_name.strip()
        else:
            c = pycountry.countries.lookup(code)
            pretty_name = c.name
    return code, pretty_continent, pretty_name

def spath(name):
    return name.replace('/','-')

########### Main loop #############
provided_countries = {}

fmake.write("$(BASE_DIR)/geocoder-nlp/.directory:\n\tmkdir -p $(BASE_DIR)/geocoder-nlp/\n\ttouch $(BASE_DIR)/geocoder-nlp/.directory\n\n")

for continent in Countries.keys():
    for country in Countries[continent]:

        code2, pretty_continent, pretty_country = namecode(continent, country)

        provided_countries[ continent + "/" + country ] = { "id": continent + "/" + country,
                                                            "type": "territory",
                                                            "name": pretty_continent + "/" + pretty_country,
                                                            "postal_country": { "path": "postal/countries/" + code2 },
                                                            "osmscout": { "path": "osmscout/" + spath(continent + "/" + country) },
                                                            "geocoder_nlp": { "path": "geocoder-nlp/" + spath(os.path.join(continent, country)) } }
        
        print continent, country, code2, pretty_continent, pretty_country #, (code2.lower() in postal_countries)

        country_target = "$(BASE_DIR)/geocoder-nlp/" + spath(os.path.join(continent, country)) + ".timestamp"
        pbf = "$(DOWNLOADS_DIR)/" + pbfname(continent, country)
        all_countries += country_target + " "
        all_downloads += pbf + " "
        fmake.write(country_target + ": $(BASE_DIR)/geocoder-nlp/.directory " + pbf +
                    "\n\t$(BUILDER) $(DOWNLOADS_DIR)/" + pbfname(continent, country) + " $(BASE_DIR) " +
                    spath(os.path.join(continent, country)) + " " + code2 + "\n\n")
        fmake.write(pbf + ":$(DOWNLOADS_DIR)/.directory\n\twget %s -O$(DOWNLOADS_DIR)/%s || (rm -f $(DOWNLOADS_DIR)/%s && exit 1)\n\ttouch $(DOWNLOADS_DIR)/%s\n" %
                    (pbfurl(continent, country), pbfname(continent, country),
                     pbfname(continent, country), pbfname(continent, country)) )

fmake.write("\n$(BASE_DIR)/all_countries_done: " + all_countries + "\n\techo > $(BASE_DIR)/all_countries_done\n\n")
fmake.write("download: " + all_downloads + "\n\techo All downloaded\n\n")

# save provided countries
fjson = open("countries.json", "w")
fjson.write( json.dumps( provided_countries, sort_keys=True, indent=4, separators=(',', ': ')) )

print "\nExamine generated Makefile and run make using it. See build.sh and adjust the used executables first\n"
