extends Node3D


@onready var _world = $megastructures_world
@onready var _view_node = $view

var prefab_wall = load("res://elements/wall.tscn")
var prefab_character = load("res://elements/character.tscn")

var _character_views : Dictionary

# Called when the node enters the scene tree for the first time.
func _ready():
	build_world_view_from_scratch()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):

	var events_sequence :Array
	if Input.is_action_just_pressed("player_action_wait"):
		events_sequence = _world.player_action_wait()

	elif Input.is_key_pressed(KEY_CTRL):
		# Attacks
		if Input.is_action_just_pressed("player_action_move_up"):
			events_sequence = _world.player_action_attack(Vector2i.UP)

		elif Input.is_action_just_pressed("player_action_move_down"):
			events_sequence = _world.player_action_attack(Vector2i.DOWN)

		elif Input.is_action_just_pressed("player_action_move_left"):
			events_sequence = _world.player_action_attack(Vector2i.LEFT)

		elif Input.is_action_just_pressed("player_action_move_right"):
			events_sequence = _world.player_action_attack(Vector2i.RIGHT)

	elif Input.is_action_just_pressed("player_action_move_up"):
		events_sequence = _world.player_action_move(Vector2i.UP)

	elif Input.is_action_just_pressed("player_action_move_down"):
		events_sequence = _world.player_action_move(Vector2i.DOWN)

	elif Input.is_action_just_pressed("player_action_move_left"):
		events_sequence = _world.player_action_move(Vector2i.LEFT)

	elif Input.is_action_just_pressed("player_action_move_right"):
		events_sequence = _world.player_action_move(Vector2i.RIGHT)

	if events_sequence:
		update_world_view(events_sequence)

func _clear_view() -> void:
	print("-> Clearing view...")
	for child_node in _view_node.get_children():
		_view_node.remove_child(child_node)
		child_node.queue_free()
	print("-> Clearing view - DONE")

func build_world_view_from_scratch() -> void:
	_clear_view()

	_character_views = Dictionary()

	print("-> Building view ...")
	var walls_positions = _world.get_walls_positions()
	var characters_positions = _world.get_characters_positions()
	var player_pos = _world.get_player_positions()

	print("-> Interpreting model data ...")
	for wall_pos in walls_positions:
		var new_wall = prefab_wall.instantiate()
		new_wall.position = Vector3( wall_pos.x, 0.0, wall_pos.y )
		_view_node.add_child(new_wall)

	for body_id in characters_positions:
		var char_pos : Vector2i = characters_positions[body_id]
		var new_character = prefab_character.instantiate()
		new_character.body_id = body_id
		_character_views[ body_id ] = new_character
		var new_position := Vector3( char_pos.x, 0.0, char_pos.y )
		new_character.position = new_position
#		new_character._target_position = new_position
		_view_node.add_child(new_character)
		if player_pos.has(char_pos):
			new_character.is_player = true

	print("-> Building view - DONE")

func get_character_view(body_id:int):
	return _character_views.get(body_id)

func remove_character_view(body_id:int):
	_character_views.erase(body_id)

func update_world_view(events: Array) -> void:
	for event in events:
		match event.type_name:

			"proto2::model::events::Waited":
#				print("-> Character %s just waits." %event.body_id)
				pass

			"proto2::model::events::Moved":
#				print("-> Character %s just moved from %s to %s" % [ event.body_id, event.initial_position, event.new_position ])
				var new_position := Vector2i(event.new_position.x, event.new_position.y)
				var character_view : Node3D = get_character_view(event.body_id)
				character_view.move_to(new_position)

			"proto2::model::events::FailedToMove":
#				print("-> Character %s failed to move." % event.body_id)
				pass


			"proto2::model::events::Destroyed":
				var character_view : Node3D = get_character_view(event.destroyed_id)
				await character_view.destroy()
				remove_character_view(event.destroyed_id)

			"proto2::model::events::Attacked":
				var character_view : Node3D = get_character_view(event.attacker)
				var target_position := Vector2i(event.target.x, event.target.y)
				await character_view.attack(target_position)

			"proto2::model::events::Evaded":
				pass
				# TODO: implement animation

