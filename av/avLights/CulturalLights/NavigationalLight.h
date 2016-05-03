#pragma once


//
// Local namespaces
//

namespace avLights
{

    //
    // Navigational light node class
    //
    // On Update() it decides, whether or not it is visible according to time and blinking period
    // On Cull() it decides about sector and also adds itself to Manager to be drawn after
    //

    class NavigationalLight : public osg::Node
    {
    public:

        // constructor
        NavigationalLight();

        // light exhibitin
        enum LightExhibitionCondition
        {
            Always = 1,
            DayTimeOnly = 2,
            FogOnly = 3,
            NightTimeOnly = 4
        };

        //
        // Interface functions
        //

        // base set up
        void SetEnableState        (bool bOn);
        void SetExhibitionCondition(LightExhibitionCondition condition);
        void SetLocalPosition      (const osg::Vec3f & vPos);
        void SetSize               (float fSize);
        void SetVisDistance        (float fVisDist);
        void SetColor              (const osg::Vec4ub & cCol);

        // sector set-up (according to +X position in planar space)
        // if input is (0, 0) - then no sectoring assumed
        void SetSectorRange(float fStartAngle, float fFinishAngle);

        // blinking sequence set-up
        void SetBlinkingSequence(const std::vector<float> & aBlinkSeq);
        void SetBlinkingDelay   (float fDelay);

        // getters
        const bool          GetEnableState  () const;
        const osg::Vec3f  & GetLocalPosition() const;
        const osg::Vec4ub & GetColor        () const;

    public:

        // cloning not assumed
        osg::Object * cloneType()                           const { return NULL; }
        osg::Object * clone    (const osg::CopyOp & copyop) const { return NULL; }

        // META_Node realization
        bool         isSameKindAs(const osg::Object * obj) const { return static_cast<const NavigationalLight *>(obj) != NULL; }
        const char * className   ()                        const { return "NavigationalLight"; }
        const char * libraryName ()                        const { return "avScene"; }

        // tricky accept method
        // yes, we have callbacks, but we don't want to make redundant work on pushing/popping and so on
        // so, we make accept() and call appropriate callbacks by ourselves
        void accept(osg::NodeVisitor & nv);

    private:

        // update callback
        void update(osg::NodeVisitor * nv);
        // cull callback
        void cull(osg::NodeVisitor * nv);

    private:

        // exhibition condition
        LightExhibitionCondition m_exhCondition;

        // enabled state
        bool m_bEnabled;

        // base info
        osg::Vec3f  m_vLocalPos;
        float       m_fSize;
        float       m_fVisDist, m_fVisDistMaxSqr;
        osg::Vec4ub m_cColor;

        // sectoring data
        float m_fSectorStart;
        float m_fSectorEnd;
        float m_fSectorFadeInv;

        // blinking data
        std::vector<osg::Vec4f> m_aBlinkSequenceData;
        float                   m_fTimeDelta;
        float                   m_fSequenceDuration;

        // saved blink alpha
        float m_fBlinkAlpha;

        // cached frame number
        int m_nCachedFrameNumber;
    };

}
