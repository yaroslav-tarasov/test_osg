#pragma once

namespace Utils
{


#if 0
    template <class T>
    class NodeCallback : public osg::NodeCallback
    {
    public:

        NodeCallback( T * object, void (T::*func)( osg::NodeVisitor * nv ), bool isPure = false )
            : _object(object)
            , _func(func)
            , _isPureCallback(isPure)
        {
        }

        NodeCallback( const NodeCallback & other, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY )
            : osg::NodeCallback(other, copyop)
            , _object(other._object)
            , _func(other._func)
            , _isPureCallback(other._isPureCallback)
        {
        }

        virtual void operator()( osg::Node * node, osg::NodeVisitor * nv )
        {
            (_object->*_func)(nv);

            if (!_isPureCallback)
                osg::NodeCallback::operator()(_object, nv);
        }

    private:

        T * _object;
        void (T::*_func)( osg::NodeVisitor * nv );
        bool _isPureCallback;
    };

    template<class T>
    inline NodeCallback<T> * makeNodeCallback( T * object, void (T::*func)( osg::NodeVisitor * nv ), bool isPure = false )
    {
        return new NodeCallback<T>(object, func, isPure);
    }
#endif


    template <class T>
    class NodeCallback : public osg::NodeCallback
    {
    public:
        typedef std::function<void(osg::NodeVisitor * nv)> callback_f;
    public:

        NodeCallback( T * object, callback_f func, bool isPure = false )
            : _object(object)
            , _func(func)
            , _isPureCallback(isPure)
        {
        }

        NodeCallback( const NodeCallback & other, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY )
            : osg::NodeCallback(other, copyop)
            , _object(other._object)
            , _func(other._func)
            , _isPureCallback(other._isPureCallback)
        {
        }

        virtual void operator()( osg::Node * node, osg::NodeVisitor * nv )
        {
            _func(nv);

            if (!_isPureCallback)
                osg::NodeCallback::operator()(_object, nv);
        }

    private:

        T * _object;
        callback_f _func;
        bool _isPureCallback;
    };

    template<class T>
    inline NodeCallback<T> * makeNodeCallback( T * object, typename NodeCallback<T>::callback_f func, bool isPure = false )
    {
        return new NodeCallback<T>(object, func, isPure);
    }

    template<class T>
    inline NodeCallback<T> * makeNodeCallback( T * object, void (T::*func)( osg::NodeVisitor * nv ), bool isPure = false )
    {
        return new NodeCallback<T>(object, /*func*/std::bind(func,object,sp::_1), isPure);
    }

    template<class T, class U>
    inline NodeCallback<T> * makeNodeCallback( T * object, U * callback_owner, void (U::*func)( osg::NodeVisitor * nv ), bool isPure = false )
    {
        return new NodeCallback<T>(object, /*func*/std::bind(func,callback_owner,sp::_1), isPure);
    }
}