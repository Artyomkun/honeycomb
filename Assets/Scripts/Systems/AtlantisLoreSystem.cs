using UnityEngine;

namespace RealLife5D.Systems
{
	public class AtlantisLoreSystem : MonoBehaviour
	{
		[Header("Lore Flags")] 
		public bool remembersAtlantis;
		public bool foundAtlanteanRuins;
		public bool activatedCrystalGrid;
		
		public void TriggerMemory()
		{
			if (!remembersAtlantis)
			{
				remembersAtlantis = true;
				Debug.Log("Вспышка памяти об Атлантиде...");
			}
		}
	}
}