using UnityEngine;
using UnityEngine.UI;

namespace RealLife5D.UI
{
	public class MinimalHealthUI : MonoBehaviour
	{
		public Slider healthSlider;
		public Text temperatureText;
		private PlayerController player;
		
		void Start()
		{
			player = FindObjectOfType<PlayerController>();
		}
		
		void Update()
		{
			if (player == null || healthSlider == null) return;
			healthSlider.value = player.GetHealthPercentage();
			// Энергию не показываем. Температуру — по событию/изменению (заглушка):
			if (temperatureText != null)
			{
				temperatureText.text = "36.6°C"; // TODO: связать с механикой тела
			}
		}
	}
}