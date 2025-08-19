using UnityEngine;
using System.Collections.Generic;
using System;

namespace RealLife5D.Systems
{
    [System.Serializable]
    public class QuestData
    {
        public string id;
        public string title;
        public string description;
        public QuestType questType;
        public int requiredChakraLevel;
        public int requiredDimension;
        public List<QuestObjective> objectives;
        public List<QuestReward> rewards;
        public bool isCompleted;
        public bool isActive;
        public float timeLimit; // В минутах, 0 = без ограничений
        public QuestDifficulty difficulty;
    }
    
    public enum QuestType
    {
        Physical,       // Физические квесты
        Emotional,      // Эмоциональные квесты
        Mental,         // Ментальные квесты
        Spiritual,      // Духовные квесты
        Dimensional,    // Квесты измерений
        WorldSpecific,  // Специфичные для мира
        Chakra,         // Квесты развития чакр
        Moon           // Квесты для доступа к Луне
    }
    
    public enum QuestDifficulty
    {
        Easy = 1,
        Normal = 2,
        Hard = 3,
        Extreme = 4,
        Impossible = 5
    }
    
    [System.Serializable]
    public class QuestObjective
    {
        public string description;
        public int targetAmount;
        public int currentAmount;
        public bool isCompleted;
        public ObjectiveType type;
    }
    
    public enum ObjectiveType
    {
        Collect,        // Собрать предметы
        Kill,           // Уничтожить врагов
        Interact,       // Взаимодействовать с объектами
        Travel,         // Путешествовать
        Meditate,       // Медитировать
        Heal,           // Исцелять
        Create,         // Создавать
        Destroy         // Уничтожать
    }
    
    [System.Serializable]
    public class QuestReward
    {
        public RewardType type;
        public string itemId;
        public int amount;
        public float value;
    }
    
    public enum RewardType
    {
        Experience,     // Опыт
        ChakraEnergy,   // Энергия чакр
        Items,          // Предметы
        Abilities,      // Способности
        WorldAccess,    // Доступ к мирам
        DimensionShift  // Сдвиг измерения
    }
    
    public class QuestSystem : MonoBehaviour
    {
        [Header("Quest Data")]
        public List<QuestData> allQuests = new List<QuestData>();
        public List<QuestData> activeQuests = new List<QuestData>();
        public List<QuestData> completedQuests = new List<QuestData>();
        
        [Header("Quest Generation")]
        public int maxActiveQuests = 5;
        public float questGenerationInterval = 300f; // 5 минут
        
        private GameManager gameManager;
        private ChakraSystem chakraSystem;
        private DimensionSystem dimensionSystem;
        private WorldSystem worldSystem;
        private float lastQuestGeneration;
        
        void Start()
        {
            gameManager = GameManager.Instance;
            chakraSystem = GetComponent<ChakraSystem>();
            dimensionSystem = GetComponent<DimensionSystem>();
            worldSystem = GetComponent<WorldSystem>();
            
            InitializeQuests();
            GenerateInitialQuests();
        }
        
        void Update()
        {
            // Проверяем завершение квестов
            CheckQuestCompletion();
            
            // Генерируем новые квесты
            if (Time.time - lastQuestGeneration > questGenerationInterval)
            {
                GenerateNewQuests();
                lastQuestGeneration = Time.time;
            }
        }
        
        private void InitializeQuests()
        {
            // Физические квесты (1-3 чакры)
            CreateQuest("PHY_001", "Первые шаги", "Изучите основы физического мира", 
                QuestType.Physical, 1, 1, QuestDifficulty.Easy);
            
            CreateQuest("PHY_002", "Выносливость", "Пробегите 1 км без остановки", 
                QuestType.Physical, 2, 1, QuestDifficulty.Normal);
            
            CreateQuest("PHY_003", "Сила воли", "Поднимите тяжелый предмет 10 раз", 
                QuestType.Physical, 3, 2, QuestDifficulty.Normal);
            
            // Эмоциональные квесты (4-6 чакры)
            CreateQuest("EMO_001", "Сердечная чакра", "Помогите 5 людям в беде", 
                QuestType.Emotional, 4, 3, QuestDifficulty.Hard);
            
            CreateQuest("EMO_002", "Творчество", "Создайте произведение искусства", 
                QuestType.Emotional, 5, 3, QuestDifficulty.Normal);
            
            CreateQuest("EMO_003", "Интуиция", "Примите 10 правильных решений подряд", 
                QuestType.Emotional, 6, 3, QuestDifficulty.Hard);
            
            // Ментальные квесты (7-9 чакры)
            CreateQuest("MEN_001", "Мудрость", "Изучите древний текст", 
                QuestType.Mental, 7, 4, QuestDifficulty.Hard);
            
            CreateQuest("MEN_002", "Просветление", "Проведите 24 часа в медитации", 
                QuestType.Mental, 8, 4, QuestDifficulty.Extreme);
            
            CreateQuest("MEN_003", "Создание реальности", "Измените физический закон", 
                QuestType.Mental, 9, 4, QuestDifficulty.Impossible);
            
            // Духовные квесты (10-12 чакры)
            CreateQuest("SPI_001", "5D понимание", "Осознайте все измерения", 
                QuestType.Spiritual, 10, 5, QuestDifficulty.Extreme);
            
            CreateQuest("SPI_002", "Единство", "Соединитесь с космическим сознанием", 
                QuestType.Spiritual, 11, 5, QuestDifficulty.Impossible);
            
            CreateQuest("SPI_003", "Кураторство", "Достигните Луны и встретьтесь с Куратором", 
                QuestType.Spiritual, 12, 5, QuestDifficulty.Impossible);
            
            // Квесты измерений
            CreateQuest("DIM_001", "2D переход", "Перейдите в двумерное измерение", 
                QuestType.Dimensional, 2, 2, QuestDifficulty.Normal);
            
            CreateQuest("DIM_002", "4D время", "Управляйте временем", 
                QuestType.Dimensional, 7, 4, QuestDifficulty.Hard);
            
            CreateQuest("DIM_003", "5D дух", "Достигните духовного измерения", 
                QuestType.Dimensional, 10, 5, QuestDifficulty.Extreme);
            
            // Специфичные для мира квесты
            CreateWorldSpecificQuests();
        }
        
        private void CreateQuest(string id, string title, string description, 
            QuestType type, int chakraLevel, int dimension, QuestDifficulty difficulty)
        {
            QuestData quest = new QuestData
            {
                id = id,
                title = title,
                description = description,
                questType = type,
                requiredChakraLevel = chakraLevel,
                requiredDimension = dimension,
                objectives = GenerateObjectives(type, difficulty),
                rewards = GenerateRewards(type, difficulty),
                isCompleted = false,
                isActive = false,
                timeLimit = GetTimeLimit(difficulty),
                difficulty = difficulty
            };
            
            allQuests.Add(quest);
        }
        
        private void CreateWorldSpecificQuests()
        {
            // Квесты для разных типов миров
            CreateQuest("WORLD_APO_001", "Выживание", "Выживите в постапокалиптическом мире", 
                QuestType.WorldSpecific, 2, 2, QuestDifficulty.Hard);
            
            CreateQuest("WORLD_UTOP_001", "Гармония", "Достигните полной гармонии с миром", 
                QuestType.WorldSpecific, 3, 3, QuestDifficulty.Normal);
            
            CreateQuest("WORLD_TECH_001", "Кибернетика", "Станьте киборгом", 
                QuestType.WorldSpecific, 4, 3, QuestDifficulty.Hard);
            
            CreateQuest("WORLD_MYST_001", "Магия", "Изучите 10 заклинаний", 
                QuestType.WorldSpecific, 5, 3, QuestDifficulty.Hard);
            
            CreateQuest("WORLD_ALIEN_001", "Контакт", "Установите связь с инопланетянами", 
                QuestType.WorldSpecific, 6, 3, QuestDifficulty.Extreme);
            
            CreateQuest("WORLD_TIME_001", "Временная петля", "Выйдите из временной петли", 
                QuestType.WorldSpecific, 7, 4, QuestDifficulty.Extreme);
            
            CreateQuest("WORLD_QUANT_001", "Квантовая суперпозиция", "Существуйте в двух состояниях одновременно", 
                QuestType.WorldSpecific, 8, 4, QuestDifficulty.Impossible);
        }
        
        private List<QuestObjective> GenerateObjectives(QuestType type, QuestDifficulty difficulty)
        {
            List<QuestObjective> objectives = new List<QuestObjective>();
            
            int objectiveCount = (int)difficulty;
            
            for (int i = 0; i < objectiveCount; i++)
            {
                QuestObjective objective = new QuestObjective
                {
                    description = GenerateObjectiveDescription(type, i),
                    targetAmount = UnityEngine.Random.Range(1, 10) * (int)difficulty,
                    currentAmount = 0,
                    isCompleted = false,
                    type = GetObjectiveType(type, i)
                };
                
                objectives.Add(objective);
            }
            
            return objectives;
        }
        
        private string GenerateObjectiveDescription(QuestType type, int index)
        {
            switch (type)
            {
                case QuestType.Physical:
                    string[] physicalTasks = { "Пробежать", "Поднять", "Проплыть", "Прыгнуть", "Присесть" };
                    return $"{physicalTasks[index % physicalTasks.Length]} {UnityEngine.Random.Range(1, 20)} раз";
                    
                case QuestType.Emotional:
                    string[] emotionalTasks = { "Помочь", "Пожалеть", "Поддержать", "Выслушать", "Утешить" };
                    return $"{emotionalTasks[index % emotionalTasks.Length]} {UnityEngine.Random.Range(1, 10)} человек";
                    
                case QuestType.Mental:
                    string[] mentalTasks = { "Изучить", "Решить", "Понять", "Запомнить", "Анализировать" };
                    return $"{mentalTasks[index % mentalTasks.Length]} {UnityEngine.Random.Range(1, 5)} предметов";
                    
                case QuestType.Spiritual:
                    string[] spiritualTasks = { "Медитировать", "Молиться", "Созерцать", "Размышлять", "Практиковать" };
                    return $"{spiritualTasks[index % spiritualTasks.Length]} {UnityEngine.Random.Range(1, 12)} часов";
                    
                default:
                    return $"Выполнить задание {index + 1}";
            }
        }
        
        private ObjectiveType GetObjectiveType(QuestType type, int index)
        {
            switch (type)
            {
                case QuestType.Physical:
                    return ObjectiveType.Travel;
                case QuestType.Emotional:
                    return ObjectiveType.Interact;
                case QuestType.Mental:
                    return ObjectiveType.Collect;
                case QuestType.Spiritual:
                    return ObjectiveType.Meditate;
                default:
                    return ObjectiveType.Interact;
            }
        }
        
        private List<QuestReward> GenerateRewards(QuestType type, QuestDifficulty difficulty)
        {
            List<QuestReward> rewards = new List<QuestReward>();
            
            // Базовый опыт
            rewards.Add(new QuestReward
            {
                type = RewardType.Experience,
                amount = (int)difficulty * 100,
                value = 0
            });
            
            // Энергия чакр
            rewards.Add(new QuestReward
            {
                type = RewardType.ChakraEnergy,
                amount = (int)difficulty * 50,
                value = 0
            });
            
            // Специальные награды в зависимости от типа
            switch (type)
            {
                case QuestType.Chakra:
                    rewards.Add(new QuestReward
                    {
                        type = RewardType.Abilities,
                        itemId = "ChakraBoost",
                        amount = 1,
                        value = 0
                    });
                    break;
                    
                case QuestType.Dimensional:
                    rewards.Add(new QuestReward
                    {
                        type = RewardType.DimensionShift,
                        itemId = "DimensionKey",
                        amount = 1,
                        value = 0
                    });
                    break;
                    
                case QuestType.WorldSpecific:
                    rewards.Add(new QuestReward
                    {
                        type = RewardType.WorldAccess,
                        itemId = "WorldPortal",
                        amount = 1,
                        value = 0
                    });
                    break;
            }
            
            return rewards;
        }
        
        private float GetTimeLimit(QuestDifficulty difficulty)
        {
            switch (difficulty)
            {
                case QuestDifficulty.Easy: return 60f;      // 1 час
                case QuestDifficulty.Normal: return 180f;   // 3 часа
                case QuestDifficulty.Hard: return 360f;     // 6 часов
                case QuestDifficulty.Extreme: return 720f;  // 12 часов
                case QuestDifficulty.Impossible: return 1440f; // 24 часа
                default: return 0f;
            }
        }
        
        private void GenerateInitialQuests()
        {
            // Генерируем начальные квесты, доступные игроку
            List<QuestData> availableQuests = GetAvailableQuests();
            
            for (int i = 0; i < Mathf.Min(maxActiveQuests, availableQuests.Count); i++)
            {
                ActivateQuest(availableQuests[i]);
            }
        }
        
        private void GenerateNewQuests()
        {
            if (activeQuests.Count < maxActiveQuests)
            {
                List<QuestData> availableQuests = GetAvailableQuests();
                List<QuestData> inactiveQuests = availableQuests.FindAll(q => !q.isActive && !q.isCompleted);
                
                if (inactiveQuests.Count > 0)
                {
                    QuestData randomQuest = inactiveQuests[UnityEngine.Random.Range(0, inactiveQuests.Count)];
                    ActivateQuest(randomQuest);
                }
            }
        }
        
        private List<QuestData> GetAvailableQuests()
        {
            return allQuests.FindAll(q => 
                q.requiredChakraLevel <= gameManager.currentChakraLevel &&
                q.requiredDimension <= gameManager.currentDimension);
        }
        
        public void ActivateQuest(QuestData quest)
        {
            if (!quest.isActive && !quest.isCompleted)
            {
                quest.isActive = true;
                activeQuests.Add(quest);
                Debug.Log($"Квест активирован: {quest.title}");
            }
        }
        
        public void CompleteQuest(QuestData quest)
        {
            if (quest.isActive && !quest.isCompleted)
            {
                quest.isCompleted = true;
                quest.isActive = false;
                activeQuests.Remove(quest);
                completedQuests.Add(quest);
                
                // Выдаем награды
                GiveRewards(quest);
                
                Debug.Log($"Квест завершен: {quest.title}");
                
                // Проверяем, можно ли повысить уровень чакр
                CheckChakraAdvancement();
            }
        }
        
        private void GiveRewards(QuestData quest)
        {
            foreach (QuestReward reward in quest.rewards)
            {
                switch (reward.type)
                {
                    case RewardType.Experience:
                        // Добавляем опыт
                        break;
                        
                    case RewardType.ChakraEnergy:
                        if (chakraSystem != null)
                        {
                            chakraSystem.currentEnergy += reward.amount;
                        }
                        break;
                        
                    case RewardType.Abilities:
                        // Разблокируем способности
                        break;
                        
                    case RewardType.WorldAccess:
                        if (worldSystem != null)
                        {
                            // Разблокируем доступ к миру
                        }
                        break;
                        
                    case RewardType.DimensionShift:
                        if (dimensionSystem != null)
                        {
                            // Активируем сдвиг измерения
                        }
                        break;
                }
            }
        }
        
        private void CheckChakraAdvancement()
        {
            // Проверяем, достаточно ли выполнено квестов для повышения уровня чакр
            int completedQuestsCount = completedQuests.Count;
            int currentChakraLevel = gameManager.currentChakraLevel;
            
            // Примерная логика: каждые 5 квестов = новый уровень чакр
            if (completedQuestsCount >= currentChakraLevel * 5)
            {
                gameManager.AdvanceChakraLevel();
            }
        }
        
        private void CheckQuestCompletion()
        {
            foreach (QuestData quest in activeQuests.ToArray())
            {
                if (IsQuestCompleted(quest))
                {
                    CompleteQuest(quest);
                }
            }
        }
        
        private bool IsQuestCompleted(QuestData quest)
        {
            foreach (QuestObjective objective in quest.objectives)
            {
                if (!objective.isCompleted)
                {
                    return false;
                }
            }
            return true;
        }
        
        public void UpdateObjective(string questId, int objectiveIndex, int progress)
        {
            QuestData quest = activeQuests.Find(q => q.id == questId);
            if (quest != null && objectiveIndex < quest.objectives.Count)
            {
                QuestObjective objective = quest.objectives[objectiveIndex];
                objective.currentAmount += progress;
                
                if (objective.currentAmount >= objective.targetAmount)
                {
                    objective.isCompleted = true;
                }
            }
        }
        
        public List<QuestData> GetActiveQuests()
        {
            return activeQuests;
        }
        
        public List<QuestData> GetCompletedQuests()
        {
            return completedQuests;
        }
        
        public QuestData GetQuestById(string questId)
        {
            return allQuests.Find(q => q.id == questId);
        }
    }
}