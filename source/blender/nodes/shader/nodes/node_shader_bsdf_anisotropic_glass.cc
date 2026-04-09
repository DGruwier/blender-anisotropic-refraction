/* SPDX-FileCopyrightText: 2026 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "node_shader_util.hh"

#include "UI_interface_layout.hh"
#include "UI_resources.hh"

namespace blender {

namespace nodes::node_shader_bsdf_anisotropic_glass_cc {

static void node_declare(NodeDeclarationBuilder &b)
{
  b.use_custom_socket_order();

  b.add_output<decl::Shader>("BSDF"_ustr);
  b.add_default_layout();

  b.add_input<decl::Color>("Color"_ustr).default_value({1.0f, 1.0f, 1.0f, 1.0f});
  b.add_input<decl::Float>("Roughness"_ustr)
      .default_value(0.0f)
      .min(0.0f)
      .max(1.0f)
      .subtype(PROP_FACTOR);
  b.add_input<decl::Float>("IOR"_ustr).default_value(1.45f).min(0.0f).max(1000.0f);
  b.add_input<decl::Float>("Anisotropy"_ustr)
      .default_value(0.0f)
      .min(-1.0f)
      .max(1.0f);
  b.add_input<decl::Float>("Rotation"_ustr)
      .default_value(0.0f)
      .min(0.0f)
      .max(1.0f)
      .subtype(PROP_FACTOR);
  b.add_input<decl::Vector>("Normal"_ustr).hide_value();
  b.add_input<decl::Vector>("Tangent"_ustr).hide_value();
  b.add_input<decl::Float>("Weight"_ustr).available(false);

  PanelDeclarationBuilder &film = b.add_panel("Thin Film"_ustr).default_closed(true);
  film.add_input<decl::Float>("Thin Film Thickness"_ustr)
      .default_value(0.0)
      .min(0.0f)
      .max(100000.0f)
      .subtype(PROP_WAVELENGTH)
      .description("Thickness of the film in nanometers");
  film.add_input<decl::Float>("Thin Film IOR"_ustr)
      .default_value(1.33f)
      .min(1.0f)
      .max(1000.0f)
      .description("Index of refraction (IOR) of the thin film");
}

static void node_shader_buts_anisotropic_glass(
    ui::Layout &layout, bContext * /*C*/, PointerRNA *ptr)
{
  layout.prop(ptr, "distribution", ui::ITEM_R_SPLIT_EMPTY_NAME, "", ICON_NONE);
  layout.prop(ptr, "primary_camera_only", ui::ITEM_R_SPLIT_EMPTY_NAME, std::nullopt, ICON_NONE);
}

static void node_shader_init_anisotropic_glass(bNodeTree * /*ntree*/, bNode *node)
{
  node->custom1 = SHD_GLOSSY_GGX;
  node->custom2 = 1;
}

static int node_shader_gpu_bsdf_anisotropic_glass(GPUMaterial *mat,
                                                  bNode *node,
                                                  bNodeExecData * /*execdata*/,
                                                  GPUNodeStack *in,
                                                  GPUNodeStack *out)
{
  if (!in[5].link) {
    GPU_link(mat, "world_normals_get", &in[5].link);
  }

  GPU_material_flag_set(mat, GPU_MATFLAG_GLOSSY | GPU_MATFLAG_REFRACT);
  if (in[0].might_be_tinted()) {
    GPU_material_flag_set(
        mat, GPU_MATFLAG_REFLECTION_MAYBE_COLORED | GPU_MATFLAG_REFRACTION_MAYBE_COLORED);
  }

  return GPU_stack_link(mat, node, "node_bsdf_anisotropic_glass", in, out);
}

NODE_SHADER_MATERIALX_BEGIN
#ifdef WITH_MATERIALX
{
  if (to_type_ != NodeItem::Type::BSDF) {
    return empty();
  }

  NodeItem color = get_input_value("Color", NodeItem::Type::Color3);
  NodeItem roughness = get_input_value("Roughness", NodeItem::Type::Vector2);
  NodeItem ior = get_input_value("IOR", NodeItem::Type::Float);
  NodeItem normal = get_input_link("Normal", NodeItem::Type::Vector3);
  NodeItem thin_film_thickness = get_input_value("Thin Film Thickness", NodeItem::Type::Float);
  NodeItem thin_film_ior = get_input_value("Thin Film IOR", NodeItem::Type::Float);

  return create_node("dielectric_bsdf",
                     NodeItem::Type::BSDF,
                     {{"normal", normal},
                      {"tint", color},
                      {"roughness", roughness},
                      {"ior", ior},
                      {"thinfilm_thickness", thin_film_thickness},
                      {"thinfilm_ior", thin_film_ior},
                      {"scatter_mode", val(std::string("RT"))}});
}
#endif
NODE_SHADER_MATERIALX_END

}  // namespace nodes::node_shader_bsdf_anisotropic_glass_cc

void register_node_type_sh_bsdf_anisotropic_glass()
{
  namespace file_ns = nodes::node_shader_bsdf_anisotropic_glass_cc;

  static bke::bNodeType ntype;

  sh_node_type_base(&ntype, "ShaderNodeBsdfAnisotropicGlass", SH_NODE_BSDF_ANISOTROPIC_GLASS);
  ntype.ui_name = "Anisotropic Glass BSDF";
  ntype.ui_description =
      "Anisotropic rough dielectric for Cycles, with approximate isotropic viewport and "
      "interchange fallback";
  ntype.enum_name_legacy = "BSDF_ANISOTROPIC_GLASS";
  ntype.nclass = NODE_CLASS_SHADER;
  ntype.declare = file_ns::node_declare;
  ntype.gather_link_search_ops = search_link_ops_for_shader_bsdf_node;
  ntype.add_ui_poll = object_shader_nodes_poll;
  ntype.draw_buttons = file_ns::node_shader_buts_anisotropic_glass;
  bke::node_type_size_preset(ntype, bke::eNodeSizePreset::Middle);
  ntype.initfunc = file_ns::node_shader_init_anisotropic_glass;
  ntype.gpu_fn = file_ns::node_shader_gpu_bsdf_anisotropic_glass;
  ntype.materialx_fn = file_ns::node_shader_materialx;

  bke::node_register_type(ntype);
}

}  // namespace blender
