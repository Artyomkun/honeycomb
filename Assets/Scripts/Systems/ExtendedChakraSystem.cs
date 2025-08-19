using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
	[System.Serializable]
	public class ExtendedChakra
	{
		public int index; // 0..153
		public string name;
		public string mantra;
		public List<string> abilities = new List<string>();
		public float energyMultiplier = 1f;
		public bool unlocked;
	}
	
	public class ExtendedChakraSystem : MonoBehaviour
	{
		[Header("154 Chakras")]
		public List<ExtendedChakra> chakras = new List<ExtendedChakra>(154);
		public int currentChakraIndex = 0; // 0..153
		
		private BirthProfileSystem birthProfile;
		private CosmoenergySystem cosmo;
		
		void Awake()
		{
			birthProfile = GetComponent<BirthProfileSystem>();
			cosmo = GetComponent<CosmoenergySystem>();
		}
		
		void Start()
		{
			Initialize154();
		}
		
		public void Initialize154()
		{
			chakras.Clear();
			for (int i = 0; i < 154; i++)
			{
				var ch = new ExtendedChakra
				{
					index = i,
					name = $"Чакра {i+1}",
					mantra = (i == 5 || i == 6) ? "Тишина" : "OM",
					energyMultiplier = 1f + i * 0.02f,
					unlocked = false
				};
				if (i == 3) ch.abilities.Add("Любовь");
				if (i == 6) ch.abilities.Add("Духовность");
				chakras.Add(ch);
			}
			// Предрасположенность рождения
			int bias = birthProfile?.initialChakraBiasIndex ?? 0;
			currentChakraIndex = Mathf.Clamp(bias, 0, 153);
			chakras[currentChakraIndex].unlocked = true;
		}
	}
}