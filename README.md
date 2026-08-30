# P1_ADA_Angarita_Castano_Mira

## Integrantes

- Emmanuel Castano
- Luis Miguel Angarita
- Mira

Los apellidos normalizados se ordenan alfabéticamente y se concatenan como
`angaritacastanomira`. La semilla reproducible es `2009`.

## Estructura

- `src/`: codigo fuente del modulo FB y del modulo BT.
- `tests/`: utilidades y pruebas del proyecto.
- `resources/diccionario.txt`: diccionario sintetico con 500 candidatos.
- `results/`: espacio para guardar resultados experimentales.

## Compilacion

Desde la raiz del proyecto, usando C++17:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Isrc src/main.cpp src/fb_password.cpp -o tests/build/Debug/fb_app.exe
```

El modulo FB se compila de forma independiente para conservar intacto el
archivo existente de Backtracking.

### Modulo BT (Backtracking)

El modulo BT tiene su propia funcion `main` y se compila de forma
independiente. Desde la raiz del proyecto, usando C++17:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Isrc src/BT_Angarita.cpp -o tests/build/Debug/bt_app.exe
```

Para generar las contrasenas debe ejecutarse desde la raiz del proyecto,
ya que escribe la salida en `results/PasswordsBT.txt`:

```powershell
.\tests\build\Debug\bt_app.exe
```

El programa recorre los cinco casos de prueba (variando `n` y la politica de
minimos), imprime el numero de cadenas escritas y el tiempo por caso, y deja el
resultado completo en `results/PasswordsBT.txt`.

## Ejecucion

Validacion comun del curso:

```powershell
.\tests\build\Debug\fb_app.exe --validate
```

Generacion y ejecucion automatica de las cinco instancias privadas de la
Seccion 9.1:

```powershell
.\tests\build\Debug\fb_app.exe --generate-instances
```

Busqueda por fuerza bruta:

```powershell
.\tests\build\Debug\fb_app.exe --brute HASH A1 MIN MAX
```

Ataque secuencial por diccionario:

```powershell
.\tests\build\Debug\fb_app.exe --dictionary HASH resources/diccionario.txt
```

Cada ejecucion muestra estado, texto plano cuando se encuentra, candidatos
evaluados, tiempo en milisegundos y pertenencia al diccionario cuando aplica.

## Reproduccion de instancias

El comando `--generate-instances` usa los alfabetos A1 y A2, las longitudes
`4, 4, 5, 5, 6` y el generador:

```text
x0 = semilla
x(i+1) = (1103515245 * xi + 12345) mod 2^31
```

Las contrasenas se generan alternando A1, A2, A1, A2 y A1. Los hashes se
calculan con SHA-256 y se imprimen junto con cada instancia.
