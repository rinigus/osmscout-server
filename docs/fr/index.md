# Serveur OSM Scout

Le serveur OSM Scout peut être utilisé pour remplacer en lieu et place les
services web de fournitures de cartes, de recherche, et de navigation. Ainsi,
il est possible, si le téléphone dispose du serveur et d'une application de
cartographie, de l'utiliser hors ligne. Le serveur OSM Scout est développé
principalement pour Sailfish OS, mais peut aussi être utilisé sur un système
Linux conventionnel.

Dans ce document, le manuel d'utilisation est fourni avec la description pour
mettre en oeuvre le serveur avec les logiciels de cartographie populaires de
Sailfish OS afin de fonctionner hors ligne. Pour les développeurs, voir la
[page GitHub](https://github.com/rinigus/osmscout-server) du projet et son
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).


## Mode d'opération

Contrairement à la plupart des solutions de navigation hors ligne fournis par
d'autres applications, le serveur est l'une des deux parties nécessaires à son
utilisation. Les utilisations doivent avoir le serveur ainsi qu'un logiciel client
accédant au serveur, lancé simultanément et communicant l'un avec l'autre.

Après une configuration initiale, les utilisateurs vont principalement avoir
le serveur lancé en arrière plan et accéder aux cartes et instructions de navigation
à travers le logiciel client. L'interface graphique du serveur n'est présente
que pour gérer les cartes présentes sur l'appareil. Sur Sailfish OS, le mode d'opération
normal requière que le serveur soit lancé en tant qu'application affichant une vignette
sur l'écran principal, et qu'un client soit aussi lancé quand nécessaire.


## Configurer le serveur

Plusieurs étapes sont nécessaires pour mettre en route le serveur.
Le guide suivant est destiné aux utilisateurs de SailfishOS.

### Modules

Le serveur utilise une approche modulaire avec deux modules nécessaires pour
son fonctionnement. Veuillez tout d'abord ouvrir le magasin d'application Jolla
(Jolla Store) ou OpenRepos et installer _OSM Scout Server Module: Route_ et
_OSM Scout Server Module: Fonts_. Après installation de ces modules, veuillez
redémarrer le serveur si celui ci le demande. Les modules sont utilisés automatiquement
et ne nécessitent pas d'être lancés par l'utilisateur.


### Stockage

Le serveur OSM Scout nécessite de stocker les cartes sur l'appareil. Les besoins
de stockage peuvent être conséquents. Pour stocker les cartes et les gérer, un
dossier séparé est nécessaire. Veuillez noter, qu'à travers l'interface de gestion
des cartes, le serveur OSM Scout peut supprimer, sur votre demande, des fichiers
de ce dossier. Ainsi, il est important de créer ce dossier et de l'assigner à OSM Scout
Manager. Voir le [tutoriel sur la création du dossier de stockage](storage.html) pour
un exemple de comment le faire à l'aide de
[FileCase](https://openrepos.net/content/cepiperez/filecase-0).


### Gestionnaire de cartes

Pour télécharger, mettre à jour ou supprimer des cartes, utilisez le gestionnaire
de cartes. Le choix initial des cartes et leur téléchargement est décrit dans
[Tutoriel de téléchargement avec le gestionnaire de cartes](manager.html).

Après que les cartes aient été téléchargées, vous êtes prêts à configurer le
logiciel client. Voir la section correspondante plus bas pour la façon de
le configurer.

### Sélection de la langue

Le serveur utilise une bibliothèque de traitement automatique du langage naturel
(NLP) qui permet le décodage d'adresses dans de nombreuses langues -
[libpostal](https://github.com/openvenues/libpostal). Pour limiter les ressources
consommées, veuillez spécifier les langues souhaitées comme indiqué dans le
[tutoriel de sélection de la langue] (languages.html).

### Profil

Pour simplifier la configuration, le serveur OSM Scout utilise des profiles.
Au premier démarrage, le serveur demande de choisir l'un des profils.
Plus tard, il est possible de changer de profil en suivant le
[tutoriel de sélection de profil](profiles.html).

### Paramètres

De nombreux paramètrtres sont disponibles pour régler finement le fonctionnement
du serveur. Voir quelques exemples dans [les exemples de paramètres](settings_misc.html).


## Configuration du client

Après que le serveur ait été paramétré et que les cartes aient été téléchargées,
l'accès au serveur doit être configuré du côté du(des) client(s).

Pour [Poor Maps](https://openrepos.net/content/otsaloma/poor-maps),
les instructions sont [ici](poor_maps.html).

Pour [modRana](https://openrepos.net/content/martink/modrana-0),
les instructions sont [là](modrana.html).

Après que le client ait été configuré, vous pouvez l'utiliser avec le serveur OSM Scout
pour afficher des cartes hors ligne.

