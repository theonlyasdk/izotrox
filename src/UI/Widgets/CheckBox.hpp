#pragma once


#include "UI/Widgets/Label.hpp"
#include "UI/Widgets/Widget.hpp"

namespace Izo {
	class CheckBox : Widget {
	public:
		CheckBox(bool state);
		CheckBox(bool state, std::string label);
		~CheckBox() = default;
		void draw_content(Painter& painter) override;
    	void measure(int parent_w, int parent_h) override; 
    private:
    	Label m_label;
    	bool m_checked;
	};
};