#pragma once

/**
 * \file App.h
 * \brief Declaração da classe principal da aplicação.
 */

namespace genai {

/**
 * \brief Informações básicas da aplicação (versão, nome).
 */
class AppInfo {
public:
    /** \brief Nome do aplicativo. */
    static constexpr const char* Name = "GenAI E-Book Reader";

    /** \brief Versão corrente do aplicativo. Sincronizada com CMake/README/Doxygen. */
        static constexpr const char* Version = "0.1.8";

    /** \brief Descrição do aplicativo. */
    static constexpr const char* Description = "GenAI E-Book Reader é um leitor de e-books com recursos de IA integrados para aprimorar a experiência de leitura.";
};


/**
 * \brief Informações dos desenvolvedores.
 */
struct DeveloperInfo {
    const char* name;
    const char* email;
    const char* site;
};

/** \brief Lista de desenvolvedores. */
static const DeveloperInfo Developers[] = {
    {"Carlos Delfino", "consultoria@carlosdelfino.eti.br", "https://www.rapport.com.br"},
    {"Beto Byte", "betobyte@gmail.com", "https://ibova.com.br"}
};

} // namespace genai
