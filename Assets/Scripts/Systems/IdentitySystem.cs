using UnityEngine;
using System.Collections.Generic;

namespace RealLife5D.Systems
{
	public enum GenderIdentity
	{
		Male,
		Female,
		NonBinary,
		Agender,
		Bigender,
		Genderfluid,
		Other
	}
	
	public enum RelationshipType
	{
		Single,
		Monogamous,
		Polyamorous,
		Aromantic,
		Asexual,
		OpenRelationship,
		CivilUnion,
		Marriage
	}
	
	public enum Religion
	{
		Atheism,
		Agnosticism,
		Christianity,
		Islam,
		Judaism,
		Buddhism,
		Hinduism,
		Sikhism,
		Taoism,
		Shinto,
		Paganism,
		Other
	}
	
	[System.Serializable]
	public class Personality
	{
		public float openness = 0.5f;
		public float conscientiousness = 0.5f;
		public float extraversion = 0.5f;
		public float agreeableness = 0.5f;
		public float neuroticism = 0.5f;
		
		public List<string> biases = new List<string>(); // предубеждения
	}
	
	public class IdentitySystem : MonoBehaviour
	{
		[Header("Identity")]
		public string playerName;
		public GenderIdentity gender = GenderIdentity.Other;
		public RelationshipType relationship = RelationshipType.Single;
		public Religion religion = Religion.Other;
		public Personality personality = new Personality();
		
		public delegate void ChangeRequest<T>(T newValue, System.Action<bool> onConfirm);
		
		// Запрос изменения с предупреждением (только через подтверждение)
		public void RequestChangeGender(GenderIdentity newGender, System.Action<bool> onConfirm)
		{
			// Здесь можно вывести UI предупреждения и позвать onConfirm(true/false)
			onConfirm?.Invoke(false); // по умолчанию отклоняем без UI; интеграция с UI требуется
		}
		
		public void RequestChangeReligion(Religion newReligion, System.Action<bool> onConfirm)
		{
			onConfirm?.Invoke(false);
		}
		
		public void RequestChangeRelationship(RelationshipType newType, System.Action<bool> onConfirm)
		{
			onConfirm?.Invoke(false);
		}
	}
}