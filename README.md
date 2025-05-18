# HordaCTRL
Controlador de fermentação baseado no ESP8266. duas saídas, Compressor da geladeira e aquecimento.
Pode ser integrado via MQTT para visualizar e controlar
# Integração com Home assitant
![WhatsApp Image 2025-05-17 at 23 09 19](https://github.com/user-attachments/assets/3785a076-1423-47dd-9591-e21577557605)![WhatsApp Image 2025-04-21 at 11 02 11](https://github.com/user-attachments/assets/abb5f350-4ea6-4e9c-a4a6-9313fe1ff0e2)
# Montagem
**Todos os links são apenas referencias, podem ser usados quaisquer equivalentes encontrados no mercado nacional**
## Microcontrolador.
Qualquer Modulo que use o ESP8266 pode ser utilizado, eu usei ESP12E + adaptador. mas qualquer outro pode ser usado, como o NodeMCU, Wemos D1, ESP12F, ESP07 etc. contanto que o modulo tenha os pinos de IO necessários. o Software deve funcionar com o ESP32, mas não foi testado.  
https://a.aliexpress.com/_mK0qb4P , https://a.aliexpress.com/_m0WH8Bv  
no caso dos módulos como ESP12, é importante ter o adaptador para facilitar a soldagem  
https://a.aliexpress.com/_mPTtiX1  
e também um conversor USB TTL (3.3V) para fazer a programação usando o Arduino IDE  
https://a.aliexpress.com/_mtpfSM7  
Caso você queira ter uma bateria de Li_ion para manter a medição funcionando mesmo sem luz, você precisará de um carregador para a bateria, eu usei um módulo TP4055.  
https://a.aliexpress.com/_mNtQHND  
e um regulador para baixar a tensão da bateria para 3.3V, eu usei um modulo com o AMS1117 3.3v  
https://a.aliexpress.com/_mOzXOlN  
Se você usar uma placa de desenvolvimento pronta para usar no Arduino (NodeMCU, Wemos D1 etc).  
https://a.aliexpress.com/_msxZrkB  
voce não precisa do adaptador, do conversor USB TTL , do regulador (U2). nem dos componentes S1, S2, R4 e R9
## Display
O Software está escrito para usar um display gráfico OLED 128x64 via I2C.
eu usei um display de 1.3" baseado no controlador SH1106, ele é bastante comum.  
https://a.aliexpress.com/_mtqo1Sb  
existe também uma versão menor de 0.96" e a maioria é baseado no controlador SSD1306 , essa versão pode ser usada, basta descomentar as linhas que fazem referência ao controlador do display SSD1306 e comentar as que fazem referência ao SH1106. as vezes pode ser necessário "inverter" o display dependendo da montagem, isso pode ser feito no código de forma muito fácil, comente a linha abaixo no modulo setup() e o display será invertido na vertical.  
```
display.flipScreenVertically()
```
O Display é dividido em 4 sessões.\
- barra de status no alto a direita, mostra, da direita para a esquerda a intensidade do sinal WiFi, se está conectado ao servidor MQTT, se está gelando, aquecendo ou ambos desligados e a carga da bateria\
- Temperatura ajustada.  No alto a esquerda\ 
- Temperatura atual do sensor principal, parte inferior a direita (normalmente sensor do fermentador 1)\
- Demais temperaturas, parte inferior a esquerda (A = temperatura ambiente, I = Temperatura interna da geladeira, 2 = Temperatura do sensor do segundo fermentador)
## Sensor de temperatura
O sensor usado é o DS18B20, que é um sensor digital I2C com boa precisão.
o Software suporta 4 sensores
 - 1º Temperatura ambiente
 - 2º Temperatura interna na geladeira
 - 3º Temperatura do fermentador 1
 - 4º Temperatura do fermentador 2

Como os sensores são todos ligados no mesmo barramento, o software não tem como identificar qual sensor é usado para cada função, existe uma rotina que memoriza os sensores quando o circuito é ligado, Mais abaixo explico como configurar os sensores.\
Eu usei sensores sem proteção para medir a temperatura ambiente, instalado fora da geladeira, e interna, instalado dentro da geladeira (https://a.aliexpress.com/_mLc1ivV) .
e sensores encapsulados em inox e a prova d'agua para os fermentadores (https://a.aliexpress.com/_m07BU6f).
![WhatsApp Image 2025-04-25 at 14 47 26](https://github.com/user-attachments/assets/3a46b935-dc83-44da-a4e7-06d64d4befc1)  
Sensor sem proteção https://a.aliexpress.com/_mLc1ivV  
Sensor a prova d'agua https://a.aliexpress.com/_m07BU6f 
## Encoder
Para ajustar a temperatura foi usado um encoder rotativo (potenciômetro infinito)
https://a.aliexpress.com/_msf4IYL
## Acionamento do compressor da geladeira e da resistência de aquecimento
As saídas do circuito são do tipo open collector. Com o transístor indicado no esquema, podem controlar 600mA e 40V. No conector alem das saídas OC, temos 3.3v, isso é suficiente para acionar relês de estado solido (SSR) e controlar as cargas 110 ou 220VAC, podem ser usados relês mecânicos, mas alguma adaptação pode ser necessária.  
https://a.aliexpress.com/_mKYsubD
# Instalando o Firmware
Use o arduino IDE, instale as ferramentas para ESP8266. tem vários tutoriais na internet, mas basta seguir os passos abaixo:  
- Adicione a url https://arduino.esp8266.com/stable/package_esp8266com_index.json como gerenciador de placas adicional, use o caminho Arquivo/Preferencias/Gerenciadores de placas.
- Abra o Gerenciador de placas e instale a plataforma ESP8266
- Configure o IDE conforme sua placa, se usar o ESP12E, as configurações principais estão no esquema elétrico.
- Conecte a placa à USB
- Selecione a porta
- Compile e faça o upload
- Depois disso caso o circuito esteja conectado ao WiFi, voce pode usar o arduino IDE para gravar atualizações via WiFi 
# Uso
## Primeiro uso após a montagem
No primeiro uso é necessário configurar os sensores. para garantir o funcionamento correto, siga os passos abaixo:
1. Mantenha apenas o sensor de temperatura ambiente conectado e ligue o circuito, quando a temperatura aparece no display, desligue o circuito
2. Mantendo o sensor de temperatura ambiente, conecte o sensor de temperatura interna da geladeira, ligue e aguarde as temperaturas aparecerem na tela, desligue o circuito
3. Mantendo os dois anteriores conectados, conecte o sensor do fermentador 1 (sensor principal) e ligue o circuito, aguarde até as temperaturas aparecerem no display e desligue
4. Mantendo todos os anteriores conecte o sensor do fermentador 2 e ligue o circuito. ele está pronto para uso\

**Obs:**  
a) O Fermentador 2 é opcional, se não for usar, pode desconsiderar o passo 4\
b) Caso no momento de ligar o sensor do fermentador 1 não estiver conectado, a regulação acontecerá pelo sensor de temperatura interna, que passará a aparecer como sensor principal\
c) a avaliação dos sensores é feita sempre que o circuito for ligado, então se você trocar o sensor do fermentador 1, basta ligar o circuito com ele conectado (sem o sensor Ferm2) que vai funcionar. mas caso voce troque o sensor de Temperatura ambiente ou interna, precisará refazer os passo para garantir que os sensores sejam memorizados corretamente\
d) todo o processo acima é "logado" via interface serial, caso tenha alguma dúvida você pode acompanhar com um  cabo serial (ou USB) e um programa de terminal (ou o próprio monitor do Arduino IDE).  
## Ajustando a temperatura
Para ajustar a temperatura, pressione o botão até ouvir o bip, gire o botão no sentido horário para aumentar a temperatura, ou anti-horário para diminuir, os incrementos são sempre de 0.5 graus Celsius. Quando estiver ajustando, a temperatura atual irá para o canto superior a esquerda e a temperatura ajustada aparecerá na parte principal da tela. pressione novamente o botão para sair do ajuste e gravar a nova temperatura. caso o botão de ajuste não seja movido pro mais de 10 segundos o controlador sai do modo de ajuste, gravando o valor que está no display.
## Conectando a Rede Wifi e Servidor MQTT
Não é necessário conectar a uma rede WiFi, nem tampouco a um servidor MQTT. conectando à rede, você pode atualizar o firmware usando o Arduino OTA, conectado ao um servidor MQTT voce pode receber status e configurar a temperatura via servidor.
Para comfigurar, desligue o circuito, mantenha pressionado o botão e ligue. o Display mostrara que está em modo de configuração e mostrara a Rede e endereço para conectar.
conecte um Celular, tablet ou PC a Rede mostrada, acesse a pagina indicada.
Configure o WiFi a ser utilizado e senha. configure tambem o Servidor MQTT , usuário e senha do servidor
O Controlador enviará mensagens de status para o servidor, e aceitará mensagem de configuração. Caso você tenha HomeAssistant voce pode criar um "device" do tipo Climate e controlar a geladeira. um exemplo de yaml para o device está nos arquivos do projeto. Atenção, a comunicação MQTT não é criptografada, somente use servidores em sua rede interna, não na internet pública.
# Licença e Garantias
Este equipamento está coberto pela licença https://creativecommons.org/licenses/by-nc-sa/4.0/  
Você pode montar e usar este equipamento, pode modificar e criar outras versões, mas precisa manter a licença original e os dados do autor.  
**Comercializar este produto, qualquer parte ou derivado não é permitido sem consentimento expresso do autor.**  
O compartilhamento destas informações tem caráter informativo apenas, este não é um produto pronto sob os aspectos de uso ou segurança. ao usar as informações compartilhadas você aceita qualquer risco e entende que não é fornecido nenhum tipo de garantia ou supporte.
