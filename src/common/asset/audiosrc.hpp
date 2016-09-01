#ifndef MOTOR_H_ASSET_AUDIOSRC_HPP
#define MOTOR_H_ASSET_AUDIOSRC_HPP

#include <xmmintrin.h>
#include <cmath>
#include "math/sse.hpp"
#define _mm_pow_ps(x, y) exp_ps( _mm_mul_ps(y, log_ps( x )))

namespace Motor{
    class AudioBuffer{
        friend class AudioMixer;
        std::vector<__m128> samples;
        size_t ptr;
        bool destructive_advance;
    protected:
    public:
        AudioBuffer(){
            ptr = 0;
        }
        void Clear() {
            return samples.clear();
        }
        virtual size_t Size() const {
            return samples.size() * 4 - ptr;
        }
        const float * Data() const {
            return reinterpret_cast<const float*>(samples.data() + ptr);
        }
        virtual void Advance( size_t sample_num ){
            ptr += sample_num / 4;
            if( ptr > Size() ){
                ptr = Size();
            }
        }
        virtual void Rewind(){
            ptr = 0;
        }
        __m128 & operator [](size_t idx){
            return samples[idx];
        }
        void Push( float * samples_in, size_t length ){
            for( size_t i = 0; i + 3 < length; i += 4 ){
                samples.push_back(_mm_load_ps(samples_in + i));
            }
        }
        void Print( ){
            const float * data = Data();
            for( size_t i = 0; i < Size(); ++i ){
                if( i % 4 == 0 ){
                    printf("\n");
                }
                printf("%f\t", data[i]);
            }
        }
    };

    //template<class T>
    class AudioMixer{
    protected:
        static void Clamp( const AudioBuffer & dst, const AudioBuffer & src, size_t & amount ){
            amount = amount > dst.Size() ? dst.Size() : amount;
            amount = amount > src.Size() ? src.Size() : amount;
        }
        static void Add( AudioBuffer & dst, AudioBuffer & src, size_t & amount ){
            Clamp( dst, src, amount );
            for( size_t i = 0; i < amount / 4; ++i ){
                dst[i] = _mm_add_ps( dst[i], src[i] );
            }
            src.Advance( amount );
        }
    public:
        virtual void Mix( AudioBuffer & dst, AudioBuffer & src, size_t amount ){
            Add( dst, src, amount );
        }
        virtual void Mix( AudioBuffer & dst ){

        }
    };

    template<size_t ratio_num, size_t ratio_denom, size_t threshold_num, size_t threshold_denom, size_t decay>
    class AudioMixerCompress : public AudioMixer{
        size_t decay_amt;
        __m128 attenuate;
        float attenuatef;
    public:
        AudioMixerCompress(){
            decay_amt = 0;
            attenuate = _mm_set_ps(1,1,1,1);
            attenuatef = 1;
        }
        void Mix( AudioBuffer & dst, AudioBuffer & src, size_t amount ){
            Add( dst, src, amount );
            Mix( dst );
        }
        void Mix( AudioBuffer & dst ){
            const float ratio_recip = (float) ratio_denom / (float) ratio_num;
            const float threshold = (float) threshold_num / (float) threshold_denom;
            const float base = pow( threshold, 1 - ratio_recip );
            const float decay_recip = (float) 1 / (float) (decay * 4);
            const float decay_recip2 = (float) 1 / (float) (decay/5+1);
            //threshold ^ (1 - 1/ratio) * volume^(1/ratio)
            __m128 zero = _mm_set_ps( 0, 0, 0, 0 );
            __m128 one = _mm_set_ps( 1, 1, 1, 1 );
            __m128 cmp = _mm_set_ps( threshold, threshold, threshold, threshold );
            __m128 bse = _mm_set_ps( base, base, base, base );
            __m128 recip = _mm_set_ps( ratio_recip, ratio_recip, ratio_recip, ratio_recip );
            __m128 dec = _mm_set_ps( decay_recip, decay_recip, decay_recip, decay_recip );
            __m128 dec2 = _mm_set_ps( decay_recip2, decay_recip2, decay_recip2, decay_recip2 );
            for( size_t i = 0; i < dst.Size() / 4; ++i ){
                __m128 mask = _mm_cmpgt_ps( dst[i], cmp );
                if( ((uint32_t*)&mask)[0] || ((uint32_t*)&mask)[1] || ((uint32_t*)&mask)[2] || ((uint32_t*)&mask)[3] ){
                    decay_amt = decay;
                }
                if( decay_amt > 0 ){
                    float lerp_s = (float)(decay - decay_amt) * 4;
                    __m128 lerp;

                    if( decay == decay_amt-- ){
                        lerp = zero;
                    }
                    else{
                        lerp = _mm_set_ps( lerp_s , lerp_s - 1, lerp_s - 2, lerp_s - 3 );
                    }

                    lerp = _mm_mul_ps( lerp, dec );
                    mask = _mm_cmpneq_ps( zero, dst[i] );
                    __m128 compressed = _mm_and_ps( mask, _mm_mul_ps( bse, _mm_pow_ps(dst[i], recip) ) );
                    __m128 diff = _mm_mul_ps( _mm_sub_ps( dst[i], compressed ), lerp );
                    dst[i] = _mm_add_ps( compressed, diff );
                    mask = _mm_cmpgt_ps( dst[i], one );
                    if( ((uint32_t*)&mask)[0] || ((uint32_t*)&mask)[1] || ((uint32_t*)&mask)[2] || ((uint32_t*)&mask)[3] ){
                        float amt = ((float*)&dst[i])[0];
                        amt = ((float*)&dst[i])[1] > amt ? ((float*)&dst[i])[1] : amt;
                        amt = ((float*)&dst[i])[2] > amt ? ((float*)&dst[i])[2] : amt;
                        amt = ((float*)&dst[i])[3] > amt ? ((float*)&dst[i])[3] : amt;
                        amt = 1/ amt;
                        attenuate = _mm_set_ps( amt, amt, amt, amt );
                    }
                    dst[i] = _mm_mul_ps( dst[i], attenuate );
                    attenuate = _mm_add_ps( attenuate, _mm_mul_ps(dec2, _mm_sub_ps(one, attenuate) ) );
                }
            }
        }
        void Mix2( AudioBuffer & dst ){
            const float ratio_recip = (float) ratio_denom / (float) ratio_num;
            const float threshold = (float) threshold_num / (float) threshold_denom;
            const float base = pow( threshold, 1 - ratio_recip );
            const float decay_recip = (float) 1 / (float) (decay * 4);
            const float decay_recip2 = (float) 1 / (float) (decay/5+1);
            //threshold ^ (1 - 1/ratio) * volume^(1/ratio)
            const float zero = 0;
            const float one = 1;
            const float cmp = threshold;
            const float bse = base;
            const float recip = ratio_recip;
            const float dec = decay_recip;
            const float dec2 = decay_recip2;
            for( size_t i = 0; i < dst.Size() / 4; ++i ){
                for( int j = 0; j < 4; ++j ){
                    float & d = ((float*)&dst[i])[j];
                    if( d > cmp ){
                        decay_amt = decay;
                    }
                    if( decay_amt > 0 ){
                        const float lerp_s = (float)(decay - decay_amt) * 4;
                        const float lerp = decay == decay_amt-- ? zero : lerp_s * dec;
                        const float compressed = bse * pow( d, recip );
                        const float diff = ( d - compressed ) * lerp;
                        d = compressed + diff;
                        if( d > 1 ){
                            attenuatef = 1 / d;
                        }
                        d = d * attenuatef;
                    }
                }
                attenuatef = attenuatef + dec2 * (one - attenuatef);
            }
        }
    };

    class AudioSrc{
    public:
        AudioSrc();
        //virtual
    };
}

#endif
