# HordaCTRL
Controlador de fermentação baseado no ESP8266. duas saidas para geladeira e aquecimento.
Pode ser integrado via MQTT para visualizar e controlar
# Integraçao com Home assitant
![WhatsApp Image 2025-04-21 at 11 02 13](https://github.com/user-attachments/assets/3f98d28e-f103-45e4-bfec-ce2d34830286)
![WhatsApp Image 2025-04-21 at 11 02 11](https://github.com/user-attachments/assets/abb5f350-4ea6-4e9c-a4a6-9313fe1ff0e2)
# Montagem
## Microcontrolador.
Qualquer Modulo que use o ESP8266 pode ser utilizado, eu usei ESP12E + adpatador. mas qquer outro pode ser usado, como o NodeMCU, Wemos D1, ESP12F, ESP07 etc. contanto que o modulo tenha os pinos de IO necessarios. o Software deve funcionar com o ESP32, mas não foi testado.

https://a.aliexpress.com/_mK0qb4P

https://a.aliexpress.com/_m0WH8Bv

no caso dos modulos como ESP12, é importante ter o adpatador para facilitar a soldagem

https://a.aliexpress.com/_mPTtiX1

e tambem um conversor USB TTL (3.3V) para fazer a programação usando o Arduino IDE

https://a.aliexpress.com/_mtpfSM7

Caso voce queira ter uma bateria de Li_ion para manter a mediçaõ funcionando mesmo sem luz, voce precisa de um carregador para a bateria, eu usei um modulo TP4055.

https://a.aliexpress.com/_mNtQHND

e um regulador para baixar a tensão da bateria para 3.3V, eu usei um modulo com o AMS1117 3.3v

https://a.aliexpress.com/_mOzXOlN

Se voce usar uma placa de desenvolvimento pronta para usar no Arduino (NodeMCU, Wemos D1 etc).

https://a.aliexpress.com/_msxZrkB

voce não precisa do adptador, do conversor USB TTL , do regulador (U2). nem dos componentes S1, S2, R4 e R9
## Display
O Software está escrito para usar um display grafico OLED 128x64 via I2C.
eu usei um display de 1.3" baseado no controlador SH1106, ele é bastante comum.

https://a.aliexpress.com/_mtqo1Sb

existe tambem uma versão menos de 0.96" e a maioria é baseado no controlador SSD1306 , o software tambem supporta, basta descomentar as linhas que fazem referencia ao controlador do display (SH1106 ou SSD1306). as vezes pode ser necessario "inverter" o display dependendo da montagem, isso pode ser feito no codigo de forma muito facil

'display.flipScreenVertically()'

## Sensor de temperatura
O sensor usado é o DS1B20, que é um sensor digital I2C com boa precisão.
o Software supporta 4 sensores
1º Temperatura ambiente
2º Temperatura interna na geladeira
3º Temperatura do fermentador 1
4º Temperatura do fermentador 2
Como os sensores são todos ligados no mesmo barramento, o software não tem como identificar qual sensor é usado para cada função, então existe uma rotina que memoriza os sensores na quando o circuito é ligado, abaixo explico como configurar isso
Eu usei um sensor sem proteção para medir a temperatura ambiente (fora da geladeira) e interna (dentro da geladeira).
e sensores encapsulados em inox para os fermentadores.

Sensor sem proteção



# Licença
Este equipamento esta coberto pela licença https://creativecommons.org/licenses/by-nc-sa/4.0/
Voce pode montar e usar este equipamento, pode modificar e criar outras versões, mas precisa manter a licença original e os dados do autor.
Comercializar este produto, qualquer parte ou derivado não é permitido sem consentimento expresso do autor.
