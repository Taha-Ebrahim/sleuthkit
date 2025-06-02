#include "catch.hpp"
#include "tsk/fs/tsk_fs_i.h"
#include "tsk/fs/tsk_fs.h"
#include <cstring>
#include <string>

//Only one non-static function in tsk/fs/fls_lib.cpp 

// Helper function fr opening fs from image
static TSK_FS_INFO* open_test_fs() {
    const char *img_path = "test/data/image/image.dd";
    TSK_IMG_INFO *img = tsk_img_open_sing(img_path, TSK_IMG_TYPE_DETECT, 0);
    if (!img) {
        WARN("Could not open test image: " << img_path << ". Skipping test.");
        return nullptr;
    }
    TSK_FS_INFO *fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
    if (!fs) {
        tsk_img_close(img);
        WARN("Could not open filesystem in image: " << img_path << ". Skipping test.");
        return nullptr;
    }
    return fs;
}

// Test tsk_fs_fls with a null prefix (tpre is nullptr)
TEST_CASE("tsk_fs_fls_null_tpre", "[fls]") {
    TSK_FS_INFO *fs = open_test_fs();
    if (!fs) {
        return;
    }
    
    uint8_t result = tsk_fs_fls(fs, (TSK_FS_FLS_FLAG_ENUM)0, 2, TSK_FS_DIR_WALK_FLAG_ALLOC, nullptr, 0);
    REQUIRE(result == 0);
    
    tsk_fs_close(fs);
}

// Test tsk_fs_fls with an empty string as prefix
TEST_CASE("tsk_fs_fls_empty_tpre", "[fls]") {
    TSK_FS_INFO *fs = open_test_fs();
    if (!fs) {
        return;
    }
    
    const char *empty = "";
    uint8_t result = tsk_fs_fls(fs, (TSK_FS_FLS_FLAG_ENUM)0, 2, TSK_FS_DIR_WALK_FLAG_ALLOC, empty, 0);
    REQUIRE(result == 0);
    
    tsk_fs_close(fs);
}

// Test tsk_fs_fls with a non-empty prefix string
TEST_CASE("tsk_fs_fls_nonempty_tpre", "[fls]") {
    TSK_FS_INFO *fs = open_test_fs();
    if (!fs) {
        return;
    }
    
    const char *pre = "prefix";
    uint8_t result = tsk_fs_fls(fs, (TSK_FS_FLS_FLAG_ENUM)0, 2, TSK_FS_DIR_WALK_FLAG_ALLOC, pre, 0);
    REQUIRE(result == 0);
    
    tsk_fs_close(fs);
}

// Test tsk_fs_fls with disk image file and the root inode
TEST_CASE("tsk_fs_fls_integration_image_dd", "[fls][integration]") {
    const char *img_path = "test/data/image/image.dd";
    TSK_IMG_INFO *img = tsk_img_open_sing(img_path, TSK_IMG_TYPE_DETECT, 0);
    if (!img) {
        WARN("Could not open test image: " << img_path << ". Skipping test.");
        return; // image not available
    }
    TSK_FS_INFO *fs = tsk_fs_open_img(img, 0, TSK_FS_TYPE_DETECT);
    if (!fs) {
        tsk_img_close(img);
        WARN("Could not open filesystem in image: " << img_path << ". Skipping test.");
        return;
    }
    TSK_INUM_T root_inum = 2;
    uint8_t result = tsk_fs_fls(fs, (TSK_FS_FLS_FLAG_ENUM)0, root_inum, (TSK_FS_DIR_WALK_FLAG_ENUM)0, nullptr, 0);
    REQUIRE(result == 0);
    tsk_fs_close(fs);
    tsk_img_close(img);
} 