using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
	[System.Serializable]
	public class DiseaseEntry
	{
		public string id;
		public string name;
		public string type; // Physical/Mental/Psychological/Spiritual/Astral
		public string description;
		public string[] symptoms;
	}
	
	[System.Serializable]
	public class DiseaseCatalogData
	{
		public DiseaseEntry[] diseases;
	}
	
	public class DiseaseCatalog : MonoBehaviour
	{
		public TextAsset catalogJson; // Resources-бандл или прямой ассет
		public List<DiseaseEntry> entries = new List<DiseaseEntry>();
		
		public void Load()
		{
			entries.Clear();
			if (catalogJson == null)
			{
				Debug.LogWarning("DiseaseCatalog: catalogJson not assigned. Using empty catalog.");
				return;
			}
			try
			{
				var data = JsonUtility.FromJson<DiseaseCatalogData>(catalogJson.text);
				if (data?.diseases != null) entries.AddRange(data.diseases);
			}
			catch (System.Exception e)
			{
				Debug.LogWarning($"DiseaseCatalog: JSON parse error {e.Message}");
			}
		}
		
		public DiseaseEntry FindById(string id)
		{
			return entries.Find(e => e.id == id);
		}
	}
}