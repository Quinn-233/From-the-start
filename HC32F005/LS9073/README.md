若增加颜色，请同时修改如下两个结构类型变量：

1、

tag_ligeff_solid_color//常量颜色序号

请在LIGEFF_SOLID_COLOR_LAVENDER后添加新增颜色名称



2、

g_ligeff_solid_color//常量颜色表

格式：

{ .value = 0xCBBEFF00 },// b: 0xCB  g: 0xBE  r: 0xFF  w:0

{ .value = 0xFF7F9F00 },

......

只需在最后按格式添加颜色即可