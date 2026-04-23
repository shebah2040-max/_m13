#include "logging/AuditLogger.h"
#include "test_support.h"

#include <cstdio>
#include <filesystem>

using namespace m130::logging;

int appendsAndLinks()
{
    AuditLogger al;
    auto dir = std::filesystem::temp_directory_path();
    auto file = (dir / "m130_audit_test.log").string();
    std::filesystem::remove(file);
    M130_REQUIRE(al.open(file));
    al.setHmacKey("test-key");
    auto e1 = al.append("command", "u1", "ARM", "rocket", "ok");
    auto e2 = al.append("command", "u1", "LAUNCH", "rocket", "ok");
    M130_REQUIRE(!e1.this_hash.empty());
    M130_REQUIRE_EQ(e2.prev_hash, e1.this_hash);
    M130_REQUIRE_EQ(al.lastHash(), e2.this_hash);
    std::filesystem::remove(file);
    return 0;
}

int verifyDetectsTamper()
{
    AuditLogger al;
    auto dir = std::filesystem::temp_directory_path();
    auto file = (dir / "m130_audit_verify.log").string();
    std::filesystem::remove(file);
    M130_REQUIRE(al.open(file));
    al.setHmacKey("k");
    for (int i = 0; i < 5; ++i) {
        al.append("c", "u", "A", "t", std::to_string(i));
    }

    // Verify the intact file.
    std::size_t bad = 0;
    M130_REQUIRE(al.verifyFile(file, &bad));

    // Tamper: rewrite line 3 payload but keep hash column.
    std::ifstream in(file);
    std::vector<std::string> lines;
    std::string ln;
    while (std::getline(in, ln)) lines.push_back(ln);
    in.close();
    M130_REQUIRE(lines.size() >= 3);
    // Replace a char in the detail (6th field) of line 3 to break the mac.
    auto pos = lines[2].find("|", lines[2].find("|", lines[2].find("|", lines[2].find("|", lines[2].find("|") + 1) + 1) + 1) + 1);
    if (pos != std::string::npos && pos + 1 < lines[2].size()) {
        lines[2][pos + 1] = 'X';
    }
    std::ofstream out(file, std::ios::trunc);
    for (auto& l : lines) out << l << '\n';
    out.close();

    M130_REQUIRE(!al.verifyFile(file, &bad));
    std::filesystem::remove(file);
    return 0;
}

int run()
{
    M130_RUN(appendsAndLinks);
    M130_RUN(verifyDetectsTamper);
    return 0;
}

M130_TEST_MAIN()
