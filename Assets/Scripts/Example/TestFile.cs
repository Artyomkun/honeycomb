using UnityEngine;

namespace RealLife5D.Example
{
    /// <summary>
    /// Тестовый файл для демонстрации работы с файлами
    /// </summary>
    public class TestFile : MonoBehaviour
    {
        [Header("Test Settings")]
        public string testMessage = "Привет! Это тестовый файл";
        public int testNumber = 42;
        
        void Start()
        {
            Debug.Log($"Тестовое сообщение: {testMessage}");
            Debug.Log($"Тестовое число: {testNumber}");
        }
        
        public void TestFunction()
        {
            Debug.Log("Тестовая функция выполнена!");
        }
    }
}