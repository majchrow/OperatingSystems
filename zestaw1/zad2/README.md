Plik makefile korzysta z shared biblioteki .so, którą należy umieścić w folderze Libs. Bibliotekę można otrzymać wywołując polecenie make w folderze zad1.
Można również wykorzystać bibliotekę .a, lecz wtedy trzeba zmodyfikować kompilację pliku main.c w Makefile.
W pliku raport2.txt, wygenerowanym przez polecenie make, otrzymujemy wyniki testów, zawartych w pliku make. W związku z tym, że polecenie systemowe "find" robi cache wyników, wszystkie 
wyniki pomiaru czasu są uśredniane(średnia z pięciu tych samych testów), tym samym bardziej rzetelne.
Z wyników możemy zaobserwować, że nawet po wykonaniu sporej ilości instrukcji, zmierzony czas systemowy oraz użytkownika jest równy 0.
 

