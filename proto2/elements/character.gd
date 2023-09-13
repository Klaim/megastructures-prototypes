extends Node3D

@onready var _model := $model

@onready var is_player := false :
	set(value):
		is_player = value
		if is_player:
			_model.material = _model.material.duplicate()
			_model.material.albedo_color = Color.DARK_BLUE

var _animations = []

var _target_position := Vector3.ZERO

var body_id

# Called when the node enters the scene tree for the first time.
func _ready():
	_target_position = position

func cancel_all_animations() -> void:
	for animation in _animations:
		animation.kill()
	_animations.clear()

func start_animation() -> Tween:
	cancel_all_animations()
	var animation = get_tree().create_tween().bind_node(self).set_trans(Tween.TRANS_LINEAR)
	_animations.push_back(animation)
	return animation

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if _target_position != position:
		position = lerp(position, _target_position, 0.1)

func move_to(new_position: Vector2i):
#	print("====> moving to %s" % new_position)
	_target_position = Vector3(new_position.x, 0.0, new_position.y) * 1.0 # FIXME: setup global meters per squares


func destroy() -> void:
	var animation : Tween = start_animation()
	animation.tween_property(self, "global_position", global_position + (Vector3.DOWN * 2.0), 0.5)
	await animation.finished
	get_parent().remove_child(self)
	queue_free()

