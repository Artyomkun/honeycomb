using UnityEngine;
using UnityEngine.SceneManagement;

namespace RealLife5D.Systems
{
	public class ReincarnationSystem : MonoBehaviour
	{
		[Header("Reincarnation State")]
		public int reincarnationCount = 0;
		public string lastKarmaTier = "Neutral";
		
		private GameManager gameManager;
		private WorldSystem worldSystem;
		private KarmaSystem karmaSystem;
		
		void Start()
		{
			gameManager = GameManager.Instance;
			worldSystem = GetComponent<WorldSystem>();
			karmaSystem = GetComponent<KarmaSystem>();
		}
		
		public void Reincarnate()
		{
			if (gameManager == null || worldSystem == null)
			{
				Debug.LogWarning("ReincarnationSystem: отсутствуют ссылки на менеджеры.");
				return;
			}
			
			// Фиксируем карму текущей жизни
			KarmaTier tier = karmaSystem != null ? karmaSystem.GetKarmaTier() : KarmaTier.Neutral;
			lastKarmaTier = tier.ToString();
			reincarnationCount = PlayerPrefs.GetInt("ReincarnationCount", 0) + 1;
			PlayerPrefs.SetInt("ReincarnationCount", reincarnationCount);
			PlayerPrefs.SetString("LastKarmaTier", lastKarmaTier);
			
			// Применяем эффекты кармы при перерождении
			ApplyKarmaEffectsOnRebirth(tier);
			
			// Переход в следующий мир по порядку
			gameManager.currentWorldIndex = (gameManager.currentWorldIndex + 1) % gameManager.totalWorlds;
			worldSystem.TransitionToWorld(gameManager.currentWorldIndex);
			
			// Сохраняем и загружаем сцену
			PlayerPrefs.Save();
			SceneManager.LoadScene("World_" + gameManager.currentWorldIndex);
		}
		
		private void ApplyKarmaEffectsOnRebirth(KarmaTier tier)
		{
			// Отрицательная карма: понижение чакры на 1 (минимум 1)
			if (tier == KarmaTier.Dark || tier == KarmaTier.Shadow)
			{
				if (gameManager.currentChakraLevel > 1)
				{
					gameManager.currentChakraLevel = Mathf.Max(1, gameManager.currentChakraLevel - 1);
				}
			}
			// Положительная карма: восстановление энергии и устойчивости
			else if (tier == KarmaTier.Light || tier == KarmaTier.Radiant)
			{
				var chakraSystem = GetComponent<ChakraSystem>();
				if (chakraSystem != null)
				{
					chakraSystem.currentEnergy = chakraSystem.maxEnergy;
				}
				var dimensionSystem = GetComponent<DimensionSystem>();
				if (dimensionSystem != null)
				{
					dimensionSystem.realityStability = Mathf.Min(100f, dimensionSystem.realityStability + 10f);
				}
			}
		}
	}
}