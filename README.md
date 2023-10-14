Комплект из двух программ, одна из которых выполняет функции сервера, а вторая выполняет функции клиента.    
Клиент, после установления соединения с сервером, посылает серверу запросы с периодом 1 сек.    
Сервер принимает запросы, обрабатывает их и отправляет ответы клиенту.  
Клиент принимает ответы и выводит их на экран.  
В программе использоуются функции работы с сокетами без блокировки  
Поддерживается следующий порядок запросов клиента и ответов сервера:  
запрос 1; ответ 1; запрос 2; ответ 2; … ; запрос i; ответ i; …    
Для поддержания такого порядка,  на сервере используется очередь, принимаемые запросы помещаются в конец этой очереди. Затем выбирать из начала очереди запрос, обрабатывать его и отправлять ответ клиенту.  
Очередь запросов на обработку является критическим ресурсом, поэтому работа с очередью  производится с помощью мьютексов.  