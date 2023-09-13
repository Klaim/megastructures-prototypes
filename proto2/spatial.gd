extends Object

class_name Spatial

const METERS_PER_SQUARE := 1.0

static func to_view_position(grid_position : Vector2i) -> Vector3:
	return Vector3(grid_position.x * METERS_PER_SQUARE, 0.0, grid_position.y * METERS_PER_SQUARE)

