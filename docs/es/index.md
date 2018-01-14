
# OSM Scout Server

OSM Scout Server es una alternativa a los servicios de mapas 
con conexión de datos que proporcionan mapas, realiza búsquedas, y 
calcula rutas. Si el dispositivo tiene instalados un servidor
y un cliente de mapas y ambos se ejecutan al mismo tiempo es posible
obtener mapas sin conexión. OSM Scout Server se ha desarrollado
principalmente para el sistema operativo Sailfish aunque también se
puede usar en un sistema Linux convencional.

Aquí, la guía de usuario proporciona una descripción de la configuración
del servidor y de los clientes populares para el funcionamiento sin
conexión en un dispositivo con Sailfish. Para obtener información del
desarrollador, consulta el proyecto en la
[página de GitHub](https://github.com/rinigus/osmscout-server) y el
[README](https://github.com/rinigus/osmscout-server/blob/master/README.md).

## Funcionamiento

A diferencia de las soluciones a la navegación sin conexión proporcionadas
por otras aplicaciones, el servidor es una de las dos partes imprescindibles
para su funcionamiento. Los usuarios deben tener el servidor y un cliente que
acceda al servidor ejecutándose _simultáneamente_ y comunicándose entre
sí.

Después de la configuración inicial, los usuarios en primer lugar deben
ejecutar el servidor y tenerlo abierto en segundo plano para poder acceder
a los mapas y obtener las instrucciones de navegación a través del cliente.
La interfaz gráfica del servidor sólo es necesaria para gestionar los
mapas en el dispositivo. En Sailfish, el funcionamiento normal requiere
tener el servidor ejecutándose en segundo plano, mostrándose su cubierta
en la pantalla de inicio, y el cliente abierto cuando se necesite.

## Configuración del servidor

Para configurar el servidor son necesarios varios pasos. La siguiente
guía está dirigida a los usuarios del sistema operativo Sailfish.

### Módulos

El servidor usa el enfoque modular con dos módulos independientes que
son necesarios para su funcionamiento. Por favor, dirígete a la
tienda de Jolla o a OpenRepos e instala _OSM Scout Server Module: Route_
y _OSM Scout Server Module: Fonts_. Después de la instalación de los
módulos reinicia el servidor si así se indica. Los módulos se
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
hace. 

### Selección del idioma

El servidor usa la librería del procesamiento de lenguaje natural (PLN)
que permite el análisis de las direcciones postales en muchos idiomas -
[libpostal](https://github.com/openvenues/libpostal). Para limitar el
uso de recursos, por favor, especifica los idiomas tal y como se muestra
en el [Tutorial de selección del idioma](languages.html).

### Perfil

Para simplificar la configuración, OSM Scout Server usa perfiles. En el 
primer inicio de la aplicación se solicitará que selecciones un perfil.
Más tarde puedes cambiar el perfil siguiendo las instrucciones del
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

Para [modRana](https://openrepos.net/content/martink/modrana-0),
las instrucciones están [aquí](modrana.html).

Después de configurar el cliente, pues usarlo junto a OSM Scout Server
para el acceso a los mapas sin conexión.


## Ejecución

Como se describe más arriba, cuando se usa OSM Scout Server necesitas ejecutar el servidor y el cliente al mismo tiempo. Hay dos formas de hacerlo:

### Automática

Si habilitas la activación automática sólo tienes que iniciar el cliente. El cliente accederá al servidor y éste, o bien se ejecuta como aplicación GUI en segundo plano, o bien, si no se ha iniciado, se ejecuta como un servicio.

### Manual

* Inicia OSM Scout Server y minimízalo, mostrándose su cubierta en la página de inicio
* Inicia el cliente (Poor Maps, modRana, o cualquier otro)
* Cuando termines, cierra el servidor y el cliente.


## Implementación de la activación automática

Al habilitar la activación automática, OSM Scout Server interactúa con _systemd_ creando los archivos _service_ y _socket_ en el directorio home del usuario que ejecuta el servidor. Además, la activación del socket se efectúa ejecutando `systemctl`. En Sailfish eso da como resultado la creación o modificación de 

```
/home/nemo/.config/systemd/user/osmscout-server.service
/home/nemo/.config/systemd/user/osmscout-server.socket
/home/nemo/.config/systemd/user/user-session.target.wants
```

Si quieres eliminar la activación automática de forma manual, ejecuta 

```
systemctl disable osmscout-server.socket
```

y después elimina los archivos _service_ y _socket_. En Sailfish, elimina
```
/home/nemo/.config/systemd/user/osmscout-server.service
/home/nemo/.config/systemd/user/osmscout-server.socket
```
