using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
    [System.Serializable]
    public class DimensionData
    {
        public int level;
        public string name;
        public string description;
        public Color dimensionColor;
        public float realityShift;
        public List<string> features;
        public bool isUnlocked;
        public int requiredChakraLevel;
    }
    
    public class DimensionSystem : MonoBehaviour
    {
        [Header("Dimension Data")]
        public List<DimensionData> dimensions = new List<DimensionData>();
        
        [Header("Current State")]
        public float realityStability = 100f;
        public float dimensionalShift = 0f;
        
        private GameManager gameManager;
        private Camera mainCamera;
        
        void Start()
        {
            gameManager = GameManager.Instance;
            mainCamera = Camera.main;
            InitializeDimensions();
        }
        
        private void InitializeDimensions()
        {
            // 1D - Физический мир
            dimensions.Add(new DimensionData
            {
                level = 1,
                name = "1D - Физический Мир",
                description = "Базовый физический мир, где существуют только материальные объекты",
                dimensionColor = Color.gray,
                realityShift = 0f,
                features = new List<string> { "Только материя", "Базовые физические законы", "Ограниченное восприятие" },
                isUnlocked = true,
                requiredChakraLevel = 1
            });
            
            // 2D - Плоский мир
            dimensions.Add(new DimensionData
            {
                level = 2,
                name = "2D - Плоский Мир",
                description = "Двумерный мир с ограниченным движением и восприятием",
                dimensionColor = Color.blue,
                realityShift = 0.1f,
                features = new List<string> { "Двумерное движение", "Плоские объекты", "Ограниченная глубина" },
                isUnlocked = false,
                requiredChakraLevel = 2
            });
            
            // 3D - Наш мир
            dimensions.Add(new DimensionData
            {
                level = 3,
                name = "3D - Наш Мир",
                description = "Трехмерный мир, который мы знаем - с глубиной, высотой и шириной",
                dimensionColor = Color.white,
                realityShift = 0.2f,
                features = new List<string> { "Полное 3D восприятие", "Время как четвертое измерение", "Физические законы" },
                isUnlocked = true,
                requiredChakraLevel = 3
            });
            
            // 4D - Временное измерение
            dimensions.Add(new DimensionData
            {
                level = 4,
                name = "4D - Временное Измерение",
                description = "Мир, где время становится управляемым измерением",
                dimensionColor = Color.cyan,
                realityShift = 0.5f,
                features = new List<string> { "Управление временем", "Видение прошлого и будущего", "Временные петли" },
                isUnlocked = false,
                requiredChakraLevel = 7
            });
            
            // 5D - Духовное измерение
            dimensions.Add(new DimensionData
            {
                level = 5,
                name = "5D - Духовное Измерение",
                description = "Высшее измерение, где душа и сознание преобладают над материей",
                dimensionColor = Color.magenta,
                realityShift = 1.0f,
                features = new List<string> { "Создание реальности", "Межпространственные путешествия", "Единство с космосом" },
                isUnlocked = false,
                requiredChakraLevel = 10
            });
        }
        
        public void OnDimensionChange(int newDimension)
        {
            if (newDimension <= dimensions.Count)
            {
                DimensionData dimension = dimensions[newDimension - 1];
                dimension.isUnlocked = true;
                
                // Применяем эффекты нового измерения
                ApplyDimensionEffects(dimension);
                
                // Обновляем UI и эффекты
                UpdateDimensionUI(dimension);
                
                Debug.Log($"Измерение {dimension.name} разблокировано! Новые возможности: {string.Join(", ", dimension.features)}");
            }
        }
        
        private void ApplyDimensionEffects(DimensionData dimension)
        {
            // Применяем визуальные эффекты
            dimensionalShift = dimension.realityShift;
            
            // Изменяем цветовую палитру мира
            if (mainCamera != null)
            {
                // Добавляем пост-процессинг эффекты для разных измерений
                switch (dimension.level)
                {
                    case 1: // 1D - монохромный
                        SetCameraEffect(Color.gray, 0.8f);
                        break;
                    case 2: // 2D - синий оттенок
                        SetCameraEffect(Color.blue, 0.3f);
                        break;
                    case 3: // 3D - нормальные цвета
                        SetCameraEffect(Color.white, 0f);
                        break;
                    case 4: // 4D - циановый оттенок
                        SetCameraEffect(Color.cyan, 0.4f);
                        break;
                    case 5: // 5D - магический эффект
                        SetCameraEffect(Color.magenta, 0.6f);
                        break;
                }
            }
            
            // Применяем физические изменения
            ApplyPhysicsChanges(dimension);
        }
        
        private void SetCameraEffect(Color tintColor, float intensity)
        {
            // Здесь можно добавить пост-процессинг эффекты
            // Для простоты просто изменяем цвет камеры
            if (mainCamera != null)
            {
                // Можно добавить компонент PostProcessing для более сложных эффектов
                Debug.Log($"Применен эффект измерения: {tintColor} с интенсивностью {intensity}");
            }
        }
        
        private void ApplyPhysicsChanges(DimensionData dimension)
        {
            // Изменяем физические законы в зависимости от измерения
            switch (dimension.level)
            {
                case 1: // 1D - ограниченная физика
                    Physics.gravity = new Vector3(0, -9.81f, 0);
                    break;
                case 2: // 2D - плоская физика
                    Physics.gravity = new Vector3(0, -9.81f, 0);
                    break;
                case 3: // 3D - нормальная физика
                    Physics.gravity = new Vector3(0, -9.81f, 0);
                    break;
                case 4: // 4D - изменчивая физика
                    Physics.gravity = new Vector3(0, -9.81f, 0);
                    // Добавляем временные эффекты
                    break;
                case 5: // 5D - духовная физика
                    Physics.gravity = new Vector3(0, -9.81f, 0);
                    // Добавляем духовные эффекты
                    break;
            }
        }
        
        private void UpdateDimensionUI(DimensionData dimension)
        {
            // Обновляем UI для отображения текущего измерения
            // Здесь можно добавить анимации и эффекты
        }
        
        public bool IsDimensionUnlocked(int level)
        {
            if (level > 0 && level <= dimensions.Count)
            {
                return dimensions[level - 1].isUnlocked;
            }
            return false;
        }
        
        public DimensionData GetDimensionData(int level)
        {
            if (level > 0 && level <= dimensions.Count)
            {
                return dimensions[level - 1];
            }
            return null;
        }
        
        public float GetRealityStability()
        {
            return realityStability;
        }
        
        public void ModifyRealityStability(float amount)
        {
            realityStability = Mathf.Clamp(realityStability + amount, 0f, 100f);
        }
        
        public float GetDimensionalShift()
        {
            return dimensionalShift;
        }
        
        public void TriggerRealityShift()
        {
            // Вызывается при особых событиях для создания сдвига реальности
            dimensionalShift += 0.1f;
            realityStability -= 5f;
            
            // Применяем визуальные эффекты сдвига
            if (mainCamera != null)
            {
                // Добавляем эффекты искажения реальности
                Debug.Log("Происходит сдвиг реальности!");
            }
        }
        
        public void StabilizeReality()
        {
            // Стабилизируем реальность
            dimensionalShift = Mathf.Max(0, dimensionalShift - 0.05f);
            realityStability = Mathf.Min(100f, realityStability + 2f);
        }
    }
}