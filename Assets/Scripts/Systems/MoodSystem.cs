using UnityEngine;

namespace RealLife5D.Systems
{
	public class MoodSystem : MonoBehaviour
	{
		[Header("Mood (0..1)")]
		public float calm;     // Спокойствие
		public float love;     // Любовь/сострадание
		public float joy;      // Радость
		public float fear;     // Страх
		public float anger;    // Злость
		public float sadness;  // Печаль
		public float stress;   // Стресс
		
		[Header("Decay per minute")]
		public float positiveDecay = 0.1f;
		public float negativeDecay = 0.05f;
		
		[Header("Psychological Profile (0..1)")]
		public float resilience = 0.5f;     // Психологическая устойчивость
		public float empathy = 0.5f;        // Эмпатия/сострадание
		public float discipline = 0.5f;     // Самодисциплина
		public float spirituality = 0.5f;   // Духовность/устремленность
		
		void Update()
		{
			// Естественная релаксация: положительные состояния медленно снижаются, негативные тоже спадают
			float dt = Time.deltaTime / 60f;
			calm = Mathf.Max(0f, calm - positiveDecay * dt);
			love = Mathf.Max(0f, love - positiveDecay * dt);
			joy = Mathf.Max(0f, joy - positiveDecay * dt);
			fear = Mathf.Max(0f, fear - negativeDecay * dt);
			anger = Mathf.Max(0f, anger - negativeDecay * dt);
			sadness = Mathf.Max(0f, sadness - negativeDecay * dt);
			stress = Mathf.Max(0f, stress - negativeDecay * dt);
		}
		
		public void AddCalm(float v)  { calm = Mathf.Clamp01(calm + v); }
		public void AddLove(float v)  { love = Mathf.Clamp01(love + v); }
		public void AddJoy(float v)   { joy = Mathf.Clamp01(joy + v); }
		public void AddFear(float v)  { fear = Mathf.Clamp01(fear + v); }
		public void AddAnger(float v) { anger = Mathf.Clamp01(anger + v); }
		public void AddSadness(float v){ sadness = Mathf.Clamp01(sadness + v); }
		public void AddStress(float v){ stress = Mathf.Clamp01(stress + v); }
		
		public float GetWellbeingIndex()
		{
			// Сводный индекс благополучия: положительные минус негативные с учетом устойчивости
			float positives = (calm + love + joy) / 3f;
			float negatives = (fear + anger + sadness + stress) / 4f;
			float raw = positives - negatives;
			// Устойчивость смягчает негатив
			raw += (resilience - 0.5f) * 0.3f;
			return Mathf.Clamp(raw, -1f, 1f);
		}
		
		public void ApplyMeditationBoost()
		{
			AddCalm(0.2f);
			AddLove(0.1f);
			AddJoy(0.05f);
			AddStress(-0.2f);
			AddFear(-0.1f);
		}
		
		public void ApplyHelpOthersBoost()
		{
			AddLove(0.15f);
			AddJoy(0.1f);
			AddStress(-0.05f);
			// Эмпатия растет от помощи
			empathy = Mathf.Clamp01(empathy + 0.01f);
		}
		
		public void ApplyDamageShock(float damage)
		{
			float f = Mathf.Clamp01(damage / 100f);
			AddStress(0.2f * f);
			AddFear(0.15f * f);
			AddCalm(-0.1f * f);
		}
		
		public void ApplyStagnationPenalty()
		{
			// Застой снижает радость/любовь и повышает стресс/печаль
			AddJoy(-0.1f);
			AddLove(-0.1f);
			AddSadness(0.1f);
			AddStress(0.1f);
		}
	}
}