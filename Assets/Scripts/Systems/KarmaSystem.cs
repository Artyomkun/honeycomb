using UnityEngine;

namespace RealLife5D.Systems
{
	public enum KarmaTier
	{
		Dark = -2,
		Shadow = -1,
		Neutral = 0,
		Light = 1,
		Radiant = 2
	}
	
	public enum KarmaAction
	{
		Meditation,
		HealSelf,
		HealOthers,
		HelpNPC,
		HarmNPC,
		Truth,
		Lie
	}
	
	public class KarmaSystem : MonoBehaviour
	{
		[Header("Karma State")]
		public float currentKarma = 0f; // диапазон [-1000, 1000]
		public float minKarma = -1000f;
		public float maxKarma = 1000f;
		public float passiveDecayPerMinute = 0f; // при желании можно включить спад к нейтрали
		
		void Update()
		{
			if (passiveDecayPerMinute > 0f && Mathf.Abs(currentKarma) > 0.1f)
			{
				float sign = Mathf.Sign(currentKarma);
				currentKarma = Mathf.MoveTowards(currentKarma, 0f, passiveDecayPerMinute / 60f * Time.deltaTime);
			}
		}
		
		public void AddKarma(float amount)
		{
			currentKarma = Mathf.Clamp(currentKarma + amount, minKarma, maxKarma);
		}
		
		public void AddKarmaForAction(KarmaAction action)
		{
			switch (action)
			{
				case KarmaAction.Meditation:
					AddKarma(2f);
					break;
				case KarmaAction.HealSelf:
					AddKarma(1f);
					break;
				case KarmaAction.HealOthers:
					AddKarma(5f);
					break;
				case KarmaAction.HelpNPC:
					AddKarma(3f);
					break;
				case KarmaAction.HarmNPC:
					AddKarma(-6f);
					break;
				case KarmaAction.Truth:
					AddKarma(2f);
					break;
				case KarmaAction.Lie:
					AddKarma(-2f);
					break;
			}
		}
		
		public KarmaTier GetKarmaTier()
		{
			if (currentKarma >= 400f) return KarmaTier.Radiant;
			if (currentKarma >= 100f) return KarmaTier.Light;
			if (currentKarma <= -400f) return KarmaTier.Dark;
			if (currentKarma <= -100f) return KarmaTier.Shadow;
			return KarmaTier.Neutral;
		}
	}
}