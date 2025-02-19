#ifndef SCENE_H
#define SCENE_H

namespace lum
{
    class Scene
    {
    public:
        bool loaded{};

    public:
        Scene();
        virtual ~Scene();

        virtual void Setup() = 0;
        virtual void Input() = 0;
        virtual void Update(float p_delta) = 0;
        virtual void DebugDraw();
    };
}

#endif // !SCENE_H
