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
    static constexpr const char* Version = "0.1.1";
};

} // namespace genai
