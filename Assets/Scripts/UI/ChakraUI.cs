using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;

namespace RealLife5D.UI
{
    /// <summary>
    /// UI система для отображения чакр и их состояния
    /// </summary>
    public class ChakraUI : MonoBehaviour
    {
        [Header("Chakra Display")]
        public Transform chakraContainer;
        public GameObject chakraPrefab;
        public List<ChakraDisplay> chakraDisplays = new List<ChakraDisplay>();
        
        [Header("Energy Display")]
        public Slider energySlider;
        public Text energyText;
        public Image energyFill;
        
        [Header("Chakra Colors")]
        public Color[] chakraColors = new Color[]
        {
            Color.red,      // Муладхара
            Color.orange,   // Свадхистана
            Color.yellow,   // Манипура
            Color.green,    // Анахата
            Color.blue,     // Вишудха
            Color.indigo,   // Аджна
            Color.magenta,  // Сахасрара
            Color.cyan,     // Антардхана
            Color.white,    // Парадхана
            Color.gold,     // Махадхана
            Color.clear,    // Парамадхана
            Color.rainbow   // Атмадхана
        };
        
        private ChakraSystem chakraSystem;
        private GameManager gameManager;
        
        void Start()
        {
            InitializeUI();
        }
        
        void Update()
        {
            UpdateEnergyDisplay();
            UpdateChakraStates();
        }
        
        private void InitializeUI()
        {
            gameManager = GameManager.Instance;
            if (gameManager != null)
            {
                chakraSystem = gameManager.GetComponent<ChakraSystem>();
            }
            
            CreateChakraDisplays();
            UpdateUI(gameManager.currentChakraLevel);
        }
        
        private void CreateChakraDisplays()
        {
            if (chakraContainer == null || chakraPrefab == null) return;
            
            // Очищаем существующие
            foreach (Transform child in chakraContainer)
            {
                Destroy(child.gameObject);
            }
            chakraDisplays.Clear();
            
            // Создаем новые
            for (int i = 0; i < 12; i++)
            {
                GameObject chakraObj = Instantiate(chakraPrefab, chakraContainer);
                ChakraDisplay display = chakraObj.GetComponent<ChakraDisplay>();
                
                if (display != null)
                {
                    display.Initialize(i + 1, chakraColors[i]);
                    chakraDisplays.Add(display);
                }
            }
        }
        
        public void UpdateUI(int currentLevel)
        {
            if (chakraDisplays.Count == 0) return;
            
            for (int i = 0; i < chakraDisplays.Count; i++)
            {
                bool isUnlocked = i < currentLevel;
                bool isCurrent = i == currentLevel - 1;
                
                chakraDisplays[i].UpdateState(isUnlocked, isCurrent);
            }
        }
        
        private void UpdateEnergyDisplay()
        {
            if (chakraSystem == null || energySlider == null) return;
            
            float energyPercent = chakraSystem.GetEnergyPercentage();
            energySlider.value = energyPercent;
            
            if (energyText != null)
            {
                energyText.text = $"Энергия: {chakraSystem.currentEnergy:F0}/{chakraSystem.maxEnergy:F0}";
            }
            
            if (energyFill != null)
            {
                energyFill.color = Color.Lerp(Color.red, Color.green, energyPercent);
            }
        }
        
        private void UpdateChakraStates()
        {
            if (chakraSystem == null) return;
            
            for (int i = 0; i < chakraDisplays.Count; i++)
            {
                bool isUnlocked = chakraSystem.IsChakraUnlocked(i + 1);
                chakraDisplays[i].UpdateState(isUnlocked, false);
            }
        }
        
        public void OnChakraClicked(int chakraLevel)
        {
            if (chakraSystem != null && chakraSystem.IsChakraUnlocked(chakraLevel))
            {
                ChakraData chakraData = chakraSystem.GetChakraData(chakraLevel);
                ShowChakraInfo(chakraData);
            }
        }
        
        private void ShowChakraInfo(ChakraData chakraData)
        {
            if (chakraData == null) return;
            
            Debug.Log($"Чакра: {chakraData.name}");
            Debug.Log($"Описание: {chakraData.description}");
            Debug.Log($"Мантра: {chakraData.mantra}");
            Debug.Log($"Способности: {string.Join(", ", chakraData.abilities)}");
        }
    }
    
    /// <summary>
    /// Отдельный элемент UI для отображения чакры
    /// </summary>
    public class ChakraDisplay : MonoBehaviour
    {
        [Header("UI Elements")]
        public Image chakraIcon;
        public Text levelText;
        public Text nameText;
        public GameObject lockIcon;
        public GameObject currentIndicator;
        
        private int chakraLevel;
        private Color chakraColor;
        private Button chakraButton;
        
        public void Initialize(int level, Color color)
        {
            chakraLevel = level;
            chakraColor = color;
            
            if (chakraIcon != null)
            {
                chakraIcon.color = color;
            }
            
            if (levelText != null)
            {
                levelText.text = level.ToString();
            }
            
            if (nameText != null)
            {
                nameText.text = GetChakraName(level);
            }
            
            chakraButton = GetComponent<Button>();
            if (chakraButton != null)
            {
                chakraButton.onClick.AddListener(() => OnChakraClicked());
            }
        }
        
        public void UpdateState(bool isUnlocked, bool isCurrent)
        {
            if (lockIcon != null)
            {
                lockIcon.SetActive(!isUnlocked);
            }
            
            if (currentIndicator != null)
            {
                currentIndicator.SetActive(isCurrent);
            }
            
            if (chakraIcon != null)
            {
                Color iconColor = chakraColor;
                if (!isUnlocked)
                {
                    iconColor.a = 0.3f; // Полупрозрачный для заблокированных
                }
                chakraIcon.color = iconColor;
            }
        }
        
        private void OnChakraClicked()
        {
            ChakraUI chakraUI = FindObjectOfType<ChakraUI>();
            if (chakraUI != null)
            {
                chakraUI.OnChakraClicked(chakraLevel);
            }
        }
        
        private string GetChakraName(int level)
        {
            string[] names = {
                "Муладхара", "Свадхистана", "Манипура", "Анахата",
                "Вишудха", "Аджна", "Сахасрара", "Антардхана",
                "Парадхана", "Махадхана", "Парамадхана", "Атмадхана"
            };
            
            return level > 0 && level <= names.Length ? names[level - 1] : "Неизвестно";
        }
    }
}