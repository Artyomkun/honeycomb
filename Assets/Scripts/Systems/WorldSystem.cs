using UnityEngine;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace RealLife5D.Systems
{
    [System.Serializable]
    public class WorldData
    {
        public int index;
        public string name;
        public string description;
        public WorldType worldType;
        public float difficulty;
        public List<string> uniqueFeatures;
        public Color worldColor;
        public bool isUnlocked;
        public int requiredChakraLevel;
    }
    
    public enum WorldType
    {
        Normal,         // Обычная Земля
        Apocalyptic,    // Постапокалиптический мир
        Utopian,        // Утопический мир
        Dystopian,      // Дистопический мир
        Technological,  // Технологически развитый
        Mystical,       // Мистический мир
        Alien,          // Мир с инопланетянами
        TimeShifted,    // Мир с измененным временем
        Dimensional,    // Мир с искаженными измерениями
        Quantum         // Квантовый мир
    }
    
    public class WorldSystem : MonoBehaviour
    {
        [Header("World Data")]
        public List<WorldData> worlds = new List<WorldData>();
        
        [Header("Current State")]
        public int currentWorldIndex = 0;
        public float worldStability = 100f;
        public bool isTransitioning = false;
        
        private GameManager gameManager;
        
        void Start()
        {
            gameManager = GameManager.Instance;
            InitializeWorlds();
        }
        
        private void InitializeWorlds()
        {
            // Создаем 54 уникальных мира
            for (int i = 0; i < 54; i++)
            {
                WorldData world = GenerateWorld(i);
                worlds.Add(world);
            }
        }
        
        private WorldData GenerateWorld(int index)
        {
            WorldData world = new WorldData();
            world.index = index;
            
            // Генерируем уникальные характеристики для каждого мира
            switch (index)
            {
                case 0: // Наш мир
                    world.name = "Земля-0 (Наш Мир)";
                    world.description = "Стандартная Земля, которую мы знаем";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = true;
                    world.requiredChakraLevel = 1;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 1: // Постапокалиптический мир
                    world.name = "Земля-1 (Руины)";
                    world.description = "Мир после ядерной войны";
                    world.worldType = WorldType.Apocalyptic;
                    world.difficulty = 2.5f;
                    world.worldColor = Color.red;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 2;
                    world.uniqueFeatures = new List<string> { "Радиация", "Мутанты", "Разрушенные города" };
                    break;
                    
                case 2: // Утопический мир
                    world.name = "Земля-2 (Утопия)";
                    world.description = "Идеальный мир без конфликтов";
                    world.worldType = WorldType.Utopian;
                    world.difficulty = 0.5f;
                    world.worldColor = Color.cyan;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 3;
                    world.uniqueFeatures = new List<string> { "Мир и гармония", "Продвинутые технологии", "Духовное развитие" };
                    break;
                    
                case 3: // Технологический мир
                    world.name = "Земля-3 (Технократия)";
                    world.description = "Мир, управляемый искусственным интеллектом";
                    world.worldType = WorldType.Technological;
                    world.difficulty = 2.0f;
                    world.worldColor = Color.blue;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 4;
                    world.uniqueFeatures = new List<string> { "AI правительство", "Кибернетика", "Виртуальная реальность" };
                    break;
                    
                case 4: // Мистический мир
                    world.name = "Земля-4 (Магия)";
                    world.description = "Мир, где магия реальна";
                    world.worldType = WorldType.Mystical;
                    world.difficulty = 3.0f;
                    world.worldColor = Color.magenta;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 5;
                    world.uniqueFeatures = new List<string> { "Магические существа", "Заклинания", "Древние артефакты" };
                    break;
                    
                case 5: // Инопланетный мир
                    world.name = "Земля-5 (Контакт)";
                    world.description = "Мир, где люди и инопланетяне сосуществуют";
                    world.worldType = WorldType.Alien;
                    world.difficulty = 2.8f;
                    world.worldColor = Color.yellow;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 6;
                    world.uniqueFeatures = new List<string> { "Инопланетные технологии", "Гибриды", "Космические порталы" };
                    break;
                    
                case 6: // Временной мир
                    world.name = "Земля-6 (Время)";
                    world.description = "Мир с искаженным временем";
                    world.worldType = WorldType.TimeShifted;
                    world.difficulty = 3.5f;
                    world.worldColor = Color.orange;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 7;
                    world.uniqueFeatures = new List<string> { "Временные петли", "Ускоренное время", "Временные аномалии" };
                    break;
                    
                case 7: // Квантовый мир
                    world.name = "Земля-7 (Квант)";
                    world.description = "Мир, где квантовая физика проявляется макроскопически";
                    world.worldType = WorldType.Quantum;
                    world.difficulty = 4.0f;
                    world.worldColor = Color.purple;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 8;
                    world.uniqueFeatures = new List<string> { "Квантовая телепортация", "Суперпозиция", "Квантовые существа" };
                    break;
                    
                default: // Остальные миры генерируются случайно
                    world.name = $"Земля-{index}";
                    world.description = GenerateRandomDescription();
                    world.worldType = (WorldType)Random.Range(0, System.Enum.GetValues(typeof(WorldType)).Length);
                    world.difficulty = Random.Range(1.0f, 5.0f);
                    world.worldColor = new Color(Random.value, Random.value, Random.value);
                    world.isUnlocked = false;
                    world.requiredChakraLevel = Random.Range(1, 12);
                    world.uniqueFeatures = GenerateRandomFeatures();
                    break;
            }
            
            return world;
        }
        
        private string GenerateRandomDescription()
        {
            string[] descriptions = {
                "Мир с уникальными физическими законами",
                "Параллельная реальность с иными правилами",
                "Альтернативная версия Земли",
                "Мир, где возможно невозможное",
                "Реальность с измененными константами",
                "Параллельное измерение существования",
                "Альтернативная временная линия",
                "Мир с искаженной историей",
                "Реальность с новыми возможностями",
                "Параллельный поток времени"
            };
            
            return descriptions[Random.Range(0, descriptions.Length)];
        }
        
        private List<string> GenerateRandomFeatures()
        {
            List<string> allFeatures = new List<string>
            {
                "Измененная гравитация", "Другая атмосфера", "Иные формы жизни",
                "Магические явления", "Технологические чудеса", "Временные аномалии",
                "Пространственные искажения", "Энергетические поля", "Квантовые эффекты",
                "Психические способности", "Духовные существа", "Астральные планы"
            };
            
            List<string> selectedFeatures = new List<string>();
            int featureCount = Random.Range(2, 5);
            
            for (int i = 0; i < featureCount; i++)
            {
                string feature = allFeatures[Random.Range(0, allFeatures.Count)];
                if (!selectedFeatures.Contains(feature))
                {
                    selectedFeatures.Add(feature);
                }
            }
            
            return selectedFeatures;
        }
        
        public void TransitionToWorld(int worldIndex)
        {
            if (worldIndex >= 0 && worldIndex < worlds.Count)
            {
                if (isTransitioning) return;
                
                isTransitioning = true;
                currentWorldIndex = worldIndex;
                
                WorldData targetWorld = worlds[worldIndex];
                
                // Проверяем требования для доступа к миру
                if (gameManager.currentChakraLevel >= targetWorld.requiredChakraLevel)
                {
                    // Начинаем переход
                    StartCoroutine(WorldTransition(targetWorld));
                }
                else
                {
                    Debug.LogWarning($"Недостаточно чакр для доступа к миру {targetWorld.name}. Требуется: {targetWorld.requiredChakraLevel}");
                    isTransitioning = false;
                }
            }
        }
        
        private System.Collections.IEnumerator WorldTransition(WorldData targetWorld)
        {
            Debug.Log($"Начинается переход в мир: {targetWorld.name}");
            
            // Эффекты перехода
            yield return new WaitForSeconds(2f);
            
            // Загружаем новую сцену
            string sceneName = $"World_{targetWorld.index}";
            
            try
            {
                SceneManager.LoadScene(sceneName);
            }
            catch (System.Exception e)
            {
                Debug.LogWarning($"Сцена {sceneName} не найдена. Создаем новый мир.");
                // Здесь можно создать процедурно сгенерированный мир
                CreateProceduralWorld(targetWorld);
            }
            
            isTransitioning = false;
        }
        
        private void CreateProceduralWorld(WorldData worldData)
        {
            // Создаем процедурно сгенерированный мир
            Debug.Log($"Создается процедурный мир: {worldData.name}");
            
            // Здесь можно добавить логику генерации ландшафта, зданий, NPC и т.д.
            // В зависимости от типа мира
        }
        
        public WorldData GetCurrentWorld()
        {
            if (currentWorldIndex >= 0 && currentWorldIndex < worlds.Count)
            {
                return worlds[currentWorldIndex];
            }
            return null;
        }
        
        public List<WorldData> GetUnlockedWorlds()
        {
            return worlds.FindAll(w => w.isUnlocked);
        }
        
        public List<WorldData> GetAvailableWorlds(int chakraLevel)
        {
            return worlds.FindAll(w => w.requiredChakraLevel <= chakraLevel);
        }
        
        public void UnlockWorld(int worldIndex)
        {
            if (worldIndex >= 0 && worldIndex < worlds.Count)
            {
                worlds[worldIndex].isUnlocked = true;
                Debug.Log($"Мир {worlds[worldIndex].name} разблокирован!");
            }
        }
        
        public float GetWorldStability()
        {
            return worldStability;
        }
        
        public void ModifyWorldStability(float amount)
        {
            worldStability = Mathf.Clamp(worldStability + amount, 0f, 100f);
        }
    }
}