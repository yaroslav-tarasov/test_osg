#pragma once 

inline void subscribeEvent(const String& widget, const String& event, const Event::Subscriber& method)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();

    if (root->isChild(widget))
    {
        Window* window = root->getChild(widget);
        window->subscribeEvent(event, method);
    }
}

inline bool isCheckboxSelected(const CEGUI::String& checkbox)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    // Check
    if (root->isChild(checkbox))
    {
        ToggleButton* button = static_cast<ToggleButton*>(root->getChild(checkbox));
        return button->isSelected();
    }
    return false;
}

inline std::string getEditboxText(const CEGUI::String& editbox)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    // Check
    if (root->isChild(editbox))
    {
        Editbox* button = static_cast<Editbox*>(root->getChild(editbox));
        return std::string(button->getText().c_str());
    }
    return "";
}

inline bool setEditboxText(const CEGUI::String& editbox, const CEGUI::String& text)
{
    GUIContext& context = System::getSingleton().getDefaultGUIContext();
    CEGUI::Window* root = context.getRootWindow();
    // Check
    if (root->isChild(editbox))
    {
        Editbox* button = static_cast<Editbox*>(root->getChild(editbox));
        button->setText(text);
        return true;
    }
    return false;
}
