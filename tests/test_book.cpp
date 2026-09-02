#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "core/models/book.hpp"

using namespace libsorter::core;

TEST_CASE("Book serialization") {
    Book b;
    b.id = "123";
    b.title = "Test Book";
    b.authors = {"Author One"};
    b.year = 2023;
    b.genre = "Fiction";
    b.pages = 300;
    b.weight = 450;
    b.location_id = "A1";
    b.status = BookStatus::Available;
    b.borrowed_by = "";
    b.tags = {"tag1"};
    b.created_at = std::chrono::system_clock::now();
    b.updated_at = std::chrono::system_clock::now();

    nlohmann::json j = b;
    CHECK(j["id"] == "123");
    CHECK(j["title"] == "Test Book");
    CHECK(j["status"] == "available");
    CHECK(j["genre"] == "Fiction");
    CHECK(j["weight"] == 450);
    CHECK(j["borrowed_at"].is_null());

    Book b2 = j.get<Book>();
    CHECK(b2.id == b.id);
    CHECK(b2.title == b.title);
    CHECK(b2.status == b.status);
    CHECK(b2.genre == b.genre);
    CHECK(b2.weight == b.weight);
}

TEST_CASE("Book status conversion") {
    CHECK(status_to_string(BookStatus::Available) == "available");
    CHECK(status_to_string(BookStatus::Borrowed) == "borrowed");
    CHECK(status_to_string(BookStatus::Lost) == "lost");

    CHECK(status_from_string("available") == BookStatus::Available);
    CHECK(status_from_string("borrowed") == BookStatus::Borrowed);
    CHECK(status_from_string("lost") == BookStatus::Lost);
    CHECK(status_from_string("unknown") == BookStatus::Available);
}
