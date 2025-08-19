using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
	[System.Serializable]
	public class CosmoChannel
	{
		public string id;
		public string name;
		public float resonance; // 0..1
		public List<int> chakraIndices = new List<int>(); // какие чакры (0..153) усиливает канал
	}
	
	public class CosmoenergySystem : MonoBehaviour
	{
		[Header("Channels")] 
		public List<CosmoChannel> channels = new List<CosmoChannel>();
		public float globalResonance; // средняя резонансность с профилем
		
		private BirthProfileSystem birthProfile;
		private GameManager gm;
		
		void Start()
		{
			gm = GameManager.Instance;
			birthProfile = gm.GetComponent<BirthProfileSystem>();
			InitializeChannels();
			ComputeGlobalResonance();
		}
		
		private void InitializeChannels()
		{
			// Прототип: несколько базовых каналов
			channels.Add(CreateChannel("SOL", "Солнечный канал", new int[]{ 6, 10, 33, 72 }));
			channels.Add(CreateChannel("LUN", "Лунный канал", new int[]{ 1, 4, 28, 90 }));
			channels.Add(CreateChannel("TER", "Земной канал", new int[]{ 0, 2, 3, 15 }));
			channels.Add(CreateChannel("AKA", "Канал Акаши", new int[]{ 100, 120, 140, 153 }));
		}
		
		private CosmoChannel CreateChannel(string id, string name, int[] chakraIdx)
		{
			var ch = new CosmoChannel{ id = id, name = name, resonance = 0f };
			ch.chakraIndices.AddRange(chakraIdx);
			return ch;
		}
		
		private void ComputeGlobalResonance()
		{
			if (birthProfile == null) { globalResonance = 0.5f; return; }
			float aff = birthProfile.cosmoChannelAffinity;
			foreach (var ch in channels)
			{
				// Чем ближе предрасположенность к индексам канала, тем выше резонанс
				ch.resonance = 0.3f + aff * 0.7f;
			}
			globalResonance = 0.3f + aff * 0.7f;
		}
	}
}