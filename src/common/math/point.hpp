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

    //
    // Given a pair of points and velocities point_a/velocity_a and point_b/velocity_b, and we assume each point moves along the vector of velocity
    // over the interval t from 0..1, we get the following equations:
    // ```  pos_a(t) = velocity_a * t + point_a
    //      pos_b(t) = velocity_b * t + point_b ```
    //
    // We can then find the square of the distance between these two points with respect to t using the Pythagorean theorem:
    // ```  dist(t) = sum_components( (pos_b - pos_a) ^ 2 ) ```
    //
    // We now have a sum of quadratic equations. We can find the point where the points come closest by finding the inflection point of the curve,
    // which is where the first derivative is zero. The derivative is:
    // ```  ddist(t)/dt = sum_components( 2 * (velocity_b - velocity_a) * (point_b - point_a + velocity_b * t - velocity_a * t) )   ```
    //
    // Now to find the inflection point, we set this derivative to zero and solve for t. At this point I just plugged it into WolframAlpha
    // and got the following:
    //
    // ```  sum_components( velocity_b * (point_a - point_b) + velocity_a * (point_b - point_a) )
    // t = -------------------------------------------------------------------------------------------
    //                      sum_components( (velocity_b - velocity_a) ^ 2 )                             ```
    //
    // With the restriction that `sum_components( velocity_a ^ 2 + velocity_b ^ 2 )` is not equal to `2 * sum_components( velocity_a * velocity_b )`
    // In which case, the trajectories may be considered parallel.
    //

    namespace Detail{
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
    /// Given point_a and point_b traveling at velocity_a and velocity_b respectively, find the time at which the points come closest to each other.
    template<class T, size_t dimensions>
    T PointFindPassTime( const Point<T, dimensions> & point_a, const Point<T, dimensions> & point_b,
                         const Point<T, dimensions> & velocity_a, const Point<T, dimensions> & velocity_b ){
        if( Detail::PointFindPass<T, dimensions, dimensions>::CheckL( point_a, point_b, velocity_a, velocity_b ) ==
            Detail::PointFindPass<T, dimensions, dimensions>::CheckR( point_a, point_b, velocity_a, velocity_b ) ){
            return T();
        }
        return  Detail::PointFindPass<T, dimensions, dimensions>::Numerator( point_a, point_b, velocity_a, velocity_b ) /
                Detail::PointFindPass<T, dimensions, dimensions>::Denom( point_a, point_b, velocity_a, velocity_b );
    }
}


#endif
