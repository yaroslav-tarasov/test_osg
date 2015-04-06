#pragma once

#ifdef _DEBUG
#pragma comment(lib, "BulletCollision_Debug.lib")
#pragma comment(lib, "LinearMath_Debug.lib")
#pragma comment(lib, "BulletDynamics_Debug.lib")
#pragma comment(lib, "BulletWorldImporter_Debug.lib")
#pragma comment(lib, "BulletFileLoader_Debug.lib")
#else 
#pragma comment(lib, "BulletCollision.lib")
#pragma comment(lib, "LinearMath.lib")
#pragma comment(lib, "BulletDynamics.lib")   
#pragma comment(lib, "BulletWorldImporter.lib")
#pragma comment(lib, "BulletFileLoader.lib")
#endif