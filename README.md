# Whiteboard
https://awwapp.com/b/uayrdssah0d6y/

# FaQ
 -Si possono aggiungere file al template?
 -Si

 -Aprire FIFO in maniera non bloccante?
 -O_NONBLOCKING

 -Se la ack_list è piena, come ci muoviamo? blocchiamo l'ultimo device che deve consegnare il messaggio e lo facciamo aspettare finchè non si libera posto?
 -La ack_list è abbastanza grande da non avere queste problematiche
 (raddoppia proprio prima di saturarsi?)

 -Quanti messaggi può portare un device dentro se stesso?
 -Numero arbitrario grande (array di 20 messaggi)

 -Quando un device consegna un messaggio, lo deve eliminare dalla propria lista interna o tenerlo?
 -Una volta consegnato il messaggio, lo elimina dalla propria lista interna (deve riuscire a capire se è l'ultimo della lista (esempio: counter nel messaggio oppure una funzione che controlli se il determinato device ha ricevuto il messaggio))

-La consegna dei messaggi NON deve avere un ordine particolare sequenziale


# TODOs
- *Cesare* Ripulire debug ricezione
- Refactoring Device
- Refactoring Message .c/.h
- Refactoring FIFO
- Contrllare commenti
- Inserire controlli d'errore sulle syscall
- Testare adeguatamente
  - Aggiungere in server 