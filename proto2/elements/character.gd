extends Node3D

@onready var _model := $model

@onready var is_player := false :
	set(value):
		is_player = value
		if is_player:
			_model.material = _model.material.duplicate()
			_model.material.albedo_color = Color.DARK_BLUE


var _target_position := Vector3.ZERO

var body_id

# Called when the node enters the scene tree for the first time.
func _ready():
	_target_position = position


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if _target_position != position:
		position = lerp(position, _target_position, 0.1)

func move_to(new_position: Vector2i):
	print("====> moving to %s" % new_position)
	_target_position = Vector3(new_position.x, 0.0, new_position.y) * 1.0 # FIXME: setup global meters per squares




