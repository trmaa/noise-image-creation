#version 130

uniform sampler2D old_texture;
uniform sampler2D new_texture;
uniform sampler2D in_texture;

in vec2 frag_uv;
out vec4 out_color;

void main() {
	ivec2 tex_size = textureSize(in_texture, 0);
	vec2 uv = gl_FragCoord.xy / vec2(tex_size);
    uv.y = 1.0 - uv.y;

	vec3 old_col = texture(old_texture, uv).rgb;
	vec3 new_col = texture(new_texture, uv).rgb;
	vec3 target_col = texture(in_texture, uv).rgb;

	float old_diff = length(old_col - target_col);
	float new_diff = length(new_col - target_col);

	bool condition = new_diff < old_diff;
	vec3 result = (condition) ? new_col : old_col;

	out_color = vec4(result, 1.0);
}
