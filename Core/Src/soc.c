#include "soc.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "main.h"

void SOC_updateCurrent(batteryModule *batt);

uint16_t SOC_offsetFilter(uint16_t measuredX, uint16_t lowerX, uint16_t upperX,
                          uint16_t lowerY, uint16_t upperY);
uint16_t SOC_searchCapacity(uint16_t data[][2], uint16_t voltage, uint16_t size);

uint16_t SOC_getChargeData0C(uint16_t voltage);
uint16_t SOC_getChargeData25C(uint16_t voltage);
uint16_t SOC_getChargeData40C(uint16_t voltage);

void SOC_getInitialCharge(batteryModule *batt) {
    uint32_t voltage;
    for (int i = 0; i < NUM_CELLS; ++i) {
        voltage += batt->cell_volt[i];
    }
    voltage /= (NUM_DEVICES * 10);

    uint16_t temperature;
    for (int i = 0; i < NUM_THERM_TOTAL; ++i) {
        temperature += batt->cell_temp[i];
    }
    temperature /= NUM_DEVICES;

    int tempCharts[] = {0, 25, 40};

    int minDiff = 100;
    int selectTemp;
    for (int i = 0; i < 3; ++i) {
        if (abs(temperature - tempCharts[i]) < minDiff) {
            minDiff = abs(temperature - tempCharts[i]);
            selectTemp = tempCharts[i];
        }
    }

    switch (selectTemp) {
        case 0:
            batt->soc = SOC_getChargeData0C((uint16_t) voltage) * NUM_DEVICES;
            break;
        case 25:
            batt->soc = SOC_getChargeData25C((uint16_t) voltage) * NUM_DEVICES;
            break;
        default:
            batt->soc = SOC_getChargeData40C((uint16_t) voltage) * NUM_DEVICES;
            break;
    }
}

void SOC_updateCurrent(batteryModule *batt) {
    uint32_t adcValue = 0;
    HAL_ADC_Start(&hadc2);
    if (HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY) == HAL_OK) {
        adcValue = HAL_ADC_GetValue(&hadc2);
    }
    printf("ADC VALUE %u/4096\n", adcValue);
    HAL_ADC_Stop(&hadc2);
    float voltage = ((float)adcValue / ADC_RESOLUTION) * V_REF;
    batt->current = (voltage / (MAX_SHUNT_VOLTAGE * SHUNT_OPAMP_RATIO)) * MAX_SHUNT_AMPAGE + 500;
}

void SOC_updateCharge(batteryModule *batt, uint32_t elapsed_time) {

	HAL_ADCEx_Calibration_Start(&hadc2);
    SOC_updateCurrent(batt);
    batt->soc -= (uint16_t)(batt->current * (float)(elapsed_time / 3600000.0f));
}

uint16_t SOC_searchCapacity(uint16_t data[][2], uint16_t target, uint16_t size) {
    // Get the two closest capacities
    uint16_t left = 0;
    uint16_t right = size - 1;

    if (target >= data[left][0]) {
    	return data[left][1];
    }

    if (target <= data[right][0]) {
    	return data[right][1];
    }

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (data[mid][0] == target) {
            return data[mid][1];
        }

        if (data[mid][0] > target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }

        if (left == right - 1) {
        	break;
        }
    }

    return data[right][1];
}

uint16_t SOC_getChargeData0C(uint16_t voltage) {
    int datalen = 202;
    uint16_t data[][2] = {
        {4172, 2924}, {4131, 2910}, {4119, 2896}, {4109, 2882}, {4100, 2868},
        {4093, 2854}, {4087, 2840}, {4082, 2826}, {4077, 2812}, {4073, 2798},
        {4069, 2784}, {4066, 2770}, {4063, 2756}, {4060, 2742}, {4058, 2728},
        {4055, 2714}, {4053, 2700}, {4050, 2686}, {4048, 2672}, {4046, 2658},
        {4044, 2644}, {4042, 2630}, {4039, 2616}, {4037, 2602}, {4035, 2588},
        {4032, 2574}, {4030, 2560}, {4026, 2546}, {4023, 2533}, {4020, 2519},
        {4016, 2505}, {4012, 2491}, {4007, 2477}, {4003, 2463}, {3998, 2449},
        {3993, 2435}, {3987, 2421}, {3982, 2407}, {3976, 2393}, {3970, 2379},
        {3964, 2365}, {3958, 2351}, {3952, 2337}, {3946, 2323}, {3939, 2309},
        {3933, 2295}, {3927, 2281}, {3921, 2267}, {3915, 2253}, {3909, 2239},
        {3903, 2225}, {3898, 2211}, {3892, 2197}, {3888, 2183}, {3883, 2169},
        {3878, 2155}, {3873, 2141}, {3869, 2127}, {3864, 2113}, {3860, 2099},
        {3856, 2085}, {3852, 2071}, {3849, 2057}, {3845, 2043}, {3842, 2029},
        {3838, 2015}, {3834, 2001}, {3831, 1987}, {3827, 1973}, {3824, 1959},
        {3820, 1945}, {3817, 1931}, {3813, 1917}, {3809, 1903}, {3805, 1889},
        {3802, 1876}, {3798, 1862}, {3793, 1848}, {3789, 1834}, {3785, 1820},
        {3781, 1806}, {3776, 1792}, {3771, 1778}, {3767, 1764}, {3762, 1750},
        {3757, 1736}, {3751, 1722}, {3746, 1708}, {3740, 1694}, {3735, 1680},
        {3729, 1666}, {3724, 1652}, {3718, 1638}, {3713, 1624}, {3707, 1610},
        {3701, 1596}, {3695, 1582}, {3690, 1568}, {3684, 1554}, {3679, 1540},
        {3673, 1526}, {3668, 1512}, {3662, 1498}, {3657, 1484}, {3651, 1470},
        {3646, 1456}, {3640, 1442}, {3634, 1428}, {3628, 1414}, {3623, 1400},
        {3617, 1386}, {3611, 1372}, {3606, 1358}, {3600, 1344}, {3595, 1330},
        {3589, 1316}, {3584, 1302}, {3579, 1288}, {3574, 1274}, {3569, 1260},
        {3565, 1247}, {3561, 1233}, {3556, 1219}, {3552, 1205}, {3548, 1191},
        {3543, 1177}, {3539, 1163}, {3535, 1149}, {3531, 1135}, {3526, 1121},
        {3522, 1107}, {3517, 1093}, {3512, 1079}, {3506, 1065}, {3501, 1051},
        {3496, 1037}, {3491, 1023}, {3485, 1010}, {3480, 995},  {3475, 981},
        {3470, 967},  {3464, 953},  {3458, 939},  {3452, 925},  {3447, 911},
        {3441, 897},  {3434, 883},  {3428, 869},  {3421, 855},  {3415, 841},
        {3408, 827},  {3401, 813},  {3394, 799},  {3387, 785},  {3379, 771},
        {3371, 757},  {3363, 743},  {3355, 729},  {3346, 715},  {3338, 701},
        {3329, 687},  {3320, 673},  {3310, 659},  {3300, 645},  {3290, 631},
        {3280, 617},  {3270, 604},  {3260, 590},  {3249, 576},  {3239, 562},
        {3229, 548},  {3219, 534},  {3209, 520},  {3199, 506},  {3190, 492},
        {3180, 478},  {3171, 464},  {3162, 450},  {3152, 436},  {3143, 422},
        {3133, 408},  {3122, 394},  {3112, 380},  {3101, 366},  {3089, 352},
        {3076, 338},  {3063, 324},  {3048, 310},  {3032, 296},  {3015, 282},
        {2996, 268},  {2976, 254},  {2952, 240},  {2926, 226},  {2896, 212},
        {2861, 198},  {2820, 184},  {2770, 170},  {2706, 156},  {2619, 142},
        {2517, 130},  {2500, 128}};

    return SOC_searchCapacity(data, voltage, datalen);
}

uint16_t SOC_getChargeData25C(uint16_t voltage) {
    int datalen = 202;
    uint16_t data[][2] = {
        {4178, 2921}, {4156, 2907}, {4145, 2892}, {4134, 2878}, {4126, 2863},
        {4119, 2849}, {4112, 2834}, {4107, 2820}, {4102, 2805}, {4098, 2791},
        {4094, 2776}, {4090, 2762}, {4088, 2747}, {4085, 2733}, {4082, 2718},
        {4080, 2704}, {4078, 2689}, {4076, 2675}, {4074, 2660}, {4072, 2646},
        {4070, 2631}, {4069, 2617}, {4067, 2602}, {4065, 2588}, {4063, 2573},
        {4062, 2559}, {4059, 2544}, {4057, 2530}, {4054, 2516}, {4052, 2501},
        {4048, 2487}, {4044, 2472}, {4040, 2458}, {4036, 2443}, {4032, 2429},
        {4027, 2414}, {4022, 2400}, {4016, 2385}, {4011, 2371}, {4005, 2356},
        {3999, 2342}, {3993, 2327}, {3987, 2313}, {3981, 2298}, {3975, 2284},
        {3968, 2269}, {3962, 2255}, {3955, 2240}, {3949, 2226}, {3942, 2211},
        {3935, 2197}, {3928, 2182}, {3921, 2168}, {3915, 2153}, {3908, 2139},
        {3902, 2124}, {3895, 2110}, {3890, 2095}, {3884, 2081}, {3879, 2066},
        {3874, 2052}, {3870, 2038}, {3866, 2023}, {3862, 2009}, {3858, 1994},
        {3855, 1980}, {3851, 1965}, {3847, 1951}, {3844, 1936}, {3840, 1922},
        {3837, 1907}, {3834, 1893}, {3830, 1878}, {3826, 1864}, {3822, 1849},
        {3819, 1835}, {3815, 1820}, {3811, 1806}, {3807, 1791}, {3803, 1777},
        {3799, 1762}, {3795, 1748}, {3790, 1733}, {3786, 1719}, {3782, 1704},
        {3777, 1690}, {3773, 1675}, {3768, 1661}, {3764, 1646}, {3759, 1632},
        {3754, 1617}, {3750, 1603}, {3745, 1588}, {3740, 1574}, {3735, 1559},
        {3729, 1545}, {3724, 1531}, {3718, 1516}, {3712, 1502}, {3706, 1487},
        {3700, 1473}, {3694, 1458}, {3687, 1444}, {3680, 1429}, {3674, 1415},
        {3667, 1400}, {3661, 1386}, {3655, 1371}, {3648, 1357}, {3642, 1342},
        {3636, 1328}, {3629, 1313}, {3622, 1299}, {3614, 1284}, {3607, 1270},
        {3600, 1255}, {3594, 1241}, {3587, 1226}, {3581, 1212}, {3574, 1197},
        {3567, 1183}, {3561, 1168}, {3555, 1154}, {3551, 1139}, {3547, 1125},
        {3543, 1110}, {3539, 1096}, {3536, 1081}, {3532, 1067}, {3528, 1052},
        {3524, 1038}, {3520, 1024}, {3515, 1009}, {3509, 995},  {3502, 980},
        {3493, 966},  {3486, 951},  {3480, 937},  {3474, 922},  {3468, 908},
        {3461, 893},  {3455, 879},  {3448, 864},  {3441, 850},  {3434, 835},
        {3427, 821},  {3420, 806},  {3412, 792},  {3405, 777},  {3397, 763},
        {3389, 748},  {3381, 734},  {3373, 719},  {3365, 705},  {3356, 690},
        {3347, 676},  {3338, 661},  {3329, 647},  {3319, 632},  {3309, 618},
        {3299, 603},  {3289, 589},  {3279, 574},  {3269, 560},  {3259, 546},
        {3249, 531},  {3239, 517},  {3229, 502},  {3219, 488},  {3210, 473},
        {3201, 459},  {3192, 444},  {3183, 430},  {3173, 415},  {3164, 401},
        {3155, 386},  {3146, 372},  {3136, 357},  {3126, 343},  {3115, 328},
        {3104, 314},  {3092, 299},  {3079, 285},  {3065, 270},  {3051, 256},
        {3035, 241},  {3019, 227},  {3001, 212},  {2982, 198},  {2962, 183},
        {2940, 169},  {2916, 154},  {2890, 140},  {2861, 125},  {2829, 111},
        {2792, 96},   {2751, 82},   {2703, 67},   {2647, 53},   {2596, 42},
        {2581, 39},   {2500, 24}};

    return SOC_searchCapacity(data, voltage, datalen);
}

uint16_t SOC_getChargeData40C(uint16_t voltage) {
    int datalen = 202;
    uint16_t data[][2] = {
        {4182, 2920}, {4163, 2905}, {4151, 2891}, {4141, 2876}, {4132, 2862},
        {4125, 2847}, {4118, 2833}, {4112, 2818}, {4107, 2804}, {4103, 2789},
        {4099, 2775}, {4096, 2761}, {4093, 2746}, {4089, 2732}, {4087, 2717},
        {4085, 2703}, {4082, 2688}, {4080, 2674}, {4079, 2659}, {4077, 2645},
        {4075, 2630}, {4073, 2616}, {4072, 2601}, {4070, 2587}, {4068, 2572},
        {4067, 2558}, {4065, 2543}, {4062, 2529}, {4060, 2514}, {4057, 2500},
        {4054, 2486}, {4051, 2471}, {4047, 2457}, {4043, 2442}, {4038, 2428},
        {4034, 2413}, {4029, 2399}, {4024, 2384}, {4018, 2370}, {4013, 2355},
        {4007, 2341}, {4002, 2326}, {3996, 2312}, {3990, 2297}, {3984, 2283},
        {3978, 2268}, {3971, 2254}, {3965, 2239}, {3959, 2225}, {3953, 2210},
        {3947, 2196}, {3940, 2182}, {3933, 2167}, {3926, 2153}, {3919, 2138},
        {3912, 2124}, {3905, 2109}, {3899, 2095}, {3893, 2080}, {3887, 2066},
        {3882, 2051}, {3878, 2037}, {3874, 2022}, {3870, 2008}, {3866, 1993},
        {3862, 1979}, {3859, 1964}, {3855, 1950}, {3852, 1935}, {3848, 1921},
        {3845, 1906}, {3841, 1892}, {3837, 1878}, {3834, 1863}, {3830, 1849},
        {3827, 1834}, {3823, 1820}, {3819, 1805}, {3815, 1791}, {3811, 1776},
        {3807, 1762}, {3803, 1747}, {3799, 1733}, {3795, 1718}, {3790, 1704},
        {3786, 1689}, {3782, 1675}, {3777, 1660}, {3773, 1646}, {3768, 1631},
        {3763, 1617}, {3758, 1603}, {3753, 1588}, {3747, 1574}, {3741, 1559},
        {3735, 1545}, {3728, 1530}, {3721, 1516}, {3714, 1501}, {3708, 1487},
        {3701, 1472}, {3695, 1458}, {3688, 1443}, {3681, 1429}, {3675, 1414},
        {3668, 1400}, {3662, 1385}, {3655, 1371}, {3649, 1356}, {3643, 1342},
        {3637, 1327}, {3630, 1313}, {3623, 1299}, {3616, 1284}, {3608, 1270},
        {3602, 1255}, {3595, 1241}, {3588, 1226}, {3582, 1212}, {3575, 1197},
        {3568, 1183}, {3562, 1168}, {3556, 1154}, {3552, 1139}, {3548, 1125},
        {3544, 1110}, {3540, 1096}, {3537, 1081}, {3533, 1067}, {3529, 1052},
        {3525, 1038}, {3520, 1023}, {3515, 1009}, {3509, 995},  {3500, 980},
        {3492, 966},  {3485, 951},  {3479, 937},  {3473, 922},  {3467, 908},
        {3460, 893},  {3453, 879},  {3446, 864},  {3439, 850},  {3432, 835},
        {3424, 821},  {3417, 806},  {3409, 792},  {3402, 777},  {3394, 763},
        {3386, 748},  {3378, 734},  {3370, 720},  {3361, 705},  {3353, 691},
        {3344, 676},  {3335, 662},  {3326, 647},  {3316, 633},  {3306, 618},
        {3297, 604},  {3287, 589},  {3277, 575},  {3267, 560},  {3257, 546},
        {3247, 531},  {3238, 517},  {3228, 502},  {3219, 488},  {3210, 473},
        {3201, 459},  {3192, 444},  {3183, 430},  {3173, 416},  {3164, 401},
        {3155, 387},  {3146, 372},  {3136, 358},  {3125, 343},  {3115, 329},
        {3103, 314},  {3091, 300},  {3078, 285},  {3065, 271},  {3050, 256},
        {3035, 242},  {3019, 227},  {3001, 213},  {2983, 198},  {2963, 184},
        {2941, 169},  {2918, 155},  {2892, 141},  {2864, 126},  {2832, 112},
        {2795, 97},   {2753, 83},   {2705, 68},   {2648, 54},   {2581, 39},
        {2506, 26},   {2500, 25}};

    return SOC_searchCapacity(data, voltage, datalen);
}

