#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace numsol {

template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
    requires(N > 0) && std::same_as<typename Allocator::value_type, T>
class vector_storage {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;

private:
    using alloc_traits = std::allocator_traits<Allocator>;

    static constexpr bool nothrow_move = std::is_nothrow_move_constructible_v<T>;

    union RawBuffer {
        value_type inline_buf[N];
        pointer heap_ptr;
        constexpr RawBuffer() noexcept {}
        constexpr ~RawBuffer() noexcept {}
    };

    RawBuffer buf_{};
    pointer data_ = nullptr;
    size_type size_ = 0;
    size_type capacity_ = N;
    [[no_unique_address]] Allocator alloc_{};

    [[nodiscard]] constexpr pointer inline_address() noexcept { return std::launder(reinterpret_cast<pointer>(&buf_)); }
    [[nodiscard]] constexpr bool is_heap() const noexcept { return capacity_ != N; }

    constexpr void dealloc_if_heap() noexcept {
        if (is_heap()) alloc_traits::deallocate(alloc_, buf_.heap_ptr, capacity_);
        data_ = inline_address();
        capacity_ = N;
    }

    constexpr void adopt(vector_storage& other) {
        if (other.is_heap()) {
            buf_.heap_ptr = std::exchange(other.buf_.heap_ptr, nullptr);
            data_ = std::exchange(other.data_, other.inline_address());
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, N);
        } else {
            std::uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
            other.destroy_elements();
        }
    }

public:
    constexpr vector_storage() noexcept(std::is_nothrow_default_constructible_v<Allocator>) : data_(inline_address()) {}

    constexpr explicit vector_storage(const Allocator& alloc) noexcept : data_(inline_address()), alloc_(alloc) {}

    vector_storage(const vector_storage&) = delete;
    vector_storage& operator=(const vector_storage&) = delete;

    constexpr vector_storage(vector_storage&& other) noexcept(nothrow_move && alloc_traits::is_always_equal::value)
        : data_(inline_address()), alloc_(std::move(other.alloc_)) {
        adopt(other);
    }

    constexpr vector_storage(vector_storage&& other,
                             const Allocator& alloc) noexcept(nothrow_move && alloc_traits::is_always_equal::value)
        : data_(inline_address()), alloc_(alloc) {
        if (alloc_ == other.alloc_) {
            adopt(other);
        } else {
            ensure_capacity(other.size_);
            std::uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
            other.destroy_elements();
        }
    }

    constexpr ~vector_storage() {
        destroy_elements();
        dealloc_if_heap();
    }

    constexpr vector_storage& operator=(vector_storage&& other) noexcept(
        nothrow_move &&
        (alloc_traits::propagate_on_container_move_assignment::value || alloc_traits::is_always_equal::value)) {
        if (this == &other) return *this;

        destroy_elements();
        dealloc_if_heap();

        if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
            alloc_ = std::move(other.alloc_);
            adopt(other);
        } else if (alloc_ == other.alloc_) {
            adopt(other);
        } else {
            ensure_capacity(other.size_);
            std::uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
            other.destroy_elements();
        }
        return *this;
    }

    constexpr void reset() noexcept {
        destroy_elements();
        dealloc_if_heap();
    }

    constexpr void replace_allocator(const Allocator& a) { alloc_ = a; }

    [[nodiscard]] constexpr iterator begin() noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_; }
    [[nodiscard]] constexpr iterator end() noexcept { return data_ + size_; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return data_ + size_; }

    [[nodiscard]] constexpr pointer data() noexcept { return data_; }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return data_; }
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return capacity_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] constexpr bool is_inline() const noexcept { return !is_heap(); }

    [[nodiscard]] static constexpr size_type inline_capacity() noexcept { return N; }

    [[nodiscard]] constexpr Allocator& allocator() noexcept { return alloc_; }
    [[nodiscard]] constexpr const Allocator& allocator() const noexcept { return alloc_; }

    [[nodiscard]] constexpr reference operator[](size_type i) noexcept { return data_[i]; }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { return data_[i]; }

    constexpr void set_size(size_type n) noexcept { size_ = n; }
    constexpr void bump_size(size_type d = 1) noexcept { size_ += d; }
    constexpr void drop_size(size_type d = 1) noexcept { size_ -= d; }

    template <typename... Args>
    constexpr reference construct_at(pointer p, Args&&... args) {
        return *std::construct_at(p, std::forward<Args>(args)...);
    }

    constexpr void destroy_at(pointer p) noexcept { std::destroy_at(p); }
    constexpr void destroy_range(pointer f, pointer l) noexcept { std::destroy(f, l); }
    constexpr void destroy_elements() noexcept {
        std::destroy(begin(), end());
        size_ = 0;
    }

    template <std::input_iterator It>
    constexpr pointer construct_range_n(pointer dst, size_type n, It first) {
        return std::uninitialized_copy_n(std::move(first), n, dst);
    }

    constexpr pointer fill_n(pointer dst, size_type n, const_reference value) {
        return std::uninitialized_fill_n(dst, n, value);
    }

    constexpr pointer default_construct_n(pointer dst, size_type n) {
        return std::uninitialized_default_construct_n(dst, n);
    }

    constexpr pointer relocate_construct_from(pointer dst, pointer src, size_type n) {
        return std::uninitialized_move_n(src, n, dst);
    }

    constexpr void ensure_capacity(size_type required) {
        if (required <= capacity_) return;
        grow(std::max(required, capacity_ * 2));
    }

    constexpr void grow(size_type new_cap) {
        if (new_cap <= capacity_) return;
        auto new_mem = alloc_traits::allocate(alloc_, new_cap);
        try {
            std::uninitialized_move(begin(), end(), new_mem);
        } catch (...) {
            alloc_traits::deallocate(alloc_, new_mem, new_cap);
            throw;
        }
        destroy_elements();
        dealloc_if_heap();
        buf_.heap_ptr = new_mem;
        data_ = new_mem;
        capacity_ = new_cap;
    }

    constexpr void shrink_to_fit() {
        if (is_inline()) return;
        if (size_ == capacity_) return;

        if (size_ <= N) {
            auto old_heap = buf_.heap_ptr;
            auto old_cap = capacity_;
            std::uninitialized_move(old_heap, old_heap + size_, inline_address());
            std::destroy_n(old_heap, size_);
            alloc_traits::deallocate(alloc_, old_heap, old_cap);
            data_ = inline_address();
            capacity_ = N;
        } else {
            auto new_mem = alloc_traits::allocate(alloc_, size_);
            try {
                std::uninitialized_move(begin(), end(), new_mem);
            } catch (...) {
                alloc_traits::deallocate(alloc_, new_mem, size_);
                throw;
            }
            destroy_elements();
            dealloc_if_heap();
            buf_.heap_ptr = new_mem;
            data_ = new_mem;
            capacity_ = size_;
        }
    }

    constexpr void clear() noexcept { destroy_elements(); }

    constexpr void make_gap(size_type idx, size_type count) {
        if (count == 0) return;
        auto tail = size_ - idx;
        if (tail == 0) {
            size_ += count;
            return;
        }

        if (tail <= count) {
            std::uninitialized_move(begin() + idx, end(), begin() + idx + count);
            std::destroy(begin() + idx, end());
        } else {
            std::uninitialized_move(end() - count, end(), end());
            std::move_backward(begin() + idx, end() - count, end());
            std::destroy(begin() + idx, begin() + idx + count);
        }
        size_ += count;
    }

    constexpr void swap(vector_storage& other) noexcept(nothrow_move &&
                                                        (alloc_traits::propagate_on_container_swap::value ||
                                                         alloc_traits::is_always_equal::value)) {
        if (this == &other) return;
        if constexpr (alloc_traits::propagate_on_container_swap::value) {
            using std::swap;
            swap(alloc_, other.alloc_);
        }

        auto tmp = vector_storage(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }
};

template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
    requires(N > 0) && std::same_as<typename Allocator::value_type, T>
class small_vector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using storage_type = vector_storage<T, N, Allocator>;
    using alloc_traits = std::allocator_traits<Allocator>;

    static constexpr bool nothrow_move = std::is_nothrow_move_constructible_v<T>;
    static constexpr bool always_equal = alloc_traits::is_always_equal::value;

    storage_type store_;

    [[nodiscard]] constexpr size_type idx_of(const_iterator it) const noexcept {
        return static_cast<size_type>(it - cbegin());
    }

public:
    constexpr small_vector() noexcept(std::is_nothrow_default_constructible_v<Allocator>) = default;

    constexpr explicit small_vector(const Allocator& alloc) noexcept : store_(alloc) {}

    constexpr small_vector(size_type count, const_reference value, const Allocator& alloc = Allocator())
        : store_(alloc) {
        store_.ensure_capacity(count);
        store_.fill_n(store_.begin(), count, value);
        store_.set_size(count);
    }

    constexpr explicit small_vector(size_type count, const Allocator& alloc = Allocator()) : store_(alloc) {
        store_.ensure_capacity(count);
        store_.default_construct_n(store_.begin(), count);
        store_.set_size(count);
    }

    template <std::input_iterator It>
    constexpr small_vector(It first, It last, const Allocator& alloc = Allocator()) : store_(alloc) {
        if constexpr (std::forward_iterator<It>) {
            auto n = static_cast<size_type>(std::distance(first, last));
            store_.ensure_capacity(n);
            store_.construct_range_n(store_.begin(), n, first);
            store_.set_size(n);
        } else {
            for (; first != last; ++first) emplace_back(*first);
        }
    }

    template <std::ranges::input_range R>
        requires(!std::same_as<std::remove_cvref_t<R>, small_vector>) &&
                std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    constexpr small_vector(std::from_range_t, R&& rg, const Allocator& alloc = Allocator())
        : small_vector(std::ranges::begin(rg), std::ranges::end(rg), alloc) {}

    constexpr small_vector(std::initializer_list<value_type> il, const Allocator& alloc = Allocator())
        : small_vector(il.begin(), il.end(), alloc) {}

    constexpr small_vector(const small_vector& other)
        : store_(alloc_traits::select_on_container_copy_construction(other.get_allocator())) {
        auto n = other.size();
        store_.ensure_capacity(n);
        store_.construct_range_n(store_.begin(), n, other.begin());
        store_.set_size(n);
    }

    constexpr small_vector(const small_vector& other, const Allocator& alloc) : store_(alloc) {
        auto n = other.size();
        store_.ensure_capacity(n);
        store_.construct_range_n(store_.begin(), n, other.begin());
        store_.set_size(n);
    }

    constexpr small_vector(small_vector&& other) = default;

    constexpr small_vector(small_vector&& other, const Allocator& alloc) noexcept(nothrow_move && always_equal)
        : store_(std::move(other.store_), alloc) {}

    constexpr ~small_vector() = default;

    constexpr small_vector& operator=(const small_vector& other) {
        if (this == &other) return *this;
        if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
            if (get_allocator() != other.get_allocator()) {
                store_.reset();
                store_.replace_allocator(other.get_allocator());
            }
        }
        assign(other.begin(), other.end());
        return *this;
    }

    constexpr small_vector& operator=(small_vector&& other) = default;

    constexpr small_vector& operator=(std::initializer_list<value_type> il) {
        assign(il.begin(), il.end());
        return *this;
    }

    constexpr void assign(size_type count, const_reference value) {
        clear();
        store_.ensure_capacity(count);
        store_.fill_n(store_.begin(), count, value);
        store_.set_size(count);
    }

    template <std::input_iterator It>
    constexpr void assign(It first, It last) {
        clear();
        if constexpr (std::forward_iterator<It>) {
            auto n = static_cast<size_type>(std::distance(first, last));
            store_.ensure_capacity(n);
            store_.construct_range_n(store_.begin(), n, first);
            store_.set_size(n);
        } else {
            for (; first != last; ++first) emplace_back(*first);
        }
    }

    constexpr void assign(std::initializer_list<value_type> il) { assign(il.begin(), il.end()); }

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    constexpr void assign_range(R&& rg) {
        assign(std::ranges::begin(rg), std::ranges::end(rg));
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return store_.allocator(); }

    [[nodiscard]] constexpr iterator begin() noexcept { return store_.begin(); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return store_.begin(); }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return store_.begin(); }
    [[nodiscard]] constexpr iterator end() noexcept { return store_.end(); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return store_.end(); }
    [[nodiscard]] constexpr const_iterator cend() const noexcept { return store_.end(); }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    [[nodiscard]] constexpr bool empty() const noexcept { return store_.empty(); }
    [[nodiscard]] constexpr size_type size() const noexcept { return store_.size(); }
    [[nodiscard]] constexpr size_type capacity() const noexcept { return store_.capacity(); }
    [[nodiscard]] static constexpr size_type inline_capacity() noexcept { return N; }

    constexpr void reserve(size_type new_cap) { store_.ensure_capacity(new_cap); }
    constexpr void shrink_to_fit() { store_.shrink_to_fit(); }

    [[nodiscard]] constexpr reference operator[](size_type i) noexcept { return store_[i]; }
    [[nodiscard]] constexpr const_reference operator[](size_type i) const noexcept { return store_[i]; }

    [[nodiscard]] constexpr reference at(size_type i) {
        if (i >= size()) throw std::out_of_range("small_vector::at");
        return store_[i];
    }
    [[nodiscard]] constexpr const_reference at(size_type i) const {
        if (i >= size()) throw std::out_of_range("small_vector::at");
        return store_[i];
    }

    [[nodiscard]] constexpr reference front() noexcept { return store_[0]; }
    [[nodiscard]] constexpr const_reference front() const noexcept { return store_[0]; }
    [[nodiscard]] constexpr reference back() noexcept { return store_[size() - 1]; }
    [[nodiscard]] constexpr const_reference back() const noexcept { return store_[size() - 1]; }

    [[nodiscard]] constexpr pointer data() noexcept { return store_.data(); }
    [[nodiscard]] constexpr const_pointer data() const noexcept { return store_.data(); }

    constexpr void clear() noexcept { store_.clear(); }

    template <typename... Args>
    constexpr reference emplace_back(Args&&... args) {
        if (size() == capacity()) store_.grow(capacity() * 2);
        auto&& r = store_.construct_at(store_.begin() + size(), std::forward<Args>(args)...);
        store_.bump_size();
        return r;
    }

    constexpr reference push_back(const_reference value) { return emplace_back(value); }
    constexpr reference push_back(T&& value) { return emplace_back(std::move(value)); }

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    constexpr void append_range(R&& rg) {
        for (auto&& v : rg) emplace_back(std::forward<decltype(v)>(v));
    }

    constexpr void pop_back() noexcept {
        store_.drop_size();
        store_.destroy_at(store_.begin() + size());
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args) {
        auto idx = idx_of(pos);
        if (size() == capacity()) store_.grow(capacity() * 2);

        if (idx == size()) {
            store_.construct_at(store_.begin() + idx, std::forward<Args>(args)...);
            store_.bump_size();
        } else {
            auto tmp = value_type{std::forward<Args>(args)...};
            store_.make_gap(idx, 1);
            store_.construct_at(store_.begin() + idx, std::move(tmp));
        }
        return store_.begin() + idx;
    }

    constexpr iterator insert(const_iterator pos, const_reference value) { return emplace(pos, value); }
    constexpr iterator insert(const_iterator pos, T&& value) { return emplace(pos, std::move(value)); }

    constexpr iterator insert(const_iterator pos, size_type count, const_reference value) {
        auto idx = idx_of(pos);
        if (count == 0) return store_.begin() + idx;

        auto tmp = small_vector(count, value, get_allocator());
        store_.ensure_capacity(size() + count);
        store_.make_gap(idx, count);
        store_.construct_range_n(store_.begin() + idx, count, std::make_move_iterator(tmp.begin()));
        return store_.begin() + idx;
    }

    template <std::input_iterator It>
    constexpr iterator insert(const_iterator pos, It first, It last) {
        auto idx = idx_of(pos);
        if (first == last) return store_.begin() + idx;
        auto tmp = small_vector(first, last, get_allocator());
        auto count = tmp.size();
        store_.ensure_capacity(size() + count);
        store_.make_gap(idx, count);
        store_.construct_range_n(store_.begin() + idx, count, std::make_move_iterator(tmp.begin()));
        return store_.begin() + idx;
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> il) {
        return insert(pos, il.begin(), il.end());
    }

    template <std::ranges::input_range R>
        requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    constexpr iterator insert_range(const_iterator pos, R&& rg) {
        return insert(pos, std::ranges::begin(rg), std::ranges::end(rg));
    }

    constexpr iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

    constexpr iterator erase(const_iterator first, const_iterator last) {
        auto idx = idx_of(first);
        auto count = static_cast<size_type>(last - first);
        if (count == 0) return store_.begin() + idx;

        auto dst = store_.begin() + idx;
        auto src = dst + count;
        auto stop = store_.end();

        std::move(src, stop, dst);
        store_.destroy_range(stop - count, stop);
        store_.drop_size(count);
        return dst;
    }

    constexpr void resize(size_type count) {
        if (count > size()) {
            store_.ensure_capacity(count);
            store_.default_construct_n(store_.begin() + size(), count - size());
            store_.set_size(count);
        } else {
            store_.destroy_range(store_.begin() + count, store_.end());
            store_.set_size(count);
        }
    }

    constexpr void resize(size_type count, const_reference value) {
        if (count > size()) {
            store_.ensure_capacity(count);
            store_.fill_n(store_.begin() + size(), count - size(), value);
            store_.set_size(count);
        } else {
            store_.destroy_range(store_.begin() + count, store_.end());
            store_.set_size(count);
        }
    }

    constexpr void swap(small_vector& other) noexcept(noexcept(store_.swap(other.store_))) {
        store_.swap(other.store_);
    }
};

template <typename T, std::size_t N, typename A>
[[nodiscard]] constexpr bool operator==(const small_vector<T, N, A>& a, const small_vector<T, N, A>& b) {
    return std::ranges::equal(a, b);
}

template <typename T, std::size_t N, typename A>
[[nodiscard]] constexpr auto operator<=>(const small_vector<T, N, A>& a, const small_vector<T, N, A>& b) {
    return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), std::compare_three_way{});
}

template <typename T, std::size_t N, typename A>
constexpr void swap(small_vector<T, N, A>& a, small_vector<T, N, A>& b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
}

}  // namespace numsol
