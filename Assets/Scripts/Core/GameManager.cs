using UnityEngine;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace RealLife5D.Core
{
    public class GameManager : MonoBehaviour
    {
        public static GameManager Instance { get; private set; }
        
        [Header("Game State")]
        public int currentDimension = 3; // Текущее измерение (1-5)
        public int currentChakraLevel = 1; // Текущий уровень чакр (1-12)
        public int currentWorldIndex = 0; // Индекс текущего мира (0-53)
        public bool isDead = false; // Статус смерти игрока
        
        [Header("World Settings")]
        public int totalWorlds = 54; // Общее количество миров
        public int totalChakras = 12; // Общее количество чакр
        
        [Header("UI References")]
        public GameObject dimensionUI;
        public GameObject chakraUI;
        public GameObject worldUI;
        
        private PlayerController playerController;
        private ChakraSystem chakraSystem;
        private DimensionSystem dimensionSystem;
        private WorldSystem worldSystem;
        
        void Awake()
        {
            if (Instance == null)
            {
                Instance = this;
                DontDestroyOnLoad(gameObject);
                InitializeSystems();
            }
            else
            {
                Destroy(gameObject);
            }
        }
        
        void Start()
        {
            LoadGameState();
            UpdateUI();
        }
        
        private void InitializeSystems()
        {
            chakraSystem = GetComponent<ChakraSystem>();
            dimensionSystem = GetComponent<DimensionSystem>();
            worldSystem = GetComponent<WorldSystem>();
            
            if (chakraSystem == null) chakraSystem = gameObject.AddComponent<ChakraSystem>();
            if (dimensionSystem == null) dimensionSystem = gameObject.AddComponent<DimensionSystem>();
            if (worldSystem == null) worldSystem = gameObject.AddComponent<WorldSystem>();
        }
        
        public void AdvanceChakraLevel()
        {
            if (currentChakraLevel < totalChakras)
            {
                currentChakraLevel++;
                chakraSystem.OnChakraLevelUp(currentChakraLevel);
                
                // Проверяем, можно ли перейти в следующее измерение
                if (currentChakraLevel >= 3 && currentDimension < 5)
                {
                    AdvanceDimension();
                }
                
                SaveGameState();
                UpdateUI();
            }
        }
        
        public void AdvanceDimension()
        {
            if (currentDimension < 5 && currentChakraLevel >= GetRequiredChakraForDimension(currentDimension + 1))
            {
                currentDimension++;
                dimensionSystem.OnDimensionChange(currentDimension);
                SaveGameState();
                UpdateUI();
            }
        }
        
        private int GetRequiredChakraForDimension(int dimension)
        {
            switch (dimension)
            {
                case 1: return 1;
                case 2: return 2;
                case 3: return 3;
                case 4: return 7;
                case 5: return 10;
                default: return 12;
            }
        }
        
        public void OnPlayerDeath()
        {
            isDead = true;
            
            // Переход в другой мир
            int newWorldIndex = Random.Range(0, totalWorlds);
            if (newWorldIndex == currentWorldIndex)
            {
                newWorldIndex = (newWorldIndex + 1) % totalWorlds;
            }
            
            currentWorldIndex = newWorldIndex;
            worldSystem.TransitionToWorld(currentWorldIndex);
            
            // Сохраняем прогресс чакр и измерений
            SaveGameState();
            
            // Перезагружаем сцену в новом мире
            SceneManager.LoadScene("World_" + currentWorldIndex);
        }
        
        public void CheckMoonAccess()
        {
            if (currentChakraLevel >= 12 && currentDimension >= 5)
            {
                // Игрок достиг Луны и может стать Куратором
                SceneManager.LoadScene("Moon");
            }
        }
        
        private void UpdateUI()
        {
            if (dimensionUI != null)
                dimensionUI.GetComponent<DimensionUI>()?.UpdateUI(currentDimension);
            
            if (chakraUI != null)
                chakraUI.GetComponent<ChakraUI>()?.UpdateUI(currentChakraLevel);
            
            if (worldUI != null)
                worldUI.GetComponent<WorldUI>()?.UpdateUI(currentWorldIndex);
        }
        
        private void SaveGameState()
        {
            PlayerPrefs.SetInt("CurrentDimension", currentDimension);
            PlayerPrefs.SetInt("CurrentChakraLevel", currentChakraLevel);
            PlayerPrefs.SetInt("CurrentWorldIndex", currentWorldIndex);
            PlayerPrefs.Save();
        }
        
        private void LoadGameState()
        {
            currentDimension = PlayerPrefs.GetInt("CurrentDimension", 3);
            currentChakraLevel = PlayerPrefs.GetInt("CurrentChakraLevel", 1);
            currentWorldIndex = PlayerPrefs.GetInt("CurrentWorldIndex", 0);
        }
        
        public void ResetGame()
        {
            PlayerPrefs.DeleteAll();
            currentDimension = 3;
            currentChakraLevel = 1;
            currentWorldIndex = 0;
            isDead = false;
            SaveGameState();
            UpdateUI();
        }
    }
}