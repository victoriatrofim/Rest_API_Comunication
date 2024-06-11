# TEMA 4 - PCOM
                                        COMUNICATIE CU REST API

* Pornit de la laboratorul 9

Am implementat această aplicație client în limbajul de programare C, folosind biblioteca "parson" pentru parsarea JSON-ului. Aplicația comunică cu un server pentru a realiza diverse acțiuni, cum ar fi înregistrarea utilizatorilor, autentificarea, accesarea unei biblioteci de cărți și gestionarea înregistrărilor de cărți.

Programul începe prin setarea variabilelor și structurilor necesare pentru gestionarea inputului utilizatorului, comunicarea cu serverul și stocarea cookie-urilor și tokenurilor JSON Web. Pentru inceput, inițializez o variabilă command în care voi citi comenzile de la tastatură. Într-un ciclu while, cât timp nu introduc comanda "exit", îndeplinesc diverse comenzi.

# Funcționalități

- Register
În cadrul comenzii register, verific dacă am cookie-uri. Dacă da, înseamnă că sunt deja logat și returnez un mesaj de eroare, deoarece userul trebuie sa fie deloghat înainte de a putea înregistra un nou cont. În cazul în care nu am cookie-uri, citesc de la tastatură un username și o parolă pe care le adaug în obiectul JSON.
Serializez obiectul JSON într-un string și îl trimit ca parametru către funcția compute_post_request().
După trimiterea cererii către server și primirea răspunsului, verific dacă răspunsul conține erori și afișez mesajul corespunzător.

- Login
Comanda login verifică dacă sunt deja logat prin verificarea existenței cookie-urilor. Dacă sunt deja logat, afișez un mesaj de eroare și aștept o nouă comandă.
Dacă nu sunt logat, citesc username-ul și parola, le adaug în obiectul JSON, serializându-l ulterior și trimițând cererea către server.
După primirea răspunsului, verific dacă autentificarea a fost realizată cu succes, și, în caz afirmativ, stochez cookie-urile de sesiune. Pentru asta caut in response linia cu "Set-cookie: ", trec de ea si salvez scrict parea de cookie.

- Enter Library
Comanda enter_library verifică dacă sunt logat prin verificarea existenței cookie-urilor.
Dacă nu sunt logat, afișez un mesaj de eroare. În caz contrar, trimit o cerere de tip GET către server pentru a obține acces la bibliotecă.
Dacă răspunsul conține un token, îl stochez și afișez un mesaj de succes. Pentru aceasta, la fel ca si la pasul anterior caut in response linia care contine campul token si extrag de acolo tokenul meu.

- Get Books
Comanda get_books verifică dacă am acces la bibliotecă prin verificarea existenței tokenului .
Dacă nu am acces, afișez un mesaj de eroare. În caz contrar, trimit o cerere de tip GET către server pentru a obține lista de cărți disponibile, extrag campul data din response și afișez răspunsul.

- Get Book
Comanda get_book funcționează similar cu get_books, dar necesită un ID de carte specific pe care il citim de la stdin.
După citirea ID-ului, trimit o cerere GET către server pentru a obține detalii despre cartea specificată și afișez răspunsul.

- Add Book
Comanda add_book verifică dacă am acces la bibliotecă la fel cu ajutorul tokenului și, dacă nu am, afișez un mesaj de eroare. În caz contrar, citesc detaliile cărții de la tastatură și le adaug în obiectul JSON.
citirea pentru fiecare se face cu fgets pentru a prinde si denumirile care au spatii, iar dupa citire se elimina newlineul din vector.
Dupa citirea tuturor datelor, verificam daca macar una din ele nu contin nimic, adica a fost introdus un '/n' sau ' ', se afiseaza eroarea corespunzatoare.
o abordare diferita folosesc pentru numarul paginii care trebuie obligatoriu sa fie int, se verifica daca int_page_countul este 0 inseamna ca inputul nu a fost unul valid, respectiv se returneaza o eroare.
După serializarea obiectului JSON, deschidem o conexiune, trimit o cerere de tip POST către server pentru a adăuga cartea și afișez un mesaj de succes.

- Delete Book
Comanda delete_book necesită un ID de carte specific. După citirea ID-ului de la tastatură, deschid o conexiune, trimit o cerere DELETE către server pentru a șterge cartea specificată și afișez răspunsul, indicând dacă operațiunea a fost realizată cu succes sau dacă a apărut o eroare. Determin existenta unei erori cautand in response daca exista un camp error.

- Logout
Comanda logout verifică dacă sunt logat prin existenta cookiurilor și, dacă nu sunt, afișez un mesaj de eroare.
În caz contrar, pornesc o conexiune, trimit o cerere GET către server pentru a mă deloga, șterg tokenul, resetez cookies și afișez un mesaj de succes.

La sfarsitul functiei am grija sa eliberez memoria alocata.

Libraria JSON
- Pentru a parsa raspunsurile primite de la server am ales sa folosesc libraria parson, deoarece codul este scris in C, respectiv mi s-a parut potrivita pentru implementare.

