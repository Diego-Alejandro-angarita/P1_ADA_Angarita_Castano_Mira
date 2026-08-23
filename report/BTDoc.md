# Documentación — Generación de contraseñas por *Backtracking*

**Proyecto 1 — Análisis y Diseño de Algoritmos**
Módulo *Backtracking* (BT). Archivos documentados: [`src/BT_Angarita.cpp`](../src/BT_Angarita.cpp) y [`src/BTDatos.hpp`](../src/BTDatos.hpp).

---

## 1. Contexto del problema

- **Diseñador de política (este módulo, BT).** Dada una *política de contraseñas* —longitud `n` y una serie de mínimos por clase de carácter (minúsculas, mayúsculas, dígitos, símbolos), más la regla de no repetir dos caracteres iguales consecutivos— el objetivo es **construir** las contraseñas que la política admite. No se ataca nada: se *genera* el conjunto de cadenas válidas (o se demuestra que está vacío).


Backtracking evita construir el espacio completo. Construye cada cadena **carácter a carácter** y, en cada prefijo, se pregunta: *"¿este prefijo todavía puede completarse hasta una contraseña válida?"*. Si la respuesta es no, **abandona toda la rama** sin generar ninguno de sus descendientes. Es decir, no genera-y-luego-filtra: **filtra mientras genera**, y el filtrado poda subárboles enteros de una sola vez.

---

## 2. Fundamentación teórica

### Definición (en nuestras palabras)

*Backtracking* es una búsqueda en profundidad sobre el **árbol de soluciones parciales**. Se extiende una solución candidata un paso a la vez; antes de profundizar, una **prueba de factibilidad** decide si el candidato parcial puede aún conducir a una solución completa. Si no puede, se **retrocede** (se deshace la última decisión) y se prueba la siguiente alternativa. El proceso equivale a recorrer un árbol *podando* las ramas que se demuestran imposibles.

En **nuestra instancia**, una "solución parcial" es un **prefijo** de contraseña de longitud `pos`; extenderla es fijar el carácter en la posición `pos`; y la prueba de factibilidad es la desigualdad `faltan > n - pos` (Sección 6).

### Los cuatro ingredientes, aterrizados en el código

| Concepto teórico | Cómo se ve en `BT_Angarita.cpp` |
|---|---|
| **Estado parcial** | El prefijo `password[0..pos-1]`, la posición actual `pos`, y los cuatro contadores `nLower, nUpper, nDigit, nSymbol` de clases ya colocadas. |
| **Condición de factibilidad** | `faltan <= pol.n - pos`: los mínimos que aún faltan caben en las posiciones que quedan. Si se viola, `return` (poda). |
| **Poda** | La línea `if (faltan > pol.n - pos) return;` y la regla `if (... c == password[pos-1]) continue;`. La primera corta subárboles imposibles; la segunda evita ramas prohibidas por la política. |
| **Árbol de búsqueda** | Raíz = cadena vacía (`pos = 0`). Cada nodo tiene hasta 67 hijos (un carácter del alfabeto). Las hojas están en profundidad `n`. Una hoja es solución si el prefijo completo cumple todos los mínimos. |

### La distinción que exige el enunciado

Backtracking **no mejora el orden de complejidad en el peor caso**. El peor caso sigue siendo `O(67^n)`: si la política es tan permisiva que ninguna rama puede podarse antes de la hoja, el árbol se recorre completo. Lo que backtracking cambia es el **número de nodos efectivamente visitados en la práctica**: cuando la política es restrictiva, la poda elimina subárboles enteros y el recorrido real es órdenes de magnitud menor que `67^n`. Es una mejora **empírica** (constante/factor sobre instancias reales), no **asintótica**. La Sección 14 mide esta diferencia con números concretos.

---

## 3. Modelamiento

### Alfabeto

Definido en [`BTDatos.hpp`](../src/BTDatos.hpp) como `ALFABETO_BT`, con **67 símbolos**:

| Clase | Caracteres | Cantidad |
|---|---|---|
| Minúsculas | `a … z` | 26 |
| Mayúsculas | `A … Z` | 26 |
| Dígitos | `0 … 9` | 10 |
| Símbolos | `! @ # $ %` | 5 |
| **Total** | | **67** |

Denotamos el alfabeto por **Σ**, con `|Σ| = 67`. Cada carácter pertenece a exactamente una de las cuatro clases disjuntas: minúsculas `L`, mayúsculas `U`, dígitos `D` y símbolos `S`.

### Modelamiento del problema

**Estado parcial.** Un estado parcial es un **prefijo** `s = s₀ s₁ … s_{k-1} ∈ Σ^k` de longitud `k` (en el código, `k = pos`, con `0 ≤ k ≤ n`). Para no re-escanear el prefijo en cada nodo, el estado se enriquece con cuatro **contadores acumulados** que resumen las clases ya colocadas:

```
c_L(s) = #{ i < k : s_i ∈ L }      (nLower)
c_U(s) = #{ i < k : s_i ∈ U }      (nUpper)
c_D(s) = #{ i < k : s_i ∈ D }      (nDigit)
c_S(s) = #{ i < k : s_i ∈ S }      (nSymbol)
```

Así, el estado del nodo es la tupla `(s, k, c_L, c_U, c_D, c_S)`. En la implementación, `s` es el buffer `password`, `k` es `pos`, y los cuatro contadores son `nLower, nUpper, nDigit, nSymbol` (la política `pol` y el par `contador, limite` acompañan la recursión pero son constantes/globales, no parte del estado del nodo).

**Estado inicial.** El **prefijo vacío** `s = ε`, es decir `k = 0` y todos los contadores en 0. Es la raíz del árbol de búsqueda.

**Estados terminales.** Los prefijos de longitud completa, `k = n` (las cadenas de `Σ^n`). Un terminal es **solución** si además satisface toda la política; gracias a la poda de factibilidad, todo prefijo que alcanza `k = n` **ya** cumple los mínimos, por lo que el caso base solo emite y cuenta.

**Política (derivada de la semilla).** La política vive en el `struct PoliticaBT`. Sus cuatro mínimos provienen de la **semilla** del equipo (calculada en [`tests/verificarSemilla.cpp`](../tests/verificarSemilla.cpp) a partir de los apellidos ordenados). Los valores instanciados son:

```
minLower  = 2      minUpper  = 2
minDigit  = 3      minSymbol = 1     ->  suma de mínimos = 8
prohibirSeguidos = true            (regla de no consecutivos)
```

Es decir, toda contraseña válida debe tener **≥2 minúsculas, ≥2 mayúsculas, ≥3 dígitos, ≥1 símbolo** y **ningún par de caracteres iguales adyacentes**. La suma de mínimos, **8**, es la longitud mínima factible: ninguna cadena más corta que 8 puede cumplir la política (dato clave para el caso `n = 6`).

### Entradas y salidas

**Entrada.**

- El **alfabeto** `Σ` (67 símbolos, fijo en `ALFABETO_BT`).
- La **longitud** `n` de la contraseña (el *driver* recorre `n ∈ {8, 6, 10}`).
- Los **parámetros de la política** asignada al equipo: `minLower, minUpper, minDigit, minSymbol` y la bandera `prohibirSeguidos`.
- Un **límite** `limite = 3500` de cadenas a emitir por cada `n` (control práctico de tamaño de salida, no parte del problema teórico).

**Salida.** Para cada `n`, esta implementación calcula el **conjunto de contraseñas válidas** —cada cadena se escribe, una por línea, en [`results/PasswordsBT.txt`](../results/PasswordsBT.txt)— **acotado por `limite`**: se emiten las primeras `min(#soluciones, 3500)` contraseñas en el orden de recorrido en profundidad. Adicionalmente, cada ejecución devuelve el **conteo** de cadenas emitidas (`contador`) y el **tiempo** de la corrida. En resumen: la instancia solicitada produce el **conjunto** de contraseñas válidas (truncado por el límite) **y** su conteo; cuando la instancia es infactible (p. ej. `n = 6`), la salida es el conjunto vacío y conteo 0.

### Restricciones (condiciones verificables sobre el prefijo parcial)

Cada regla de la política se formaliza como una condición sobre el prefijo `s` de longitud `k`, comprobable **incrementalmente** al extenderlo con un carácter `c` en la posición `k`:

- **R1 — Pertenencia al alfabeto.** `s_i ∈ Σ` para todo `i`. Se garantiza por construcción: el bucle solo prueba caracteres `c ∈ ALFABETO_BT`.

- **R2 — Longitud exacta.** Una solución cumple `|s| = n`. Verificable en el terminal: `k = n`.

- **R3 — No consecutivos** (si `prohibirSeguidos`). `∀ i ∈ [1, k-1] : s_i ≠ s_{i-1}`. Como el prefijo previo ya la respeta, al añadir `c` en la posición `k` basta la comprobación **local**:
  ```
  k = 0  ∨  c ≠ s_{k-1}
  ```
  (en el código: `pos > 0 && c == password[pos-1]` ⇒ se descarta ese hijo).

- **R4 — Mínimos por clase.** Una solución cumple `c_L(s) ≥ minLower`, `c_U(s) ≥ minUpper`, `c_D(s) ≥ minDigit`, `c_S(s) ≥ minSymbol`. Sobre un prefijo **parcial** esto no puede exigirse todavía (aún faltan caracteres), pero sí puede verificarse su **factibilidad**: definiendo el déficit
  ```
  faltan(s) = max(0, minLower  − c_L) + max(0, minUpper  − c_U)
            + max(0, minDigit  − c_D) + max(0, minSymbol − c_S)
  ```
  el prefijo `s` **puede** extenderse a una solución solo si `faltan(s) ≤ n − k` (los requisitos pendientes caben en las posiciones libres). La violación `faltan(s) > n − k` es la condición verificable que **poda** la rama (Sección 4). En el terminal `k = n` se tiene `faltan(s) = 0`, lo que equivale a que R4 se cumple con igualdad de garantía.

---

## 4. Diseño algorítmico

### La función de factibilidad `faltan > n - pos`

La poda central es puramente aritmética. En un prefijo cualquiera se calcula cuántos caracteres de cada clase **todavía faltan** para alcanzar los mínimos:

```
faltan = max(0, minLower  - nLower)
       + max(0, minUpper  - nUpper)
       + max(0, minDigit  - nDigit)
       + max(0, minSymbol - nSymbol)
```

`faltan` es una **cota inferior** del número de posiciones que aún deben "gastarse" obligatoriamente en clases deficitarias. Por otro lado, `n - pos` es el número de posiciones **libres** que quedan por llenar.

La lógica matemática: si los requisitos pendientes exceden los espacios disponibles —`faltan > n - pos`— entonces **es imposible** completar el prefijo cumpliendo la política, *sin importar qué caracteres se coloquen después*. En ese instante toda la rama es infactible y se poda con `return`. La cota es **admisible** (nunca poda una rama que sí tenía solución) porque cada unidad de `faltan` requiere consumir al menos una posición distinta.

Un corolario barato pero potente: en la raíz, con todos los contadores en 0, `faltan = suma de mínimos = 8`. Si `n < 8`, la desigualdad se viola de inmediato y **el árbol entero se poda en el primer nodo** (ver `n = 6`).

### Contadores por valor (no se deshace manualmente)

Los contadores `nLower, nUpper, nDigit, nSymbol` se pasan **por valor** en cada llamada recursiva:

```cpp
backtracking(password, pos + 1,
             nLower + (esMinuscula(c) ? 1 : 0),
             ... );
```

Esto es una decisión de diseño deliberada. En backtracking clásico se muta un estado compartido y hay que **deshacer** (`--nLower`) al retroceder; olvidar un *undo* es una fuente típica de errores. Al pasar los contadores por valor, cada nodo recibe su **propia copia** ya actualizada; cuando la recursión retorna, la copia se descarta sola y el marco del llamador conserva intactos sus valores. El "deshacer" es automático por semántica de la pila de llamadas. El costo (copiar cuatro `int`) es despreciable frente a la claridad y la ausencia de errores de estado. El buffer `password` sí se comparte por referencia, pero cada posición se **sobrescribe** antes de usarse, así que no requiere undo explícito.

### Estructura para las variantes

`main()` fija un límite configurable `MAX_CADENAS` y recorre un conjunto de longitudes `n`. Para cada `n`:

1. Construye una `PoliticaBT` con esa `n` (los mínimos son los de la semilla).
2. Llama a `generarPassword`, que primero hace **detección temprana** de infactibilidad (`minTotal > n`) y, si procede, lanza el backtracking.
3. Mide el tiempo con `clock()` y reporta cadenas escritas y segundos, tanto a archivo (`results/PasswordsBT.txt`) como a consola.

El *driver* entregado recorre `n ∈ {8, 6, 10}` con la política base. Para la experimentación de las **5 variantes** (Sección 11) se extiende este mismo esquema variando también la política (relajada / casi vacía), reutilizando la función `backtracking` sin cambios.

---

## 5. Pseudocódigo

Notación del curso (no C++ literal), fiel a lo implementado:

```
Algoritmo BACKTRACKING(prefijo, pos, nL, nU, nD, nS, pol)
  Entrada: prefijo parcial, posición actual, contadores por clase, política
  Salida : emite (por efecto lateral) las contraseñas válidas, hasta 'limite'

  si contador ≥ limite entonces
      retornar                               // corte por límite de salida

  faltan ← max(0, pol.minL − nL) + max(0, pol.minU − nU)
         + max(0, pol.minD − nD) + max(0, pol.minS − nS)
  si faltan > pol.n − pos entonces
      retornar                               // PODA: rama infactible

  si pos = pol.n entonces                     // hoja: prefijo completo y válido
      Emitir(prefijo)
      contador ← contador + 1
      retornar

  para cada c en ALFABETO hacer               // ramificación (≤ 67 hijos)
      si pol.prohibirSeguidos ∧ pos > 0 ∧ c = prefijo[pos−1] entonces
          continuar                           // poda regla no-consecutivos
      prefijo[pos] ← c
      BACKTRACKING(prefijo, pos+1,
                   nL + esMin(c), nU + esMay(c),
                   nD + esDig(c), nS + esSim(c), pol)
      si contador ≥ limite entonces retornar
  fin para
fin Algoritmo


Algoritmo GENERAR(pol, limite)
  minTotal ← pol.minL + pol.minU + pol.minD + pol.minS
  si minTotal > pol.n entonces                // detección temprana
      Reportar "longitud insuficiente"; retornar 0
  contador ← 0
  BACKTRACKING(prefijo_vacío, 0, 0,0,0,0, pol)
  retornar contador
```

---

## 6. Implementación (fragmentos no triviales)

### Poda anticipada por factibilidad

El corazón del ahorro. Se evalúa **antes** de ramificar, en cada nodo:

```cpp
int faltan = 0;
if (pol.minLower  - nLower  > 0) faltan += pol.minLower  - nLower;
if (pol.minUpper  - nUpper  > 0) faltan += pol.minUpper  - nUpper;
if (pol.minDigit  - nDigit  > 0) faltan += pol.minDigit  - nDigit;
if (pol.minSymbol - nSymbol > 0) faltan += pol.minSymbol - nSymbol;
if (faltan > pol.n - pos) return;   // rama imposible -> retroceder
```

Los `if` implementan el `max(0, ·)`: solo cuentan las clases **deficitarias** (si ya se cumplió un mínimo, no aporta a `faltan`).

### Detección de la variante infactible (`n = 6`)

Antes de tocar el árbol, `generarPassword` compara la suma de mínimos contra `n`:

```cpp
int minTotal = pol.minLower + pol.minUpper + pol.minDigit + pol.minSymbol; // = 8
if (minTotal > pol.n) {           // p.ej. n = 6  ->  8 > 6
    // reporta longitud insuficiente y retorna 0, sin explorar nada
    return 0;
}
```

Con la política base (`minTotal = 8`), la longitud `n = 6` se descarta aquí mismo: **0 cadenas, sin recorrer el árbol**. Aunque esta guarda no estuviera, la poda de factibilidad cortaría igualmente en la raíz (`faltan = 8 > 6 = n − 0`); la guarda solo lo hace explícito y produce un mensaje legible.

### Regla de no-consecutivos durante la ramificación

```cpp
for (char c : ALFABETO_BT) {
    if (pol.prohibirSeguidos && pos > 0 && c == password[pos - 1])
        continue;               // salta el hijo que repetiría el carácter previo
    password[pos] = c;
    backtracking(password, pos + 1, /* contadores actualizados por valor */ ...);
    if (contador >= limite) return;
}
```

La restricción se aplica **en el momento de generar** cada hijo (no se genera y luego se descarta), y el chequeo `contador >= limite` propaga el corte por límite hacia arriba en la pila.

---

## 7. Análisis de complejidad

Sea `A = 67` (tamaño del alfabeto) y `n` la longitud.

**Temporal.**
- *Peor caso:* `O(A^n) = O(67^n)`. Si ninguna rama puede podarse antes de la hoja (política vacía), se visita el árbol completo. Backtracking **no** rebaja este orden.
- *Con la regla de no-consecutivos:* el árbol tiene a lo sumo `A · (A−1)^(n−1) = 67 · 66^(n−1)` hojas, que sigue siendo `Θ(67^n)` en orden.
- *Mejor caso:* `O(1)` amortizado por la poda: si `minTotal > n`, se corta en la raíz (o antes de recursar). Ejemplo real: `n = 6`.

**Espacial.**
- `O(n)` por la profundidad de la pila de recursión (a lo sumo `n` marcos activos) más el buffer `password` de tamaño `n`. Los contadores por valor añaden `O(1)` por marco, es decir `O(n)` en total. **No** se almacena el conjunto de soluciones en memoria: cada cadena se **escribe a disco** y se descarta, por lo que el espacio es independiente del número de soluciones.

**Caso mejor / peor / promedio (según la política):**

| Escenario | Política | Efecto | Costo real |
|---|---|---|---|
| **Mejor** | Muy restrictiva (`minTotal > n`) | Poda en la raíz | `O(1)` — caso `n = 6` |
| **Peor** | Casi vacía (mins = 0) | Ninguna poda de factibilidad | `Θ(67^n)` — variante V5 |
| **Promedio** | Restrictiva pero factible | Poda subárboles deficitarios | ≪ `67^n`, pero mismo orden |

---

## 8. Casos de prueba

### Instancia de referencia común

- **`n = 6`, política de validación (la de la semilla, `minTotal = 8`).** Resultado esperado: **0 soluciones** (la longitud no alcanza para los mínimos). Sirve para validar tanto la detección temprana como la poda en la raíz.

### Las 5 variantes propias (parámetros exactos)

| Variante | `n` | minL | minU | minD | minS | Σmín | no-consec. | Naturaleza |
|---|---|---|---|---|---|---|---|---|
| **V1** | 6 | 2 | 2 | 3 | 1 | 8 | sí | Infactible (`Σ > n`) |
| **V2** | 8 | 2 | 2 | 3 | 1 | 8 | sí | Factible, ajuste justo (`Σ = n`) |
| **V3** | 10 | 2 | 2 | 3 | 1 | 8 | sí | Factible, holgura 2 |
| **V4** | 8 | 1 | 1 | 1 | 1 | 4 | sí | Política relajada |
| **V5** | 8 | 0 | 0 | 0 | 0 | 0 | no | Casi vacía (peor caso de poda) |

V1–V3 usan la **política de la semilla** variando solo `n` (son las tres corridas del *driver* entregado, `n ∈ {8,6,10}`). V4 y V5 varían la política para exhibir los extremos de restrictividad.

### Corrección 67 vs. 69 símbolos

El enunciado original (PDF) mencionaba **69** símbolos en el alfabeto. El alfabeto realmente implementado y usado en todo el módulo tiene **67** (26 + 26 + 10 + 5), como se verifica directamente sobre `ALFABETO_BT`. El equipo reconcilió esta discrepancia (confirmada con el docente): el conteo correcto para esta implementación es **67**, y todas las cotas teóricas de este informe usan `67^n`, no `69^n`. Los 2 símbolos de diferencia simplemente no forman parte del conjunto `! @ # $ %` empleado.

---

## 9. Experimentación

### Cómo se midió

- **Tiempo:** `clock()` de `<ctime>`, diferencia `fin − inicio` dividida por `CLOCKS_PER_SEC`, por corrida.
- **Nodos visitados (con poda):** contador global incrementado una vez por entrada a la función de backtracking (cada estado parcial examinado). El *driver* entregado no lo expone; se midió con el arnés instrumentado que llama a la misma lógica.
- **Nodos generados (sin poda):** número de nodos de una versión que recorre el árbol completo (misma regla de no-consecutivos) y valida solo en la hoja. Verificado además contra la fórmula cerrada `1 + Σ_{i=1..n} 67·66^(i−1)`.
- Todas las corridas de las 5 variantes usan `limite = 3500` cadenas (igual que el *driver*).

### Tabla de tiempos — 5 variantes (límite = 3500 soluciones)

| Variante | `n` | Soluciones | Nodos visitados | Tiempo |
|---|---|---|---|---|
| V1 (infactible) | 6 | 0 | 1 | ~0.000 s |
| V2 (ajuste justo) | 8 | 3500 | 34 608 | 0.001 s |
| V3 (holgura 2) | 10 | 3500 | 34 610 | ~0.000 s |
| V4 (relajada) | 8 | 3500 | 35 487 | 0.001 s |
| V5 (casi vacía) | 8 | 3500 | 3 560 | ~0.000 s |

Contraste con la corrida del *driver* entregado ([`results/PasswordsBT.txt`](../results/PasswordsBT.txt)): `n = 8 → 3500` cadenas en 0.004 s; `n = 6 → 0` en 0 s; `n = 10 → 3500` en 0.003 s (tiempo total de ejecución 0.01 s). Consistente con la tabla.

### Gráfica tiempo vs. tamaño de entrada

Con el tope de 3500 soluciones, los tiempos de las variantes factibles quedan en el **piso de resolución del reloj** (≈ 0–4 ms): una vez factible, encontrar 3500 cadenas cuesta prácticamente lo mismo para `n = 8` y `n = 10`. El crecimiento **real** con el tamaño se aprecia mejor en la enumeración **completa sin tope** (Sección 14):

```
tiempo (s) — enumeración completa, política (1,1,1,1)
 2.4 |                                   ● sin poda (n=4)
     |
 1.8 |
     |
 1.2 |
     |
 0.6 |
     |             ○ con poda (n=4)
 0.0 |___●___○________________________________
         n=3        n=4
   (n=3 es infactible: ambos triviales salvo el árbol sin poda)
```

Lectura: sin poda el tiempo salta de 0.027 s (`n=3`) a 2.367 s (`n=4`) — factor ~87×, coherente con el crecimiento `×66` por nivel. Con poda, la misma instancia factible baja a 0.177 s.

---

## 10. Resultados y análisis de resultados

### Qué se encontró al correr las 5 variantes

- **V1 (`n = 6`) — 0 soluciones, poda casi total.** Este **no es un error**, es el resultado correcto y esperado: `Σmín = 8 > 6 = n`, así que ninguna contraseña de longitud 6 puede cumplir la política. El algoritmo lo detecta en **1 solo nodo** (poda en la raíz) o en la guarda temprana, sin explorar el árbol. Es la demostración empírica del **mejor caso**.
- **V2 y V3 (factibles).** Alcanzan el tope de 3500 cadenas visitando ~34 600 nodos. El número de nodos es casi idéntico entre `n = 8` y `n = 10` porque la búsqueda en profundidad encuentra las primeras 3500 soluciones "por la izquierda" del árbol y se corta; la `n` mayor apenas añade profundidad a esas mismas ramas.
- **V4 (relajada).** Más nodos (35 487): al exigir menos, la poda de factibilidad actúa más tarde, así que se examinan algo más de estados antes de juntar 3500 soluciones.
- **V5 (casi vacía).** Curiosamente la de **menos** nodos (3 560) *bajo el tope de 3500*: como toda cadena es válida, las 3500 primeras hojas se emiten casi sin ramificar. Esto ilustra un matiz importante: **con tope de soluciones**, la política permisiva termina antes; el **peor caso teórico** de V5 (recorrer `67^n` completo) solo se manifiesta si se **quita el tope** o si se cuentan *todas* las soluciones.

### Contraste teórico vs. empírico

La cota teórica del espacio es `67^n` (corrección de `69^n` → `67^n`, Sección 10). Para `n = 8` eso es `≈ 4.06 × 10^14` cadenas. Empíricamente, la búsqueda con poda y tope emite 3500 cadenas visitando ~3.5 × 10^4 nodos: **diez órdenes de magnitud por debajo** del espacio completo. La lección coincide con la Sección 4: backtracking **no** cambió el orden `67^n`, pero el número de nodos *realmente visitados* es diminuto frente a la cota. La medición sin tope (Sección 14) confirma que, cuando sí se recorre todo, el costo crece `×66` por nivel, tal como predice la teoría.

---

## 11. Comparación algorítmica: con poda vs. sin poda

Para medir el efecto **aislado** de la poda de factibilidad se enumeran **todas** las soluciones (sin tope) sobre instancias tratables (mismo alfabeto de 67, `n` pequeño). Ambas versiones respetan la regla de no-consecutivos, de modo que definen **el mismo conjunto de soluciones**; la única diferencia es que "con poda" aplica `faltan > n − pos` y "sin poda" valida únicamente en la hoja.

| Instancia | Nodos **sin poda** | Nodos **con poda** | Reducción | Tiempo sin poda | Tiempo con poda | Soluciones (ambas) | ¿Coinciden? |
|---|---|---|---|---|---|---|---|
| `n=3`, mins (1,1,1,1), Σ=4 → **infactible** | 296 342 | **1** | **99.9997 %** | 0.027 s | ~0.000 s | 0 | ✔ sí |
| `n=4`, mins (1,1,1,1), Σ=4 → factible | 19 558 574 | 5 248 322 | **73.17 %** | 2.367 s | 0.177 s | 811 200 | ✔ sí |

**Verificación de equivalencia.** En ambas instancias las dos versiones reportan **exactamente el mismo número de soluciones** (0 y 811 200 respectivamente), lo que confirma que la poda es **admisible**: nunca descarta una rama que contuviera una solución válida.

**Interpretación.**
- Cuando la instancia es **infactible** (`Σmín > n`), la poda es dramática: recorta 296 341 de 296 342 nodos (99.9997 %) cortando en la raíz. Sin poda hay que generar el árbol entero solo para descubrir que ninguna hoja es válida.
- Cuando la instancia es **factible pero ajustada**, la poda aún elimina ~73 % de los nodos y acelera el tiempo ~13× (2.367 s → 0.177 s), sin perder ni una solución.

Esto cierra el argumento del enunciado: **misma complejidad asintótica, misma respuesta, pero muchísimos menos nodos visitados** — exactamente lo que backtracking promete.

---

### Apéndice — reproducibilidad

- Fuente del algoritmo: [`src/BT_Angarita.cpp`](../src/BT_Angarita.cpp), [`src/BTDatos.hpp`](../src/BTDatos.hpp).
- Salida del *driver*: [`results/PasswordsBT.txt`](../results/PasswordsBT.txt).
- Semilla y política: [`tests/verificarSemilla.cpp`](../tests/verificarSemilla.cpp).
- Compilador usado en las mediciones: `g++ (MinGW-w64) 13.2.0`, `-O2`.
- Los conteos de nodos y la comparación con/sin poda se obtuvieron con un arnés instrumentado que reutiliza la misma función `backtracking`; las cifras "sin poda" se validaron contra la fórmula cerrada `1 + Σ_{i=1..n} 67·66^(i−1)`.
