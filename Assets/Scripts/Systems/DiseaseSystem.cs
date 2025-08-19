using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
	public enum DiseaseType
	{
		Physical,    // Физические
		Mental,      // Ментальные (когнитивные)
		Psychological, // Психологические (эмоции/поведение)
		Spiritual,   // Духовные (смысл/ценности/связь)
		Astral       // Астральные (тонкие тела/энергетика)
	}
	
	[System.Serializable]
	public class Disease
	{
		public string id;
		public string name;
		public DiseaseType type;
		public float severity; // 0..1
		public string description;
		public List<string> symptoms = new List<string>();
		public bool isActive;
	}
	
	public class DiseaseSystem : MonoBehaviour
	{
		[Header("State")]
		public List<Disease> activeDiseases = new List<Disease>();
		
		[Header("Links")]
		public MoodSystem moodSystem;
		public ChakraSystem chakraSystem;
		public KarmaSystem karmaSystem;
		
		void Start()
		{
			var gm = GameManager.Instance;
			if (gm != null)
			{
				moodSystem = gm.GetComponent<MoodSystem>();
				chakraSystem = gm.GetComponent<ChakraSystem>();
				karmaSystem = gm.GetComponent<KarmaSystem>();
			}
		}
		
		void Update()
		{
			// Эволюция болезней зависит от настроения и профиля
			for (int i = 0; i < activeDiseases.Count; i++)
			{
				var d = activeDiseases[i];
				float delta = GetProgressDelta(d);
				d.severity = Mathf.Clamp01(d.severity + delta * Time.deltaTime);
				if (d.severity <= 0.01f) d.isActive = false;
			}
			activeDiseases.RemoveAll(d => !d.isActive);
		}
		
		public void AddDisease(Disease disease)
		{
			if (disease == null) return;
			disease.isActive = true;
			activeDiseases.Add(disease);
		}
		
		public void CureDisease(string id, float amount)
		{
			var d = activeDiseases.Find(x => x.id == id);
			if (d != null)
			{
				d.severity = Mathf.Clamp01(d.severity - amount);
				if (d.severity <= 0.01f) d.isActive = false;
			}
		}
		
		public float GetTotalSeverity()
		{
			float total = 0f;
			foreach (var d in activeDiseases) total += d.severity;
			return total;
		}
		
		private float GetProgressDelta(Disease d)
		{
			// Базовая прогрессия (можно параметризовать)
			float baseDelta = 0f;
			
			if (moodSystem == null) return baseDelta;
			float wellbeing = moodSystem.GetWellbeingIndex(); // -1..1
			
			// Негативные эмоции усиливают прогресс
			float negativeLoad = (moodSystem.fear + moodSystem.anger + moodSystem.sadness + moodSystem.stress) / 4f; // 0..1
			float positiveLoad = (moodSystem.calm + moodSystem.love + moodSystem.joy) / 3f; // 0..1
			
			// Влияние на разные классы болезней
			switch (d.type)
			{
				case DiseaseType.Physical:
					baseDelta += 0.15f * (negativeLoad - positiveLoad);
					// Дисциплина и чакры низших уровней помогают
					baseDelta -= 0.05f * GetDiscipline();
					break;
				case DiseaseType.Psychological:
					baseDelta += 0.25f * (negativeLoad - positiveLoad);
					// Эмпатия и любовь снижают
					baseDelta -= 0.1f * moodSystem.empathy;
					baseDelta -= 0.1f * moodSystem.love;
					break;
				case DiseaseType.Mental:
					baseDelta += 0.2f * (moodSystem.stress - moodSystem.calm);
					// Уменьшаем при высокой дисциплине
					baseDelta -= 0.07f * GetDiscipline();
					break;
				case DiseaseType.Spiritual:
					baseDelta += 0.2f * (0.5f - moodSystem.spirituality);
					// Любовь и духовность исцеляют
					baseDelta -= 0.12f * moodSystem.love;
					baseDelta -= 0.12f * moodSystem.spirituality;
					break;
				case DiseaseType.Astral:
					baseDelta += 0.18f * (negativeLoad - positiveLoad);
					// Баланс чакр помогает: чем выше текущая чакра и энергия, тем лучше
					baseDelta -= 0.05f * GetChakraBalanceFactor();
					break;
			}
			
			// Карма влияет на общее течение: светлая замедляет, темная ускоряет
			if (karmaSystem != null)
			{
				switch (karmaSystem.GetKarmaTier())
				{
					case KarmaTier.Radiant:
						baseDelta -= 0.08f;
						break;
					case KarmaTier.Light:
						baseDelta -= 0.04f;
						break;
					case KarmaTier.Shadow:
						baseDelta += 0.04f;
						break;
					case KarmaTier.Dark:
						baseDelta += 0.08f;
						break;
				}
			}
			
			return baseDelta;
		}
		
		private float GetDiscipline()
		{
			return GameManager.Instance?.GetComponent<MoodSystem>()?.discipline ?? 0.5f;
		}
		
		private float GetChakraBalanceFactor()
		{
			if (chakraSystem == null) return 0.5f;
			int level = GameManager.Instance.currentChakraLevel;
			float energy = chakraSystem.GetEnergyPercentage(); // 0..1
			// Чем выше уровень и выше энергия, тем больше защита
			return Mathf.Clamp01(level / 12f * 0.5f + energy * 0.5f);
		}
	}
}