
# OSM Scout Server

OSM Scout Server es una alternativa a los servicios de mapas con
conexión de datos que proporciona mapas, realiza búsquedas, y calcula
rutas. Como resultado, es posible un procedimiento que no requiere una
conexión de datos si el dispositivo tiene instalados un servidor y un
cliente de mapas y ambos están ejecutándose al mismo tiempo. OSM Scout
Server está desarrollado principalmente para el sistema operativo
Sailfish aunque también podría usarse en un sistema Linux convencional
como consola o aplicación QtQuick.

Esta guía de usuario proporciona una descripción de la configuración
del servidor y de los clientes populares para el funcionamiento sin
conexión en un dispositivo con Sailfish. Para obtener información
del desarrollador, consulta el proyecto en la
[página de GitHub](https://github.com/rinigus/osmscout-server) y el 
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).


## Funcionamiento

A diferencia de las soluciones a la navegación sin conexión
que proporcionan otras aplicaciones, el servidor es una de las dos
partes imprescindibles para el funcionamiento. Los usuarios deben
tener el servidor y un cliente que acceda al servidor ejecutándose
_simultáneamente_ y comunicándose entre sí.

Después de la configuración inicial, los usuarios deben ejecutar en
primer lugar el servidor y tenerlo abierto en segundo plano para
acceder a los mapas y obtener las instrucciones de navegación a
través del cliente. La interfaz gráfica del servidor sólo es necesaria
para gestionar los mapas en el dispositivo.

En Sailfish, el funcionamiento normal requiere tener el servidor
ejecutándose en segundo plano, mostrándose su cubierta en la pantalla
de inicio, y el cliente abierto cuando lo necesites. Alternativamente,
el servidor puede activarse automáticamente con _systemd_ a petición
del cliente. Este modo de funcionamiento permite al cliente acceder al
servidor sin que se muestre la interfaz gráfica del servidor.


## Configuración del servidor

Para configurar el servidor son necesarios varios pasos. La siguiente
guía está dirigida a los usuarios del sistema operativo Sailfish.

### Módulos

El servidor usa el enfoque modular. Aunque en la versión actual del
servidor y con la configuración por defecto no es necesario ningún
módulo, quizás necesites alguno de los módulos con el usuario que
selecciones en la configuración. Los módulos, si son necesarios, se
comprueban en cada inicio del servidor. Cuando sea necesario, el
servidor solicitará la instalación del módulo. En ese caso, dirígete a
la tienda de Jolla o a OpenRepos e instala el correspondiente módulo.
Después de la instalación de los módulos reinicia el servidor si así lo
indica. Los módulos se usan automáticamente y los usuarios no tienen
que iniciarlos.

Ten en cuenta que el módulo _OSM Scout Server Module: Route_ que se
usaba antes está incorporado al servidor principal ejecutable y puede
desinstalarse cuando uses la versión 1.8 o superior de OSM Scout Server.

### Almacenamiento

OSM Scout Server necesita almacenar los mapas. Los requisitos de
almacenamiento pueden ser significativos. Para almacenar los mapas y
gestionarlos es necesaria una carpeta independiente. Por favor,
ten en cuenta que, como parte de la gestión, OSM Scout Server puede
borrar, a petición tuya, ficheros de esa carpeta. Por lo tanto,
es importante que crees esa carpeta y después la asignes como
almacenamiento en OSM Scout Server.
Consulta el [Tutorial de asignación de almacenamiento](storage.html)
con un ejemplo de cómo hacerlo con
[FileCase](https://openrepos.net/content/cepiperez/filecase-0).

### Gestor de mapas

Para descargar, actualizar y eliminar mapas, usa el gestor de mapas.
La suscripción de los mapas y su descarga se describe en el
[Tutorial del gestor de mapas](manager.html). 

Una vez que has descargado los mapas, estás preparado para configurar
el cliente. Ve a la sección correspondiente más abajo para ver cómo se
hace. 

### Selección del idioma

El servidor usa la librería del procesamiento de lenguaje natural (PLN)
que permite el análisis de las direcciones postales en muchos idiomas -
[libpostal](https://github.com/openvenues/libpostal). Para limitar el
uso de recursos, por favor, especifica los idiomas tal y como se muestra
en el [Tutorial de selección del idioma](languages.html).

### Perfil

Para simplificar la configuración, OSM Scout Server usa perfiles. En el 
primer inicio, la aplicación solicitará que selecciones un perfil. Más
tarde puedes cambiar el perfil siguiendo las instrucciones del
[Tutorial de selección del perfil](profiles.html).

Los clientes que usan teselas MapboxGL (a veces denominadas teselas
vectoriales) son compatibles con la configuración por defecto de OSM
Scout Server. En el momento de escribir esta guía son [WhoGo
Maps](https://openrepos.net/content/otsaloma/whogo-maps) y la
aplicación de deporte [Laufhelden](https://openrepos.net/content/jdrescher/laufhelden).

Para a quellos clientes que requieren teselas renderizadas del
servidor, como [Poor
Maps](https://openrepos.net/content/otsaloma/poor-maps) y
[modRana](https://openrepos.net/content/martink/modrana-0), es necesario
seleccionar el perfil de OSM Scout Server que incluye _teselas de mapas
de bit_. Por favor, configura el perfil si quieres usar estos clientes.


### Ajustes

Hay múltiples ajustes que pueden ser útiles para mejorar el
funcionamiento del servidor. Entre otros ajustes está la preferencia
del idioma, unidades, y si el servidor se activa automáticamente.
Consulta algunos ejemplos en
[Ejemplos de ajustes](settings_misc.html).


## Configuración del cliente

Una vez que has configurado el servidor y has descargado los mapas, 
tienes que configurar el acceso al servidor en el cliente o clientes.

Para [Pure Maps](https://openrepos.net/content/rinigus/pure-maps) y
[WhoGo Maps](https://openrepos.net/content/otsaloma/whogo-maps) sigue
las instrucciones más abajo de Poor Maps. Sólo sustituye la selección
de 'Mapas de referencia' por la selección de 'Mapas' en el menú de
WhoGo Maps.

Para [Poor Maps](https://openrepos.net/content/otsaloma/poor-maps)
sigue las instrucciones están [aquí](poor_maps.html).

Para [modRana](https://openrepos.net/content/martink/modrana-0),
las instrucciones están [aquí](modrana.html).

Después de configurar el cliente, pues usarlo junto a OSM Scout Server
para el acceso a los mapas sin conexión.


## Ejecución

Como se describe más arriba, cuando uses OSM Scout Server necesitas ejecutar el servidor y el cliente al mismo tiempo. Hay dos formas de hacerlo:

### Automática

Si habilitas la activación automática sólo tienes que iniciar el cliente. El cliente accederá al servidor y éste, o bien se ejecuta como aplicación GUI en segundo plano, o bien, si no se ha iniciado, se ejecuta como un servicio.

### Manual

* Inicia OSM Scout Server y minimízalo, mostrándose su cubierta en la página de inicio
* Inicia el cliente (Poor Maps, modRana, o cualquier otro)
* Cuando termines, cierra el servidor y el cliente.


## Geocodificador 

### Analizadores

Los analizadores son responsables de dividir la cadena de búsqueda
introducida en una dirección. El geocodificador funciona con libpostal
para analizar la cadena de búsqueda introducida y es de esperar que 
analice la dirección en su forma natural según la combinación de
idioma/país seleccionada. Además del análisis automático con libpostal,
es posible usar un analizador "primitivo" que coge la cadena de
búsqueda, la separa por comas y construye la jerarquía asumiendo que la
cadena de búsqueda se introdujo desde el detalle más fino de la
dirección o del POI a la región. Por ejemplo,

```
número_casa, calle, ciudad
```

Al igual que libpostal, el analizador primitivo soporta códigos
postales. Para ello, introduce `post:` seguido del código postal en
cualquier parte de la jerarquía o simplemente sólo para buscar por
código postal. Por ejemplo,

```
número_casa, calle, ciudad, post: 12345
```

o `post:12345`. Ten encuenta que los espacios entre la palabra clave
`post:` y el código postal se ignoran.

### Etiquetas y alias

Para distinguir los tipos de objetos, el geocodificador usa etiquetas
que se importan desde OpenStreetMap. Las etiquetas también están
asociadas con alias de manera dependiente al idioma. Las etiquetas y sus alias están enumeradas en [tags](../tags).

Los alias y etiquetas se importan desde el listado de frases especiales
mantenido por Nominatim en
[https://wiki.openstreetmap.org/wiki/Nominatim/Special_Phrases](https://wiki.openstreetmap.org/wiki/Nominatim/Special_Phrases). Si detectas que falta algo o quieres corregirlo, por favor, corrígelo en la fuente.


## Implementación de la activación automática

Al habilitar la activación automática, OSM Scout Server interactúa con _systemd_ creando los archivos _service_ y _socket_ en el directorio home del usuario que ejecuta el servidor. Además, la activación del socket se efectúa ejecutando `systemctl`. En Sailfish eso da como resultado la creación o modificación de 

```
/home/nemo/.config/systemd/user/osmscout-server.service
/home/nemo/.config/systemd/user/osmscout-server.socket
/home/nemo/.config/systemd/user/user-session.target.wants
```

Si quieres eliminar la activación automática de forma manual, ejecuta 

```
systemctl --user disable osmscout-server.socket
```

y después elimina los archivos _service_ y _socket_. En Sailfish, elimina
```
/home/nemo/.config/systemd/user/osmscout-server.service
/home/nemo/.config/systemd/user/osmscout-server.socket
```
