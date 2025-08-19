using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
    [System.Serializable]
    public class ChakraData
    {
        public int level;
        public string name;
        public string description;
        public Color chakraColor;
        public string mantra;
        public List<string> abilities;
        public float energyMultiplier;
        public bool isUnlocked;
    }
    
    public class ChakraSystem : MonoBehaviour
    {
        [Header("Chakra Data")]
        public List<ChakraData> chakras = new List<ChakraData>();
        
        [Header("Current State")]
        public float currentEnergy = 100f;
        public float maxEnergy = 100f;
        public float energyRegenerationRate = 5f;
        
        private GameManager gameManager;
        
        void Start()
        {
            gameManager = GameManager.Instance;
            InitializeChakras();
        }
        
        void Update()
        {
            RegenerateEnergy();
        }
        
        private void InitializeChakras()
        {
            // 7 базовых чакр
            // Сакральные чакры (1-3)
            chakras.Add(new ChakraData
            {
                level = 1,
                name = "Муладхара (Корневая)",
                description = "Корневая чакра - основа физического существования",
                chakraColor = Color.red,
                mantra = "LAM",
                abilities = new List<string> { "Базовая выносливость", "Физическая сила", "Чувство безопасности" },
                energyMultiplier = 1.0f,
                isUnlocked = true
            });
            
            chakras.Add(new ChakraData
            {
                level = 2,
                name = "Свадхистана (Сакральная)",
                description = "Сакральная чакра - центр эмоций, творчества и сексуальной энергии",
                chakraColor = Color.orange,
                mantra = "VAM",
                abilities = new List<string> { "Эмоциональная стабильность", "Творческие способности", "Сексуальная энергия" },
                energyMultiplier = 1.2f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 3,
                name = "Манипура (Солнечное сплетение)",
                description = "Чакра солнечного сплетения - воля, сила и самодисциплина",
                chakraColor = Color.yellow,
                mantra = "RAM",
                abilities = new List<string> { "Сила воли", "Лидерские качества", "Метаболизм" },
                energyMultiplier = 1.5f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 4,
                name = "Анахата (Сердечная)",
                description = "Сердечная чакра - любовь и сострадание",
                chakraColor = Color.green,
                mantra = "YAM",
                abilities = new List<string> { "Безусловная любовь", "Эмпатия", "Исцеление" },
                energyMultiplier = 2.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 5,
                name = "Вишудха (Горловая)",
                description = "Горловая чакра - коммуникация и самовыражение",
                chakraColor = Color.blue,
                mantra = "HAM",
                abilities = new List<string> { "Красноречие", "Творческое самовыражение", "Истина" },
                energyMultiplier = 2.5f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 6,
                name = "Аджна (Третий глаз)",
                description = "Чакра третьего глаза - интуиция и мудрость",
                chakraColor = Color.indigo,
                mantra = "Тишина",
                abilities = new List<string> { "Интуиция", "Ясновидение", "Мудрость" },
                energyMultiplier = 3.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 7,
                name = "Сахасрара (Коронная)",
                description = "Коронная чакра - высшее сознание и духовность",
                chakraColor = Color.magenta,
                mantra = "Тишина",
                abilities = new List<string> { "Просветление", "Связь с божественным", "Единство", "Духовность" },
                energyMultiplier = 4.0f,
                isUnlocked = false
            });
            
            // 5 высших чакр
            chakras.Add(new ChakraData
            {
                level = 8,
                name = "Антардхана (Скрытая)",
                description = "Скрытая чакра - доступ к параллельным мирам",
                chakraColor = Color.cyan,
                mantra = "KRIM",
                abilities = new List<string> { "Межпространственные путешествия", "Телепортация", "Видение других измерений" },
                energyMultiplier = 5.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 9,
                name = "Парадхана (Высшая)",
                description = "Высшая чакра - создание реальности",
                chakraColor = Color.white,
                mantra = "HRIM",
                abilities = new List<string> { "Манипуляция реальностью", "Создание объектов", "Изменение времени" },
                energyMultiplier = 7.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 10,
                name = "Махадхана (Великая)",
                description = "Великая чакра - понимание всех измерений",
                chakraColor = Color.gold,
                mantra = "SRIM",
                abilities = new List<string> { "Понимание 5D", "Доступ к Луне", "Предварительное кураторство" },
                energyMultiplier = 10.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 11,
                name = "Парамадхана (Абсолютная)",
                description = "Абсолютная чакра - единство со всем сущим",
                chakraColor = Color.clear,
                mantra = "PRAM",
                abilities = new List<string> { "Единство с космосом", "Бессмертие", "Полное понимание" },
                energyMultiplier = 15.0f,
                isUnlocked = false
            });
            
            chakras.Add(new ChakraData
            {
                level = 12,
                name = "Атмадхана (Душевная)",
                description = "Душевная чакра - готовность стать Куратором Земель",
                chakraColor = Color.rainbow,
                mantra = "ATMA",
                abilities = new List<string> { "Статус Куратора Земель", "Наблюдение и баланс 54 Земель", "Бесконечная мудрость" },
                energyMultiplier = 20.0f,
                isUnlocked = false
            });
        }
        
        public void OnChakraLevelUp(int newLevel)
        {
            if (newLevel <= chakras.Count)
            {
                // Проверяем понимание предыдущих уровней
                if (newLevel == 4 && !HasMasteredPreviousChakras(3))
                {
                    Debug.Log("Для 4-й чакры нужно полностью понять предыдущие 3 уровня!");
                    return;
                }
                
                if (newLevel == 7 && !HasMasteredPreviousChakras(6))
                {
                    Debug.Log("Для 7-й чакры нужно полностью понять предыдущие 6 уровней!");
                    return;
                }
                
                ChakraData chakra = chakras[newLevel - 1];
                chakra.isUnlocked = true;
                
                // Обновляем максимальную энергию
                maxEnergy = 100f * chakra.energyMultiplier;
                currentEnergy = maxEnergy;
                
                // Активируем способности чакры
                ActivateChakraAbilities(chakra);
                
                Debug.Log($"Чакра {chakra.name} разблокирована! Новые способности: {string.Join(", ", chakra.abilities)}");
            }
        }
        
        private bool HasMasteredPreviousChakras(int upToLevel)
        {
            for (int i = 1; i <= upToLevel; i++)
            {
                if (!chakras[i - 1].isUnlocked || !IsChakraMastered(i))
                {
                    return false;
                }
            }
            return true;
        }
        
        private bool IsChakraMastered(int level)
        {
            // Проверяем, что игрок действительно понимает чакру
            // Это может включать выполнение определенных действий, медитаций и т.д.
            return true; // Упрощенная проверка
        }
        
        private void ActivateChakraAbilities(ChakraData chakra)
        {
            // Здесь можно добавить логику активации конкретных способностей
            switch (chakra.level)
            {
                case 4: // Анахата - исцеление
                    // Активируем способность исцеления
                    break;
                case 6: // Аджна - интуиция
                    // Активируем способность предвидения
                    break;
                case 8: // Антардхана - межпространственные путешествия
                    // Активируем телепортацию
                    break;
                case 10: // Махадхана - доступ к Луне
                    gameManager.CheckMoonAccess();
                    break;
            }
        }
        
        public bool IsChakraUnlocked(int level)
        {
            if (level > 0 && level <= chakras.Count)
            {
                return chakras[level - 1].isUnlocked;
            }
            return false;
        }
        
        public ChakraData GetChakraData(int level)
        {
            if (level > 0 && level <= chakras.Count)
            {
                return chakras[level - 1];
            }
            return null;
        }
        
        public void UseEnergy(float amount)
        {
            currentEnergy = Mathf.Max(0, currentEnergy - amount);
        }
        
        private void RegenerateEnergy()
        {
            if (currentEnergy < maxEnergy)
            {
                currentEnergy = Mathf.Min(maxEnergy, currentEnergy + energyRegenerationRate * Time.deltaTime);
            }
        }
        
        public float GetEnergyPercentage()
        {
            return currentEnergy / maxEnergy;
        }
        
        public bool CanUseAbility(float energyCost)
        {
            return currentEnergy >= energyCost;
        }
    }
}