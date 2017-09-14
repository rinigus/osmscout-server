
# OSM Scout Server

OSM Scout Server kan användas som ersättning för online-karttjänster och
tillhandahåller kartor, söktjänst och ruttberäkning. OSM Scout Server
möjliggör därmed offline-användnig  om enheten har både server och klient
installerad. OSM Scout Server är huvudsakligen utvecklad för Sailfish OS,
men kan också användas i andra reguljära Linux-system.

Det här är en användarhandledning med beskrivning av serverinställning och
populära klienter för offline-drift.
För utvecklingsinformation, se projektets
[GitHub-sida](https://github.com/rinigus/osmscout-server)
och
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).


## Driftläge

Till skillnad från offline-navigeringslösningar som tillhandahålls av
andra applikationer, är servern en av två delar som krävs för användning.
Användaren måste ha servern och en klient som körs _samtidigt_ och kan
kommunicera med varandra.

Efter installation, kommer servern främst att köras i bakgrunden medan
åtkomst till kartor och navigeringsinstruktioner fås via klienten.
Serverns GUI behövs endast för att hantera kartor på enheten.

I Sailfish OS skulle det normala driftläget kräva att servern körs som
minimerat program och klienten öppnas vid behov. Alternativt kan servern
aktiveras automatiskt av _systemd_ vid anrop från klienten. Ett sådant 
driftläge låter klienten komma åt servern utan att exponera serverns GUI.


## Konfigurera servern

Det krävs flera steg för att konfigurera servern. Följande handledning
riktar sig till Sailfish OS-användare.

### Moduler

Servern använder ett modulsystem med två separata moduler som krävs
för drift. Fortsätt till Jolla Store eller OpenRepos och installera
_OSM Scout Server modul: Route_ och _OSM Scout Server modul: Fonts_.
Starta om servern vid anmodan, efter installation av moduler.
Modulerna används automatiskt och behöver inte startas av användaren.

### Lagring

OSM Scout Server behöver lagra kartor, och lagringskraven kan vara
betydande. För att lagra kartor och hantera dem krävs en separat mapp.
Observera att OSM Scout Server, som en del av hanteringen, kan ta
bort filer från den mappen på ditt kommando. Således är det viktigt
att allokera en sådan mapp och tilldela den till OSM Scout Manager.
Se [Lagringshandledningen](storage.html) som exempel på hur
man gör det med
[FileCase](https://openrepos.net/content/cepiperez/filecase-0).

### Karthanterare

Använd karthanterarenför att ladda ner, uppdatera och ta bort kartor.
Prenumeration på kartor och deras nerladdning, beskrivs i
[Karthanteringsguiden](manager.html). 

Efter att kartorna laddats ner, är du klar att fortsätta med
konfiguration av kartklienten. Välj motsvarande avdelning nedan,
för hjälp med konfigurationen.

### Språkval

Servern använder "natural language processing (NLP)", som täcker
behandlingen av adresser på ett stort antal språk -
[libpostal](https://github.com/openvenues/libpostal). För att begränsa resursanvändningen bör du specificera språken, som visat i
[Språkvalsguiden](languages.html).

### Profil

OSM Scout Server använder profiler, för att förenkla konfigurationen.
Du ombeds att välja profil vid första starten och du kan ändra
profil senare, som visat i
[Profilvalsguiden](profiles.html).

### Inställningar

Det finns flera inställningar som kan vara användbara för att ställa
in serverfunktioner. Bland andra inställningar inkluderar detta
språkinställningar, enhetsinställningar och om servern aktiveras
automatiskt. Se några exmpel i
[Inställningsexempel](settings_misc.html).


## Konfigurera klienten

Efter att servern har konfigurerats och kartor laddats ner, måste
åtkomst till servern konfigureras i klenten/klienterna.

För [Poor Maps](https://openrepos.net/content/otsaloma/poor-maps),
finns instruktioner [här](poor_maps.html).

För [modRana](https://openrepos.net/content/martink/modrana-0),
finns instruktioner [här](modrana.html).

Efter klientkonfiguration, kan du använda den/dem tillsammans med
OSM Scout Server för åtkomst till offline-kartor.
