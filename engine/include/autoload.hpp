#ifndef AUTOLOAD_H
#define AUTOLOAD_H

namespace lum
{
    class Autoload
    {
    public:
        bool loaded{};

    public:
        Autoload();
        virtual ~Autoload();

        virtual void Setup() = 0;
        virtual void Update(float p_delta) = 0;
    };
}

#endif // !AUTOLOAD_H
