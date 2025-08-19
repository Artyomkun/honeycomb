using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
    /// <summary>
    /// Система Крийя-йоги для духовного развития и пробуждения Кундалини
    /// Основана на древних учениях и практиках самопознания
    /// </summary>
    public class KriyaYogaSystem : MonoBehaviour
    {
        [Header("Kriya Settings")]
        public float kriyaEnergyMultiplier = 2.0f;
        public float kundaliniAwakeningThreshold = 1000f;
        public float pranaFlowRate = 10f;
        
        [Header("Chakra Activation")]
        public bool[] chakraActivated = new bool[12];
        public float[] chakraEnergyLevels = new float[12];
        public float[] kundaliniFlow = new float[12];
        
        [Header("Current State")]
        public bool isKriyaActive = false;
        public float currentKriyaTime = 0f;
        public int currentKriyaLevel = 1;
        public float kundaliniEnergy = 0f;
        public float pranaEnergy = 0f;
        
        [Header("Kriya Techniques")]
        public List<KriyaTechnique> availableTechniques = new List<KriyaTechnique>();
        
        private ChakraSystem chakraSystem;
        private GameManager gameManager;
        private PlayerController playerController;
        
        // Крийя техники для разных уровней
        private Dictionary<int, KriyaTechnique> kriyaTechniques;
        
        void Start()
        {
            InitializeSystem();
            InitializeKriyaTechniques();
        }
        
        void Update()
        {
            if (isKriyaActive)
            {
                UpdateKriyaPractice();
            }
            
            UpdatePranaFlow();
            UpdateKundaliniFlow();
        }
        
        private void InitializeSystem()
        {
            gameManager = GameManager.Instance;
            if (gameManager != null)
            {
                chakraSystem = gameManager.GetComponent<ChakraSystem>();
            }
            
            playerController = FindObjectOfType<PlayerController>();
            
            // Инициализируем чакры
            for (int i = 0; i < 12; i++)
            {
                chakraActivated[i] = false;
                chakraEnergyLevels[i] = 0f;
                kundaliniFlow[i] = 0f;
            }
        }
        
        private void InitializeKriyaTechniques()
        {
            kriyaTechniques = new Dictionary<int, KriyaTechnique>
            {
                { 1, new KriyaTechnique("Муладхара Крийя", "LAM", "Пробуждение корневой чакры", 1.0f, "Заземление и стабильность") },
                { 2, new KriyaTechnique("Свадхистана Крийя", "VAM", "Активация сакральной энергии", 1.2f, "Творчество и эмоции") },
                { 3, new KriyaTechnique("Манипура Крийя", "RAM", "Пробуждение солнечного сплетения", 1.5f, "Воля и сила") },
                { 4, new KriyaTechnique("Анахата Крийя", "YAM", "Открытие сердечной чакры", 2.0f, "Любовь и сострадание") },
                { 5, new KriyaTechnique("Вишудха Крийя", "HAM", "Активация горловой чакры", 2.5f, "Коммуникация и истина") },
                { 6, new KriyaTechnique("Аджна Крийя", "Тишина", "Пробуждение третьего глаза", 3.0f, "Интуиция и мудрость") },
                { 7, new KriyaTechnique("Сахасрара Крийя", "Тишина", "Открытие коронной чакры", 4.0f, "Духовность и единство") },
                { 8, new KriyaTechnique("Антардхана Крийя", "KRIM", "Межпространственные путешествия", 5.0f, "Доступ к параллельным мирам") },
                { 9, new KriyaTechnique("Парадхана Крийя", "HRIM", "Создание реальности", 7.0f, "Манипуляция материей") },
                { 10, new KriyaTechnique("Махадхана Крийя", "SRIM", "Понимание 5D", 10.0f, "Доступ к Луне") },
                { 11, new KriyaTechnique("Парамадхана Крийя", "PRAM", "Единство с космосом", 15.0f, "Бессмертие") },
                { 12, new KriyaTechnique("Атмадхана Крийя", "ATMA", "Статус Куратора", 20.0f, "Бесконечная мудрость") }
            };
        }
        
        public void StartKriyaPractice(int kriyaLevel)
        {
            if (kriyaLevel < 1 || kriyaLevel > 12) return;
            
            // Проверяем требования для Крийя практики
            if (!CanPracticeKriya(kriyaLevel)) return;
            
            currentKriyaLevel = kriyaLevel;
            isKriyaActive = true;
            currentKriyaTime = 0f;
            
            // Активируем соответствующую чакру
            ActivateChakra(kriyaLevel);
            
            // Применяем эффекты Крийя
            ApplyKriyaEffects(true);
            
            Debug.Log($"Начинается Крийя практика уровня {kriyaLevel}: {kriyaTechniques[kriyaLevel].name}");
            Debug.Log($"Мантра: {kriyaTechniques[kriyaLevel].mantra}");
            Debug.Log($"Описание: {kriyaTechniques[kriyaLevel].description}");
            Debug.Log($"Эффект: {kriyaTechniques[kriyaLevel].effect}");
        }
        
        public void StopKriyaPractice()
        {
            if (!isKriyaActive) return;
            
            isKriyaActive = false;
            
            // Применяем эффекты завершения
            ApplyKriyaEffects(false);
            
            // Проверяем прогресс
            CheckKriyaProgress();
            
            Debug.Log("Крийя практика завершена");
        }
        
        private void UpdateKriyaPractice()
        {
            if (!isKriyaActive) return;
            
            float timeMultiplier = CalculateKriyaMultiplier();
            currentKriyaTime += Time.deltaTime * timeMultiplier;
            
            // Увеличиваем энергию Кундалини
            float kundaliniGain = Time.deltaTime * kriyaTechniques[currentKriyaLevel].energyMultiplier * kriyaEnergyMultiplier;
            kundaliniEnergy = Mathf.Min(kundaliniAwakeningThreshold, kundaliniEnergy + kundaliniGain);
            
            // Увеличиваем энергию чакры
            chakraEnergyLevels[currentKriyaLevel - 1] += Time.deltaTime * kriyaTechniques[currentKriyaLevel].energyMultiplier;
            
            // Проверяем пробуждение Кундалини
            if (kundaliniEnergy >= kundaliniAwakeningThreshold)
            {
                AwakenKundalini();
            }
            
            // Проверяем завершение практики
            float requiredTime = GetRequiredKriyaTime(currentKriyaLevel);
            if (currentKriyaTime >= requiredTime)
            {
                CompleteKriyaPractice();
            }
        }
        
        private float CalculateKriyaMultiplier()
        {
            float multiplier = 1.0f;
            
            // Бонус за правильную чакру
            if (currentKriyaLevel == gameManager.currentChakraLevel)
            {
                multiplier *= 1.5f;
            }
            
            // Бонус за активированные чакры
            int activatedChakras = 0;
            for (int i = 0; i < currentKriyaLevel; i++)
            {
                if (chakraActivated[i]) activatedChakras++;
            }
            
            multiplier *= (1f + activatedChakras * 0.1f);
            
            return multiplier;
        }
        
        private bool CanPracticeKriya(int kriyaLevel)
        {
            // Базовые Крийя доступны всегда
            if (kriyaLevel <= 3) return true;
            
            // Для 4-й Крийя нужны предыдущие 3 чакры
            if (kriyaLevel == 4)
            {
                return HasMasteredChakras(1, 3);
            }
            
            // Для 7-й Крийя нужны предыдущие 6 чакр
            if (kriyaLevel == 7)
            {
                return HasMasteredChakras(1, 6);
            }
            
            // Для высших Крийя нужен соответствующий уровень
            return gameManager.currentChakraLevel >= kriyaLevel;
        }
        
        private bool HasMasteredChakras(int fromLevel, int toLevel)
        {
            for (int i = fromLevel; i <= toLevel; i++)
            {
                if (!chakraActivated[i - 1])
                {
                    return false;
                }
            }
            return true;
        }
        
        private void ActivateChakra(int chakraLevel)
        {
            if (chakraLevel > 0 && chakraLevel <= 12)
            {
                chakraActivated[chakraLevel - 1] = true;
                Debug.Log($"Чакра {chakraLevel} активирована через Крийя практику!");
            }
        }
        
        private void ApplyKriyaEffects(bool isStarting)
        {
            if (playerController == null) return;
            
            if (isStarting)
            {
                // Замедляем движение во время Крийя
                playerController.moveSpeed *= 0.2f;
                playerController.runSpeed *= 0.2f;
                
                // Добавляем визуальные эффекты
                StartCoroutine(KriyaVisualEffects());
            }
            else
            {
                // Восстанавливаем нормальную скорость
                playerController.moveSpeed = 5f;
                playerController.runSpeed = 8f;
            }
        }
        
        private IEnumerator KriyaVisualEffects()
        {
            // Здесь можно добавить пост-процессинг эффекты
            // Например, изменение цветов, размытие, свечение и т.д.
            
            while (isKriyaActive)
            {
                // Пульсация эффектов
                yield return new WaitForSeconds(0.5f);
            }
        }
        
        private void UpdatePranaFlow()
        {
            // Обновляем поток Праны через все чакры
            for (int i = 0; i < 12; i++)
            {
                if (chakraActivated[i])
                {
                    pranaEnergy += Time.deltaTime * pranaFlowRate * (i + 1) * 0.1f;
                }
            }
        }
        
        private void UpdateKundaliniFlow()
        {
            // Обновляем поток Кундалини через позвоночник
            for (int i = 0; i < 12; i++)
            {
                if (chakraActivated[i])
                {
                    kundaliniFlow[i] += Time.deltaTime * kundaliniEnergy * 0.01f;
                }
            }
        }
        
        private void AwakenKundalini()
        {
            Debug.Log("🔥 КУНДАЛИНИ ПРОБУЖДЕНА! 🔥");
            Debug.Log("Открываются новые возможности и измерения!");
            
            // Даем бонусы к энергии
            if (chakraSystem != null)
            {
                chakraSystem.currentEnergy = chakraSystem.maxEnergy * 2f;
                chakraSystem.maxEnergy *= 1.5f;
            }
            
            // Активируем все чакры
            for (int i = 0; i < 12; i++)
            {
                chakraActivated[i] = true;
                chakraEnergyLevels[i] = 100f;
            }
            
            // Проверяем доступ к Луне
            gameManager.CheckMoonAccess();
        }
        
        private float GetRequiredKriyaTime(int kriyaLevel)
        {
            // Время для каждой Крийя практики
            float[] times = { 5f, 10f, 15f, 20f, 25f, 30f, 40f, 50f, 60f, 75f, 90f, 120f };
            return kriyaLevel > 0 && kriyaLevel <= times.Length ? times[kriyaLevel - 1] : 30f;
        }
        
        private void CheckKriyaProgress()
        {
            float progress = currentKriyaTime / GetRequiredKriyaTime(currentKriyaLevel);
            
            if (progress >= 0.9f)
            {
                Debug.Log($"Отличная Крийя практика! Прогресс: {progress:P0}");
                
                // Даем бонус к энергии
                if (chakraSystem != null)
                {
                    chakraSystem.currentEnergy = chakraSystem.maxEnergy;
                }
            }
            else if (progress >= 0.6f)
            {
                Debug.Log($"Хорошая Крийя практика. Прогресс: {progress:P0}");
            }
            else
            {
                Debug.Log($"Короткая Крийя практика. Прогресс: {progress:P0}");
            }
        }
        
        private void CompleteKriyaPractice()
        {
            Debug.Log($"Крийя практика уровня {currentKriyaLevel} завершена!");
            
            // Даем опыт и энергию
            if (chakraSystem != null)
            {
                float energyBonus = kriyaTechniques[currentKriyaLevel].energyMultiplier * 100f;
                chakraSystem.currentEnergy = Mathf.Min(chakraSystem.maxEnergy, 
                    chakraSystem.currentEnergy + energyBonus);
            }
            
            // Проверяем, можно ли повысить уровень чакр
            CheckChakraAdvancement();
            
            StopKriyaPractice();
        }
        
        private void CheckChakraAdvancement()
        {
            // Логика повышения уровня чакр на основе Крийя практики
            int activatedCount = 0;
            for (int i = 0; i < 12; i++)
            {
                if (chakraActivated[i]) activatedCount++;
            }
            
            if (activatedCount >= gameManager.currentChakraLevel + 1)
            {
                gameManager.AdvanceChakraLevel();
            }
        }
        
        public KriyaTechnique GetCurrentTechnique()
        {
            if (kriyaTechniques.ContainsKey(currentKriyaLevel))
            {
                return kriyaTechniques[currentKriyaLevel];
            }
            return null;
        }
        
        public float GetKriyaProgress()
        {
            return currentKriyaTime / GetRequiredKriyaTime(currentKriyaLevel);
        }
        
        public bool IsKriyaActive()
        {
            return isKriyaActive;
        }
        
        public float GetKundaliniEnergy()
        {
            return kundaliniEnergy;
        }
        
        public float GetPranaEnergy()
        {
            return pranaEnergy;
        }
        
        public bool IsChakraActivated(int chakraLevel)
        {
            return chakraLevel > 0 && chakraLevel <= 12 ? chakraActivated[chakraLevel - 1] : false;
        }
    }
    
    /// <summary>
    /// Техника Крийя для конкретной чакры
    /// </summary>
    [System.Serializable]
    public class KriyaTechnique
    {
        public string name;
        public string mantra;
        public string description;
        public float energyMultiplier;
        public string effect;
        
        public KriyaTechnique(string name, string mantra, string description, float energyMultiplier, string effect)
        {
            this.name = name;
            this.mantra = mantra;
            this.description = description;
            this.energyMultiplier = energyMultiplier;
            this.effect = effect;
        }
    }
}