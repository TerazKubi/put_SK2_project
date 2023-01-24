Tic Tac Toe extended

Jest to rozszerzona wersja znanej gry kółko i krzyżyk.
W normalnej wersji mamy 9 pól gdzie gracze wstawiają swoje znaki.
W tym przypadku w każdym z 9 pól znajduje sie kolejna gra kółko i krzyżyk.
Co daje nam 9 "małych" gier i jedną "dużą".

Podczas swojej tury każdy gracz wstawia swój ruch w "małej" grze zaznaczonej na czerwono.
Jeśli żadne nie zostanie zaznaczone wtedy może wstawić w dowolne miejsce.

Każdy ruch gracza wskazuje "małą" gre w której ruch odda przeciwinik w swojej następnej turze.
Np jeśli gracz A wykona ruch w środkowej małej grze i będzie to prawy górny kwadracik to gracz B będzie musiał,
wykonać ruch w prawej górnej małej grze.
Jeśli natomiast prawa górna mała gra została już rozstrzygnięta wtedy gracz B może wykonać ruch w dowolnym miejscu.

Gra kończy się jeśli ułożenie rozstrzygniętych małych gier będzie jednym z możliwości wygrania (tak jak w standardowej grze Tic Tac Toe)


Serwer gry kożysta z socketów aby umożliwić klientą połączenie oraz wątki aby obsługiwać wszystkich graczy jednocześnie.
Klient po poprawnym połączeniu zostaje umieszczony w pierwszym pokoju gdzie jest wolne miejsce.

Ilość graczy jest nieograniczona. W razie potrzeby są tworzone nowe pokoje. Dzieje się to dynamicznie.

Serwer wymienia z klientem dane tj. dane dot. ruchu gracza, informacja o zmianie tury lub zwycięstwie.
Walidacja ruchów dzieje się po stronie serwera. W razie błędu odpowiednia informacja jest wysyłana do klienta i wyświetlana jest wskazówka np kiedy gracz
próbuje wykonać ruch w niedozwolonym miejscu.

Po zakończeniu gry klienci muszą połączyć się ponownie by zargać jeszcze raz.

Jeśli jeden z graczy opóści gre podczas rozgrywki plansza jest resetowana a gracz który został w pokoju musi zaczekać na nowego przeciwnika. 
