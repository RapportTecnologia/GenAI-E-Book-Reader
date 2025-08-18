#pragma once

/**
 * \file Reader.h
 * \brief Interface mínima para o leitor de e-books (MVP).
 */

#include <string>
#include <cstdint>

namespace genai {

/**
 * \brief Representa uma posição no livro (página ou índice lógico).
 */
struct Location {
    std::uint32_t page = 0; ///< Página atual (se aplicável)
};

/**
 * \brief Resultado mínimo de abertura/carregamento de um livro.
 */
struct OpenResult {
    bool ok = false;                 ///< Se a abertura foi bem-sucedida
    std::string message;             ///< Mensagem de erro/diagnóstico
    std::uint32_t totalPages = 0;    ///< Número de páginas estimado (se aplicável)
};

/**
 * \brief Interface do leitor de e-books do MVP.
 */
class Reader {
public:
    virtual ~Reader() = default;

    /** \brief Abre um arquivo de e-book (PDF/EPUB - placeholder). */
    virtual OpenResult open(const std::string& filePath) = 0;

    /** \brief Navega para a posição solicitada (página). */
    virtual bool goTo(const Location& loc) = 0;

    /** \brief Obtém a localização atual. */
    virtual Location current() const = 0;
};

/**
 * \brief Implementação de placeholder (sem renderização real).
 */
class DummyReader : public Reader {
    Location loc_{};
    std::uint32_t total_{0};
public:
    OpenResult open(const std::string& filePath) override {
        (void)filePath;
        total_ = 100; // placeholder
        loc_.page = 1;
        return {true, "opened (dummy)", total_};
    }
    bool goTo(const Location& loc) override {
        if (loc.page == 0 || loc.page > total_) return false;
        loc_ = loc; return true;
    }
    Location current() const override { return loc_; }
};

} // namespace genai
