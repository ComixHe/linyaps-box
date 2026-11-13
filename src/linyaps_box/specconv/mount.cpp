// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "linyaps_box/specconv/mount.h"

#include "linyaps_box/os/kernel_constants.h"
#include "linyaps_box/utils/utils.h"

#include <sys/mount.h>

#include <array>

namespace linyaps_box::specconv {

using namespace linyaps_box::config;

namespace {

struct vfs_flag_mapping
{
    vfs_flag config;
    unsigned long native;
};

struct propagation_flag_mapping
{
    propagation_flag config;
    unsigned long native;
};

// Single source of truth for both conversion directions.  The static_asserts
// below turn a newly declared flag into a compile error instead of a silently
// dropped mount option.
constexpr auto vfs_flag_mappings = std::array{
    vfs_flag_mapping{ vfs_flag::bind, MS_BIND },
    vfs_flag_mapping{ vfs_flag::rec, MS_REC },
    vfs_flag_mapping{ vfs_flag::ro, MS_RDONLY },
    vfs_flag_mapping{ vfs_flag::nosuid, MS_NOSUID },
    vfs_flag_mapping{ vfs_flag::nodev, MS_NODEV },
    vfs_flag_mapping{ vfs_flag::noexec, MS_NOEXEC },
    vfs_flag_mapping{ vfs_flag::noatime, MS_NOATIME },
    vfs_flag_mapping{ vfs_flag::nodiratime, MS_NODIRATIME },
    vfs_flag_mapping{ vfs_flag::relatime, MS_RELATIME },
    vfs_flag_mapping{ vfs_flag::strictatime, MS_STRICTATIME },
    vfs_flag_mapping{ vfs_flag::sync, MS_SYNCHRONOUS },
    vfs_flag_mapping{ vfs_flag::dirsync, MS_DIRSYNC },
    vfs_flag_mapping{ vfs_flag::iversion, MS_I_VERSION },
    vfs_flag_mapping{ vfs_flag::lazytime, MS_LAZYTIME },
    vfs_flag_mapping{ vfs_flag::mand, MS_MANDLOCK },
    vfs_flag_mapping{ vfs_flag::silent, MS_SILENT },
    vfs_flag_mapping{ vfs_flag::nosymfollow, linyaps_box::os::sys::ms_nosymfollow },
    vfs_flag_mapping{ vfs_flag::remount, MS_REMOUNT },
};

constexpr auto propagation_flag_mappings = std::array{
    propagation_flag_mapping{ propagation_flag::private_, MS_PRIVATE },
    propagation_flag_mapping{ propagation_flag::slave, MS_SLAVE },
    propagation_flag_mapping{ propagation_flag::shared, MS_SHARED },
    propagation_flag_mapping{ propagation_flag::unbindable, MS_UNBINDABLE },
    propagation_flag_mapping{ propagation_flag::rec, MS_REC },
};

[[nodiscard]] constexpr auto mapped_vfs_flags() noexcept -> utils::bitflags<vfs_flag>
{
    utils::bitflags<vfs_flag> mask{ };
    for (const auto &entry : vfs_flag_mappings) {
        mask |= entry.config;
    }
    return mask;
}

[[nodiscard]] constexpr auto mapped_propagation_flags() noexcept
  -> utils::bitflags<propagation_flag>
{
    utils::bitflags<propagation_flag> mask{ };
    for (const auto &entry : propagation_flag_mappings) {
        mask |= entry.config;
    }
    return mask;
}

static_assert(mapped_vfs_flags() == utils::bitflags<vfs_flag>::all(),
              "every vfs_flag needs an OS mapping in vfs_flag_mappings");
static_assert(mapped_propagation_flags() == utils::bitflags<propagation_flag>::all(),
              "every propagation_flag needs an OS mapping in propagation_flag_mappings");

} // namespace

auto to_vfs_flags(utils::bitflags<vfs_flag> flags) noexcept -> unsigned long
{
    unsigned long out{ 0 };
    for (const auto &entry : vfs_flag_mappings) {
        if (flags.contains(entry.config)) {
            out |= entry.native;
        }
    }
    return out;
}

auto to_propagation_flags(utils::bitflags<propagation_flag> flags) noexcept -> unsigned long
{
    unsigned long out{ 0 };
    for (const auto &entry : propagation_flag_mappings) {
        if (flags.contains(entry.config)) {
            out |= entry.native;
        }
    }
    return out;
}

auto to_rootfs_propagation(rootfs_propagation prop) noexcept -> unsigned long
{
    switch (prop) {
    case rootfs_propagation::private_:
        return MS_PRIVATE;
    case rootfs_propagation::shared:
        return MS_SHARED;
    case rootfs_propagation::slave:
        return MS_SLAVE;
    case rootfs_propagation::unbindable:
        return MS_UNBINDABLE;
    }

    utils::unreachable("unknown propagation type");
}

auto from_vfs_flags(unsigned long ms) noexcept -> utils::bitflags<vfs_flag>
{
    utils::bitflags<vfs_flag> out{ };
    for (const auto &entry : vfs_flag_mappings) {
        if ((ms & entry.native) != 0) {
            out |= entry.config;
        }
    }
    return out;
}

} // namespace linyaps_box::specconv
