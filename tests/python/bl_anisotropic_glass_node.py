# SPDX-FileCopyrightText: 2026 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

import unittest

import bpy


class AnisotropicGlassNodeTest(unittest.TestCase):
    def test_defaults_and_socket_layout(self):
        material = bpy.data.materials.new("AnisotropicGlassNodeTest")
        material.use_nodes = True

        node = material.node_tree.nodes.new("ShaderNodeBsdfAnisotropicGlass")

        self.assertEqual(node.bl_idname, "ShaderNodeBsdfAnisotropicGlass")
        self.assertEqual(
            list(node.inputs.keys()),
            [
                "Color",
                "Roughness",
                "IOR",
                "Anisotropy",
                "Rotation",
                "Normal",
                "Tangent",
                "Weight",
                "Thin Film Thickness",
                "Thin Film IOR",
            ],
        )
        self.assertEqual(node.distribution, "GGX")
        self.assertTrue(node.primary_camera_only)
        self.assertAlmostEqual(node.inputs["Anisotropy"].default_value, 0.0)
        self.assertAlmostEqual(node.inputs["Rotation"].default_value, 0.0)


if __name__ == "__main__":
    unittest.main(argv=[__file__])
