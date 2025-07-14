/*
 * test_apfs.cpp
 *
 * Unit tests for apfs.cpp
 *
 * Author: Taha Ebrahim
 */
#include "tsk/pool/tsk_apfs.hpp"

#include "tsk/libtsk.h"

#include "tsk/fs/tsk_apfs.hpp"
#include "tsk/fs/apfs_fs.hpp" 
#include "tsk/fs/tsk_fs_i.h"
#include "tsk/img/legacy_cache.h"

#include "catch.hpp"

#include <cstdlib>    // for std::getenv
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using img_t = std::pair<TSK_IMG_INFO *const, const TSK_OFF_T>;

TEST_CASE("test building") {
    int x = 1;
    REQUIRE(x == 1);
}
TEST_CASE("APFSPool opens from raw image") {
    char* env = getenv("SLEUTHKIT_TEST_DATA_DIR");
    REQUIRE(env != nullptr);

    // std::string path = std::string(env) + "/images/apfs_encrypted.dmg";
    std::string path = "/Users/tahaebrahim/Desktop/weird volume/copy.dmg";
    const char* image_paths[] = { path.c_str() };

    TSK_IMG_INFO* img = tsk_img_open_utf8(1, image_paths, TSK_IMG_TYPE_RAW, 512);
    REQUIRE(img != nullptr);

    // Create vector of img_t: (TSK_IMG_INFO*, offset)
    std::vector<img_t> imgs = { std::make_tuple(img, 40 * 512) };

    // Construct pool using APFS_POOL_NX_BLOCK_LAST_KNOWN_GOOD
    APFSPool pool(std::move(imgs), APFS_POOL_NX_BLOCK_LAST_KNOWN_GOOD);
    REQUIRE_NOTHROW([&]() {
        auto sb = pool.nx();
    });
    auto sb = pool.nx();

    SECTION("nx() returns valid superblock") {
        auto sb = pool.nx();
        REQUIRE(sb != nullptr);
    }

    SECTION("known_versions() returns at least one") {
        auto versions = pool.known_versions();
        REQUIRE(!versions.empty());
        REQUIRE(versions[0].xid > 0);
    }
    
    SECTION("read() can access known-good data") {
        char buf[4096];
        REQUIRE(pool.read(0, buf, sizeof(buf)) > 0);
    }
    
    SECTION("volumes() parses valid volumes") {
        auto vols = pool.volumes();
        REQUIRE(vols.size() == pool.num_vols());
    }
    
    SECTION("clear_cache() does not throw") {
        REQUIRE_NOTHROW(pool.clear_cache());
    }
    SECTION("uuid is non-null") {
        REQUIRE(!(pool.uuid() == TSKGuid()));
    }
    
    SECTION("block size is sane") {
        REQUIRE(pool.block_size() >= 4096);
        REQUIRE(pool.dev_block_size() >= 512);
    }
    
    SECTION("volume count matches num_vols") {
        auto vols = pool.volumes();
        REQUIRE(pool.num_vols() == static_cast<int>(vols.size()));
    }
    
    SECTION("first_img_offset matches expected") {
        REQUIRE(pool.first_img_offset() == 40 * 512);
    }
    tsk_img_close(img);
    
}
