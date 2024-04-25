## Daemon - Mitigacion de ataque DDoS

Daemon desarrollado en C para el **Trabajo Practico N°1** de la materia Software Libre

### Integrantes
---
- Rojas, Agustin
- Priano Sobenko, Bautista

### Start
---
**Comandos Daemon**
```
./daemonEx -start

./daemonEx -stop

./daemonEx -pause

./daemonEx -continue
```

**Test con nmap**
- -p- : Escaneo de todos los puertos disponibles [1-65535]
- -sV : Deteccion de los servicios en funcionamiento en los puertos abiertos encontrados.
```
nmap -p- -sV {direccion-ip}
```

**Apertura de Socket**
Utilizado para abrir el puerto indicado y testear si es detectado por el nmap
```
python3 socketOpening.py {puerto}
```



