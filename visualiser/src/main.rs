use std::{collections::HashMap, path::PathBuf};
use anyhow::Result;
use bevy::{diagnostic::{DiagnosticsStore, FrameTimeDiagnosticsPlugin}, input::mouse::{MouseMotion, MouseWheel}};
use clap::Parser;

use bevy::prelude::*;
use helper::CompressedBlock;

mod lines;

#[derive(Parser)]
struct Args {
    /// Path to the base file
    base_file: PathBuf,
    /// Path to the compressed output
    input: PathBuf,
}

#[derive(Resource)]
struct BaseData(helper::BaseData);

#[derive(Resource)]
struct BlockResource(HashMap<ChunkIndex, Vec<CompressedBlock>>);

#[derive(Resource, Default)]
struct ViewSettings {
    chunk: ChunkIndex,
    z_layer: usize,
}

#[derive(Resource)]
struct CameraSettings {
    distance: f32,
    angle: f32,
}

#[derive(Component)]
struct Voxel;

#[derive(Component)]
struct Block;

#[derive(Resource, Default)]
struct VoxelResources {
    mesh: Handle<Mesh>,
    materials: HashMap<u8, Handle<StandardMaterial>>,
    line_materials: HashMap<u8, Handle<lines::LineMaterial>>,
}

fn main() -> Result<()> {
    let args = Args::parse();
    let base_data = BaseData(helper::read_base_file(args.base_file)?);
    let compressed_blocks = helper::read_output_file(args.input, &base_data.0.tags)?;

    let mut block_resources = HashMap::new();
    for mut block in compressed_blocks {
        // chunk index from pos
        let index = ChunkIndex::new(
            block.x_pos / base_data.0.parent_size_x as u16,
            block.y_pos / base_data.0.parent_size_y as u16,
            block.z_pos / base_data.0.parent_size_z as u16,
        );
        // chunk relative pos
        block.x_pos %= base_data.0.parent_size_x as u16;
        block.y_pos %= base_data.0.parent_size_y as u16;
        block.z_pos %= base_data.0.parent_size_z as u16;

        let blocks = match block_resources.get_mut(&index) {
            Some(v) => v,
            None => {
                block_resources.insert(index.clone(), vec![]);
                block_resources.get_mut(&index).unwrap()
            }
        };
        blocks.push(block);
    }

    let camera_settings = CameraSettings {
        distance: (base_data.0.parent_size_x as f32 + base_data.0.parent_size_y as f32),
        angle: 0.0,
    };

    App::new()
        .insert_resource(base_data)
        .insert_resource(camera_settings)
        .insert_resource(BlockResource(block_resources))
        .init_resource::<VoxelResources>()
        .init_resource::<ViewSettings>()
        .add_plugins((
            DefaultPlugins,
            FrameTimeDiagnosticsPlugin,
            lines::LinesPlugin,
        ))
        .add_systems(Startup, setup)
        .add_systems(Update, update_text)
        .add_systems(
            Update, 
            (
                keyboard_input,
                update_visable_chunk.run_if(resource_changed::<ViewSettings>),
                mouse_motion,
                // could be an Or<> for resource_change but oh well
                update_camera.run_if(resource_changed::<CameraSettings>),
                update_camera.run_if(resource_changed::<ViewSettings>),
            )
        )
        .run();

    Ok(())
}

#[derive(Component)]
struct FpsText(f32);

#[derive(Component)]
struct ChunkText;

fn setup(
    mut commands: Commands,
    base_data: Res<BaseData>,
    asset_server: Res<AssetServer>,
    mut meshes: ResMut<Assets<Mesh>>,
    mut materials: ResMut<Assets<StandardMaterial>>,
    mut line_materials: ResMut<Assets<lines::LineMaterial>>,
    mut ambient_light: ResMut<AmbientLight>,
    mut view_settings: ResMut<ViewSettings>,
    mut voxel_resources: ResMut<VoxelResources>,
) {
    ambient_light.color = Color::WHITE;
    ambient_light.brightness = 500.0;

    view_settings.z_layer = base_data.0.parent_size_z;

    voxel_resources.mesh = meshes.add(Cuboid::default());

    let mut tag_colors = Vec::new();
    let count = base_data.0.tags.len() as f32;
    for (i, (tag_id, tag_str)) in base_data.0.tags.iter().enumerate() {
        let color = Color::hsv(i as f32 / count * 360., 7.0, 7.0);
        voxel_resources.materials.insert(
            *tag_id, 
            materials.add(StandardMaterial {
                base_color: Color::hsv(i as f32 / count * 360., 1.0, 1.0),
                ..default()
            })
        );
        voxel_resources.line_materials.insert(
            *tag_id,
            line_materials.add(lines::LineMaterial {
                color: color.into(),
            })
        );
        tag_colors.push((tag_str.clone(), color));
    }

    // UI
    let font = asset_server.load("fonts/RobotoMono-Medium.ttf");

    let root_uinode = commands
        .spawn(NodeBundle {
            style: Style {
                width: Val::Percent(100.),
                height: Val::Percent(100.),
                justify_content: JustifyContent::SpaceBetween,
                ..default()
            },
            ..default()
        })
        .id();

    let left_column = commands.spawn(NodeBundle {
        style: Style {
            flex_direction: FlexDirection::Column,
            align_items: AlignItems::Start,
            flex_grow: 1.,
            margin: UiRect::axes(Val::Px(10.), Val::Px(5.)),
            ..default()
        },
        ..default()
    }).with_children(|builder| {
            // fps text
            builder.spawn((TextBundle::from_section(
                "fps:",
                TextStyle {
                    font: font.clone(),
                    font_size: 30.0,
                    color: Color::srgb(0.8, 0.2, 0.7),
                },
            ),
                FpsText(0.0)
            ));

            // Chunk xyz text
            builder.spawn((TextBundle::from_section(
                "Chunk",
                TextStyle {
                    font: font.clone(),
                    font_size: 30.0,
                    color: Color::srgb(0.8, 0.2, 0.7),
                },
            ),
                ChunkText
            ));

            // tags
            for (tag, color) in tag_colors {
                builder.spawn(TextBundle::from_section(
                    tag,
                    TextStyle {
                        font: font.clone(),
                        font_size: 30.0,
                        color,
                    })
                );
            }
            // help
            builder.spawn(TextBundle::from_section(
                "W/A/S/D: Change chunk horizontal\nQ/E: Change chunk vertiacal\nUp/Down: Change layer",
                TextStyle {
                    font: font.clone(),
                    font_size: 20.0,
                    color: Color::srgb(0.8, 0.8, 0.8),
                }
            ));

        })
        .id();

    commands
        .entity(root_uinode)
        .push_children(&[left_column]);


    commands.spawn(Camera3dBundle::default());
}

fn keyboard_input(
    keys: Res<ButtonInput<KeyCode>>,
    base_data: Res<BaseData>,
    mut view_settings: ResMut<ViewSettings>,
) {
    // change z layer
    if keys.just_pressed(KeyCode::ArrowUp) {
        if view_settings.z_layer < base_data.0.parent_size_z {
            view_settings.z_layer += 1;
        }
    } else if keys.just_pressed(KeyCode::ArrowDown) {
        if view_settings.z_layer > 1 {
            view_settings.z_layer -= 1;
        }
    }
    // change chunk
    if keys.just_pressed(KeyCode::KeyW) {
        let num_chunks = base_data.0.x_count / base_data.0.parent_size_x;
        if (view_settings.chunk.x as usize) < num_chunks - 1 {
            view_settings.chunk.x += 1;
        }
    } else if keys.just_pressed(KeyCode::KeyS) {
        if view_settings.chunk.x as usize > 0 {
            view_settings.chunk.x -= 1;
        }
    } else if keys.just_pressed(KeyCode::KeyD) {
        let num_chunks = base_data.0.y_count / base_data.0.parent_size_y;
        if (view_settings.chunk.y as usize) < num_chunks - 1 {
            view_settings.chunk.y += 1;
        }
    } else if keys.just_pressed(KeyCode::KeyA) {
        if view_settings.chunk.y as usize > 0 {
            view_settings.chunk.y -= 1;
        }
    } else if keys.just_pressed(KeyCode::KeyE) {
        let num_chunks = base_data.0.z_count / base_data.0.parent_size_z;
        if (view_settings.chunk.z as usize) < num_chunks - 1 {
            view_settings.chunk.z += 1;
        }
    } else if keys.just_pressed(KeyCode::KeyQ) {
        if view_settings.chunk.z as usize > 0 {
            view_settings.chunk.z -= 1;
        }
    }
}

fn update_visable_chunk(
    mut commands: Commands,
    base_data: Res<BaseData>,
    blocks_resource: Res<BlockResource>,
    view_settings: Res<ViewSettings>,
    voxel_resources: Res<VoxelResources>,
    cube_mesh_res: Res<lines::CubeLineMesh>,
    voxels_query: Query<Entity, Or<(With<Voxel>, With<Block>)>>,
    mut text_query: Query<&mut Text, With<ChunkText>>,
) {
    // update text
    for mut text in text_query.iter_mut() {
        text.sections[0] = format!(
            "Chunk ({}, {}, {})",
            view_settings.chunk.x,
            view_settings.chunk.y,
            view_settings.chunk.z
        ).into();
    }

    // remove old voxels
    for entity in voxels_query.iter() {
        commands.entity(entity).despawn();
    }

    // get start index of chunk
    // there is probably a better way to do this but oh well
    let mut index = view_settings.chunk.z as usize * base_data.0.parent_size_z;
    index *= base_data.0.y_count;
    index += view_settings.chunk.y as usize * base_data.0.parent_size_y;
    index *= base_data.0.x_count;
    index += view_settings.chunk.x as usize * base_data.0.parent_size_x;

    // spawn new voxels
    for z in 0..view_settings.z_layer {
        for y in 0..base_data.0.parent_size_y {
            for x in 0..base_data.0.parent_size_x {
                let tag = base_data.0.data[index];
                commands.spawn((
                    PbrBundle {
                        mesh: voxel_resources.mesh.clone(),
                        material: voxel_resources.materials.get(&tag).unwrap().clone(),
                        transform: Transform::from_xyz(x as f32, z as f32, y as f32),
                        ..default()
                    },
                    Voxel,
                ));
                index += 1;
            }
            index += base_data.0.x_count - base_data.0.parent_size_x;
        }
        index += (base_data.0.y_count - base_data.0.parent_size_y) * base_data.0.x_count;
    }
    
    // blocks to draw
    const EMPTY: &Vec<CompressedBlock> = &vec![];
    let blocks_iter = blocks_resource.0.get(&view_settings.chunk)
        .unwrap_or(&EMPTY)
        .iter()
        .filter(|block| block.z_pos < view_settings.z_layer as u16);
    for block in blocks_iter {
        let trans = Transform::default()
            .with_translation(Vec3::new(
                block.x_size as f32 / 2.0 + block.x_pos as f32 - 0.5,
                block.z_size as f32 / 2.0 + block.z_pos as f32 - 0.5,
                block.y_size as f32 / 2.0 + block.y_pos as f32 - 0.5
            ))
            .with_scale(Vec3::new(block.x_size as f32, block.z_size as f32, block.y_size as f32));
        commands.spawn((MaterialMeshBundle {
            mesh: cube_mesh_res.0.clone(),
            transform: trans,
            material: voxel_resources.line_materials
                .get(&block.tag).unwrap().clone(),
            ..default()
        },
            Block
        ));
    }
}

fn mouse_motion(
    buttons: Res<ButtonInput<MouseButton>>,
    mut evr_motion: EventReader<MouseMotion>,
    mut evr_scroll: EventReader<MouseWheel>,
    time: Res<Time>,
    mut camera_settings: ResMut<CameraSettings>,
) {
    if buttons.pressed(MouseButton::Left) {
        for ev in evr_motion.read() {
            camera_settings.angle -= ev.delta.x * time.delta_seconds() * 0.5;
        }
    }
    for ev in evr_scroll.read() {
        camera_settings.distance += ev.y;
    }
}

fn update_camera(
    base_data: Res<BaseData>,
    view_settings: Res<ViewSettings>,
    camera_settings: ResMut<CameraSettings>,
    mut camera_query: Query<&mut Transform, With<Camera>>
) {
    for mut trans in camera_query.iter_mut() {
        let center_point = Vec3::new(
            base_data.0.parent_size_x as f32 / 2.0,
            0.0,
            base_data.0.parent_size_y as f32 / 2.0
        );

        let rotation = Quat::from_rotation_y(camera_settings.angle);

        *trans = Transform::default()
            .with_translation(Vec3::new(
                base_data.0.parent_size_x as f32 / 2.0 + camera_settings.distance,
                4.0 + camera_settings.distance,
                base_data.0.parent_size_y as f32 / 2.0
            ));

        trans.translate_around(center_point, rotation);
        *trans = trans.looking_at(center_point, Vec3::Y);
        trans.translation += Vec3::new(0.0, view_settings.z_layer as f32, 0.0);
    }
}

fn update_text(
    time: Res<Time>,
    diagnostics: Res<DiagnosticsStore>,
    mut query: Query<(&mut Text, &mut FpsText)>,
) {
    for (mut text, mut color) in &mut query {
        if let Some(fps) = diagnostics.get(&FrameTimeDiagnosticsPlugin::FPS) {
            color.0 += 50.0 * time.delta_seconds();
            if let Some(fps) = fps.smoothed() {
                text.sections[0].value = format!("fps: {:.2}", fps);
                text.sections[0].style.color = Color::hsv(color.0, 0.6, 0.7);
            }
        }
    }
}

#[derive(Default, Hash, PartialEq, Eq, Clone)]
struct ChunkIndex {
    x: u16,
    y: u16,
    z: u16,
}

impl ChunkIndex {
    fn new(x: u16, y: u16, z: u16) -> ChunkIndex {
        ChunkIndex {
            x,
            y,
            z
        }
    }
}
