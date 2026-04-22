Simple DirectMedia Layer (SDL) 3.0 — Mali FBDEV (S905X)

https://www.libsdl.org/

Simple DirectMedia Layer (SDL) é uma biblioteca multiplataforma voltada para acesso de baixo nível a áudio, entrada (teclado, mouse, joystick) e vídeo. É amplamente utilizada por players de vídeo, emuladores e diversos jogos.

Modificação: Suporte Mali-450 (FBDEV)

Esta build inclui uma implementação customizada para GPUs Mali-450 utilizando framebuffer (fbdev), com foco em dispositivos baseados no S905X.

Destaques

- SDL3 rodando nativamente no Mali-450
- Driver Mali ativado e integrado ao SDL3
- Backend baseado em fbdev adaptado para kernel 3.14
- Compatibilidade com dispositivos S905 / S905X com 1GB de RAM
- Execução de ports via PortMaster utilizando SDL3
- Alto nível de compatibilidade em testes práticos

Testes

Foram desenvolvidos e executados testes completos, incluindo:

- Criação e gerenciamento de janelas via EGL
- Testes de cores e renderização
- Criação e exibição de imagens
- Testes práticos em jogos
- Testes em todo o sistema Next OS
- Integração e validação com SDL2 (compat) e SDL3

Testado com sucesso em:

- Street Fighter III: 3rd Strike (PS2 Port)
- Street Fighter Anniversary Collection (PS2 Port)

Ambos rodando próximos de 100% de desempenho utilizando SDL3.

Observações

- Esta não é uma implementação oficial do SDL
- Desenvolvida especificamente para hardware legado (Mali-450 + kernel 3.14)
- Pode exigir ajustes adicionais dependendo da ROM ou build utilizada

Status

- Em fase final de testes
- Correções em andamento após atualização da toolchain (2026)
- Lançamento público previsto em breve

Contribuição

Possível integração futura com o projeto EmuELEC ou distribuição para a comunidade.