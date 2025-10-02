# Diretrizes do Web Flasher YUMA

## Propósito
Este diretório contém o web flasher baseado em esptool-js para flashing de firmware YUMA em dispositivos ESP32 e ESP8266 diretamente pelo browser.

## Estrutura de Arquivos
- `index.html` - Interface completa do web flasher com esptool-js integrado
- Acesso aos assets de firmware via `../assets/firmware/` e `../assets/manifest.json`

## Tecnologias Utilizadas
- **esptool-js**: Biblioteca JavaScript para flashing via Web Serial API
- **Web Serial API**: Comunicação serial diretamente do browser (Chrome/Edge v89+)
- **ES6 Modules**: Importação moderna da biblioteca esptool-js
- **Responsive Design**: Interface adaptável para desktop e mobile

## Funcionalidades Implementadas
- Seleção de chip (ESP32/ESP8266) e variante (standard/oled)
- Conexão automática via Web Serial API com detecção de chip
- Flash de firmware com barra de progresso e logs em tempo real
- Erase completo da flash memory
- Console de debugging com timestamps
- Verificação de compatibilidade de browser
- Status indicators visuais (success/error/info)

## Compatibilidade
- **Browsers**: Chrome/Edge v89+ (Web Serial API requerido)
- **Protocolo**: HTTPS ou localhost (contexto seguro obrigatório)
- **Dispositivos**: ESP32, ESP8266 com drivers USB-Serial (CP210x, CH340, FT232)

## Integração com Build System
- Usa manifest.json gerado pelo `make manifest`
- Carrega firmwares da estrutura `assets/firmware/{chip}/{variant}/`
- Compatível com offsets de flash corretos para cada chip
- Sincronizado com versionamento git do projeto

## Servidor de Desenvolvimento
```bash
make serve  # Inicia servidor em http://localhost:8000
```

## Fluxo de Uso
1. Usuário acessa http://localhost:8000
2. Seleciona chip (ESP32/ESP8266) e variante (standard/oled)
3. Clica "Connect Device" - browser abre diálogo de seleção de porta
4. Conecta automaticamente e detecta chip
5. Clica "Flash Firmware" - carrega automaticamente o firmware correto
6. Progresso mostrado em tempo real até conclusão

## Considerações de Segurança
- Web Serial API só funciona em contexto seguro (HTTPS/localhost)
- Usuário precisa autorizar acesso à porta serial explicitamente
- Firmwares carregados localmente (sem upload de arquivos externos)
- Verificação de integridade via detecção automática de chip

## Debugging
- Console JavaScript com logs detalhados de todas operações
- Verificação automática de carregamento da biblioteca esptool-js
- Status de Web Serial API e contexto seguro
- Logs de progresso durante flash com timestamps

## Referências de Projetos
- **esptool-js**: https://github.com/espressif/esptool-js - Biblioteca oficial JavaScript do Espressif para flashing via Web Serial API
- **diyflasher**: https://github.com/valerio-vaccaro/diyflasher - Implementação de referência para web flasher ESP32/ESP8266

## Manutenção
- Manter esptool-js atualizado via CDN unpkg.com
- Verificar compatibilidade com novas versões do Chrome/Edge
- Atualizar offsets de flash se mudanças no PlatformIO
- Sincronizar com mudanças na estrutura de assets do projeto