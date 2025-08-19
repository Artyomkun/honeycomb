using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;

namespace RealLife5D.UI
{
    /// <summary>
    /// UI система для Крийя-йоги и управления духовными практиками
    /// </summary>
    public class KriyaYogaUI : MonoBehaviour
    {
        [Header("Kriya Display")]
        public Transform kriyaContainer;
        public GameObject kriyaTechniquePrefab;
        public List<KriyaTechniqueDisplay> kriyaDisplays = new List<KriyaTechniqueDisplay>();
        
        [Header("Energy Display")]
        public Slider kundaliniSlider;
        public Text kundaliniText;
        public Image kundaliniFill;
        
        public Slider pranaSlider;
        public Text pranaText;
        public Image pranaFill;
        
        [Header("Practice Controls")]
        public Button startPracticeButton;
        public Button stopPracticeButton;
        public Text practiceStatusText;
        public Slider practiceProgressSlider;
        
        [Header("Chakra Status")]
        public Transform chakraStatusContainer;
        public GameObject chakraStatusPrefab;
        public List<ChakraStatusDisplay> chakraStatusDisplays = new List<ChakraStatusDisplay>();
        
        private KriyaYogaSystem kriyaSystem;
        private GameManager gameManager;
        
        void Start()
        {
            InitializeUI();
        }
        
        void Update()
        {
            UpdateEnergyDisplays();
            UpdatePracticeStatus();
            UpdateChakraStatus();
        }
        
        private void InitializeUI()
        {
            gameManager = GameManager.Instance;
            if (gameManager != null)
            {
                kriyaSystem = gameManager.GetComponent<KriyaYogaSystem>();
            }
            
            CreateKriyaTechniqueDisplays();
            CreateChakraStatusDisplays();
            SetupButtons();
        }
        
        private void CreateKriyaTechniqueDisplays()
        {
            if (kriyaContainer == null || kriyaTechniquePrefab == null) return;
            
            // Очищаем существующие
            foreach (Transform child in kriyaContainer)
            {
                Destroy(child.gameObject);
            }
            kriyaDisplays.Clear();
            
            // Создаем новые для всех 12 уровней
            for (int i = 0; i < 12; i++)
            {
                GameObject kriyaObj = Instantiate(kriyaTechniquePrefab, kriyaContainer);
                KriyaTechniqueDisplay display = kriyaObj.GetComponent<KriyaTechniqueDisplay>();
                
                if (display != null)
                {
                    display.Initialize(i + 1, kriyaSystem);
                    kriyaDisplays.Add(display);
                }
            }
        }
        
        private void CreateChakraStatusDisplays()
        {
            if (chakraStatusContainer == null || chakraStatusPrefab == null) return;
            
            // Очищаем существующие
            foreach (Transform child in chakraStatusContainer)
            {
                Destroy(child.gameObject);
            }
            chakraStatusDisplays.Clear();
            
            // Создаем статус для всех чакр
            for (int i = 0; i < 12; i++)
            {
                GameObject statusObj = Instantiate(chakraStatusPrefab, chakraStatusContainer);
                ChakraStatusDisplay display = statusObj.GetComponent<ChakraStatusDisplay>();
                
                if (display != null)
                {
                    display.Initialize(i + 1);
                    chakraStatusDisplays.Add(display);
                }
            }
        }
        
        private void SetupButtons()
        {
            if (startPracticeButton != null)
            {
                startPracticeButton.onClick.AddListener(OnStartPracticeClicked);
            }
            
            if (stopPracticeButton != null)
            {
                stopPracticeButton.onClick.AddListener(OnStopPracticeClicked);
            }
        }
        
        private void UpdateEnergyDisplays()
        {
            if (kriyaSystem == null) return;
            
            // Обновляем Кундалини
            if (kundaliniSlider != null)
            {
                float kundaliniPercent = kriyaSystem.GetKundaliniEnergy() / 1000f; // 1000 - максимум
                kundaliniSlider.value = kundaliniPercent;
            }
            
            if (kundaliniText != null)
            {
                kundaliniText.text = $"Кундалини: {kriyaSystem.GetKundaliniEnergy():F0}/1000";
            }
            
            if (kundaliniFill != null)
            {
                float kundaliniPercent = kriyaSystem.GetKundaliniEnergy() / 1000f;
                kundaliniFill.color = Color.Lerp(Color.red, Color.yellow, kundaliniPercent);
            }
            
            // Обновляем Прану
            if (pranaSlider != null)
            {
                float pranaPercent = kriyaSystem.GetPranaEnergy() / 1000f; // 1000 - максимум
                pranaSlider.value = pranaPercent;
            }
            
            if (pranaText != null)
            {
                pranaText.text = $"Прана: {kriyaSystem.GetPranaEnergy():F0}/1000";
            }
            
            if (pranaFill != null)
            {
                float pranaPercent = kriyaSystem.GetPranaEnergy() / 1000f;
                pranaFill.color = Color.Lerp(Color.blue, Color.cyan, pranaPercent);
            }
        }
        
        private void UpdatePracticeStatus()
        {
            if (kriyaSystem == null) return;
            
            bool isActive = kriyaSystem.IsKriyaActive();
            
            if (startPracticeButton != null)
            {
                startPracticeButton.interactable = !isActive;
            }
            
            if (stopPracticeButton != null)
            {
                stopPracticeButton.interactable = isActive;
            }
            
            if (practiceStatusText != null)
            {
                if (isActive)
                {
                    practiceStatusText.text = $"Крийя практика: {kriyaSystem.GetCurrentTechnique()?.name}";
                }
                else
                {
                    practiceStatusText.text = "Крийя не активна";
                }
            }
            
            if (practiceProgressSlider != null)
            {
                practiceProgressSlider.value = kriyaSystem.GetKriyaProgress();
            }
        }
        
        private void UpdateChakraStatus()
        {
            if (kriyaSystem == null) return;
            
            for (int i = 0; i < chakraStatusDisplays.Count; i++)
            {
                bool isActivated = kriyaSystem.IsChakraActivated(i + 1);
                chakraStatusDisplays[i].UpdateStatus(isActivated);
            }
        }
        
        private void OnStartPracticeClicked()
        {
            // Начинаем практику на текущем уровне чакр
            if (kriyaSystem != null && gameManager != null)
            {
                int currentLevel = gameManager.currentChakraLevel;
                kriyaSystem.StartKriyaPractice(currentLevel);
            }
        }
        
        private void OnStopPracticeClicked()
        {
            if (kriyaSystem != null)
            {
                kriyaSystem.StopKriyaPractice();
            }
        }
        
        public void OnKriyaTechniqueClicked(int kriyaLevel)
        {
            if (kriyaSystem != null)
            {
                kriyaSystem.StartKriyaPractice(kriyaLevel);
            }
        }
    }
    
    /// <summary>
    /// Отдельный элемент UI для отображения техники Крийя
    /// </summary>
    public class KriyaTechniqueDisplay : MonoBehaviour
    {
        [Header("UI Elements")]
        public Text levelText;
        public Text nameText;
        public Text descriptionText;
        public Button practiceButton;
        public GameObject lockIcon;
        public GameObject activeIndicator;
        
        private int kriyaLevel;
        private KriyaYogaSystem kriyaSystem;
        
        public void Initialize(int level, KriyaYogaSystem system)
        {
            kriyaLevel = level;
            kriyaSystem = system;
            
            if (levelText != null)
            {
                levelText.text = level.ToString();
            }
            
            if (nameText != null)
            {
                nameText.text = GetKriyaName(level);
            }
            
            if (descriptionText != null)
            {
                descriptionText.text = GetKriyaDescription(level);
            }
            
            if (practiceButton != null)
            {
                practiceButton.onClick.AddListener(() => OnPracticeClicked());
            }
        }
        
        private void OnPracticeClicked()
        {
            KriyaYogaUI kriyaUI = FindObjectOfType<KriyaYogaUI>();
            if (kriyaUI != null)
            {
                kriyaUI.OnKriyaTechniqueClicked(kriyaLevel);
            }
        }
        
        private string GetKriyaName(int level)
        {
            string[] names = {
                "Муладхара Крийя", "Свадхистана Крийя", "Манипура Крийя", "Анахата Крийя",
                "Вишудха Крийя", "Аджна Крийя", "Сахасрара Крийя", "Антардхана Крийя",
                "Парадхана Крийя", "Махадхана Крийя", "Парамадхана Крийя", "Атмадхана Крийя"
            };
            
            return level > 0 && level <= names.Length ? names[level - 1] : "Неизвестно";
        }
        
        private string GetKriyaDescription(int level)
        {
            string[] descriptions = {
                "Пробуждение корневой чакры", "Активация сакральной энергии", "Пробуждение солнечного сплетения", "Открытие сердечной чакры",
                "Активация горловой чакры", "Пробуждение третьего глаза", "Открытие коронной чакры", "Межпространственные путешествия",
                "Создание реальности", "Понимание 5D", "Единство с космосом", "Статус Куратора"
            };
            
            return level > 0 && level <= descriptions.Length ? descriptions[level - 1] : "Описание недоступно";
        }
    }
    
    /// <summary>
    /// Отображение статуса чакры в системе Крийя
    /// </summary>
    public class ChakraStatusDisplay : MonoBehaviour
    {
        [Header("UI Elements")]
        public Text chakraLevelText;
        public Image chakraIcon;
        public GameObject activatedIcon;
        public GameObject lockedIcon;
        
        private int chakraLevel;
        
        public void Initialize(int level)
        {
            chakraLevel = level;
            
            if (chakraLevelText != null)
            {
                chakraLevelText.text = level.ToString();
            }
            
            if (chakraIcon != null)
            {
                chakraIcon.color = GetChakraColor(level);
            }
        }
        
        public void UpdateStatus(bool isActivated)
        {
            if (activatedIcon != null)
            {
                activatedIcon.SetActive(isActivated);
            }
            
            if (lockedIcon != null)
            {
                lockedIcon.SetActive(!isActivated);
            }
        }
        
        private Color GetChakraColor(int level)
        {
            Color[] colors = {
                Color.red, Color.orange, Color.yellow, Color.green,
                Color.blue, Color.indigo, Color.magenta, Color.cyan,
                Color.white, Color.gold, Color.clear, Color.rainbow
            };
            
            return level > 0 && level <= colors.Length ? colors[level - 1] : Color.gray;
        }
    }
}