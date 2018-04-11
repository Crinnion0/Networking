#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

const float tex_scale = 1.0f / 16.0f;
const float padding = 0.00f;

in Vertex	{
	flat vec4 pos;
	flat vec4 color;
} IN[];

out Vertex	{
	flat   vec4 color;
	smooth vec2 texCoords;
} OUT;

void main()  
{  
	OUT.color 	  = IN[1].color;

	vec2 charSize = IN[0].color.xy;

	int text_char = int(IN[0].color.z) & 0xFF;
	vec2 texture_offset = vec2(padding / tex_scale * 0.25f, padding / tex_scale * 0.5f) + vec2(float(text_char % 16),  +  float( text_char / 16));	
	
	
	vec4 centrePoint 	= gl_in[0].gl_Position;
	vec4 a = centrePoint + vec4( charSize.x,  charSize.y, 0.0f, 0.0f);
	vec4 b = centrePoint + vec4(-charSize.x,  charSize.y, 0.0f, 0.0f);
	vec4 c = centrePoint + vec4( charSize.x, -charSize.y, 0.0f, 0.0f);
	vec4 d = centrePoint + vec4(-charSize.x, -charSize.y, 0.0f, 0.0f);
	

		
	gl_Position = a; 
	OUT.texCoords = (vec2(1.0f - padding, padding) + texture_offset) * tex_scale;
	EmitVertex();	
	
	gl_Position = b; 
	OUT.texCoords = (vec2(padding, padding) + texture_offset) * tex_scale;
	EmitVertex();
	
	gl_Position = c; 
	OUT.texCoords = (vec2(1.0f - padding, 1.0f - padding) + texture_offset) * tex_scale;
	EmitVertex();
	
	gl_Position = d; 
	OUT.texCoords = (vec2(padding, 1.0f - padding) + texture_offset) * tex_scale;
	EmitVertex();
}  