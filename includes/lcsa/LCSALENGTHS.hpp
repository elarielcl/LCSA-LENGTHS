
#ifndef LCSA_LCSALENGTHS_HPP
#define LCSA_LCSALENGTHS_HPP

/***
BSD 2-Clause License

Copyright (c) 2018, Adrián
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include "repair.hpp"

using namespace util_repair;

namespace LCSALENGTHS {


    class LCSALENGTHS {

    public:
        typedef uint64_t size_type;
        typedef uint32_t value_type;

    private:
        size_type m_sample;
        sdsl::int_vector<> m_sampling_vector;
        sdsl::int_vector<> m_lengths_vector;
        sdsl::sd_vector<> m_L;
        sdsl::rank_support_sd<1> m_rank_support;
        sdsl::select_support_sd<1> m_select_support;
        size_type m_len_c;
        size_type m_len_r;
        uint m_alpha;
        size_type m_size;
        sdsl::int_vector<> c_vector;
        sdsl::int_vector<> r_vector_left;
        sdsl::int_vector<> r_vector_right;
        //sdsl::byte_alphabet::char2comp_type m_char2comp;
        //sdsl::byte_alphabet::sigma_type m_sigma;

        void copy(const LCSALENGTHS& v)
        {

            c_vector = v.c_vector;
            r_vector_left = v.r_vector_left;
            r_vector_right = v.r_vector_right;
            m_translate = v.m_translate;
            m_translate_rank_support = v.m_translate_rank_support;
            m_translate_rank_support.set_vector(&m_translate);
            m_translate_select_support = v.m_translate_select_support;
            m_translate_select_support.set_vector(&m_translate);

            m_sample= v.m_sample;
            m_sampling_vector= v.m_sampling_vector;
            m_lengths_vector = v.m_lengths_vector;
            m_L = v.m_L;
            m_rank_support = v.m_rank_support;
            m_rank_support.set_vector(&m_L);
            m_select_support = v.m_select_support;
            m_select_support.set_vector(&m_L);
            m_len_c = v.m_len_c;
            m_alpha = v.m_alpha;
            m_size = v.m_size;
            //m_char2comp = v.m_char2comp;
            //m_sigma = v.m_sigma;
        }

        int translate(const int value_to_translate, const int &last, const std::vector<int> &keys,
                      const repair &repair){
            if(value_to_translate >= repair.alpha){
                return last + (value_to_translate-repair.alpha+1);
            }else{
                return keys[value_to_translate];
            }
        }

        uint encode(int x) const {
            bool neg = x < 0;
            if (neg) x = -x;
            return (x<<1) + ((neg)?0:1);
        }


        int decode(uint x) const {
            bool pos = x%2;
            return ((pos)?1:-1) * (x>>1);
        }

    public:

        sdsl::sd_vector<> m_translate;
        sdsl::rank_support_sd<1> m_translate_rank_support;
        sdsl::select_support_sd<1> m_translate_select_support;
        //sdsl::byte_alphabet::char2comp_type &char2comp = m_char2comp;
        //sdsl::byte_alphabet::sigma_type &sigma = m_sigma;

        LCSALENGTHS(){};

        LCSALENGTHS(const LCSALENGTHS& v)
        {
            copy(v);
        }
        //! Move constructor
        LCSALENGTHS(LCSALENGTHS&& v)
        {
            *this = std::move(v);
        }

        LCSALENGTHS(std::ifstream& in) {
            in.read((char *) &m_sample, sizeof(size_type));
            in.read((char *) &m_len_c, sizeof(size_type));
            in.read((char *) &m_len_r, sizeof(size_type));
            in.read((char *) &m_alpha, sizeof(uint));
            in.read((char *) &m_size, sizeof(size_type));
            m_L.load(in);
            m_sampling_vector.load(in);
            m_lengths_vector.load(in);
            sdsl::util::init_support(m_rank_support, &m_L);
            sdsl::util::init_support(m_select_support, &m_L);
            m_translate.load(in);
            sdsl::util::init_support(m_translate_rank_support, &m_translate);
            sdsl::util::init_support(m_translate_select_support, &m_translate);
            c_vector.load(in);
            r_vector_left.load(in);
            r_vector_right.load(in);
        }

        template<class t_v>
        LCSALENGTHS(t_v &v, const size_type sample = 64, bool delete_v = false, bool repair_precomputed = false,
                                  size_type max_mb = 30720) {

            m_size = v.size();
            //sdsl::byte_alphabet::sigma_type m_sigma = v.sigma;
            //sdsl::byte_alphabet::char2comp_type m_char2comp = v.char2comp;
            m_sample = sample;
            repair m_repair;
            sdsl::int_vector<> sampling_vector(m_size / m_sample + 1, 0);
            sampling_vector[0] = v[0];
            //std::cout << "Repair precomputed: " << repair_precomputed << std::endl;
            if(repair_precomputed){
                for (size_type i = m_sample; i < m_size; i= i+m_sample) {
                    if (i % m_sample == 0) {
                        sampling_vector[i / m_sample] = v[i];
                    }
                }
                m_repair.run();
            }else{
                std::map<int, int> map_terminals;
                map_terminals.insert(std::pair<int, int>(0, 1));
                int* m_diffs = new int[m_size]();
                m_diffs[0] = 0;
                for (size_type i = 1; i < m_size; i++) {
                    int diff_i = v[i] - v[i-1];
                    m_diffs[i] = diff_i;
                    if(map_terminals.find(diff_i) == map_terminals.end()){
                        map_terminals.insert(std::pair<int, int>(diff_i, 1));
                    }
                    if (i % sample == 0) {
                        sampling_vector[i / m_sample] = v[i];
                    }
                }
                m_repair.run(m_diffs,  m_size, map_terminals, max_mb);
            }

            // if(delete_v) sdsl::util::clear(v);
            m_translate = m_repair.m_translate;
            sdsl::util::init_support(m_translate_rank_support, &m_translate);
            sdsl::util::init_support(m_translate_select_support, &m_translate);

            sdsl::util::assign(m_sampling_vector, sampling_vector);
            sdsl::int_vector<> ones = m_repair.expand();
            m_L = sdsl::sd_vector<>(ones.begin(), ones.end());
            m_len_c = m_repair.lenC;
            m_len_r = m_repair.lenR;
            m_alpha = m_repair.alpha;
            sdsl::util::init_support(m_rank_support, &m_L);
            sdsl::util::init_support(m_select_support, &m_L);

            sdsl::int_vector<> c_vector(m_len_c);
            sdsl::int_vector<> r_vector_left(m_len_r);
            sdsl::int_vector<> r_vector_right(m_len_r);
            for (int i = 0; i < m_len_c; ++i)
                c_vector[i] = m_repair.c[i];

            for (int i = 0; i < m_len_r; ++i) {
                r_vector_left[i] = m_repair.rules[i].left;
                r_vector_right[i] = m_repair.rules[i].right;
            }
            sdsl::util::assign(this->c_vector, c_vector);
            sdsl::util::assign(this->r_vector_left, r_vector_left);
            sdsl::util::assign(this->r_vector_right, r_vector_right);

            m_lengths_vector = sdsl::int_vector<>(m_len_r); // One per rule
            for (int i = 0; i < m_len_r; ++i) {
                m_lengths_vector[i] = m_repair.expand(i+m_alpha); // It returns the length of the expanded rule //Could be improved using memoization
            }
            sdsl::util::bit_compress(m_lengths_vector);
            sdsl::util::bit_compress(m_sampling_vector);
            sdsl::util::bit_compress(this->c_vector);
            sdsl::util::bit_compress(this->r_vector_left);
            sdsl::util::bit_compress(this->r_vector_right);

        }


        LCSALENGTHS& operator=(const LCSALENGTHS& v)
        {
            if (this != &v) {
                copy(v);
            }
            return *this;
        }

        //! Swap method
        void swap(LCSALENGTHS& v)
        {
            if (this != &v) {
                std::swap(c_vector, v.c_vector);
                std::swap(r_vector_right, v.r_vector_right);
                std::swap(r_vector_left, v.r_vector_left);
                std::swap(m_translate, v.m_translate);
                m_translate_rank_support.swap(v.m_translate_rank_support);
                m_translate_rank_support.set_vector(&m_translate);
                m_translate_select_support.swap(v.m_translate_select_support);
                m_translate_select_support.set_vector(&m_translate);

                std::swap(m_sample, v.m_sample);
                std::swap(m_sampling_vector, v.m_sampling_vector);
                std::swap(m_lengths_vector, v.m_lengths_vector);
                std::swap(m_L, v.m_L);
                m_rank_support.swap(v.m_rank_support);
                m_rank_support.set_vector(&m_L);
                m_select_support.swap(v.m_select_support);
                m_select_support.set_vector(&m_L);
                std::swap(m_len_c, v.m_len_c);
                std::swap(m_alpha, v.m_alpha);
                std::swap(m_size, v.m_size);
//                std::swap(m_char2comp, v.m_char2comp);
//                std::swap(m_sigma, v.m_sigma);
            }
        }

        LCSALENGTHS& operator=(LCSALENGTHS&& v)
        {
            if (this != &v) {
                m_translate = std::move(v.m_translate);
                m_translate_rank_support = std::move(m_translate_rank_support);
                m_translate_rank_support.set_vector(&m_translate);
                m_translate_select_support = std::move(m_translate_select_support);
                m_translate_select_support.set_vector(&m_translate);
                c_vector = std::move(v.c_vector);
                r_vector_left = std::move(v.r_vector_left);
                r_vector_right = std::move(v.r_vector_right);

                m_sample = std::move(v.m_sample);
                m_sampling_vector = std::move(v.m_sampling_vector);
                m_lengths_vector = std::move(v.m_lengths_vector);
                m_L = std::move(v.m_L);
                m_rank_support = std::move(v.m_rank_support);
                m_rank_support.set_vector(&m_L);
                m_select_support = std::move(v.m_select_support);
                m_select_support.set_vector(&m_L);
                m_len_c = v.m_len_c;
                m_alpha = std::move(v.m_alpha);
                m_size = std::move(v.m_size);
                //m_char2comp = std::move(v.m_char2comp);
                //m_sigma = std::move( v.m_sigma);
            }
            return *this;
        }


        void expand_left(long value, size_t &v, long &actual, size_type start, size_type end) const {
            //if(actual > end) return;
            while (value >= m_alpha) {
                int left_length = (r_vector_left[value - m_alpha] >= (int)m_alpha) ? (m_lengths_vector[r_vector_left[value - m_alpha] - m_alpha]) : (1);
                if (actual + left_length > start) expand_left(r_vector_left[value - m_alpha], v, actual, start, end);
                else actual += left_length;
                if(actual > end) return;
                value = r_vector_right[value - m_alpha];
            }
            if (start < actual && actual <= end) {
                v += decode(m_translate_select_support(value+1));
            }
            actual++;
        }

        void expand_right(long value, size_t &v, long &actual, size_type start, size_type end) const{
            //if(start < actual) return;
            while (value >= m_alpha) {
                int right_length = (r_vector_right[value - m_alpha] >= (int)m_alpha) ? (m_lengths_vector[r_vector_right[value - m_alpha] - m_alpha]) : (1);
                if (!(actual - right_length <  end))
                    actual -= right_length;
                else
                    expand_right(r_vector_right[value - m_alpha], v, actual, start, end);


                if(actual < start) return;
                value = r_vector_left[value - m_alpha];
            }
            if (start < actual && actual <= end) {
                uint translated = m_translate_select_support(value+1);
                v -= decode(translated);
            }
            actual--;
        }

        void expand_interval_left(size_type i, size_type j, long &actual, size_type start, size_type end,
                                  size_t &value) const {
            for (size_type p = i; p <= j; p++) {
                expand_left(c_vector[p], value, actual, start, end);
            }
        }


        void expand_left(long rule, size_t &v, long &actual, size_type sampling_pos, size_type start, size_type end,
                         std::vector<size_type> &interval) const {
            //if(actual > start) return;
            while (rule >= m_alpha) {
                int left_length = (r_vector_left[rule - m_alpha] >= (int)m_alpha) ? (m_lengths_vector[r_vector_left[rule - m_alpha] - m_alpha]) : (1);
                if (actual + left_length > sampling_pos) expand_left(r_vector_left[rule - m_alpha], v, actual, sampling_pos, start, end, interval);
                else actual += left_length;
                if(actual > end) return;
                rule = r_vector_right[rule - m_alpha];
            }

            if (sampling_pos < actual && actual <= end) {
                v += decode(m_translate_select_support(rule+1));
                if (actual >= start) {
                    interval.push_back(v);
                }
            }
            actual++;
        }


        void expand_right(long rule, size_t &v, long &actual, size_type sampling_pos, size_type start, size_type end,
                          std::vector<size_type> &interval) const{
            //if(sampling_pos < actual) return;
            while (rule >= m_alpha) {
                int right_length = (r_vector_right[rule - m_alpha] >= (int)m_alpha) ? (m_lengths_vector[r_vector_right[rule - m_alpha] - m_alpha]) : (1);
                if (!(actual - right_length <  sampling_pos))
                    actual -= right_length;
                else
                    expand_right(r_vector_right[rule - m_alpha], v, actual, sampling_pos, start, end, interval);


                if(actual < start) return;
                rule = r_vector_left[rule - m_alpha];
            }
            if (start < actual && actual <= sampling_pos) {
                uint translated = m_translate_select_support(rule+1);
                v -= decode(translated);
                if (actual <= end+1) {
                    interval.push_back(v);
                }
            }
            actual--;
        }


        void expand_interval_right(size_type i, size_type j, long &actual, size_type start, size_type end,
                                   size_t &value) const{
            for (size_type p = j; p >= i; p--) {
                expand_right(c_vector[p], value, actual, start, end);
            }
        }

        void expand_interval_right(size_type i, size_type j, long &actual, size_type start, size_type end ,
                                   size_type sampling_pos, size_t &value, std::vector<size_type> &interval) const{
            for (size_type p = j; p >= i; p--) {
                expand_right(c_vector[p], value, actual, sampling_pos, start, end, interval);
            }
        }


        void expand_interval_left(size_type i, size_type j, long &actual, size_type sampling_pos, size_type start ,
                                  size_type end, size_t &value, std::vector<size_type> &interval) const {
            for (size_type p = i; p <= j; p++) {
                expand_left(c_vector[p], value, actual, sampling_pos, start, end, interval);
            }
        }






        std::vector<size_type> extract(size_type i, size_type j) const{

            size_type inti1, inti2, intj1, intj2, minint;
            std::vector<size_type> interval;



            //calculate i' and j'
            size_type i1 = (i / m_sample) * m_sample;
            size_type i2 = (i / m_sample+1) * m_sample;

            size_type j1 = (j / m_sample) * m_sample;
            size_type j2 = (j / m_sample+1) * m_sample;

            inti1 = i - i1;
            inti2 = i2 - i;
            intj1 = j - j1;
            intj2 = j2 - j;
            if (j2 >= m_size) intj2 = 1 + fmax(intj1, fmax(inti1 ,inti2));

            minint = fmin(inti1, fmin(inti2, fmin(intj1, intj2)));


            //interval i'-j
            if(minint == intj2){
                size_type c_start = m_rank_support(i + 1);//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                size_type c_end = m_rank_support(j2 + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_end = m_select_support(c_end+1)-1;
                //std::cout << "p-start " << p_start << std::endl;
                size_type value = m_sampling_vector[j / m_sample+1];
                //std::vector<long > diffs;

                if (j == j2) interval.push_back(value);

                expand_interval_right(c_start, c_end, p_end, i, j, j2, value, interval);
                /*
                for (uint32_t p_diff = 0; p_diff < diffs.size()-1; p_diff++) {
                    value -=  diffs[p_diff];
                }
                 */
                std::reverse(std::begin(interval), std::end(interval));
                return interval;

            } else if (minint == inti1) {
                size_type c_start = m_rank_support(i1 + 1);//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                size_type c_end = m_rank_support(j + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_start = 0;
                if (c_start > 0) p_start = m_select_support(c_start);
                //std::cout << "p-start " << p_start << std::endl;
                size_type value = m_sampling_vector[i / m_sample];


                if (i == i1) interval.push_back(value);

                expand_interval_left(c_start, c_end, p_start, i1, i, j, value, interval);
                return interval;
            } else if (minint == intj1) {

                size_type c_start = m_rank_support(i + 1);//non-inclusive rank
                size_type c_end = m_rank_support(j1 + 1);;


                //std::cout << "c-end: " << c_end << std::endl;
                long p_end = m_select_support(c_end+1)-1;
                //std::cout << "p-start " << p_start << std::endl;
                //std::vector<long > diffs;
                size_type value = m_sampling_vector[j1/m_sample];


                expand_interval_right(c_start, c_end, p_end, i, j1, j1, value, interval);
                /*
                for (uint32_t p_diff = 0; p_diff < diffs.size()-1; p_diff++) {
                    value -=  diffs[p_diff];
                }
                 */
                std::reverse(std::begin(interval), std::end(interval));

                c_start = c_end;
                //std::cout << "c-start: " << c_start << std::endl;
                c_end = m_rank_support(j + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_start = 0;
                if (c_start > 0) p_start = m_select_support(c_start);
                //std::cout << "p-start " << p_start << std::endl;
                value = m_sampling_vector[j1 / m_sample];

                interval.push_back(value);
                expand_interval_left(c_start, c_end, p_start, j1, j1, j, value, interval);
                return interval;

            } else {

                size_type c_start = m_rank_support(i + 1);//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                size_type c_end = m_rank_support(i2 + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_end = m_select_support(c_end+1)-1;
                //std::cout << "p-start " << p_start << std::endl;
                size_type value = m_sampling_vector[i2 / m_sample];
                //std::vector<long > diffs;


                expand_interval_right(c_start, c_end, p_end, i, i2, i2, value, interval);
                /*
                for (uint32_t p_diff = 0; p_diff < diffs.size()-1; p_diff++) {
                    value -=  diffs[p_diff];
                }
                 */
                std::reverse(std::begin(interval), std::end(interval));



                c_start = c_end;//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                c_end = m_rank_support(j + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_start = 0;
                if (c_start > 0) p_start = m_select_support(c_start);
                //std::cout << "p-start " << p_start << std::endl;
                value = m_sampling_vector[i2 / m_sample];

                interval.push_back(value);

                expand_interval_left(c_start, c_end, p_start, i2, i2, j, value, interval);
                return interval;

            }


        }


        size_type decompress_value(size_type i) const{
            //calculate i' and j'
            size_type i1 = (i / m_sample) * m_sample;
            size_type j1 = (i / m_sample+1) * m_sample;
            //interval i'-j
            if(j1 >= m_size || i-i1 < j1-i){
                size_type c_start = m_rank_support(i1 + 1);//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                size_type c_end = m_rank_support(i + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_start = 0;
                if (c_start > 0) p_start = m_select_support(c_start);
                //std::cout << "p-start " << p_start << std::endl;
                size_type value = m_sampling_vector[i / m_sample];
                expand_interval_left(c_start, c_end, p_start, i1, i, value);
                return value;
            }else{
                size_type c_start = m_rank_support(i + 1);//non-inclusive rank
                //std::cout << "c-start: " << c_start << std::endl;
                size_type c_end = m_rank_support(j1 + 1);
                //std::cout << "c-end: " << c_end << std::endl;
                long p_end = m_select_support(c_end+1)-1;
                //std::cout << "p-start " << p_start << std::endl;
                size_type value = m_sampling_vector[i / m_sample+1];
                //std::vector<long > diffs;
                expand_interval_right(c_start, c_end, p_end, i, j1, value);
                /*
                for (uint32_t p_diff = 0; p_diff < diffs.size()-1; p_diff++) {
                    value -=  diffs[p_diff];
                }
                 */
                return value;
            }

        }

        inline size_type operator[](const size_type &i) const {
            return decompress_value(i);
        }

        size_type size() const{
            return m_size;
        }

        void print() {
            std::cout << "Sampling" << std::endl;
            for (uint i = 0; i < m_sampling_vector.size(); i++) {
                std::cout << m_sampling_vector[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "L" << std::endl;
            for(uint i = 0; i < m_L.size(); i++){
                std::cout << m_L[i] << " ";
            }
            std::cout << std::endl;
        }

        //PENDING, check
        size_type size_in_bytes() {
            size_type size = 0;
            size += sdsl::size_in_bytes(m_L);
            size += sdsl::size_in_bytes(m_sampling_vector);
            size += sdsl::size_in_bytes(m_lengths_vector);
            size += sdsl::size_in_bytes(m_rank_support);
            size += sdsl::size_in_bytes(m_select_support);
            size += sdsl::size_in_bytes(m_translate);
            size += sdsl::size_in_bytes(m_translate_rank_support);
            size += sdsl::size_in_bytes(m_translate_select_support);

            size += sdsl::size_in_bytes(c_vector); // Podria ser bitcomprimido
            size += sdsl::size_in_bytes(r_vector_left);
            size += sdsl::size_in_bytes(r_vector_right);
            size += sizeof(int) * 2;
            return size;
        }

        /*size_type serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr, std::string name="")const
        {
            sdsl::structure_tree_node* child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_sa.serialize(out, child, "m_sa");
            written_bytes += m_isa.serialize(out, child, "m_isa");
            written_bytes += m_alphabet.serialize(out, child, "m_alphabet");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }*/

        void serialize(std::ostream& out) {
            out.write((char *) &m_sample, sizeof(size_type));
            out.write((char *) &m_len_c, sizeof(size_type));
            out.write((char *) &m_len_r, sizeof(size_type));
            out.write((char *) &m_alpha, sizeof(uint));
            out.write((char *) &m_size, sizeof(size_type));
            m_L.serialize(out);
            m_sampling_vector.serialize(out);
            m_lengths_vector.serialize(out);
            m_translate.serialize(out);
            c_vector.serialize(out);
            r_vector_left.serialize(out);
            r_vector_right.serialize(out);
        }

    };
}

#endif //LCSA_LCSALENGTHS_HPP
