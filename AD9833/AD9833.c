/*

  ******************************************************************************
  * @file 			( фаил ):   AD9833.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
*/

/* Includes ----------------------------------------------------------*/
#include "AD9833.h"


extern SPI_HandleTypeDef 							AD9833_SPI_PORT;



static void AD9833_Select(void);
static void AD9833_Unselect(void);
static void AD9833_WriteRegister(uint16_t data);
static void AD9833_WriteCfgReg(void);

/*
	******************************************************************************
	* @brief	 ( описание ):  
	* @param	( параметры ):	
	* @return  ( возвращает ):	

	******************************************************************************
*/
uint8_t _waveform = WAVEFORM_SINE;
uint8_t _sleep_mode = NO_POWERDOWN;
uint8_t _freq_source = 0;
uint8_t _phase_source = 0;
uint8_t _reset_state = 0;

static void AD9833_Select(void)
{
	HAL_GPIO_WritePin(AD9833_FSYNC_GPIO_Port, AD9833_FSYNC_Pin, GPIO_PIN_RESET);
}

static void AD9833_Unselect(void)
{
	HAL_GPIO_WritePin(AD9833_FSYNC_GPIO_Port, AD9833_FSYNC_Pin, GPIO_PIN_SET);
}

static void AD9833_WriteRegister(uint16_t data)
{
	AD9833_Select();
	uint8_t LByte = data & 0xff;
	uint8_t HByte = (data >> 8) & 0xff;
	HAL_SPI_Transmit(&AD9833_SPI_PORT, &HByte, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&AD9833_SPI_PORT, &LByte, 1, HAL_MAX_DELAY);
	AD9833_Unselect();
}

static void AD9833_WriteCfgReg(void)
{
	uint16_t cfg = 0;
	cfg |= _waveform;
	cfg |= _sleep_mode;
	cfg |= (_freq_source ? F_SELECT_CFG : 0);		//it's unimportant because don't use FREQ1
	cfg |= (_phase_source ? P_SELECT_CFG : 0);	//it's unimportant because don't use PHASE1
	cfg |= (_reset_state ? RESET_CFG : 0);
	cfg |= B28_CFG;
	AD9833_WriteRegister(cfg);
}



/*
 * @brief Set signal generation waveform
 * @param Waveform in WaveDef Type declared in .h file
 */
void AD9833_SetWaveform(WaveDef Wave)
{
	if (Wave == wave_sine){ 			_waveform = WAVEFORM_SINE; }
	else if (Wave == wave_square) {	_waveform = WAVEFORM_SQUARE; }
	else if (Wave == wave_square_div2) {	_waveform = WAVEFORM_SQUARE_DIV2; }
	else if (Wave == wave_triangle){	_waveform = WAVEFORM_TRIANGLE; }
	AD9833_WriteCfgReg();
}


/*
 * @brief Set signal generation frequency
 * @param Frequency value in uint32_t format
 */
void AD9833_SetFrequency(float freq)
{
	// TODO: calculate max frequency based on refFrequency.
	// Use the calculations for sanity checks on numbers.
	// Sanity check on frequency: Square - refFrequency / 2
	// Sine/Triangle - refFrequency / 4

	if (freq > (FMCLK >> 1)){	//bitwise FMCLK / 2
		freq = FMCLK >> 1;
	}
	else if (freq < 0){ freq = 0; }

	uint32_t freq_reg = (((float)freq * (float)((1 << 28)) / FMCLK)); // Tuning word

	uint16_t LSB = FREQ0_REG | (freq_reg & 0x3FFF);
	uint16_t MSB = FREQ0_REG | ((freq_reg & 0xFFFC000) >> 14);

	AD9833_WriteCfgReg();	// Update Config Register
	AD9833_WriteRegister(LSB);
	AD9833_WriteRegister(MSB);
}



/*
 * @brief Set signal generation phase
 * @param Phase in degrees in uint16_t format. Value can be large then 360
 */
void AD9833_SetPhase(float phase_deg)
{
	if(phase_deg < 0.0f){ phase_deg = 0.0f; }
	else if (phase_deg > 360.0f){ phase_deg = 360.0f; }
	
	uint16_t phase_val  = ((uint16_t)(phase_deg * BITS_PER_DEG)) &  0xFFF;
	AD9833_WriteRegister(PHASE0_REG | phase_val);
}


/*
 * @brief AD9833 Initial Configuration
 * @param Type of Waveform, Frequency, Phase in degrees
 */
void AD9833_Init(WaveDef Wave, float freq, float phase_deg)
{
	AD9833_OutputEnable(0);
	AD9833_SetWaveform(Wave);
	AD9833_WriteCfgReg();
	AD9833_SetFrequency(freq);
	AD9833_SetPhase(phase_deg);
	AD9833_OutputEnable(1);
}


/*
 * @brief Set Sleep Mode Function (Explained in datasheet Table 14)
 * @param Mode of sleep function defined in title
 */
void AD9833_SleepMode(uint8_t mode)
{
	_sleep_mode = mode;
	AD9833_WriteCfgReg();
}


/*
 * @brief Enable or disable the output of the AD9833
 * @param Output state (ON/OFF)
 */
void AD9833_OutputEnable(uint8_t output_state)
{
	_reset_state = !output_state;
	AD9833_WriteCfgReg();
}
//----------------------------------------------------------------------------------



/************************ (C) COPYRIGHT GKP *****END OF FILE****/
