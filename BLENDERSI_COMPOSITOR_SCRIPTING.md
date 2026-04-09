# BlenderSI Compositor Scripting Notes

This branch is using the newer compositor API shape from Blender 5.2.

## What Changed

- `scene.node_tree` is not the scene compositor root anymore.
- Use `scene.compositing_node_group` for the scene compositor node tree.
- `scene.use_nodes` is deprecated and effectively inert.
- Use `scene.render.use_compositing` to enable or disable compositing.
- `CompositorNodeComposite` is not available in this version.
- Use `NodeGroupOutput` for root compositor outputs.
- Use `CompositorNodeViewer` for preview output.
- Use `CompositorNodeOutputFile` for explicit file writes.

Reference points in source:

- `source/blender/makesrna/intern/rna_scene.cc`
- `source/blender/nodes/composite/node_composite_tree.cc`
- `source/blender/compositor/intern/scheduler.cc`
- `source/blender/compositor/intern/group_output_node_operation.cc`

## Minimal Scene Compositor Setup

```python
import bpy

scene = bpy.context.scene
scene.render.use_compositing = True

tree = scene.compositing_node_group
if tree is None:
    tree = bpy.data.node_groups.new("Scene Compositor", "CompositorNodeTree")
    scene.compositing_node_group = tree

tree.nodes.clear()

render_layers = tree.nodes.new("CompositorNodeRLayers")
group_output = tree.nodes.new("NodeGroupOutput")

if "Image" not in {item.name for item in tree.interface.items_tree}:
    tree.interface.new_socket(
        name="Image",
        in_out="OUTPUT",
        socket_type="NodeSocketColor",
    )

tree.links.new(render_layers.outputs["Image"], group_output.inputs["Image"])
```

## Preview Output

```python
viewer = tree.nodes.new("CompositorNodeViewer")
tree.links.new(render_layers.outputs["Image"], viewer.inputs["Image"])
viewer.select = True
tree.nodes.active = viewer
```

## File Output

```python
file_output = tree.nodes.new("CompositorNodeOutputFile")
file_output.base_path = "//compositor_output"
tree.links.new(render_layers.outputs["Image"], file_output.inputs["Image"])
```

## Safe Access Pattern

Use this instead of checking `scene.node_tree`:

```python
tree = scene.compositing_node_group
if tree is None:
    tree = bpy.data.node_groups.new("Scene Compositor", "CompositorNodeTree")
    scene.compositing_node_group = tree
```

## Cycles in Headless Scripts

In background or factory-startup runs, enable Cycles explicitly before selecting it:

```python
import addon_utils

addon_utils.enable("cycles", default_set=False, persistent=False)
bpy.context.scene.render.engine = "CYCLES"
```

## Useful Runtime Checks

```python
import bpy

print(hasattr(bpy.context.scene, "compositing_node_group"))
print(hasattr(bpy.types.Scene, "node_tree"))
print([name for name in dir(bpy.types) if name.startswith("CompositorNode")])
```
