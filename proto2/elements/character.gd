extends Node3D

@onready var _model := $model

@onready var is_player := false :
	set(value):
		is_player = value
		if is_player:
			_model.material = _model.material.duplicate()
			_model.material.albedo_color = Color.DARK_BLUE


# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass
