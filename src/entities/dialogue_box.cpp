#include "dialogue_box.hpp"
#include "render.hpp"

ECS_ENTT::Entity DialogueBox::createDialogueBox(std::string fileName, ECS_ENTT::Scene* scene)
{
	ECS_ENTT::Entity dialogueBoxEntity = scene->CreateEntity(fileName);
	
	scene->current_dialogue_box = fileName;
	
	ShadedMesh& meshResource = cache_resource(fileName);
	if (meshResource.effect.program.resource == 0) {
		RenderSystem::createDialogueSprite(meshResource, story_textures_path(fileName), "dialogue_box");
	}
	
	dialogueBoxEntity.AddComponent<ShadedMeshRef>(meshResource);
	ShadedMeshRef& resource = dialogueBoxEntity.GetComponent<ShadedMeshRef>();
	resource.reference_to_cache->texture.color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f};

	dialogueBoxEntity.AddComponent<DialogueBox>();
	dialogueBoxEntity.AddComponent<IgnorePhysics>();
	dialogueBoxEntity.AddComponent<IgnoreSave>();

	glm::vec3 cameraPos = scene->GetCamera()->GetPosition();

	Motion& motionComponent = dialogueBoxEntity.AddComponent<Motion>();
	motionComponent.position = vec3(dialogueBoxPosition.x + cameraPos.x, dialogueBoxPosition.y + cameraPos.y, dialogueBoxPosition.z);
	motionComponent.angle = 0.0f;
	motionComponent.velocity = { 0.0f, 0.0f, 0.0f };
	motionComponent.scale = { resource.reference_to_cache->mesh.original_size.x * dialogueBoxScale.x, resource.reference_to_cache->mesh.original_size.y * dialogueBoxScale.y, dialogueBoxScale.z };
	
	return dialogueBoxEntity;
}
