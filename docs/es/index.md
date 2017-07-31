
# OSM Scout Server

OSM Scout Server se puede utilizar en lugar de los servicios de
mapas en línea que proporcionan tiles de mapas, realiza búsquedas, y 
calcula rutas. Si el dispositivo tiene instalados un servidor
y un cliente de mapas y ambos se ejecutan al mismo tiempo es posible
obtener mapas sin conexión. OSM Scout Server se ha desarrollado
principalmente para el sistema operativo Sailfish aunque también se
puede usar en un sistema de Linux convencional.

Aquí, la guía de usuario proporciona una descripción de la configuración
del servidor y de los clientes populares para el funcionamiento sin
conexión en un dispositivo con Sailfish. Para obtener información del
desarrollador, consulta el proyecto en la
[página de GitHub](https://github.com/rinigus/osmscout-server) y el
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).

## Funcionamiento

A diferencia de las soluciones a la navegación sin conexión proporcionadas
por otras aplicaciones, el servidor es una de las dos partes imprescindibles
para el funcionamiento. Los usuarios deben tener el servidor y un cliente que
acceda al servidor ejecutándose _simultáneamente_ y comunicándose entre
sí.

Después de la configuración inicial, los usuarios tendrán principalmente
el servidor ejecutándose en segundo plano mientras acceden a los mapas
y obtienen las instrucciones de navegación a través del cliente. La
interfaz gráfica del servidor sólo es necesaria para gestionar los
mapas en el dispositivo. En el sistema operativo Sailfish, el modo normal
de funcionamiento requiere que el servidor esté ejecutándose como una
de las aplicaciones que muestran su cubierta en la pantalla de inicio
y que un cliente esté abierto si es necesario.

## Configuración del servidor

Para configurar el servidor son necesarios varios pasos. La siguiente
guía está dirigida a los usuarios del sistema operativo Sailfish.

### Módulos

El servidor usa el enfoque modular con dos módulos independientes que
son necesarios para su funcionamiento. Por favor, dirígete a la
tienda de Jolla o a OpenRepos e instala _OSM Scout Server Module: Route_
y _OSM Scout Server Module: Fonts_. Después de la instalación de los
módulos reinicia el servidor si así lo indica. Los módulos se
usan automáticamente y los usuarios no tienen que iniciarlos.

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
configura. 

### Selección del idioma

El servidor usa la librería del procesamiento de lenguaje natural (PLN)
que cubre el procesamiento de las direcciones postales en muchos idiomas -
[libpostal](https://github.com/openvenues/libpostal). Para limitar el
uso de recursos, por favor, especifica los idiomas tal y como se muestra
en el [Tutorial de selección del idioma](languages.html).

### Perfil

Para simplificar la configuración, OSM Scout Server usa perfiles. En el 
primer inicio de la aplicación se solicitará que selecciones un perfil.
Más tarde puedes seleccionar un perfil tal y como se muestra en el
[Tutorial de selección del perfil](profiles.html).

### Ajustes

Hay múltiples ajustes que pueden ser útiles para mejorar el funcionamiento
del servidor. Consulta algunos ejemplos en
[Ejemplos de ajustes](settings_misc.html).


## Configuración del cliente

Una vez que has configurado el servidor y has descargado los mapas, 
tienes que configurar el acceso al servidor en el cliente o clientes.

Para [Poor Maps](https://openrepos.net/content/otsaloma/poor-maps),
las instrucciones están [aquí](poor_maps.html).

Para [ModRana](https://openrepos.net/content/martink/modrana-0),
las instrucciones están [aquí](modrana.html).

Después de configurar el cliente, pues usarlo junto a OSM Scout Server
para el acceso a los mapas sin conexión.
