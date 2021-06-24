#pragma once

#include <vector>
#include <string>
#include <sdk/sdk.hpp>

enum menu_item_type
{
	NEXT_LINE,
	CHECK_BOX,
	COMBO_BOX,
	SLIDER_INT,
	SLIDER_FLOAT,
	COLOR_PICKER,
	KEY_BIND,
};

class menu_item
{
public:
	bool check_box_value = false;

	std::vector <std::string> combo_box_labels;
	int combo_box_value = 0;

	int slider_int_min = 0;
	int slider_int_max = 0;
	int slider_int_value = 0;

	float slider_float_min = 0.0f;
	float slider_float_max = 0.0f;
	float slider_float_value = 0.0f;

	float color_picker_value[4] = {1.f, 1.f, 1.f, 1.f};

	int keybind_key_value = 0;
	int keybind_key_mode = 0;

	menu_item_type type = NEXT_LINE;

	menu_item()
	{
		type = NEXT_LINE;
	}

	menu_item(const menu_item& item)
	{
		check_box_value = item.check_box_value;

		combo_box_labels = item.combo_box_labels;
		combo_box_value = item.combo_box_value;

		slider_int_min = item.slider_int_min;
		slider_int_max = item.slider_int_max;
		slider_int_value = item.slider_int_value;

		slider_float_min = item.slider_float_min;
		slider_float_max = item.slider_float_max;
		slider_float_value = item.slider_float_value;

		color_picker_value[0] = item.color_picker_value[0];
		color_picker_value[1] = item.color_picker_value[1];
		color_picker_value[2] = item.color_picker_value[2];
		color_picker_value[3] = item.color_picker_value[3];
		type = item.type;
	}

	menu_item& operator=(const menu_item& item)
	{
		check_box_value = item.check_box_value;

		combo_box_labels = item.combo_box_labels;
		combo_box_value = item.combo_box_value;

		slider_int_min = item.slider_int_min;
		slider_int_max = item.slider_int_max;
		slider_int_value = item.slider_int_value;

		slider_float_min = item.slider_float_min;
		slider_float_max = item.slider_float_max;
		slider_float_value = item.slider_float_value;

		color_picker_value[0] = item.color_picker_value[0];
		color_picker_value[1] = item.color_picker_value[1];
		color_picker_value[2] = item.color_picker_value[2];
		color_picker_value[3] = item.color_picker_value[3];
		type = item.type;

		return *this;
	}

	menu_item(bool value)
	{
		check_box_value = value;
		type = CHECK_BOX;
	}

	menu_item(std::vector <std::string> labels, int value) //-V818
	{
		combo_box_labels = labels; //-V820
		combo_box_value = value;

		type = COMBO_BOX;
	}

	menu_item(int min, int max, int value)
	{
		slider_int_min = min;
		slider_int_max = max;
		slider_int_value = value;

		type = SLIDER_INT;
	}

	menu_item(float min, float max, float value)
	{
		slider_float_min = min;
		slider_float_max = max;
		slider_float_value = value;

		type = SLIDER_FLOAT;
	}

	menu_item(float value[4]) //-V818
	{
		color_picker_value[0] = value[0];
		color_picker_value[1] = value[1];
		color_picker_value[2] = value[2];
		color_picker_value[3] = value[3];
		type = COLOR_PICKER;
	}

	menu_item(int key, int mode) //-V818
	{
		keybind_key_value = key;
		keybind_key_mode = mode;
		type = KEY_BIND;
	}
};