#include "CheckBox.hpp"
#include "Graphics/Font.hpp"
#include "UI/Widgets/Label.hpp"

namespace Izo {
	CheckBox::CheckBox(bool state) : m_checked(state), m_label(Label("CheckBox")) {}
	CheckBox::CheckBox(bool state, std::string label) : m_label(Label(label)) {}

	void CheckBox::draw_content(Painter& painter) {
		auto font = FontManager::the().get("system-ui");
	}

	void CheckBox::measure(int parent_w, int parent_h) {
		m_bounds = {0,0,10,10};
	}
}