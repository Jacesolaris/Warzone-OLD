varying vec2 var_TexCoords;
uniform vec2 u_Dimensions;
uniform sampler2D u_DiffuseMap;
void main ()
{
  vec2 tmpvar_1;
  tmpvar_1 = (1.0/(u_Dimensions));
  vec4 normal_2;
  vec3 bw_3;
  bw_3 = vec3(1.0, 1.0, 1.0);
  vec2 tmpvar_4;
  tmpvar_4.x = var_TexCoords.x;
  tmpvar_4.y = (var_TexCoords.y + tmpvar_1.y);
  vec4 tmpvar_5;
  tmpvar_5 = texture2D (u_DiffuseMap, tmpvar_4);
  vec2 tmpvar_6;
  tmpvar_6.x = var_TexCoords.x;
  tmpvar_6.y = (var_TexCoords.y - tmpvar_1.y);
  vec4 tmpvar_7;
  tmpvar_7 = texture2D (u_DiffuseMap, tmpvar_6);
  vec2 tmpvar_8;
  tmpvar_8.x = (var_TexCoords.x + tmpvar_1.x);
  tmpvar_8.y = var_TexCoords.y;
  vec4 tmpvar_9;
  tmpvar_9 = texture2D (u_DiffuseMap, tmpvar_8);
  vec2 tmpvar_10;
  tmpvar_10.x = (var_TexCoords.x - tmpvar_1.x);
  tmpvar_10.y = var_TexCoords.y;
  vec4 tmpvar_11;
  tmpvar_11 = texture2D (u_DiffuseMap, tmpvar_10);
  float tmpvar_12;
  tmpvar_12 = (((tmpvar_5.x + tmpvar_5.y) + tmpvar_5.z) / 3.0);
  float tmpvar_13;
  tmpvar_13 = (((tmpvar_7.x + tmpvar_7.y) + tmpvar_7.z) / 3.0);
  float tmpvar_14;
  tmpvar_14 = (((tmpvar_9.x + tmpvar_9.y) + tmpvar_9.z) / 3.0);
  float tmpvar_15;
  tmpvar_15 = (((tmpvar_11.x + tmpvar_11.y) + tmpvar_11.z) / 3.0);
  float tmpvar_16;
  tmpvar_16 = abs((tmpvar_15 - tmpvar_14));
  float tmpvar_17;
  tmpvar_17 = abs((tmpvar_12 - tmpvar_13));
  if ((tmpvar_16 > 0.085)) {
    bw_3 = vec3(1.0, 1.0, 1.0);
  } else {
    if ((tmpvar_17 > 0.085)) {
      bw_3 = vec3(1.0, 1.0, 1.0);
    } else {
      bw_3 = vec3(0.0, 0.0, 0.0);
    };
  };
  vec3 tmpvar_18;
  tmpvar_18.x = (tmpvar_15 - tmpvar_14);
  tmpvar_18.y = (100.0 * tmpvar_1.x);
  tmpvar_18.z = (tmpvar_12 - tmpvar_13);
  bw_3 = (0.5 + (0.5 * normalize(tmpvar_18).xzy));
  vec4 tmpvar_19;
  tmpvar_19.w = 0.0;
  tmpvar_19.xyz = bw_3;
  float tmpvar_20;
  vec4 tmpvar_21;
  tmpvar_21 = texture2D (u_DiffuseMap, var_TexCoords);
  tmpvar_20 = abs(tmpvar_21.x);
  vec3 tmpvar_22;
  tmpvar_22.z = 0.01;
  tmpvar_22.x = (((
    abs(texture2D (u_DiffuseMap, (var_TexCoords + (vec2(-1.0, 0.0) * tmpvar_1))).x)
   - tmpvar_20) + (tmpvar_20 - 
    abs(texture2D (u_DiffuseMap, (var_TexCoords + (vec2(1.0, 0.0) * tmpvar_1))).x)
  )) * 0.5);
  tmpvar_22.y = (((
    abs(texture2D (u_DiffuseMap, (var_TexCoords + (vec2(0.0, -1.0) * tmpvar_1))).x)
   - tmpvar_20) + (tmpvar_20 - 
    abs(texture2D (u_DiffuseMap, (var_TexCoords + (vec2(0.0, 1.0) * tmpvar_1))).x)
  )) * 0.5);
  vec4 tmpvar_23;
  tmpvar_23.w = 1.0;
  tmpvar_23.xyz = ((normalize(tmpvar_22) * 0.5) + 0.5);
  normal_2.w = tmpvar_19.w;
  vec2 vEdge_24;
  vec2 tmpvar_25;
  tmpvar_25 = (1.0/(u_Dimensions));
  vec4 tmpvar_26;
  tmpvar_26 = texture2D (u_DiffuseMap, (var_TexCoords - tmpvar_25));
  vec4 tmpvar_27;
  tmpvar_27 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(0.0, -1.0) * tmpvar_25)));
  vec4 tmpvar_28;
  tmpvar_28 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(1.0, -1.0) * tmpvar_25)));
  vec4 tmpvar_29;
  tmpvar_29 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(-1.0, 0.0) * tmpvar_25)));
  vec4 tmpvar_30;
  tmpvar_30 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(1.0, 0.0) * tmpvar_25)));
  vec4 tmpvar_31;
  tmpvar_31 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(-1.0, 1.0) * tmpvar_25)));
  vec4 tmpvar_32;
  tmpvar_32 = texture2D (u_DiffuseMap, (var_TexCoords + (vec2(0.0, 1.0) * tmpvar_25)));
  vec4 tmpvar_33;
  tmpvar_33 = texture2D (u_DiffuseMap, (var_TexCoords + tmpvar_25));
  float tmpvar_34;
  tmpvar_34 = dot ((tmpvar_26.xyz * tmpvar_26.xyz), vec3(0.3333, 0.3333, 0.3333));
  float tmpvar_35;
  tmpvar_35 = dot ((tmpvar_28.xyz * tmpvar_28.xyz), vec3(0.3333, 0.3333, 0.3333));
  float tmpvar_36;
  tmpvar_36 = dot ((tmpvar_31.xyz * tmpvar_31.xyz), vec3(0.3333, 0.3333, 0.3333));
  float tmpvar_37;
  tmpvar_37 = dot ((tmpvar_33.xyz * tmpvar_33.xyz), vec3(0.3333, 0.3333, 0.3333));
  vEdge_24.x = (((
    (tmpvar_34 - tmpvar_35)
   * 0.25) + (
    (dot ((tmpvar_29.xyz * tmpvar_29.xyz), vec3(0.3333, 0.3333, 0.3333)) - dot ((tmpvar_30.xyz * tmpvar_30.xyz), vec3(0.3333, 0.3333, 0.3333)))
   * 0.5)) + ((tmpvar_36 - tmpvar_37) * 0.25));
  vEdge_24.y = (((
    (tmpvar_34 - tmpvar_36)
   * 0.25) + (
    (dot ((tmpvar_27.xyz * tmpvar_27.xyz), vec3(0.3333, 0.3333, 0.3333)) - dot ((tmpvar_32.xyz * tmpvar_32.xyz), vec3(0.3333, 0.3333, 0.3333)))
   * 0.5)) + ((tmpvar_35 - tmpvar_37) * 0.25));
  vec3 tmpvar_38;
  tmpvar_38.z = 1.0;
  tmpvar_38.xy = (vEdge_24 * 10.0);
  normal_2.xyz = (bw_3 + normalize(tmpvar_38));
  normal_2.xyz = (normal_2.xyz + tmpvar_23.xyz);
  normal_2.xyz = (normal_2.xyz / 3.0);
  vec3 color_39;
  color_39 = ((tmpvar_21.xyz - 0.4901961) * 2.217391);
  vec3 tmpvar_40;
  tmpvar_40 = clamp (((color_39 * color_39) * (color_39 * 5.0)), 0.0, 1.0);
  color_39 = tmpvar_40;
  vec3 tmpvar_41;
  tmpvar_41 = clamp (((tmpvar_40 + 
    ((tmpvar_21.xyz - 0.0627451) * 1.164384)
  ) * 2.5), 0.0, 1.0);
  normal_2.w = clamp ((1.0 - (
    ((tmpvar_41.x + tmpvar_41.y) + tmpvar_41.z)
   / 4.0)), 0.0, 1.0);
  normal_2.w = (normal_2.w + normal_2.z);
  normal_2.w = (normal_2.w / 2.0);
  normal_2.w = (1.0 - normal_2.w);
  gl_FragColor = normal_2;
}


// stats: 132 alu 17 tex 2 flow
// inputs: 1
//  #0: var_TexCoords (high float) 2x1 [-1]
// uniforms: 1 (total size: 0)
//  #0: u_Dimensions (high float) 2x1 [-1]
// textures: 1
//  #0: u_DiffuseMap (high 2d) 0x0 [-1]
