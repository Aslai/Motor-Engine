#ifndef MOTOR_H_MATH_POINT_CMP_HPP
#define MOTOR_H_MATH_POINT_CMP_HPP

#include <cstddef>

namespace Motor{
    template<class T, size_t dimensions>
    class Point {
        T data[dimensions];
    public:
        template<class ... Args>
        Point(Args... values) :
            data{ ((T)values) ...} {
            static_assert( (sizeof...(Args) == dimensions), "Number of arguments provided does not match dimensionality of point object." );
        }
        T & operator[](size_t idx){
            return data[idx];
        }
        template<size_t idx>
        T & get() {
            static_assert( (idx < dimensions), "Out of bounds access" );
            return data[idx];
        }
        template<size_t idx>
        const T & at() const {
            static_assert( (idx < dimensions), "Out of bounds access" );
            return data[idx];
        }
        const T & at(size_t idx) const {
            return data[idx];
        }
        T & x(){
            return get<0>();
        }
        T & y(){
            return get<1>();
        }
        T & z(){
            return get<2>();
        }
        T & w(){
            return get<3>();
        }
        Point<T, 2> xy() const {
            return Point<T, 2>(x(), y());
        }
        Point<T, 3> xyz() const {
            return Point<T, 3>(x(), y(), z());
        }
    };

    template<class T, class... Args>
    Point<T, sizeof...(Args) + 1> MakePoint(T value_a, Args... rest ){
        return Point<T, sizeof...(Args) + 1>(value_a, rest...);
    }

    namespace {
        template<class T, size_t dimensions, size_t offset>
        struct PointFindPass{
            static T CheckL (   const Motor::Point<T, dimensions> & point_a, const Motor::Point<T, dimensions> & point_b,
                                const Motor::Point<T, dimensions> & velocity_a, const Motor::Point<T, dimensions> & velocity_b ){
                return  velocity_a.template at<offset-1>() * velocity_a.template at<offset-1>() +
                        velocity_b.template at<offset-1>() * velocity_b.template at<offset-1>() +
                        PointFindPass<T, dimensions, offset - 1>::CheckL(point_a, point_b, velocity_a, velocity_b);
            }

            static T CheckR (   const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return  2 * velocity_a.template at<offset-1>() * velocity_b.template at<offset-1>() +
                        PointFindPass<T, dimensions, offset - 1>::CheckR(point_a, point_b, velocity_a, velocity_b);
            }

            static T Numerator( const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return  velocity_b.template at<offset-1>() * (point_a.template at<offset-1>() - point_b.template at<offset-1>()) +
                        velocity_a.template at<offset-1>() * (point_b.template at<offset-1>() - point_a.template at<offset-1>()) +
                        PointFindPass<T, dimensions, offset - 1>::Numerator(point_a, point_b, velocity_a, velocity_b);
            }

            static T Denom  (   const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return  ( velocity_b.template at<offset-1>() - velocity_a.template at<offset-1>() ) *
                        ( velocity_b.template at<offset-1>() - velocity_a.template at<offset-1>() ) +
                        PointFindPass<T, dimensions, offset - 1>::Denom(point_a, point_b, velocity_a, velocity_b);
            }
        };
        template<class T, size_t dimensions>
        struct PointFindPass<T, dimensions, 0>{
            static T CheckL (   const Motor::Point<T, dimensions> & point_a, const Motor::Point<T, dimensions> & point_b,
                                const Motor::Point<T, dimensions> & velocity_a, const Motor::Point<T, dimensions> & velocity_b ){
                return 0;
            }

            static T CheckR (   const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return 0;
            }

            static T Numerator( const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return 0;
            }

            static T Denom  (   const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                                const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
                return 0;
            }
        };
    }
    template<class T, size_t dimensions>
    T PointFindPassTime( const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                         const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
        if( PointFindPass<T, dimensions, dimensions>::CheckL( point_a, point_b, velocity_a, velocity_b ) ==
            PointFindPass<T, dimensions, dimensions>::CheckR( point_a, point_b, velocity_a, velocity_b ) ){
            return T();
        }
        return  PointFindPass<T, dimensions, dimensions>::Numerator( point_a, point_b, velocity_a, velocity_b ) /
                PointFindPass<T, dimensions, dimensions>::Denom( point_a, point_b, velocity_a, velocity_b );
    }
}


#endif
