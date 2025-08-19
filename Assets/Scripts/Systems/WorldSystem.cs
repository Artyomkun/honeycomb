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
                    
                case 1: // Параллельная Земля
                    world.name = "Земля-1";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 2;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 2: // Параллельная Земля
                    world.name = "Земля-2";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 3;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 3: // Параллельная Земля
                    world.name = "Земля-3";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 4;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 4: // Параллельная Земля
                    world.name = "Земля-4";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 5;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 5: // Параллельная Земля
                    world.name = "Земля-5";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 6;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 6: // Параллельная Земля
                    world.name = "Земля-6";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 7;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                case 7: // Параллельная Земля
                    world.name = "Земля-7";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = 8;
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
                    break;
                    
                default: // Остальные миры - параллельные Земли
                    world.name = $"Земля-{index}";
                    world.description = "Параллельная версия Земли в том же измерении";
                    world.worldType = WorldType.Normal;
                    world.difficulty = 1.0f;
                    world.worldColor = Color.green;
                    world.isUnlocked = false;
                    world.requiredChakraLevel = Mathf.Min(index + 1, 12);
                    world.uniqueFeatures = new List<string> { "Стандартная физика", "Обычные люди", "Нормальное время" };
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