using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using System.Collections;

namespace RealLife5D.Scenes
{
    public class MoonScene : MonoBehaviour
    {
        [Header("Moon Environment")]
        public GameObject moonSurface;
        public GameObject stars;
        public GameObject earth;
        public GameObject portal;
        
        [Header("Curator")]
        public GameObject curator;
        public Transform curatorPosition;
        public Material curatorMaterial;
        
        [Header("Final Challenge")]
        public GameObject challengeOrb;
        public Transform[] challengePositions;
        public float challengeDuration = 60f;
        
        [Header("UI Elements")]
        public GameObject moonUI;
        public GameObject challengeUI;
        public GameObject victoryUI;
        public Text dialogueText;
        
        [Header("Audio")]
        public AudioSource ambientAudio;
        public AudioSource curatorVoice;
        public AudioClip[] curatorDialogues;
        
        private GameManager gameManager;
        private bool isChallengeActive = false;
        private float challengeTimer = 0f;
        private int currentChallenge = 0;
        private bool isCuratorMet = false;
        
        void Start()
        {
            InitializeMoonScene();
            StartCoroutine(MoonSequence());
        }
        
        void Update()
        {
            if (isChallengeActive)
            {
                UpdateChallenge();
            }
        }
        
        private void InitializeMoonScene()
        {
            gameManager = GameManager.Instance;
            
            // Проверяем, может ли игрок быть на Луне
            if (gameManager == null || 
                gameManager.currentChakraLevel < 12 || 
                gameManager.currentDimension < 5)
            {
                Debug.LogWarning("Игрок не готов для Луны. Возврат в основной мир.");
                SceneManager.LoadScene("MainWorld");
                return;
            }
            
            // Настраиваем лунную среду
            SetupMoonEnvironment();
            
            // Создаем Куратора
            CreateCurator();
            
            // Настраиваем финальные испытания
            SetupFinalChallenge();
        }
        
        private void SetupMoonEnvironment()
        {
            // Создаем лунную поверхность
            if (moonSurface != null)
            {
                moonSurface.transform.localScale = Vector3.one * 1000f;
                
                // Добавляем кратеры и неровности
                for (int i = 0; i < 50; i++)
                {
                    Vector3 craterPos = Random.insideUnitSphere * 500f;
                    craterPos.y = 0;
                    CreateCrater(craterPos, Random.Range(5f, 20f));
                }
            }
            
            // Настраиваем звездное небо
            if (stars != null)
            {
                stars.transform.localScale = Vector3.one * 2000f;
                
                // Анимируем звезды
                StartCoroutine(AnimateStars());
            }
            
            // Показываем Землю
            if (earth != null)
            {
                earth.transform.position = Vector3.up * 1000f;
                earth.transform.localScale = Vector3.one * 100f;
                
                // Вращаем Землю
                StartCoroutine(RotateEarth());
            }
            
            // Создаем портал для возврата
            if (portal != null)
            {
                portal.transform.position = Vector3.forward * 200f;
                portal.SetActive(false); // Скрыт до завершения испытаний
            }
        }
        
        private void CreateCrater(Vector3 position, float radius)
        {
            GameObject crater = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            crater.transform.position = position;
            crater.transform.localScale = Vector3.one * radius;
            
            // Создаем материал кратера
            Material craterMat = new Material(Shader.Find("Standard"));
            craterMat.color = new Color(0.3f, 0.3f, 0.3f);
            crater.GetComponent<Renderer>().material = craterMat;
            
            // Делаем кратер вогнутым
            crater.transform.localScale = new Vector3(radius, radius * 0.3f, radius);
        }
        
        private IEnumerator AnimateStars()
        {
            while (true)
            {
                if (stars != null)
                {
                    stars.transform.Rotate(Vector3.up, Time.deltaTime * 0.1f);
                }
                yield return null;
            }
        }
        
        private IEnumerator RotateEarth()
        {
            while (true)
            {
                if (earth != null)
                {
                    earth.transform.Rotate(Vector3.up, Time.deltaTime * 0.05f);
                }
                yield return null;
            }
        }
        
        private void CreateCurator()
        {
            if (curator == null)
            {
                curator = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                curator.name = "Curator";
                curator.transform.position = curatorPosition.position;
                curator.transform.localScale = Vector3.one * 3f;
                
                // Создаем материал Куратора
                if (curatorMaterial != null)
                {
                    curator.GetComponent<Renderer>().material = curatorMaterial;
                }
                else
                {
                    Material mat = new Material(Shader.Find("Standard"));
                    mat.color = Color.cyan;
                    mat.SetFloat("_Metallic", 1f);
                    mat.SetFloat("_Smoothness", 0.8f);
                    curator.GetComponent<Renderer>().material = mat;
                }
                
                // Добавляем свечение
                Light curatorLight = curator.AddComponent<Light>();
                curatorLight.color = Color.cyan;
                curatorLight.intensity = 2f;
                curatorLight.range = 10f;
                
                // Добавляем анимацию
                StartCoroutine(AnimateCurator());
            }
        }
        
        private IEnumerator AnimateCurator()
        {
            Vector3 startPos = curator.transform.position;
            float time = 0f;
            
            while (true)
            {
                time += Time.deltaTime;
                
                // Плавающее движение
                Vector3 newPos = startPos + Vector3.up * Mathf.Sin(time * 0.5f) * 2f;
                curator.transform.position = newPos;
                
                // Вращение
                curator.transform.Rotate(Vector3.up, Time.deltaTime * 30f);
                
                yield return null;
            }
        }
        
        private void SetupFinalChallenge()
        {
            if (challengeOrb != null)
            {
                challengeOrb.SetActive(false);
            }
            
            // Создаем позиции для испытаний
            if (challengePositions.Length == 0)
            {
                challengePositions = new Transform[3];
                for (int i = 0; i < 3; i++)
                {
                    GameObject pos = new GameObject($"ChallengePosition_{i}");
                    pos.transform.position = Vector3.forward * (100f + i * 50f);
                    challengePositions[i] = pos.transform;
                }
            }
        }
        
        private IEnumerator MoonSequence()
        {
            // Ждем немного для загрузки
            yield return new WaitForSeconds(2f);
            
            // Приветствие Куратора
            yield return StartCoroutine(CuratorGreeting());
            
            // Финальные испытания
            yield return StartCoroutine(FinalChallenges());
            
            // Награждение
            yield return StartCoroutine(Celebration());
        }
        
        private IEnumerator CuratorGreeting()
        {
            Debug.Log("Куратор: Добро пожаловать на Луну, искатель истины.");
            
            // Анимация появления Куратора
            if (curator != null)
            {
                curator.transform.localScale = Vector3.zero;
                
                float time = 0f;
                while (time < 2f)
                {
                    time += Time.deltaTime;
                    float scale = Mathf.Lerp(0f, 3f, time / 2f);
                    curator.transform.localScale = Vector3.one * scale;
                    yield return null;
                }
            }
            
            yield return new WaitForSeconds(1f);
            
            Debug.Log("Куратор: Ты прошел долгий путь через 54 мира и 12 уровней чакр.");
            yield return new WaitForSeconds(2f);
            
            Debug.Log("Куратор: Но чтобы стать Куратором, тебе нужно пройти финальные испытания.");
            yield return new WaitForSeconds(2f);
            
            Debug.Log("Куратор: Готов ли ты к последнему испытанию?");
            yield return new WaitForSeconds(1f);
            
            isCuratorMet = true;
        }
        
        private IEnumerator FinalChallenges()
        {
            Debug.Log("Начинаются финальные испытания...");
            
            // Активируем UI испытаний
            if (challengeUI != null)
            {
                challengeUI.SetActive(true);
            }
            
            // Первое испытание: Медитация
            yield return StartCoroutine(MeditationChallenge());
            
            // Второе испытание: Создание реальности
            yield return StartCoroutine(RealityCreationChallenge());
            
            // Третье испытание: Единство с космосом
            yield return StartCoroutine(CosmicUnityChallenge());
            
            Debug.Log("Все испытания пройдены!");
        }
        
        private IEnumerator MeditationChallenge()
        {
            Debug.Log("Испытание 1: Медитация в космической тишине");
            
            // Создаем орб для медитации
            if (challengeOrb != null)
            {
                challengeOrb.transform.position = challengePositions[0].position;
                challengeOrb.SetActive(true);
                
                // Анимация орба
                StartCoroutine(AnimateChallengeOrb(challengeOrb));
            }
            
            // Ждем завершения медитации
            float meditationTime = 0f;
            float requiredTime = 10f; // 10 секунд медитации
            
            while (meditationTime < requiredTime)
            {
                meditationTime += Time.deltaTime;
                
                // Проверяем, медитирует ли игрок
                if (Input.GetKey(KeyCode.Q))
                {
                    meditationTime += Time.deltaTime * 2f; // Быстрее при медитации
                }
                
                yield return null;
            }
            
            Debug.Log("Медитация завершена!");
            yield return new WaitForSeconds(1f);
        }
        
        private IEnumerator RealityCreationChallenge()
        {
            Debug.Log("Испытание 2: Создание реальности");
            
            // Перемещаем орб
            if (challengeOrb != null)
            {
                challengeOrb.transform.position = challengePositions[1].position;
            }
            
            // Игрок должен создать объект
            bool objectCreated = false;
            float timeLimit = 15f;
            float time = 0f;
            
            while (!objectCreated && time < timeLimit)
            {
                time += Time.deltaTime;
                
                // Проверяем создание объекта (например, нажатие клавиши)
                if (Input.GetKeyDown(KeyCode.C))
                {
                    CreateRealityObject();
                    objectCreated = true;
                }
                
                yield return null;
            }
            
            if (!objectCreated)
            {
                Debug.Log("Время истекло! Создайте объект быстрее.");
                yield return new WaitForSeconds(1f);
            }
            
            Debug.Log("Объект реальности создан!");
            yield return new WaitForSeconds(1f);
        }
        
        private IEnumerator CosmicUnityChallenge()
        {
            Debug.Log("Испытание 3: Единство с космосом");
            
            // Перемещаем орб
            if (challengeOrb != null)
            {
                challengeOrb.transform.position = challengePositions[2].position;
            }
            
            // Игрок должен достичь единства
            float unityProgress = 0f;
            float requiredUnity = 100f;
            
            while (unityProgress < requiredUnity)
            {
                unityProgress += Time.deltaTime * 5f; // Базовый прогресс
                
                // Ускоряем при правильных действиях
                if (Input.GetKey(KeyCode.Space))
                {
                    unityProgress += Time.deltaTime * 10f;
                }
                
                yield return null;
            }
            
            Debug.Log("Единство с космосом достигнуто!");
            yield return new WaitForSeconds(1f);
        }
        
        private void CreateRealityObject()
        {
            // Создаем красивый объект
            GameObject realityObject = GameObject.CreatePrimitive(PrimitiveType.Icosahedron);
            realityObject.transform.position = transform.position + Vector3.up * 5f;
            realityObject.transform.localScale = Vector3.one * 2f;
            
            // Создаем материал
            Material mat = new Material(Shader.Find("Standard"));
            mat.color = Color.magenta;
            mat.SetFloat("_Metallic", 1f);
            mat.SetFloat("_Smoothness", 1f);
            realityObject.GetComponent<Renderer>().material = mat;
            
            // Добавляем свечение
            Light objLight = realityObject.AddComponent<Light>();
            objLight.color = Color.magenta;
            objLight.intensity = 3f;
            objLight.range = 5f;
            
            // Анимация
            StartCoroutine(AnimateRealityObject(realityObject));
        }
        
        private IEnumerator AnimateRealityObject(GameObject obj)
        {
            Vector3 startScale = obj.transform.localScale;
            float time = 0f;
            
            while (time < 3f)
            {
                time += Time.deltaTime;
                
                // Пульсация
                float scale = 1f + Mathf.Sin(time * 5f) * 0.2f;
                obj.transform.localScale = startScale * scale;
                
                // Вращение
                obj.transform.Rotate(Vector3.up, Time.deltaTime * 90f);
                
                yield return null;
            }
        }
        
        private IEnumerator AnimateChallengeOrb(GameObject orb)
        {
            Vector3 startScale = orb.transform.localScale;
            float time = 0f;
            
            while (orb.activeInHierarchy)
            {
                time += Time.deltaTime;
                
                // Пульсация
                float scale = 1f + Mathf.Sin(time * 3f) * 0.3f;
                orb.transform.localScale = startScale * scale;
                
                // Вращение
                orb.transform.Rotate(Vector3.up, Time.deltaTime * 60f);
                
                yield return null;
            }
        }
        
        private IEnumerator Celebration()
        {
            Debug.Log("Куратор: Поздравляю! Ты прошел все испытания!");
            yield return new WaitForSeconds(2f);
            
            Debug.Log("Куратор: Отныне ты - новый Куратор 54 Земель!");
            yield return new WaitForSeconds(2f);
            
            Debug.Log("Куратор: Ты можешь наблюдать за развитием миров и управлять их балансом!");
            yield return new WaitForSeconds(2f);
            
            // Активируем портал
            if (portal != null)
            {
                portal.SetActive(true);
                
                // Анимация портала
                StartCoroutine(AnimatePortal());
            }
            
            // Показываем UI победы
            if (victoryUI != null)
            {
                victoryUI.SetActive(true);
            }
            
            yield return new WaitForSeconds(5f);
            
            Debug.Log("Игра завершена! Ты стал Куратором!");
        }
        
        private IEnumerator AnimatePortal()
        {
            if (portal != null)
            {
                float time = 0f;
                Vector3 startScale = portal.transform.localScale;
                
                while (time < 3f)
                {
                    time += Time.deltaTime;
                    
                    // Появление портала
                    float scale = Mathf.Lerp(0f, 1f, time / 3f);
                    portal.transform.localScale = startScale * scale;
                    
                    // Вращение
                    portal.transform.Rotate(Vector3.up, Time.deltaTime * 120f);
                    
                    yield return null;
                }
            }
        }
        
        public void ReturnToMainWorld()
        {
            // Если игрок стал Куратором, возврат на Землю запрещен
            if (PlayerPrefs.GetInt("IsCurator", 0) == 1)
            {
                Debug.Log("Куратор не может вернуться на Землю. Он остается на уровне Луны/Кураторства.");
                return;
            }
            
            if (gameManager != null)
            {
                PlayerPrefs.SetInt("IsCurator", 1);
                PlayerPrefs.Save();
            }
            SceneManager.LoadScene("MainWorld");
        }
        
        public void CreateNewWorld()
        {
            // Кураторы не могут создавать новые миры
            Debug.Log("Куратор: Создание новых миров - это прерогатива высших существ. Ты можешь только наблюдать и управлять существующими 54 Землями.");
            
            // Показываем сообщение игроку
            if (dialogueText != null)
            {
                dialogueText.text = "Куратор: Создание новых миров - это прерогатива высших существ.";
            }
        }
    }
}