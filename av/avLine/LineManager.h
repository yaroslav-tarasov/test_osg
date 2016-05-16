#pragma once

namespace avLine
{

class LineManager /*: public av::IObject*/ 
{
public:
	osg::ref_ptr<Line>	CreateLine( uint32_t nID, const char* szFolderName );
	//void				UpdateLine( LineSettings& cLineSettings, const LineUpdateData& cLineUpdateData );
	void				DeleteLine( uint32_t nID );

	inline bool			GetShowControlPoints() const { return _bShowControlPoints; }

	static LineManager*	GetInstance();
	static void			Create();
	static void			Release();

private:
						LineManager();
	virtual				~LineManager();
	void				EnableLines( bool bEnable );
	void				ShowControlPoints( bool bShow );

private:
	static LineManager*	_pLineManager;
	bool				_bEnabled;
	bool				_bShowControlPoints;
};

#define GetLineManager() LineManager::GetInstance()

}

