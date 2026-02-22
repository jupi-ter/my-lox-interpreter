# Whisker Programming Language

Whisker is a scripting language for game development that transpiles to C. It provides a GameMaker-like syntax while maintaining the performance of compiled native code.

## Overview

Whisker is designed around entities with lifecycle hooks and component-based architecture. Scripts define entity types, their data, and behavior, then the transpiler generates optimized C code that integrates with the RatEngine runtime.

## Installation

### Prerequisites
- GCC or compatible C compiler
- Make
- Linux/Unix environment (tested on Ubuntu)

### Building the Transpiler

```bash
git clone https://github.com/yourusername/whisker
cd whisker
make
```

This creates the `whisker` executable.

## Basic Usage

1. Write a Whisker script (`.wsk` file)
2. Transpile to C: `./whisker script.wsk`
3. The transpiler generates `game_generated.h` and `game_generated.c`
4. Compile these with your game engine

## Language Syntax

### Entity Declaration

Entities are the core building blocks. Each entity has custom fields and lifecycle hooks.

```whisker
entity Player {
    float hsp;
    float vsp;
    int health;
    
    init {
        collision.type = COLLISION_RECT;
        collision.width = 8;
        collision.height = 8;
    }
    
    on_create {
        self.hsp = 0;
        self.vsp = 0;
        self.health = 100;
    }
    
    on_update {
        // Update logic runs every frame
    }
    
    on_collision(other) {
        // Runs when colliding with another entity
    }
    
    on_destroy {
        // Cleanup when entity is destroyed
    }
}
```

### Field Types

- `float` - floating point numbers
- `int` - integers
- `bool` - boolean values
- `uint32` - unsigned 32-bit integers

### Lifecycle Hooks

**init** - Static metadata, executed once during entity creation. Used for collision setup and other configuration.

**on_create** - Runtime initialization. Sets initial values for entity fields.

**on_update** - Called every frame for each entity instance.

**on_collision(other)** - Called when this entity collides with another. The `other` parameter is the colliding entity's ID.

**on_destroy** - Called before entity is removed from the game.

### Built-in Components

Entities automatically have access to engine components:

**self** - The entity instance itself
```whisker
self.hsp = 5;
```

**transform** - Position and orientation
```whisker
transform.x = 100;
transform.y = 50;
```

**renderable** - Sprite and animation
```whisker
renderable.current_sprite_id = 2;
renderable.image_speed = 0.1;
```

**collision** - Collision shape (set in `init` block only)
```whisker
init {
    collision.type = COLLISION_RECT;
    collision.width = 16;
    collision.height = 16;
}
```

### Built-in Functions

**place_meeting(x, y, entity_type)** - Check if moving to position would collide with entity type
```whisker
if (place_meeting(transform.x + 1, transform.y, ENTITY_TYPE_WALL)) {
    // Would collide with a wall
}
```

**instance_destroy(entity_id)** - Destroy an entity
```whisker
on_collision(other) {
    instance_destroy(other);  // Destroy what we hit
    instance_destroy(eid);    // Destroy ourselves
}
```

**keyboard_check(key)** - Check if key is held down
```whisker
if (keyboard_check(KEY_RIGHT)) {
    self.hsp = 2;
}
```

Available keys: `KEY_A` through `KEY_Z`, `KEY_0` through `KEY_9`, `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`, `KEY_SPACE`, function keys, etc.

### Game Block

The `game` block defines initial entity spawning:

```whisker
game {
    spawn Player(64, 64);
    spawn Enemy(100, 50);
    spawn Wall(32, 32);
}
```

### Control Flow

Standard control flow structures:

```whisker
// If statements
if (condition) {
    // code
} else {
    // code
}

// While loops
while (condition) {
    // code
}

// For loops
for (i = 0; i < 10; i = i + 1) {
    // code
}
```

### Operators

- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `and`, `or`, `!`

## Example: Simple Game

```whisker
entity Player {
    float hsp;
    float vsp;
    
    init {
        collision.type = COLLISION_RECT;
        collision.width = 8;
        collision.height = 8;
    }
    
    on_create {
        self.hsp = 0;
        self.vsp = 0;
        renderable.current_sprite_id = 2;
        renderable.image_speed = 0.1;
    }
    
    on_update {
        self.hsp = 0;
        self.vsp = 0;
        
        if (keyboard_check(KEY_RIGHT)) self.hsp = 2;
        if (keyboard_check(KEY_LEFT)) self.hsp = -2;
        if (keyboard_check(KEY_DOWN)) self.vsp = 2;
        if (keyboard_check(KEY_UP)) self.vsp = -2;
        
        transform.x = transform.x + self.hsp;
        if (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
            if (self.hsp > 0) {
                while (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
                    transform.x = transform.x - 1;
                }
            } else {
                while (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
                    transform.x = transform.x + 1;
                }
            }
            self.hsp = 0;
        }
        
        transform.y = transform.y + self.vsp;
        if (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
            if (self.vsp > 0) {
                while (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
                    transform.y = transform.y - 1;
                }
            } else {
                while (place_meeting(transform.x, transform.y, ENTITY_TYPE_WALL)) {
                    transform.y = transform.y + 1;
                }
            }
            self.vsp = 0;
        }
    }
}

entity Wall {
    init {
        collision.type = COLLISION_RECT;
        collision.width = 8;
        collision.height = 8;
    }
    
    on_create {
        renderable.current_sprite_id = 1;
    }
}

game {
    spawn Player(64, 64);
    spawn Wall(32, 32);
    spawn Wall(96, 32);
    spawn Wall(32, 96);
    spawn Wall(96, 96);
}
```

## Integration with RatEngine

The transpiler generates two files that integrate with the RatEngine C runtime:

- `game_generated.h` - Entity type definitions and function declarations
- `game_generated.c` - Entity logic implementation

These files are automatically placed in `../RatGameC/src/` relative to the transpiler location.

## Known Issues

- Memory cleanup has a double-free bug on exit (files generate correctly, cleanup crashes)
- Entity type names use naive pluralization (Enemy becomes "enemys")
- Typed declarations not actually implemented (all become `float` regardless of declared type)

## Roadmap

### v1.0
- Separate level/game contexts
- Font rendering integration

### v1.1
- Hot reload systems
- Generic event systems
- Entity tagging
- Camera systems
- Input remapping
- Parallax scrolling
- Object pooling
- Gamepad support
- Deterministic multiplayer
- Asset hot reload

### v1.2
- Sound management
- Steamworks SDK integration

## License

MIT License - see LICENSE file for details

## Contributing

This is a personal learning project following the "Crafting Interpreters" approach. Issues and pull requests welcome.
