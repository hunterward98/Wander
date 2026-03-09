#include "wander/physics/physics.h"
#include "wander/core/math.h"
#include "wander/core/log.h"
#include <algorithm>
#include <cmath>

namespace wander {

// Standalone collision tests
bool aabb_overlaps(Rect a, Rect b) {
    return a.overlaps(b);
}

bool circle_overlaps(Vec2 c1, f32 r1, Vec2 c2, f32 r2) {
    return distance_sq(c1, c2) < (r1 + r2) * (r1 + r2);
}

bool aabb_circle_overlaps(Rect box, Vec2 center, f32 radius) {
    f32 closest_x = clamp(center.x, box.x, box.right());
    f32 closest_y = clamp(center.y, box.y, box.bottom());
    f32 dx = center.x - closest_x;
    f32 dy = center.y - closest_y;
    return (dx * dx + dy * dy) < (radius * radius);
}

PhysicsWorld::CellKey PhysicsWorld::cell_key(i32 cx, i32 cy) const {
    return (static_cast<i64>(cx) << 32) | static_cast<i64>(static_cast<u32>(cy));
}

void PhysicsWorld::init(f32 cell_size) {
    cell_size_ = cell_size;
    clear();
}

void PhysicsWorld::clear() {
    colliders_.clear();
    grid_.clear();
    active_triggers_.clear();
}

void PhysicsWorld::insert_to_grid(u32 entity_id, const Collider& col) {
    Rect bounds = col.aabb;
    if (col.type == ColliderType::Circle) {
        bounds = {col.center.x - col.radius, col.center.y - col.radius,
                  col.radius * 2, col.radius * 2};
    }

    i32 min_cx = static_cast<i32>(std::floor(bounds.x / cell_size_));
    i32 min_cy = static_cast<i32>(std::floor(bounds.y / cell_size_));
    i32 max_cx = static_cast<i32>(std::floor(bounds.right() / cell_size_));
    i32 max_cy = static_cast<i32>(std::floor(bounds.bottom() / cell_size_));

    for (i32 cy = min_cy; cy <= max_cy; cy++) {
        for (i32 cx = min_cx; cx <= max_cx; cx++) {
            grid_[cell_key(cx, cy)].push_back(entity_id);
        }
    }
}

void PhysicsWorld::remove_from_grid(u32 entity_id, const Collider& col) {
    Rect bounds = col.aabb;
    if (col.type == ColliderType::Circle) {
        bounds = {col.center.x - col.radius, col.center.y - col.radius,
                  col.radius * 2, col.radius * 2};
    }

    i32 min_cx = static_cast<i32>(std::floor(bounds.x / cell_size_));
    i32 min_cy = static_cast<i32>(std::floor(bounds.y / cell_size_));
    i32 max_cx = static_cast<i32>(std::floor(bounds.right() / cell_size_));
    i32 max_cy = static_cast<i32>(std::floor(bounds.bottom() / cell_size_));

    for (i32 cy = min_cy; cy <= max_cy; cy++) {
        for (i32 cx = min_cx; cx <= max_cx; cx++) {
            auto& cell = grid_[cell_key(cx, cy)];
            cell.erase(std::remove(cell.begin(), cell.end(), entity_id), cell.end());
        }
    }
}

void PhysicsWorld::add(const Collider& collider) {
    colliders_[collider.entity_id] = collider;
    insert_to_grid(collider.entity_id, collider);
}

void PhysicsWorld::remove(u32 entity_id) {
    auto it = colliders_.find(entity_id);
    if (it == colliders_.end()) return;
    remove_from_grid(entity_id, it->second);
    colliders_.erase(it);
}

void PhysicsWorld::update_position(u32 entity_id, Vec2 new_pos) {
    auto it = colliders_.find(entity_id);
    if (it == colliders_.end()) return;

    remove_from_grid(entity_id, it->second);

    if (it->second.type == ColliderType::AABB) {
        it->second.aabb.x = new_pos.x;
        it->second.aabb.y = new_pos.y;
    } else {
        it->second.center = new_pos;
    }

    insert_to_grid(entity_id, it->second);
}

std::vector<Collision> PhysicsWorld::check_collisions() {
    std::vector<Collision> result;
    // Track checked pairs to avoid duplicates
    std::vector<std::pair<u32, u32>> checked;

    for (auto& [id_a, col_a] : colliders_) {
        // Get cells this collider occupies
        Rect bounds_a = col_a.aabb;
        if (col_a.type == ColliderType::Circle) {
            bounds_a = {col_a.center.x - col_a.radius, col_a.center.y - col_a.radius,
                        col_a.radius * 2, col_a.radius * 2};
        }

        i32 min_cx = static_cast<i32>(std::floor(bounds_a.x / cell_size_));
        i32 min_cy = static_cast<i32>(std::floor(bounds_a.y / cell_size_));
        i32 max_cx = static_cast<i32>(std::floor(bounds_a.right() / cell_size_));
        i32 max_cy = static_cast<i32>(std::floor(bounds_a.bottom() / cell_size_));

        for (i32 cy = min_cy; cy <= max_cy; cy++) {
            for (i32 cx = min_cx; cx <= max_cx; cx++) {
                auto cell_it = grid_.find(cell_key(cx, cy));
                if (cell_it == grid_.end()) continue;

                for (u32 id_b : cell_it->second) {
                    if (id_a >= id_b) continue;  // Skip self and duplicates

                    // Check if already tested
                    auto pair = std::make_pair(id_a, id_b);
                    if (std::find(checked.begin(), checked.end(), pair) != checked.end())
                        continue;
                    checked.push_back(pair);

                    auto& col_b = colliders_[id_b];

                    // Layer mask check
                    if (!(col_a.layer & col_b.mask) && !(col_b.layer & col_a.mask))
                        continue;

                    // Collision test
                    bool overlapping = false;
                    if (col_a.type == ColliderType::AABB && col_b.type == ColliderType::AABB) {
                        overlapping = aabb_overlaps(col_a.aabb, col_b.aabb);
                    } else if (col_a.type == ColliderType::Circle && col_b.type == ColliderType::Circle) {
                        overlapping = circle_overlaps(col_a.center, col_a.radius,
                                                      col_b.center, col_b.radius);
                    } else if (col_a.type == ColliderType::AABB && col_b.type == ColliderType::Circle) {
                        overlapping = aabb_circle_overlaps(col_a.aabb, col_b.center, col_b.radius);
                    } else {
                        overlapping = aabb_circle_overlaps(col_b.aabb, col_a.center, col_a.radius);
                    }

                    if (overlapping) {
                        Collision c;
                        c.entity_a = id_a;
                        c.entity_b = id_b;

                        // Simple AABB collision normal
                        if (col_a.type == ColliderType::AABB && col_b.type == ColliderType::AABB) {
                            Vec2 ca = col_a.aabb.center();
                            Vec2 cb = col_b.aabb.center();
                            Vec2 diff = cb - ca;
                            f32 overlap_x = (col_a.aabb.w + col_b.aabb.w) * 0.5f - std::abs(diff.x);
                            f32 overlap_y = (col_a.aabb.h + col_b.aabb.h) * 0.5f - std::abs(diff.y);

                            if (overlap_x < overlap_y) {
                                c.normal = {diff.x > 0 ? 1.0f : -1.0f, 0.0f};
                                c.depth = overlap_x;
                            } else {
                                c.normal = {0.0f, diff.y > 0 ? 1.0f : -1.0f};
                                c.depth = overlap_y;
                            }
                            c.contact = ca + diff * 0.5f;
                        } else {
                            // Simplified normal for mixed types
                            Vec2 pa = col_a.type == ColliderType::Circle ? col_a.center : col_a.aabb.center();
                            Vec2 pb = col_b.type == ColliderType::Circle ? col_b.center : col_b.aabb.center();
                            c.normal = normalize(pb - pa);
                            c.depth = 0.0f;
                            c.contact = (pa + pb) * 0.5f;
                        }

                        result.push_back(c);
                    }
                }
            }
        }
    }

    return result;
}

std::vector<u32> PhysicsWorld::query_rect(Rect area, CollisionMask mask) {
    std::vector<u32> result;

    i32 min_cx = static_cast<i32>(std::floor(area.x / cell_size_));
    i32 min_cy = static_cast<i32>(std::floor(area.y / cell_size_));
    i32 max_cx = static_cast<i32>(std::floor(area.right() / cell_size_));
    i32 max_cy = static_cast<i32>(std::floor(area.bottom() / cell_size_));

    for (i32 cy = min_cy; cy <= max_cy; cy++) {
        for (i32 cx = min_cx; cx <= max_cx; cx++) {
            auto it = grid_.find(cell_key(cx, cy));
            if (it == grid_.end()) continue;

            for (u32 id : it->second) {
                auto& col = colliders_[id];
                if (!(col.layer & mask)) continue;

                bool overlapping = false;
                if (col.type == ColliderType::AABB) {
                    overlapping = aabb_overlaps(area, col.aabb);
                } else {
                    overlapping = aabb_circle_overlaps(area, col.center, col.radius);
                }

                if (overlapping) {
                    if (std::find(result.begin(), result.end(), id) == result.end()) {
                        result.push_back(id);
                    }
                }
            }
        }
    }

    return result;
}

std::vector<u32> PhysicsWorld::query_circle(Vec2 center, f32 radius, CollisionMask mask) {
    Rect bounds = {center.x - radius, center.y - radius, radius * 2, radius * 2};
    auto candidates = query_rect(bounds, mask);

    std::vector<u32> result;
    for (u32 id : candidates) {
        auto& col = colliders_[id];
        bool overlapping = false;
        if (col.type == ColliderType::Circle) {
            overlapping = circle_overlaps(center, radius, col.center, col.radius);
        } else {
            overlapping = aabb_circle_overlaps(col.aabb, center, radius);
        }
        if (overlapping) result.push_back(id);
    }
    return result;
}

RaycastHit PhysicsWorld::raycast(Vec2 origin, Vec2 direction, f32 max_distance,
                                  CollisionMask mask) {
    RaycastHit closest;
    closest.hit = false;
    closest.distance = max_distance;

    Vec2 dir = normalize(direction);

    for (auto& [id, col] : colliders_) {
        if (!(col.layer & mask)) continue;

        if (col.type == ColliderType::AABB) {
            // Ray-AABB intersection
            f32 tmin = 0.0f;
            f32 tmax = max_distance;

            for (int axis = 0; axis < 2; axis++) {
                f32 o = axis == 0 ? origin.x : origin.y;
                f32 d = axis == 0 ? dir.x : dir.y;
                f32 box_min = axis == 0 ? col.aabb.x : col.aabb.y;
                f32 box_max = axis == 0 ? col.aabb.right() : col.aabb.bottom();

                if (std::abs(d) < 0.0001f) {
                    if (o < box_min || o > box_max) { tmin = max_distance + 1; break; }
                } else {
                    f32 t1 = (box_min - o) / d;
                    f32 t2 = (box_max - o) / d;
                    if (t1 > t2) std::swap(t1, t2);
                    tmin = std::max(tmin, t1);
                    tmax = std::min(tmax, t2);
                    if (tmin > tmax) { tmin = max_distance + 1; break; }
                }
            }

            if (tmin >= 0 && tmin < closest.distance) {
                closest.hit = true;
                closest.distance = tmin;
                closest.entity_id = id;
                closest.point = origin + dir * tmin;
                // Approximate normal
                Vec2 center = col.aabb.center();
                Vec2 diff = closest.point - center;
                if (std::abs(diff.x) > std::abs(diff.y)) {
                    closest.normal = {diff.x > 0 ? 1.0f : -1.0f, 0.0f};
                } else {
                    closest.normal = {0.0f, diff.y > 0 ? 1.0f : -1.0f};
                }
            }
        }
    }

    return closest;
}

Vec2 PhysicsWorld::move_and_slide(u32 entity_id, Vec2 velocity, f32 dt) {
    auto it = colliders_.find(entity_id);
    if (it == colliders_.end()) return velocity;

    Vec2 movement = velocity * dt;
    Collider& col = it->second;

    // Try move X
    Vec2 original_pos = {col.aabb.x, col.aabb.y};
    update_position(entity_id, {original_pos.x + movement.x, original_pos.y});

    auto collisions = check_collisions();
    for (auto& c : collisions) {
        if (c.entity_a != entity_id && c.entity_b != entity_id) continue;
        u32 other = (c.entity_a == entity_id) ? c.entity_b : c.entity_a;
        auto& other_col = colliders_[other];
        if (other_col.is_trigger) continue;

        // Push back on X
        movement.x = 0;
        update_position(entity_id, {original_pos.x, original_pos.y});
        break;
    }

    // Try move Y
    Vec2 after_x_pos = {original_pos.x + movement.x, original_pos.y};
    update_position(entity_id, {after_x_pos.x, after_x_pos.y + movement.y});

    collisions = check_collisions();
    for (auto& c : collisions) {
        if (c.entity_a != entity_id && c.entity_b != entity_id) continue;
        u32 other = (c.entity_a == entity_id) ? c.entity_b : c.entity_a;
        auto& other_col = colliders_[other];
        if (other_col.is_trigger) continue;

        movement.y = 0;
        update_position(entity_id, {after_x_pos.x, after_x_pos.y});
        break;
    }

    return {movement.x / dt, movement.y / dt};
}

void PhysicsWorld::on_trigger_enter(TriggerCallback callback) {
    on_enter_ = std::move(callback);
}

void PhysicsWorld::on_trigger_exit(TriggerCallback callback) {
    on_exit_ = std::move(callback);
}

void PhysicsWorld::process_triggers(const std::vector<Collision>& collisions) {
    std::vector<std::pair<u32, u32>> current_triggers;

    for (auto& c : collisions) {
        auto& ca = colliders_[c.entity_a];
        auto& cb = colliders_[c.entity_b];
        if (!ca.is_trigger && !cb.is_trigger) continue;

        auto pair = std::make_pair(
            std::min(c.entity_a, c.entity_b),
            std::max(c.entity_a, c.entity_b)
        );
        current_triggers.push_back(pair);

        // Check if new
        if (std::find(active_triggers_.begin(), active_triggers_.end(), pair)
            == active_triggers_.end()) {
            if (on_enter_) on_enter_(pair.first, pair.second);
        }
    }

    // Check for exits
    for (auto& pair : active_triggers_) {
        if (std::find(current_triggers.begin(), current_triggers.end(), pair)
            == current_triggers.end()) {
            if (on_exit_) on_exit_(pair.first, pair.second);
        }
    }

    active_triggers_ = std::move(current_triggers);
}

} // namespace wander
