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
		size_t dangling;
    protected:
    public:
        AudioBuffer(){
            ptr = 0;
			dangling = 0;
        }
        void Clear() {
            return samples.clear();
        }
        virtual size_t Size() const {
            return samples.size() * 4 - ptr * 4 - dangling * 4;
        }
        const float * Data() const {
            return reinterpret_cast<const float*>(samples.data() + ptr);
        }
        virtual void Advance( size_t sample_num ){
            ptr += sample_num / 4;
			if (ptr + dangling > samples.size()){
                ptr = samples.size() - dangling;
            }
        }
        virtual void Rewind(){
            ptr = 0;
        }
        __m128 & operator [](size_t idx){
            return samples[idx];
		}
		void Push(float * samples_in, size_t length){
			for (size_t i = 0; i + 3 < length; i += 4){
				samples.push_back(_mm_load_ps(samples_in + i));
			}
		}
		void ToProcess(size_t amount){
			dangling = amount;
		}
		template<class T>
		static float normalize(const T & v, decltype(v) lo, decltype(v) hi){
			float value = v;
			float range = hi;
			range -= lo;
			value = value - lo - range * 0.5f;
			value /= range * 0.5f;
			return value;
		}
		template<class T>
		void PushNormalized(const T * samples_in, size_t length, T lo, T hi){
			for (size_t i = 0; i + 3 < length; i += 4){
				samples.push_back(_mm_set_ps(
					normalize(samples_in[i + 3], lo, hi),
					normalize(samples_in[i + 2], lo, hi),
					normalize(samples_in[i + 1], lo, hi),
					normalize(samples_in[i], lo, hi)
					));
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

	static float habsmax_ps(const __m128 & v){
		__m128 absmask = _mm_cmplt_ps(v, _mm_set1_ps(0));
		__m128 absv = _mm_or_ps(_mm_and_ps(absmask, _mm_mul_ps(v, _mm_set1_ps(-1))), _mm_andnot_ps(absmask, v));
		__m128 max1 = _mm_shuffle_ps(absv, absv, _MM_SHUFFLE(0, 0, 3, 2));
		__m128 max2 = _mm_max_ps(absv, max1);
		__m128 max3 = _mm_shuffle_ps(max2, max2, _MM_SHUFFLE(0, 0, 0, 1));
		__m128 max4 = _mm_max_ps(max2, max3);
		return _mm_cvtss_f32(max4);
	}
	static float lerp(const float & a, const float & b, const float & ratio){
		return a + (b - a) * ratio;
	}

    class AudioMixerLimit : public AudioMixer{
        size_t decay_amt;
        __m128 attenuate;
		float attenuatef;
		float target, base;
		size_t window_begin, window_end;
		float threshold, attack, decay;
		size_t iattack, idecay;
    public:
        AudioMixerLimit( float threshold, float attack, float decay ) : threshold(threshold), attack(attack), decay(decay){
            decay_amt = 0;
            attenuate = _mm_set_ps(1,1,1,1);
			attenuatef = 1;
			target = 1;
			base = 1;
			window_begin = window_end = 0;
			iattack = (size_t)round(attack);
			idecay = (size_t)round(decay);
			iattack = iattack - iattack % 4;
			idecay = idecay - idecay % 4;
        }
        void Mix( AudioBuffer & dst, AudioBuffer & src, size_t amount ){
            Add( dst, src, amount );
            Mix( dst );
        }
		void Mix(AudioBuffer & dst){
			for (size_t i = 0; i < dst.Size() / 4 - iattack; ++i){
				__m128 &lookahead = dst[i + iattack];
				float velocity = habsmax_ps(lookahead);
				if (velocity > threshold){
					if (i >= window_end) {
						window_begin = i + iattack;
					}
					window_end = i + iattack;
					float new_target = threshold / velocity;
					if (new_target < target){
						if (i > window_begin){
							window_begin = i + iattack;
							base = target;
						}
						else{
							base = attenuatef;
						}
						target = new_target;
					}

				}
				if (i < window_begin){
					const float dist = (float)((i + iattack - window_begin));
					const float dist0 = dist / attack;
					const float dist1 = (dist + 1.0f / 4.0f) / attack;
					const float dist2 = (dist + 2.0f / 4.0f) / attack;
					const float dist3 = (dist + 3.0f / 4.0f) / attack;
					const float attenuatef0 = base + (target - base) * dist0;
					const float attenuatef1 = base + (target - base) * dist1;
					const float attenuatef2 = base + (target - base) * dist2;
					const float attenuatef3 = base + (target - base) * dist3;
					attenuatef = attenuatef3;
					attenuate = _mm_set_ps(attenuatef3, attenuatef2, attenuatef1, attenuatef0);
				}
				else if (i < window_end){
					attenuatef = target;
					attenuate = _mm_set1_ps(attenuatef);
				}
				else if (window_end + decay > i){
					target = 1;
					float dist = (float)(i - window_end) / decay;
					attenuatef = base + (target - base) * dist;
					attenuate = _mm_set1_ps(attenuatef);
				}
				else{
					attenuatef = base = target = 1;
					attenuate = _mm_set1_ps(attenuatef);
				}
				dst[i] = _mm_mul_ps(dst[i], attenuate);
			}
			dst.ToProcess(iattack);
		}
        void Mix2( AudioBuffer & dst ){

			for (size_t i = 0; i < dst.Size() / 4 - iattack; ++i){
                for( int j = 0; j < 4; ++j ){
					float & lookahead = ((float*)&dst[i + iattack])[j];
					float & d = ((float*)&dst[i])[j];
 
                    if( abs(lookahead) > threshold ){
						if( i >= window_end ) {
							window_begin = i + iattack;
						}
						window_end = i + iattack;
						float new_target = threshold / abs(lookahead);
						if (new_target < target ){
							if (i > window_begin){
								window_begin = i + iattack;
								base = target;
							}
							else{
								base = attenuatef;
							}
							target = new_target;
						}

                    }
					if (i < window_begin){
						float dist = (float)(i + iattack - window_begin) / attack;
						attenuatef = base + (target - base) * dist;
					}
					else if (i < window_end){
						attenuatef = target;
					}
					else if (window_end + idecay > i){
						target = 1;
						float dist = (float)(i - window_end) / decay;
						attenuatef = base + (target - base) * dist;
					}
					else{
						attenuatef = base = target = 1;
					}
					d = d * attenuatef;
                }
			}
			dst.ToProcess(iattack);
        }
	};
	class AudioMixerCompress : public AudioMixer{
		float threshold, attack, decay;
		size_t iattack, idecay;
		float ratio;
		float envelope;
	public:
		AudioMixerCompress(float ratio, float threshold, float attack, float decay) : ratio(ratio), threshold(threshold), attack(attack), decay(decay){
			envelope = 0;
			iattack = (size_t)round(attack);
			idecay = (size_t)round(decay);
			iattack = iattack - iattack % 4;
			idecay = idecay - idecay % 4;
		}
		void Mix(AudioBuffer & dst, AudioBuffer & src, size_t amount){
			Add(dst, src, amount);
			Mix(dst);
		}
		void Mix(AudioBuffer & dst){
			__m128 invratio = _mm_set1_ps( 1.0f / ratio);
			__m128 attenuation_base = _mm_set1_ps(pow(threshold, 1.0f - 1.0f / ratio));
			__m128 attenuation = _mm_set1_ps(1);
			for (size_t i = 0; i < dst.Size() / 4 - iattack; ++i){
				envelope *= 1 / decay; 
				if (1 - envelope > threshold){
					attenuation = _mm_mul_ps(attenuation_base, _mm_pow_ps(_mm_set1_ps(1 - envelope), invratio));
					dst[i] = _mm_mul_ps(dst[i], attenuation);
				}
				float mx = habsmax_ps(dst[i + iattack]);
				if (mx > 1 - envelope){
					envelope = 1 - mx;
				}
			}
			dst.ToProcess(iattack);
		}
		void Mix2(AudioBuffer & dst){
			const float invratio = 1.0f / ratio;
			const float attenuation_base = pow(threshold, 1.0f - invratio);
			for (size_t i = 0; i < dst.Size() / 4 - iattack; ++i){
				envelope *= 1 / decay;
				float attenuation = attenuation_base * pow(1 - envelope, invratio);
				for (int j = 0; j < 4; ++j){
					float & lookahead = ((float*)&dst[i + iattack])[j];
					float & d = ((float*)&dst[i])[j];
					if (1 - envelope > threshold){
						d *= attenuation;
					}
					if (abs(lookahead) > 1 - envelope){
						envelope = 1 - abs(lookahead);
					}
				}
			}
			dst.ToProcess(iattack);
		}
	};
	class AudioMixerAmplify : public AudioMixer{
		float scale;
	public:
		AudioMixerAmplify(float scale) : scale(scale){

		}
		void Mix(AudioBuffer & dst, AudioBuffer & src, size_t amount){
			Add(dst, src, amount);
			Mix(dst);
		}
		void Mix(AudioBuffer & dst){
			__m128 scaling = _mm_set1_ps(scale);
			for (size_t i = 0; i < dst.Size() / 4; ++i){
				dst[i] = _mm_mul_ps(dst[i], scaling);
			}
		}
		void Mix2(AudioBuffer & dst){
			for (size_t i = 0; i < dst.Size() / 4; ++i){
				for (int j = 0; j < 4; ++j){
					float & d = ((float*)&dst[i])[j];
					d *= scale;
				}
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
