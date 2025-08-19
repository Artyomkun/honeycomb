using UnityEngine;
using System;

namespace RealLife5D.Systems
{
	[System.Serializable]
	public class BirthProfile
	{
		public DateTime birthDateUtc;
		public string birthPlace; // свободная строка
		public int soulMatrixSeed; // хэш идеи "Матрица Души"
		public int lifePath;       // производная 1..9
	}
	
	public class BirthProfileSystem : MonoBehaviour
	{
		[Header("Birth Profile")]
		public BirthProfile profile = new BirthProfile();
		
		[Header("Derived Modifiers")]
		public float initialEnergyBonus;     // влияние на стартовую энергию
		public int initialChakraBiasIndex;   // чакра, к которой есть предрасположенность (0..153)
		public float cosmoChannelAffinity;   // склонность к космоэнергетическим каналам (0..1)
		
		public void Initialize(DateTime birthDateUtc, string birthPlace)
		{
			profile.birthDateUtc = birthDateUtc;
			profile.birthPlace = birthPlace;
			profile.soulMatrixSeed = ComputeSoulMatrixSeed(birthDateUtc, birthPlace);
			profile.lifePath = ComputeLifePathNumber(birthDateUtc);
			
			DeriveModifiers();
		}
		
		private int ComputeSoulMatrixSeed(DateTime date, string place)
		{
			unchecked
			{
				int hash = 17;
				hash = hash * 31 + date.Year;
				hash = hash * 31 + date.Month;
				hash = hash * 31 + date.Day;
				hash = hash * 31 + date.Hour;
				hash = hash * 31 + date.Minute;
				hash = hash * 31 + (place?.GetHashCode() ?? 0);
				return Mathf.Abs(hash);
			}
		}
		
		private int ComputeLifePathNumber(DateTime date)
		{
			// Упрощённая нумерология: сумма цифр даты до 1..9
			string digits = date.ToString("yyyyMMdd");
			int sum = 0;
			foreach (char c in digits) if (char.IsDigit(c)) sum += (c - '0');
			while (sum > 9) { int s=0; while (sum>0){ s += sum%10; sum/=10;} sum=s; }
			return Mathf.Max(1, sum);
		}
		
		private void DeriveModifiers()
		{
			UnityEngine.Random.InitState(profile.soulMatrixSeed);
			initialEnergyBonus = UnityEngine.Random.Range(0.05f, 0.25f) + profile.lifePath * 0.01f;
			initialChakraBiasIndex = UnityEngine.Random.Range(0, 154);
			cosmoChannelAffinity = Mathf.Clamp01(0.3f + profile.lifePath * 0.05f);
		}
	}
}