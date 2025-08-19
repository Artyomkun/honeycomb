using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Player
{
    public class PlayerController : MonoBehaviour
    {
        [Header("Movement")]
        public float moveSpeed = 5f;
        public float runSpeed = 8f;
        public float jumpForce = 5f;
        public float gravity = -9.81f;
        
        [Header("Health & Energy")]
        public float maxHealth = 100f;
        public float currentHealth;
        public float healthRegeneration = 1f;
        
        [Header("Chakra Development")]
        public float meditationProgress = 0f;
        public float meditationRequired = 100f;
        public bool isMeditating = false;
        
        [Header("Interaction")]
        public float interactionRange = 3f;
        public LayerMask interactableLayer;
        
        [Header("Abilities")]
        public List<string> unlockedAbilities = new List<string>();
        public float teleportRange = 10f;
        public float healingPower = 10f;
        
        [Header("UI References")]
        public GameObject healthBar;
        public GameObject energyBar;
        public GameObject chakraUI;
        
        // Private variables
        private CharacterController characterController;
        private Vector3 velocity;
        private bool isGrounded;
        private Camera playerCamera;
        private GameManager gameManager;
        private ChakraSystem chakraSystem;
        private QuestSystem questSystem;
        private KarmaSystem karmaSystem;
        private MoodSystem moodSystem;
        private DiseaseSystem diseaseSystem;
        
        // Input
        private float horizontalInput;
        private float verticalInput;
        private bool isRunning;
        private bool isJumping;
        private bool isInteracting;
        
        void Start()
        {
            InitializeComponents();
            InitializePlayer();
        }
        
        void Update()
        {
            HandleInput();
            HandleMovement();
            HandleInteraction();
            HandleMeditation();
            UpdateUI();
        }
        
        private void InitializeComponents()
        {
            characterController = GetComponent<CharacterController>();
            playerCamera = Camera.main;
            gameManager = GameManager.Instance;
            
            if (gameManager != null)
            {
                chakraSystem = gameManager.GetComponent<ChakraSystem>();
                questSystem = gameManager.GetComponent<QuestSystem>();
                karmaSystem = gameManager.GetComponent<KarmaSystem>();
                moodSystem = gameManager.GetComponent<MoodSystem>();
                diseaseSystem = gameManager.GetComponent<DiseaseSystem>();
            }
            
            if (characterController == null)
            {
                characterController = gameObject.AddComponent<CharacterController>();
            }
        }
        
        private void InitializePlayer()
        {
            currentHealth = maxHealth;
            
            // Добавляем базовые способности
            unlockedAbilities.Add("BasicMovement");
            unlockedAbilities.Add("BasicInteraction");
            
            // Устанавливаем начальную позицию
            transform.position = Vector3.up * 2f;
        }
        
        private void HandleInput()
        {
            // Движение
            horizontalInput = Input.GetAxis("Horizontal");
            verticalInput = Input.GetAxis("Vertical");
            isRunning = Input.GetKey(KeyCode.LeftShift);
            isJumping = Input.GetKeyDown(KeyCode.Space);
            isInteracting = Input.GetKeyDown(KeyCode.E);
            
            // Способности
            if (Input.GetKeyDown(KeyCode.Q))
            {
                ToggleMeditation();
            }
            
            if (Input.GetKeyDown(KeyCode.R))
            {
                UseHealing();
            }
            
            if (Input.GetKeyDown(KeyCode.T))
            {
                UseTeleport();
            }
        }
        
        private void HandleMovement()
        {
            // Проверяем, на земле ли игрок
            isGrounded = characterController.isGrounded;
            
            if (isGrounded && velocity.y < 0)
            {
                velocity.y = -2f;
            }
            
            // Вычисляем направление движения
            Vector3 moveDirection = transform.right * horizontalInput + transform.forward * verticalInput;
            
            // Применяем скорость
            float currentSpeed = isRunning ? runSpeed : moveSpeed;
            characterController.Move(moveDirection * currentSpeed * Time.deltaTime);
            
            // Прыжок
            if (isJumping && isGrounded)
            {
                velocity.y = Mathf.Sqrt(jumpForce * -2f * gravity);
            }
            
            // Гравитация
            velocity.y += gravity * Time.deltaTime;
            characterController.Move(velocity * Time.deltaTime);
            
            // Поворот камеры
            if (Input.GetMouseButton(1)) // Правая кнопка мыши
            {
                float mouseX = Input.GetAxis("Mouse X");
                transform.Rotate(Vector3.up * mouseX * 2f);
            }
        }
        
        private void HandleInteraction()
        {
            if (isInteracting)
            {
                // Ищем объекты для взаимодействия
                Collider[] colliders = Physics.OverlapSphere(transform.position, interactionRange, interactableLayer);
                
                foreach (Collider collider in colliders)
                {
                    IInteractable interactable = collider.GetComponent<IInteractable>();
                    if (interactable != null)
                    {
                        interactable.Interact(this);
                        break;
                    }
                }
            }
        }
        
        private void HandleMeditation()
        {
            if (isMeditating)
            {
                // Увеличиваем прогресс медитации
                meditationProgress += Time.deltaTime * 10f; // 10 единиц в секунду
                
                // Проверяем завершение медитации
                if (meditationProgress >= meditationRequired)
                {
                    CompleteMeditation();
                }
                
                // Восстанавливаем здоровье и энергию
                if (chakraSystem != null)
                {
                    chakraSystem.currentEnergy = Mathf.Min(chakraSystem.maxEnergy, 
                        chakraSystem.currentEnergy + Time.deltaTime * 5f);
                }
                
                currentHealth = Mathf.Min(maxHealth, currentHealth + Time.deltaTime * healthRegeneration);
                
                // Муд/болезни влияние
                if (moodSystem != null)
                {
                    moodSystem.ApplyMeditationBoost();
                }
            }
        }
        
        private void ToggleMeditation()
        {
            if (isMeditating)
            {
                StopMeditation();
            }
            else
            {
                StartMeditation();
            }
        }
        
        private void StartMeditation()
        {
            if (!isMeditating)
            {
                isMeditating = true;
                meditationProgress = 0f;
                
                // Замедляем движение
                moveSpeed *= 0.5f;
                runSpeed *= 0.5f;
                
                // Визуальные эффекты медитации
                Debug.Log("Начинается медитация...");
                
                // Карма за медитацию
                if (karmaSystem != null)
                {
                    karmaSystem.AddKarmaForAction(KarmaAction.Meditation);
                }
                
                // Настроение в плюс
                if (moodSystem != null)
                {
                    moodSystem.AddCalm(0.1f);
                    moodSystem.AddLove(0.05f);
                }
                
                // Обновляем квесты
                if (questSystem != null)
                {
                    questSystem.UpdateObjective("MEN_002", 0, 1);
                }
            }
        }
        
        private void StopMeditation()
        {
            if (isMeditating)
            {
                isMeditating = false;
                
                // Восстанавливаем нормальную скорость
                moveSpeed = 5f;
                runSpeed = 8f;
                
                Debug.Log("Медитация прервана");
            }
        }
        
        private void CompleteMeditation()
        {
            isMeditating = false;
            meditationProgress = 0f;
            
            // Восстанавливаем нормальную скорость
            moveSpeed = 5f;
            runSpeed = 8f;
            
            // Награды за медитацию
            if (chakraSystem != null)
            {
                chakraSystem.currentEnergy = chakraSystem.maxEnergy;
            }
            
            currentHealth = maxHealth;
            
            // Разблокируем новые способности
            UnlockAbility("MeditationMastery");
            
            Debug.Log("Медитация завершена! Новые способности разблокированы.");
            
            // Обновляем квесты
            if (questSystem != null)
            {
                questSystem.UpdateObjective("MEN_002", 0, 1);
            }
        }
        
        private void UseHealing()
        {
            if (chakraSystem != null && chakraSystem.CanUseAbility(20f))
            {
                chakraSystem.UseEnergy(20f);
                
                // Исцеляем себя
                currentHealth = Mathf.Min(maxHealth, currentHealth + healingPower);
                if (karmaSystem != null)
                {
                    karmaSystem.AddKarmaForAction(KarmaAction.HealSelf);
                }
                
                // Исцеляем ближайших NPC
                Collider[] colliders = Physics.OverlapSphere(transform.position, 5f);
                foreach (Collider collider in colliders)
                {
                    NPCController npc = collider.GetComponent<NPCController>();
                    if (npc != null)
                    {
                        npc.Heal(healingPower * 0.5f);
                        if (karmaSystem != null)
                        {
                            karmaSystem.AddKarmaForAction(KarmaAction.HealOthers);
                        }
                    }
                }
                
                Debug.Log("Использована способность исцеления");
                
                // Обновляем квесты
                if (questSystem != null)
                {
                    questSystem.UpdateObjective("EMO_001", 0, 1);
                }
            }
        }
        
        private void UseTeleport()
        {
            if (chakraSystem != null && chakraSystem.CanUseAbility(30f))
            {
                chakraSystem.UseEnergy(30f);
                
                // Телепортация в направлении взгляда
                Vector3 teleportDirection = playerCamera.transform.forward;
                Vector3 teleportPosition = transform.position + teleportDirection * teleportRange;
                
                // Проверяем, можно ли телепортироваться
                if (Physics.Raycast(teleportPosition, Vector3.down, out RaycastHit hit, 10f))
                {
                    transform.position = hit.point + Vector3.up;
                    Debug.Log("Телепортация выполнена!");
                }
            }
        }
        
        private void UnlockAbility(string abilityName)
        {
            if (!unlockedAbilities.Contains(abilityName))
            {
                unlockedAbilities.Add(abilityName);
                
                // Применяем эффекты способности
                switch (abilityName)
                {
                    case "MeditationMastery":
                        meditationRequired *= 0.8f; // Быстрее медитировать
                        healingPower *= 1.5f; // Сильнее исцелять
                        break;
                        
                    case "ChakraVision":
                        interactionRange *= 1.5f; // Видеть дальше
                        break;
                        
                    case "DimensionalShift":
                        teleportRange *= 2f; // Дальше телепортироваться
                        break;
                }
                
                Debug.Log($"Способность разблокирована: {abilityName}");
            }
        }
        
        public void TakeDamage(float damage)
        {
            currentHealth -= damage;
            if (moodSystem != null)
            {
                moodSystem.ApplyDamageShock(damage);
            }
            
            if (currentHealth <= 0)
            {
                Die();
            }
        }
        
        private void Die()
        {
            Debug.Log("Игрок умер. Начинается переход в другой мир...");
            
            // Уведомляем GameManager о смерти
            if (gameManager != null)
            {
                gameManager.OnPlayerDeath();
            }
        }
        
        public void Heal(float amount)
        {
            currentHealth = Mathf.Min(maxHealth, currentHealth + amount);
        }
        
        private void UpdateUI()
        {
            // Обновляем UI элементы
            if (healthBar != null)
            {
                // Обновляем полоску здоровья
            }
            
            if (energyBar != null && chakraSystem != null)
            {
                // Обновляем полоску энергии
            }
            
            if (chakraUI != null)
            {
                // Обновляем UI чакр
            }
        }
        
        public bool HasAbility(string abilityName)
        {
            return unlockedAbilities.Contains(abilityName);
        }
        
        public float GetHealthPercentage()
        {
            return currentHealth / maxHealth;
        }
        
        public float GetMeditationProgress()
        {
            return meditationProgress / meditationRequired;
        }
    }
    
    // Интерфейс для взаимодействия
    public interface IInteractable
    {
        void Interact(PlayerController player);
    }
}