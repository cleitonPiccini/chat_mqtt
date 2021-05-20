
CHAT utilizando o Protocolo MQTT.

Broker = Mosquitto
Biblioteca de Clients = paho.c

Requisitos:

Broker Mosquitto instalado e rodando na porta 1883.
Biblioteca paho C instalada.

Compile o arquivo main.c utilizando lpaho-mqtt3cs e lpthread.

Utilizando o CHAT:

Execute a aplicação.

Escolha seu usuário entre as 4 opções de usuários, ou escolha sair da aplicação. Caso o usuário já esteja em uso a aplicação será fechada.

Após escolher seu usuário escolha entre as opções:
    1 - Conversa particular com outro usuário.    
        1.1 - Informe com qual usuário deseja conversar
        1.2 - Aguarde a resposta do outro usuário.
            1.2.1 - Será enviado 5 mensagens de convite para o usuário escolhido anteriormente. Em caso de nenhuma resposta a aplicação volta ao menu.
            1.2.2 - Caso a resposta seja sim, o CHAT para conversa entre os dois será iniciado.
                1.2.2.1 - Digite a qualquer momento --Sair, para voltar ao menu.
            1.2.3 - Caso a resposta seja não, a aplicação será redirecionada para o menu de opções novamente.

    2 - Conversa em grupo com todos os usuários online.
        2.1 - Ao escolher esta opção o CHAT com todos os usuários será iniciado.
            2.1.1 - Digite a qualquer momento --Sair, para voltar ao menu.

    3 - Aguardar o convite de outro usuário para uma conversa particular.
        3.1 - A aplicação ficará no aguardo de um convite para conversa.
        3.2 - Ao receber um convite você poderá recusar ou aceitar a conversa com outro usuário. 
            3.2.1 - Caso aceite, um CHAT de mensagens será iniciado.
            3.2.2 - Caso recuse, irá voltar ao menu.
            3.2.3 - Digite a qualquer momento --Sair, para voltar ao menu.

    5 - Sair da aplicação.

