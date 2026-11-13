// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "linyaps_box/config/linux.h"
#include "linyaps_box/config/mount.h"
#include "linyaps_box/utils/enum_traits.h"

namespace linyaps_box::specconv {

[[nodiscard]] auto to_vfs_flags(utils::bitflags<config::vfs_flag> flags) noexcept -> unsigned long;

[[nodiscard]] auto to_propagation_flags(utils::bitflags<config::propagation_flag> flags) noexcept
  -> unsigned long;

[[nodiscard]] auto to_rootfs_propagation(config::rootfs_propagation prop) noexcept -> unsigned long;

[[nodiscard]] auto from_vfs_flags(unsigned long ms) noexcept -> utils::bitflags<config::vfs_flag>;

} // namespace linyaps_box::specconv
