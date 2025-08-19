using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
    /// <summary>
    /// Система медитации для развития чакр и духовного роста
    /// Основана на учениях Сирди Дэйл, Садхгуру, Ошо и "Науке самоосознания"
    /// </summary>
    public class MeditationSystem : MonoBehaviour
    {
        [Header("Meditation Settings")]
        public float baseMeditationSpeed = 1f;
        public float chakraBonusMultiplier = 1.5f;
        public float stagnationPenalty = 0.5f;
        
        [Header("Chakra Requirements")]
        public int[] chakraLevelRequirements = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
        public float[] meditationTimeRequirements = { 10f, 20f, 30f, 45f, 60f, 90f, 120f, 180f, 240f, 300f, 360f, 480f };
        
        [Header("Current State")]
        public bool isMeditating = false;
        public float currentMeditationTime = 0f;
        public int currentChakraFocus = 1;
        public float meditationProgress = 0f;
        
        private ChakraSystem chakraSystem;
        private GameManager gameManager;
        private PlayerController playerController;
        
        // Медитационные техники для каждой чакры
        private Dictionary<int, MeditationTechnique> chakraTechniques;
        
        void Start()
        {
            InitializeSystem();
            InitializeChakraTechniques();
        }
        
        void Update()
        {
            if (isMeditating)
            {
                UpdateMeditation();
            }
        }
        
        private void InitializeSystem()
        {
            gameManager = GameManager.Instance;
            if (gameManager != null)
            {
                chakraSystem = gameManager.GetComponent<ChakraSystem>();
            }
            
            playerController = FindObjectOfType<PlayerController>();
        }
        
        private void InitializeChakraTechniques()
        {
            chakraTechniques = new Dictionary<int, MeditationTechnique>
            {
                { 1, new MeditationTechnique("Муладхара", "LAM", "Корневая чакра - заземление и стабильность", 1.0f) },
                { 2, new MeditationTechnique("Свадхистана", "VAM", "Сакральная чакра - творчество и эмоции", 1.2f) },
                { 3, new MeditationTechnique("Манипура", "RAM", "Солнечное сплетение - воля и сила", 1.5f) },
                { 4, new MeditationTechnique("Анахата", "YAM", "Сердечная чакра - любовь и сострадание", 2.0f) },
                { 5, new MeditationTechnique("Вишудха", "HAM", "Горловая чакра - коммуникация и истина", 2.5f) },
                { 6, new MeditationTechnique("Аджна", "Тишина", "Третий глаз - интуиция и мудрость", 3.0f) },
                { 7, new MeditationTechnique("Сахасрара", "Тишина", "Коронная чакра - духовность и единство", 4.0f) },
                { 8, new MeditationTechnique("Антардхана", "KRIM", "Скрытая чакра - межпространственные путешествия", 5.0f) },
                { 9, new MeditationTechnique("Парадхана", "HRIM", "Высшая чакра - создание реальности", 7.0f) },
                { 10, new MeditationTechnique("Махадхана", "SRIM", "Великая чакра - понимание 5D", 10.0f) },
                { 11, new MeditationTechnique("Парамадхана", "PRAM", "Абсолютная чакра - единство с космосом", 15.0f) },
                { 12, new MeditationTechnique("Атмадхана", "ATMA", "Душевная чакра - готовность стать Куратором", 20.0f) }
            };
        }
        
        public void StartMeditation(int chakraLevel)
        {
            if (chakraLevel < 1 || chakraLevel > 12) return;
            
            // Проверяем требования для медитации на этой чакре
            if (!CanMeditateOnChakra(chakraLevel)) return;
            
            currentChakraFocus = chakraLevel;
            isMeditating = true;
            currentMeditationTime = 0f;
            meditationProgress = 0f;
            
            // Применяем эффекты медитации
            ApplyMeditationEffects(true);
            
            Debug.Log($"Начинается медитация на чакре {chakraLevel}: {chakraTechniques[chakraLevel].name}");
            Debug.Log($"Мантра: {chakraTechniques[chakraLevel].mantra}");
            Debug.Log($"Описание: {chakraTechniques[chakraLevel].description}");
        }
        
        public void StopMeditation()
        {
            if (!isMeditating) return;
            
            isMeditating = false;
            
            // Применяем эффекты завершения медитации
            ApplyMeditationEffects(false);
            
            // Проверяем прогресс
            CheckMeditationProgress();
            
            Debug.Log("Медитация завершена");
        }
        
        private void UpdateMeditation()
        {
            if (!isMeditating) return;
            
            float timeMultiplier = CalculateTimeMultiplier();
            currentMeditationTime += Time.deltaTime * timeMultiplier;
            
            // Вычисляем прогресс
            float requiredTime = meditationTimeRequirements[currentChakraFocus - 1];
            meditationProgress = Mathf.Clamp01(currentMeditationTime / requiredTime);
            
            // Обновляем энергию чакр
            if (chakraSystem != null)
            {
                float energyGain = Time.deltaTime * chakraTechniques[currentChakraFocus].energyMultiplier;
                chakraSystem.currentEnergy = Mathf.Min(chakraSystem.maxEnergy, 
                    chakraSystem.currentEnergy + energyGain);
            }
            
            // Проверяем завершение медитации
            if (currentMeditationTime >= requiredTime)
            {
                CompleteMeditation();
            }
        }
        
        private float CalculateTimeMultiplier()
        {
            float multiplier = baseMeditationSpeed;
            
            // Бонус за правильную чакру
            if (currentChakraFocus == gameManager.currentChakraLevel)
            {
                multiplier *= chakraBonusMultiplier;
            }
            
            // Штраф за застой
            if (HasStagnation())
            {
                multiplier *= stagnationPenalty;
            }
            
            return multiplier;
        }
        
        private bool CanMeditateOnChakra(int chakraLevel)
        {
            // Базовые чакры доступны всегда
            if (chakraLevel <= 3) return true;
            
            // Для 4-й чакры нужны предыдущие 3
            if (chakraLevel == 4)
            {
                return HasMasteredChakras(1, 3);
            }
            
            // Для 7-й чакры нужны предыдущие 6
            if (chakraLevel == 7)
            {
                return HasMasteredChakras(1, 6);
            }
            
            // Для высших чакр нужен соответствующий уровень
            return gameManager.currentChakraLevel >= chakraLevel;
        }
        
        private bool HasMasteredChakras(int fromLevel, int toLevel)
        {
            for (int i = fromLevel; i <= toLevel; i++)
            {
                if (!chakraSystem.IsChakraUnlocked(i))
                {
                    return false;
                }
            }
            return true;
        }
        
        private bool HasStagnation()
        {
            // Проверяем, не застрял ли игрок на одном уровне
            // Это упрощенная логика - в реальной игре можно добавить более сложную
            return false;
        }
        
        private void ApplyMeditationEffects(bool isStarting)
        {
            if (playerController == null) return;
            
            if (isStarting)
            {
                // Замедляем движение во время медитации
                playerController.moveSpeed *= 0.3f;
                playerController.runSpeed *= 0.3f;
            }
            else
            {
                // Восстанавливаем нормальную скорость
                playerController.moveSpeed = 5f;
                playerController.runSpeed = 8f;
            }
        }
        
        private void CheckMeditationProgress()
        {
            if (meditationProgress >= 0.8f) // 80% прогресса
            {
                Debug.Log($"Отличная медитация! Прогресс: {meditationProgress:P0}");
                
                // Даем бонус к энергии
                if (chakraSystem != null)
                {
                    chakraSystem.currentEnergy = chakraSystem.maxEnergy;
                }
            }
            else if (meditationProgress >= 0.5f) // 50% прогресса
            {
                Debug.Log($"Хорошая медитация. Прогресс: {meditationProgress:P0}");
            }
            else
            {
                Debug.Log($"Короткая медитация. Прогресс: {meditationProgress:P0}");
            }
        }
        
        private void CompleteMeditation()
        {
            Debug.Log($"Медитация на чакре {currentChakraFocus} завершена!");
            
            // Даем опыт и энергию
            if (chakraSystem != null)
            {
                float energyBonus = chakraTechniques[currentChakraFocus].energyMultiplier * 50f;
                chakraSystem.currentEnergy = Mathf.Min(chakraSystem.maxEnergy, 
                    chakraSystem.currentEnergy + energyBonus);
            }
            
            // Проверяем, можно ли повысить уровень чакр
            CheckChakraAdvancement();
            
            StopMeditation();
        }
        
        private void CheckChakraAdvancement()
        {
            // Логика повышения уровня чакр на основе медитации
            // В реальной игре это может быть более сложно
        }
        
        public MeditationTechnique GetCurrentTechnique()
        {
            if (chakraTechniques.ContainsKey(currentChakraFocus))
            {
                return chakraTechniques[currentChakraFocus];
            }
            return null;
        }
        
        public float GetMeditationProgress()
        {
            return meditationProgress;
        }
        
        public bool IsMeditating()
        {
            return isMeditating;
        }
    }
    
    /// <summary>
    /// Техника медитации для конкретной чакры
    /// </summary>
    [System.Serializable]
    public class MeditationTechnique
    {
        public string name;
        public string mantra;
        public string description;
        public float energyMultiplier;
        
        public MeditationTechnique(string name, string mantra, string description, float energyMultiplier)
        {
            this.name = name;
            this.mantra = mantra;
            this.description = description;
            this.energyMultiplier = energyMultiplier;
        }
    }
}