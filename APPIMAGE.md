# O que é um AppImage?

AppImage é um formato para distribuir aplicações portáteis em Linux sem a necessidade de permissões de superusuário para instalar a aplicação. Ele permite que os usuários baixem uma aplicação, a tornem executável e a executem em praticamente qualquer distribuição Linux. A aplicação é executada em um ambiente contido, com todas as suas dependências incluídas no próprio arquivo.

## Vantagens de usar um AppImage

- **Portabilidade:** Funciona na maioria das distribuições Linux modernas (por exemplo, Ubuntu, Fedora, openSUSE, CentOS, etc.).
- **Sem instalação:** Basta tornar o arquivo executável e executá-lo. Não há necessidade de extrair ou instalar nada no sistema.
- **Não requer permissões de root:** Você pode executar a aplicação a partir de qualquer diretório em que tenha permissão de escrita, como sua pasta `home`.
- **Autocontido:** Todas as dependências necessárias para a execução da aplicação estão incluídas no arquivo AppImage. Isso evita problemas de "inferno de dependências".
- **Fácil de remover:** Para desinstalar, basta apagar o arquivo AppImage.

## Como Baixar e Usar

1.  **Faça o download do arquivo:**
    Vá para a [página de releases do projeto](https://github.com/Rapport-Tecnologia/GenAi-E-Book-Reader/releases) e baixe o arquivo `.AppImage` mais recente.

2.  **Torne o arquivo executável:**
    Abra um terminal, navegue até o diretório onde você salvou o arquivo e execute o seguinte comando:
    ```bash
    chmod a+x GenAi-E-Book-Reader-*.AppImage
    ```

3.  **Execute a aplicação:**
    Agora você pode executar a aplicação de duas maneiras:

    - **Pelo terminal:**
      ```bash
      ./GenAi-E-Book-Reader-*.AppImage
      ```

    - **Pela interface gráfica:**
      Dê um duplo clique no arquivo `.AppImage` no seu gerenciador de arquivos.

E pronto! A aplicação será iniciada.
