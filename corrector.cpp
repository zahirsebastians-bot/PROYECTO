/*****************************************************************************************************************
UNIVERSIDAD NACIONAL AUTONOMA DE MEXICO
	FACULTAD DE ESTUDIOS SUPERIORES -ARAGON-
     hola Aaron
	Computadoras y programacion.
	(c) Zahir Sebastian Sanchez Martinez 323018520
	Quiso decir: Programa principal de la aplicacion de la distancia de Levenstein.

******************************************************************************************************************/


#include "stdafx.h"
#include "corrector.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ALFABETO - 32 caracteres segun especificacion
const char alfabeto[32] = {
	'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v','w','x','y','z',
	(char)0xE1, /* á */
	(char)0xE9, /* é */
	(char)0xED, /* í */
	(char)0xF3, /* ó */
	(char)0xFA, /* ú */
	(char)0xF1  /* ñ */
};

const int TAM_ALFABETO = 32;

// Convierte cada carácter de la cadena a minúscula
void aMinusculas(char* texto) {
	for (int pos = 0; texto[pos] != '\0'; pos++) {
		texto[pos] = tolower((unsigned char)texto[pos]);
	}
}

// Determina si un carácter es un separador reconocido
bool esDelimitador(char caracter) {
	switch (caracter) {
	case ' ': case '\t': case '\n': case '\r':
	case ',': case '.': case ';':
	case '(': case ')':
		return true;
	default:
		return false;
	}
}
// Remueve caracteres de puntuación al final de una cadena
void quitarPuntuacionFinal(char* texto) {
	int longitud = strlen(texto);

	while (longitud > 0 &&
		(texto[longitud - 1] == ',' || texto[longitud - 1] == '.' ||
			texto[longitud - 1] == ';' || texto[longitud - 1] == '(' ||
			texto[longitud - 1] == ')')) {

		texto[longitud - 1] = '\0';
		longitud--;
	}
}

// Implementación de búsqueda binaria - complejidad O(log n)
int buscarEnOrden(char listado[][TAMTOKEN], int total, const char* clave) {
	int inicio = 0;
	int fin = total - 1;

	while (inicio <= fin) {
		int centro = inicio + (fin - inicio) / 2;  //Evita desbordamiento
		int resultado = strcmp(listado[centro], clave);

		if (resultado == 0) {
			return centro;
		}
		else if (resultado < 0) {
			inicio = centro + 1;
		}
		else {
			fin = centro - 1;
		}
	}
	return -1;
}

// Inserta una cadena manteniendo el arreglo en orden alfabetico
void agregarEnOrden(char listado[][TAMTOKEN], int contador[],
	int& total, const char* nuevaPalabra) {

	// Identificar la posición adecuada
	int indice = 0;
	while (indice < total && strcmp(listado[indice], nuevaPalabra) < 0) {
		indice++;
	}

	// Mover elementos una posición a la derecha
	for (int k = total; k > indice; k--) {
		strcpy(listado[k], listado[k - 1]);
		contador[k] = contador[k - 1];
	}

	// Insertar nueva entrada
	strcpy(listado[indice], nuevaPalabra);
	contador[indice] = 1;
	total++;
}


//Funciones publicas del proyecto
/*****************************************************************************************************************
	DICCIONARIO: Esta funcion crea el diccionario completo
	char *	szNombre				:	Nombre del archivo de donde se sacaran las palabras del diccionario
	char	szPalabras[][TAMTOKEN]	:	Arreglo con las palabras completas del diccionario
	int		iEstadisticas[]			:	Arreglo con el numero de veces que aparecen las palabras en el diccionario
	int &	iNumElementos			:	Numero de elementos en el diccionario
******************************************************************************************************************/
void Diccionario(
	char* szNombre,
	char szPalabras[][TAMTOKEN],
	int iEstadisticas[],
	int& iNumElementos)
{
	iNumElementos = 0;

	// Apertura del archivo en modo lectura
	FILE* archivo = fopen(szNombre, "r");
	if (!archivo) return;

	char palabra[TAMTOKEN];
	char c;
	int idx = 0;

	// Lectura continua del archivo
	while ((c = fgetc(archivo)) != EOF) {

		// Si el carácter leído es separador
		if (esDelimitador(c)) {

			// Validar si se estaba formando una palabra
			if (idx > 0) {
				palabra[idx] = '\0';

				// Normalizar palabra
				quitarPuntuacionFinal(palabra);
				aMinusculas(palabra);

				// Verificar palabra no vacía
				if (palabra[0] != '\0') {

					int pos = buscarEnOrden(szPalabras, iNumElementos, palabra);

					if (pos >= 0) {
						iEstadisticas[pos]++;      // Si existe, aumentar contador
					}
					else if (iNumElementos < NUMPALABRAS) {
						agregarEnOrden(szPalabras, iEstadisticas, iNumElementos, palabra);
					}
				}

				// Reiniciar índice para siguiente palabra
				idx = 0;
			}
		}
		else {
			// Se acumula el carácter en la palabra si aún hay espacio
			if (idx < TAMTOKEN - 1) {
				palabra[idx++] = c;
			}
		}
	}

	// palabra pendiente
	if (idx > 0) {
		palabra[idx] = '\0';

		quitarPuntuacionFinal(palabra);
		aMinusculas(palabra);

		if (palabra[0] != '\0') {

			int pos = buscarEnOrden(szPalabras, iNumElementos, palabra);

			if (pos >= 0) {
				iEstadisticas[pos]++;
			}
			else if (iNumElementos < NUMPALABRAS) {
				agregarEnOrden(szPalabras, iEstadisticas, iNumElementos, palabra);
			}
		}
	}

	fclose(archivo);
}


/*****************************************************************************************************************
	ListaCandidatas: Esta funcion recupera desde el diccionario las palabras validas y su peso
	Regresa las palabras ordenadas por su peso
	char	szPalabrasSugeridas[][TAMTOKEN],	//Lista de palabras clonadas
	int		iNumSugeridas,						//Lista de palabras clonadas
	char	szPalabras[][TAMTOKEN],				//Lista de palabras del diccionario
	int		iEstadisticas[],					//Lista de las frecuencias de las palabras
	int		iNumElementos,						//Numero de elementos en el diccionario
	char	szListaFinal[][TAMTOKEN],			//Lista final de palabras a sugerir
	int		iPeso[],							//Peso de las palabras en la lista final
	int &	iNumLista)							//Numero de elementos en la szListaFinal
******************************************************************************************************************/
void ListaCandidatas(
	char szPalabrasSugeridas[][TAMTOKEN],
	int iNumSugeridas,
	char szPalabras[][TAMTOKEN],
	int iEstadisticas[],
	int iNumElementos,
	char szListaFinal[][TAMTOKEN],
	int iPeso[],
	int& iNumLista)
{
	iNumLista = 0;

	for (int i = 0; i < iNumSugeridas; i++) {

		// CORRECCION 1: Usar el nombre correcto de la función definida arriba
		int pos = buscarEnOrden(szPalabras, iNumElementos, szPalabrasSugeridas[i]);

		if (pos >= 0) {
			bool existe = false;

			for (int j = 0; j < iNumLista && !existe; j++) {
				// CORRECCION 2: Usar strcmp estándar
				if (strcmp(szListaFinal[j], szPalabras[pos]) == 0) {
					existe = true;
				}
			}

			if (!existe) {
				// CORRECCION 3: Usar strcpy estándar
				strcpy(szListaFinal[iNumLista], szPalabras[pos]);
				iPeso[iNumLista] = iEstadisticas[pos];
				iNumLista++;
			}
		}
	}

	// Burbujazo 
	for (int i = 0; i + 1 < iNumLista; i++) {
		for (int j = 0; j + 1 < iNumLista - i; j++) {
			if (iPeso[j] < iPeso[j + 1]) {
				int tempPeso = iPeso[j];
				iPeso[j] = iPeso[j + 1];
				iPeso[j + 1] = tempPeso;

				char tempPalabra[TAMTOKEN];
				// CORRECCION 4: Usar strcpy estándar
				strcpy(tempPalabra, szListaFinal[j]);
				strcpy(szListaFinal[j], szListaFinal[j + 1]);
				strcpy(szListaFinal[j + 1], tempPalabra);
			}
		}
	}
}
/*****************************************************************************************************************
	ClonaPalabras: toma una palabra y obtiene todas las combinaciones y permutaciones requeridas por el metodo
	char *	szPalabraLeida,						// Palabra a clonar
	char	szPalabrasSugeridas[][TAMTOKEN], 	//Lista de palabras clonadas
	int &	iNumSugeridas)						//Numero de elementos en la lista
******************************************************************************************************************/
void ClonaPalabras(
	char* szPalabraLeida,
	char szPalabrasSugeridas[][TAMTOKEN],
	int& iNumSugeridas)
{
	iNumSugeridas = 0;
	int len = strlen(szPalabraLeida);

	// 0. Agregar palabra original tal cual
	memcpy(szPalabrasSugeridas[iNumSugeridas], szPalabraLeida, len + 1);
	iNumSugeridas++;

	// 1. ELIMINACIÓN - Remover cada caracter individualmente
	for (int i = 0; i < len; i++) {
		char temp[TAMTOKEN];
		int idx = 0;

		for (int j = 0; j < len; j++) {
			if (j == i) continue;
			temp[idx++] = szPalabraLeida[j];
		}
		temp[idx] = '\0';

		if (idx > 0) {
			memcpy(szPalabrasSugeridas[iNumSugeridas], temp, idx + 1);
			iNumSugeridas++;
		}
	}

	// 2. TRANSPOSICIÓN - Intercambio de elementos contiguos
	for (int i = 0; i < len - 1; i++) {
		char temp[TAMTOKEN];

		memcpy(temp, szPalabraLeida, len + 1);

		char aux = temp[i];
		temp[i] = temp[i + 1];
		temp[i + 1] = aux;

		memcpy(szPalabrasSugeridas[iNumSugeridas], temp, len + 1);
		iNumSugeridas++;
	}

	// 3. SUSTITUCIÓN - Reemplazar un caracter con cada letra del alfabeto
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < TAM_ALFABETO; j++) {
			char temp[TAMTOKEN];

			memcpy(temp, szPalabraLeida, len + 1);
			temp[i] = alfabeto[j];

			memcpy(szPalabrasSugeridas[iNumSugeridas], temp, len + 1);
			iNumSugeridas++;
		}
	}

	// 4. INSERCIÓN - Introducir cada letra en cada posición posible
	for (int i = 0; i <= len; i++) {
		for (int j = 0; j < TAM_ALFABETO; j++) {

			char temp[TAMTOKEN];
			int idx = 0;

			// Copiar la parte previa a i
			if (i > 0) {
				memcpy(temp, szPalabraLeida, i);
				idx = i;
			}

			// Insertar nueva letra
			temp[idx++] = alfabeto[j];

			// Copiar el resto de la palabra
			memcpy(temp + idx, szPalabraLeida + i, len - i);
			idx += (len - i);

			temp[idx] = '\0';

			memcpy(szPalabrasSugeridas[iNumSugeridas], temp, idx + 1);
			iNumSugeridas++;
		}
	}

	// Burbujazo para ordenar alfabeticamente
	for (int i = 0; i < iNumSugeridas - 1; i++) {
		for (int j = 0; j < iNumSugeridas - i - 1; j++) {

			if (strcmp(szPalabrasSugeridas[j], szPalabrasSugeridas[j + 1]) > 0) {

				char temp[TAMTOKEN];
				memcpy(temp, szPalabrasSugeridas[j], TAMTOKEN);
				memcpy(szPalabrasSugeridas[j], szPalabrasSugeridas[j + 1], TAMTOKEN);
				memcpy(szPalabrasSugeridas[j + 1], temp, TAMTOKEN);
			}
		}
	}
}



