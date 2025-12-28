#include "Managers/GlobalEventManager.hpp"
#include "Managers/ResourceManager.hpp"
#include "Utilities/Logger.hpp"
#include "AppContext.hpp"

GlobalEventManager::GlobalEventManager(AppContext* context)
{
    if (!context)
    {
        logger::Error("GlobalEventManager was passed a null context! Cannot initialize.");
        return;
    }
    
    m_Events.onClose = [context](const sf::Event::Closed&)
	{
		context->m_MainWindow->close();
	};

	m_Events.onGlobalKeyPress = [context](const sf::Event::KeyPressed& event)
	{
		if (event.scancode == sf::Keyboard::Scancode::Escape)
		{
			// We will want to remove this if we want escape to exit an inventory window etc.
			logger::Info("Escape key pressed! Exiting.");
			context->m_MainWindow->close();
		}
	};
	
}