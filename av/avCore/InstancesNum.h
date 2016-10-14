#pragma once


namespace osg {

#define INSTANCES_NUM_OBJECT (osg::StateAttribute::FRAME_BUFFER_OBJECT + 500)

class /*OSG_EXPORT*/ InstancesNum : public StateAttribute
{
    public :

        InstancesNum();

        InstancesNum(osg::Geode* ig, uint32_t num);

        /** Copy constructor using CopyOp to manage deep vs shallow copy. */
        InstancesNum(const InstancesNum& trans,const CopyOp& copyop=CopyOp::SHALLOW_COPY)
            : StateAttribute(trans,copyop)
            , _num(trans._num)
            {}

        META_StateAttribute(osg, InstancesNum, static_cast<osg::StateAttribute::Type>(INSTANCES_NUM_OBJECT) );

        /** Return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs. */
        virtual int compare(const StateAttribute& sa) const
        {
            // Check for equal types, then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(InstancesNum,sa)

            // Compare each parameter in turn against the rhs.
            COMPARE_StateAttribute_Parameter(_num)

            return 0; // Passed all the above comparison macros, so must be equal.
        }

        virtual bool getModeUsage(StateAttribute::ModeUsage& usage) const
        {
            // usage.usesMode(GL_BLEND);
            return true;
        }

        void setNum (uint32_t num) {_num = num;}
        uint32_t getNum () const { return _num;}


        virtual void apply(State& state) const;

    protected :

        virtual ~InstancesNum();


        uint32_t          _num;
        osg::Geode* _instGeode;
};

}


