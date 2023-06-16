extends Node3D

const METERS_PER_SQUARE = 1.0


@onready var _world = $megastructures_world
@onready var _view_node = $view

var prefab_wall = load("res://elements/wall.tscn")
var prefab_character = load("res://elements/character.tscn")


# Called when the node enters the scene tree for the first time.
func _ready():
	update_world_view()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):

	if Input.is_action_just_pressed("player_action_wait"):
		print("------ PLAYER PLAYS WAIT ... -----")
		_world.player_action_wait()
		update_world_view()
		print("------ PLAYER PLAYS WAIT - DONE -----")


func _clear_view() -> void:
	print("-> Clearing view...")
	for child_node in _view_node.get_children():
		_view_node.remove_child(child_node)
		child_node.queue_free()
	print("-> Clearing view - DONE")

func update_world_view() -> void:
	_clear_view()

	print("-> Building view ...")
	var walls_positions = _world.get_walls_positions()
	var characters_positions = _world.get_characters_positions()
	var player_pos = _world.get_player_positions()

	print("-> Interpreting model data ...")
	for wall_pos in walls_positions:
		var new_wall = prefab_wall.instantiate()
		new_wall.position = Vector3( wall_pos.x, 0.0, wall_pos.y )
		_view_node.add_child(new_wall)

	for char_pos in characters_positions:
		var new_character = prefab_character.instantiate()
		new_character.position = Vector3( char_pos.x, 0.0, char_pos.y )
		_view_node.add_child(new_character)
		if player_pos.has(char_pos):
			new_character.is_player = true

	print("-> Building view - DONE")



